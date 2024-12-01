#include <linux/module.h>
// SPDX-License-Identifier: GPL-2.0
/*
 * USB Skeleton driver - 2.2
 *
 * Copyright (C) 2001-2004 Greg Kroah-Hartman (greg@kroah.com)
 *
 * This driver is based on the 2.6.3 version of drivers/usb/usb-skeleton.c
 * but has been rewritten to be easier to read and use.
 */

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/kref.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/usb.h>


#define PIKERNELY_SUBCLASS 120
#define PIKERNELY_PROTOCOL 66

/* table of devices that work with this driver */
static const struct usb_device_id pikernely_table[] = {
    {USB_INTERFACE_INFO(255, PIKERNELY_SUBCLASS, PIKERNELY_PROTOCOL)},
    {} /* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, pikernely_table);

/* Get a minor range for your devices from the usb maintainer */
#define USB_PIKERNELY_MINOR_BASE 193

/* our private defines. if this grows any larger, use your own .h file */
#define MAX_TRANSFER (PAGE_SIZE - 512)
/*
 * MAX_TRANSFER is chosen so that the VM is not stressed by
 * allocations > PAGE_SIZE and the number of packets in a page
 * is an integer 512 is the largest possible packet on EHCI
 */
#define WRITES_IN_FLIGHT 8
/* arbitrarily chosen */

/* Structure to hold all of our device specific stuff */
struct usb_pikernely {
  struct usb_device *udev;         /* the usb device for this device */
  struct usb_interface *interface; /* the interface for this device */
  struct semaphore limit_sem;  /* limiting the number of writes in progress */
  struct usb_anchor submitted; /* in case we need to retract our submissions */
  struct urb *bulk_in_urb;     /* the urb to read data with */
  unsigned char *bulk_in_buffer; /* the buffer to receive data */
  size_t bulk_in_size;           /* the size of the receive buffer */
  size_t bulk_in_filled;         /* number of bytes in the buffer */
  size_t bulk_in_copied;         /* already copied to user space */
  __u8 bulk_in_endpointAddr;     /* the address of the bulk in endpoint */
  __u8 bulk_out_endpointAddr;    /* the address of the bulk out endpoint */
  int errors;                    /* the last request tanked */
  bool ongoing_read;             /* a read is going on */
  spinlock_t err_lock;           /* lock for errors */
  struct kref kref;
  struct mutex io_mutex; /* synchronize I/O with disconnect */
  unsigned long disconnected : 1;
  wait_queue_head_t bulk_in_wait; /* to wait for an ongoing read */
};
#define to_pikernely_dev(d) container_of(d, struct usb_pikernely, kref)

static struct usb_driver pikernely_driver;
static void pikernely_draw_down(struct usb_pikernely *dev);

static void pikernely_delete(struct kref *kref) {
  struct usb_pikernely *dev = to_pikernely_dev(kref);

  usb_free_urb(dev->bulk_in_urb);
  usb_put_intf(dev->interface);
  usb_put_dev(dev->udev);
  kfree(dev->bulk_in_buffer);
  kfree(dev);
}

static int pikernely_open(struct inode *inode, struct file *file) {
  struct usb_pikernely *dev;
  struct usb_interface *interface;
  int subminor;
  int retval = 0;

  subminor = iminor(inode);

  interface = usb_find_interface(&pikernely_driver, subminor);
  if (!interface) {
    pr_err("%s - error, can't find device for minor %d\n", __func__, subminor);
    retval = -ENODEV;
    goto exit;
  }

  dev = usb_get_intfdata(interface);
  if (!dev) {
    pr_err("%s - error, can't find interface for minor %d\n", __func__, subminor);
    retval = -ENODEV;
    goto exit;
  }

  /* increment our usage count for the device */
  kref_get(&dev->kref);

  /* save our object in the file's private structure */
  file->private_data = dev;

exit:
  return retval;
}

static int pikernely_release(struct inode *inode, struct file *file) {
  struct usb_pikernely *dev;

  dev = file->private_data;
  if (dev == NULL)
    return -ENODEV;

  /* decrement the count on our device */
  kref_put(&dev->kref, pikernely_delete);
  return 0;
}

static int pikernely_flush(struct file *file, fl_owner_t id) {
  struct usb_pikernely *dev;
  int res;

  dev = file->private_data;
  if (dev == NULL)
    return -ENODEV;

  /* wait for io to stop */
  mutex_lock(&dev->io_mutex);
  pikernely_draw_down(dev);

  /* read out errors, leave subsequent opens a clean slate */
  spin_lock_irq(&dev->err_lock);
  res = dev->errors ? (dev->errors == -EPIPE ? -EPIPE : -EIO) : 0;
  dev->errors = 0;
  spin_unlock_irq(&dev->err_lock);

  mutex_unlock(&dev->io_mutex);

  return res;
}

static void pikernely_read_bulk_callback(struct urb *urb) {
  struct usb_pikernely *dev;
  unsigned long flags;

  dev = urb->context;

  spin_lock_irqsave(&dev->err_lock, flags);
  /* sync/async unlink fau  lts aren't errors */
  if (urb->status) {
    if (!(urb->status == -ENOENT || urb->status == -ECONNRESET ||
          urb->status == -ESHUTDOWN))
      dev_err(&dev->interface->dev,
              "%s - nonzero write bulk status received: %d\n", __func__,
              urb->status);

    dev->errors = urb->status;
  } else {
    dev->bulk_in_filled = urb->actual_length;
  }

  // printk("Read %d %d", urb->actual_length, *(uint16_t*)dev->bulk_in_buffer);
  
  dev->ongoing_read = 0;
  spin_unlock_irqrestore(&dev->err_lock, flags);

  wake_up_interruptible(&dev->bulk_in_wait);
}

static int pikernely_do_read_io(struct usb_pikernely *dev, size_t count) {
  int rv;

  /* prepare a read */
  usb_fill_bulk_urb(dev->bulk_in_urb, dev->udev,
                    usb_rcvbulkpipe(dev->udev, dev->bulk_in_endpointAddr),
                    dev->bulk_in_buffer, min(dev->bulk_in_size, count),
                    pikernely_read_bulk_callback, dev);
  /* tell everybody to leave the URB alone */
  spin_lock_irq(&dev->err_lock);
  dev->ongoing_read = 1;
  spin_unlock_irq(&dev->err_lock);

  /* submit bulk in urb, which means no data to deliver */
  dev->bulk_in_filled = 0;
  dev->bulk_in_copied = 0;

  /* do it */
  rv = usb_submit_urb(dev->bulk_in_urb, GFP_KERNEL);
  if (rv < 0) {
    dev_err(&dev->interface->dev, "%s - failed submitting read urb, error %d\n",
            __func__, rv);
    rv = (rv == -ENOMEM) ? rv : -EIO;
    spin_lock_irq(&dev->err_lock);
    dev->ongoing_read = 0;
    spin_unlock_irq(&dev->err_lock);
  }

  return rv;
}

static ssize_t pikernely_read(struct file *file, char *buffer, size_t count,
                              loff_t *ppos) {
  struct usb_pikernely *dev;
  int rv;
  bool ongoing_io;
  ssize_t sent = 0;

  dev = file->private_data;

  if (!count)
    return 0;

  /* no concurrent readers */
  rv = mutex_lock_interruptible(&dev->io_mutex);
  if (rv < 0)
    return rv;

  if (dev->disconnected) { /* disconnect() was called */
    rv = -ENODEV;
    goto exit;
  }

  /* if IO is under way, we must not touch things */
retry:
  spin_lock_irq(&dev->err_lock);
  ongoing_io = dev->ongoing_read;
  spin_unlock_irq(&dev->err_lock);

  if (ongoing_io) {
    /* nonblocking IO shall not wait */
    if (file->f_flags & O_NONBLOCK) {
      rv = -EAGAIN;
      goto exit;
    }
    /*
     * IO may take forever
     * hence wait in an interruptible state
     */
    rv = wait_event_interruptible(dev->bulk_in_wait, (!dev->ongoing_read));
    if (rv < 0)
      goto exit;
  }

  /* errors must be reported */
  rv = dev->errors;
  if (rv < 0) {
    /* any error is reported once */
    dev->errors = 0;
    /* to preserve notifications about reset */
    rv = (rv == -EPIPE) ? rv : -EIO;
    /* report it */
    goto exit;
  }

  /*
   * if the buffer is filled we may satisfy the read
   * else we need to start IO
   */

  if (dev->bulk_in_filled) {
    /* we had read data */
    size_t available = dev->bulk_in_filled - dev->bulk_in_copied;
    size_t chunk = min(available, count);

    if (!available) {
      /*
       * all data has been used
       * actual IO needs to be done
       */
      rv = pikernely_do_read_io(dev, count);
      if (rv < 0)
        goto exit;
      else
        goto retry;
    }
    /*
     * data is available
     * chunk tells us how much shall be copied
     */

    if (copy_to_user(buffer + sent, dev->bulk_in_buffer + dev->bulk_in_copied, chunk))
      rv = -EFAULT;
    else
      rv = chunk;

    dev->bulk_in_copied += chunk;
    sent += chunk;
    count -= chunk;

    /*
     * if we are asked for more than we have,
     * we start IO but don't wait
     */
    if (count > 0) {
      pikernely_do_read_io(dev, count);
      goto retry;
    } else {
      rv = sent;
    }
  } else {
    /* no data in the buffer */
    rv = pikernely_do_read_io(dev, count);
    if (rv < 0)
      goto exit;
    else
      goto retry;
  }
exit:
  mutex_unlock(&dev->io_mutex);
  return rv;
}

static void pikernely_write_bulk_callback(struct urb *urb) {
  struct usb_pikernely *dev;
  unsigned long flags;

  dev = urb->context;

  /* sync/async unlink faults aren't errors */
  if (urb->status) {
    if (!(urb->status == -ENOENT || urb->status == -ECONNRESET ||
          urb->status == -ESHUTDOWN))
      dev_err(&dev->interface->dev,
              "%s - nonzero write bulk status received: %d\n", __func__,
              urb->status);

    spin_lock_irqsave(&dev->err_lock, flags);
    dev->errors = urb->status;
    spin_unlock_irqrestore(&dev->err_lock, flags);
  }

  /* free up our allocated buffer */
  usb_free_coherent(urb->dev, urb->transfer_buffer_length, urb->transfer_buffer,
                    urb->transfer_dma);
  up(&dev->limit_sem);
}

static ssize_t pikernely_write(struct file *file, const char *user_buffer,
                               size_t count, loff_t *ppos) {
  struct usb_pikernely *dev;
  int retval = 0;
  struct urb *urb = NULL;
  char *buf = NULL;
  size_t writesize = min_t(size_t, count, MAX_TRANSFER);

  dev = file->private_data;

  /* verify that we actually have some data to write */
  if (count == 0)
    goto exit;

  /*
   * limit the number of URBs in flight to stop a user from using up all
   * RAM
   */
  if (!(file->f_flags & O_NONBLOCK)) {
    if (down_interruptible(&dev->limit_sem)) {
      retval = -ERESTARTSYS;
      goto exit;
    }
  } else {
    if (down_trylock(&dev->limit_sem)) {
      retval = -EAGAIN;
      goto exit;
    }
  }

  spin_lock_irq(&dev->err_lock);
  retval = dev->errors;
  if (retval < 0) {
    /* any error is reported once */
    dev->errors = 0;
    /* to preserve notifications about reset */
    retval = (retval == -EPIPE) ? retval : -EIO;
  }
  spin_unlock_irq(&dev->err_lock);
  if (retval < 0)
    goto error;

  /* create a urb, and a buffer for it, and copy the data to the urb */
  urb = usb_alloc_urb(0, GFP_KERNEL);
  if (!urb) {
    retval = -ENOMEM;
    goto error;
  }

  buf =
      usb_alloc_coherent(dev->udev, writesize, GFP_KERNEL, &urb->transfer_dma);
  if (!buf) {
    retval = -ENOMEM;
    goto error;
  }

  if (copy_from_user(buf, user_buffer, writesize)) {
    retval = -EFAULT;
    goto error;
  }

  /* this lock makes sure we don't submit URBs to gone devices */
  mutex_lock(&dev->io_mutex);
  if (dev->disconnected) { /* disconnect() was called */
    mutex_unlock(&dev->io_mutex);
    retval = -ENODEV;
    goto error;
  }

  /* initialize the urb properly */
  usb_fill_bulk_urb(urb, dev->udev,
                    usb_sndbulkpipe(dev->udev, dev->bulk_out_endpointAddr), buf,
                    writesize, pikernely_write_bulk_callback, dev);
  urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
  usb_anchor_urb(urb, &dev->submitted);

  /* send the data out the bulk port */
  retval = usb_submit_urb(urb, GFP_KERNEL);
  mutex_unlock(&dev->io_mutex);
  if (retval) {
    dev_err(&dev->interface->dev,
            "%s - failed submitting write urb, error %d\n", __func__, retval);
    goto error_unanchor;
  }

  /*
   * release our reference to this urb, the USB core will eventually free
   * it entirely
   */
  usb_free_urb(urb);

  return writesize;

error_unanchor:
  usb_unanchor_urb(urb);
error:
  if (urb) {
    usb_free_coherent(dev->udev, writesize, buf, urb->transfer_dma);
    usb_free_urb(urb);
  }
  up(&dev->limit_sem);

exit:
  return retval;
}

static const struct file_operations pikernely_fops = {
    .owner = THIS_MODULE,
    .read = pikernely_read,
    .write = pikernely_write,
    .open = pikernely_open,
    .release = pikernely_release,
    .flush = pikernely_flush,
    .llseek = noop_llseek,
};

/*
 * usb class driver info in order to get a minor number from the usb core,
 * and to have the device registered with the driver core
 */
static struct usb_class_driver pikernely_class = {
    .name = "pikernely%d",
    .fops = &pikernely_fops,
    .minor_base = USB_PIKERNELY_MINOR_BASE,
};

static int pikernely_probe(struct usb_interface *interface,
                           const struct usb_device_id *id) {
  struct usb_pikernely *dev;
  struct usb_endpoint_descriptor *bulk_in, *bulk_out;
  int retval;

  struct usb_interface_descriptor *idesc = &interface->cur_altsetting->desc;

  if (idesc->bInterfaceSubClass != PIKERNELY_SUBCLASS ||
      idesc->bInterfaceProtocol != PIKERNELY_PROTOCOL)
    return -ENODEV;

  /* allocate memory for our device state and initialize it */
  dev = kzalloc(sizeof(*dev), GFP_KERNEL);
  if (!dev)
    return -ENOMEM;

  kref_init(&dev->kref);
  sema_init(&dev->limit_sem, WRITES_IN_FLIGHT);
  mutex_init(&dev->io_mutex);
  spin_lock_init(&dev->err_lock);
  init_usb_anchor(&dev->submitted);
  init_waitqueue_head(&dev->bulk_in_wait);

  dev->udev = usb_get_dev(interface_to_usbdev(interface));
  dev->interface = usb_get_intf(interface);
  dev_warn(&dev->udev->dev, "Loaded\n");

  /* set up the endpoint information */
  /* use only the first bulk-in and bulk-out endpoints */
  retval = usb_find_common_endpoints(interface->cur_altsetting, &bulk_in,
                                     &bulk_out, NULL, NULL);
  if (retval) {
    dev_err(&interface->dev,
            "Could not find both bulk-in and bulk-out endpoints\n");
    goto error;
  }

  dev->bulk_in_size = usb_endpoint_maxp(bulk_in);
  dev->bulk_in_endpointAddr = bulk_in->bEndpointAddress;
  dev->bulk_in_buffer = kmalloc(dev->bulk_in_size, GFP_KERNEL);
  if (!dev->bulk_in_buffer) {
    retval = -ENOMEM;
    goto error;
  }
  dev->bulk_in_urb = usb_alloc_urb(0, GFP_KERNEL);
  if (!dev->bulk_in_urb) {
    retval = -ENOMEM;
    goto error;
  }

  dev->bulk_out_endpointAddr = bulk_out->bEndpointAddress;

  /* save our data pointer in this interface device */
  usb_set_intfdata(interface, dev);

  /* we can register the device now, as it is ready */
  retval = usb_register_dev(interface, &pikernely_class);
  if (retval) {
    /* something prevented us from registering this driver */
    dev_err(&interface->dev, "Not able to get a minor for this device.\n");
    usb_set_intfdata(interface, NULL);
    goto error;
  }

  /* let the user know what node this device is now attached to */
  dev_info(&interface->dev, "Pikernely device now attached to pikernely%d",
           interface->minor);
  return 0;

error:
  /* this frees allocated memory */
  kref_put(&dev->kref, pikernely_delete);

  return retval;
}

static void pikernely_disconnect(struct usb_interface *interface) {
  struct usb_pikernely *dev;
  int minor = interface->minor;

  dev = usb_get_intfdata(interface);

  /* give back our minor */
  usb_deregister_dev(interface, &pikernely_class);

  /* prevent more I/O from starting */
  mutex_lock(&dev->io_mutex);
  dev->disconnected = 1;
  mutex_unlock(&dev->io_mutex);

  usb_kill_urb(dev->bulk_in_urb);
  usb_kill_anchored_urbs(&dev->submitted);

  /* decrement our usage count */
  kref_put(&dev->kref, pikernely_delete);

  dev_info(&interface->dev, "USB Pikernely #%d now disconnected", minor);
}

static void pikernely_draw_down(struct usb_pikernely *dev) {
  int time;

  time = usb_wait_anchor_empty_timeout(&dev->submitted, 1000);
  if (!time)
    usb_kill_anchored_urbs(&dev->submitted);
  usb_kill_urb(dev->bulk_in_urb);
}

static int pikernely_suspend(struct usb_interface *intf, pm_message_t message) {
  struct usb_pikernely *dev = usb_get_intfdata(intf);

  if (!dev)
    return 0;
  pikernely_draw_down(dev);
  return 0;
}

static int pikernely_resume(struct usb_interface *intf) { return 0; }

static int pikernely_pre_reset(struct usb_interface *intf) {
  struct usb_pikernely *dev = usb_get_intfdata(intf);

  mutex_lock(&dev->io_mutex);
  pikernely_draw_down(dev);

  return 0;
}

static int pikernely_post_reset(struct usb_interface *intf) {
  struct usb_pikernely *dev = usb_get_intfdata(intf);

  /* we are sure no URBs are active - no locking needed */
  dev->errors = -EPIPE;
  mutex_unlock(&dev->io_mutex);

  return 0;
}

static struct usb_driver pikernely_driver = {
    .name = "pikernely",
    .probe = pikernely_probe,
    .disconnect = pikernely_disconnect,
    .suspend = pikernely_suspend,
    .resume = pikernely_resume,
    .pre_reset = pikernely_pre_reset,
    .post_reset = pikernely_post_reset,
    .id_table = pikernely_table,
    .supports_autosuspend = 0,
};

module_usb_driver(pikernely_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kseniya Trubach");
MODULE_DESCRIPTION("Module for pico as logic analyzer");
MODULE_VERSION("6");