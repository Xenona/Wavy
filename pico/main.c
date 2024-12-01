/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// #include "pico/stdlib.h"
#include <hardware/gpio.h>
#include <hardware/timer.h>
#include <pico/stdio.h>
#include <pico/time.h>
#include <pico/types.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/_types.h>
// #include <pico/stdlib.h>
// #include <pico/binary_info.h>
#include "pico/multicore.h"
#include "bsp/board.h"

#include "class/vendor/vendor_device.h"
#include "device/usbd.h"

#include "proto.h"

#define NUM_BUFFERS 64
struct PICOY_PKT send_buf[NUM_BUFFERS];

uint8_t enabled = 0;
uint8_t finish = 0;

uint16_t current_packet_id = 0;
uint8_t current_write_buf = 0;
uint8_t current_write_pos = 0;
uint16_t timer_period = 0;

void __time_critical_func(writer_thread)() {
  struct PICOY_PKT *current_packet = &send_buf[current_write_buf];
  while(true) {
    uint64_t ctime = time_us_64();
    uint32_t gpio = gpio_get_all();
    // if (!enabled) continue;
    if(enabled) {
      if (current_write_pos == 0) {
        current_packet->packet_flags = 0;
        current_packet->packet_id = current_packet_id++;
        current_packet->time_start = time_us_64();
      }

      current_packet->data[current_write_pos] = gpio & 0xff;
      current_write_pos++;

      if (current_write_pos >= PICOY_BODY) {
        current_packet->time_duration = time_us_64() - current_packet->time_start;
        
        if (finish) {
          current_packet->packet_flags |= PICOY_LAST;
          enabled = false;
          finish = false;
        }

        current_write_buf++;
        if (current_write_buf >= NUM_BUFFERS)
          current_write_buf = 0;

        current_packet = &send_buf[current_write_buf];
        current_write_pos = 0;
      }
    }
    busy_wait_until(ctime + timer_period);
  }
}

int main() {
  const uint LED_PIN = 1;
  const uint READ_PIN = 0;
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  for (int i = 0; i < 8; i++) {
    gpio_init(i);
    gpio_set_dir(i, GPIO_IN);
  }

  uint readValue;

  // struct repeating_timer timer;
  bool timer_enabled;

  stdio_init_all();

  tusb_init();
  int current_read_buf = 0;
  uint16_t send_detector = 0;

  multicore_launch_core1(writer_thread);

  while (true) {
    tud_task();

    // if (!enabled && timer_enabled) {
    //   cancel_repeating_timer(&timer);
    //   timer_enabled = false;
    // }

    int cwb = current_write_buf;
    if (cwb < NUM_BUFFERS) {
      int write_cnt = 0;
      while (current_read_buf != cwb) {
        write_cnt++;
        if (tud_vendor_n_write_available(0) >=
                   sizeof(struct PICOY_PKT)) {
          // alarm
          if(send_detector != 0 && send_buf[current_read_buf].packet_id != send_detector) {
            printf("Overpressure\n");
          }
          send_detector = send_buf[current_read_buf].packet_id + 1;
          // printf("Sending %d", send_buf[current_read_buf].packet_id);
          tud_vendor_n_write(0, &send_buf[current_read_buf],
                             sizeof(struct PICOY_PKT));

          current_read_buf++;
          if (current_read_buf >= NUM_BUFFERS)
            current_read_buf = 0;
        } else { 
          break;
        }
      }

      if(write_cnt >0) {
        if(write_cnt > 1) printf("Neat");
        tud_vendor_n_flush(0);
      }
    }

    while (tud_vendor_n_available(0) >= sizeof(struct KERNELY_PKT)) {
      struct KERNELY_PKT kp = {};
      tud_vendor_n_read(0, &kp, sizeof(kp));
      printf("Recieved packet flags=%d\n", kp.state_flags);

      if (kp.state_flags & KERNELY_ENABLE) {
        if (!enabled) {
          printf("Started readingp\n");

          current_write_buf = 0;
          current_read_buf = 0;
          current_write_pos = 0;
          send_detector = 0;
          atomic_thread_fence(memory_order_relaxed);
          sleep_ms(1);

          enabled = true;
          timer_enabled = true;
          timer_period = kp.timer_period;
          // add_repeating_timer_us(-kp.timer_period, writer_callback, NULL,
                                //  &timer);
        }
      } else if (kp.state_flags & KERNELY_FINISH) {
        if (enabled) {
          printf("Sending finish\n");
          finish = true;
        }
      }
    }

    // add_repeating_timer_us(-10, writer_callback, NULL, &timer);
  }
}
