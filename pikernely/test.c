#include "proto.h"
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

// static int N = 5;
int fd;
static void *run(void *arg) {
  // int *i = (int *) arg;
  // char buf[123];
  // snprintf(buf, sizeof(buf), "thread %d", *i);
  // return buf;
  uint16_t seq = 0;
  bool last = false;
  uint8_t prev = 0;
  uint64_t prev_time = 0;
  int samples = 0;

  while (!last) {
    struct PICOY_PKT pkt;
    
    int a = read(fd, &pkt, sizeof(pkt));
    // printf("%d", a);
    if (a <= 0)
      break;

    last = pkt.packet_flags & PICOY_LAST;
    if (seq != 0 && pkt.packet_id != seq) {
      printf("Seems we lost packet %d %d\n", pkt.packet_id, seq);
    }
    seq = pkt.packet_id + 1;
  
    for(int i = 0; i < PICOY_BODY; i++) {
      samples++;
      if(pkt.data[i] != prev) {
        uint64_t time = pkt.time_start + i * pkt.time_duration / (PICOY_BODY - 1);
        printf("[%ldus](+ %ldus) =%x\n", time, time-prev_time, pkt.data[i]);
        prev_time =
        prev = pkt.data[i];
        
      }
    }
    // printf("Recieved packet %d started=%ld duration=%d last=%s\n",
    // pkt.packet_id,
    //        pkt.time_start, pkt.time_duration, last ? "y" : "n");
  }
  printf("Toal captured: %d\n", samples);

  return 0;
}

int main(int argc, char *argv[]) {
  fd = open("/dev/pikernely0", O_RDWR);
  printf("Opened %d\n", fd);

  pthread_t pt;
  pthread_create(&pt, NULL, run, NULL);

  struct KERNELY_PKT c;
  // if (argv[1][0] == 'a') {
  c.state_flags = KERNELY_ENABLE;
  c.timer_period = 5;
  printf("Kernel start\n");
  write(fd, &c, sizeof(c));

  sleep(5);

  c.state_flags = KERNELY_FINISH;
  c.timer_period = 0;
  printf("Kernel finish\n");
  write(fd, &c, sizeof(c));

  pthread_join(pt, NULL);
  printf("Kernel read all\n");
  // } else {
  //   printf("Kernel finish\n");
  //   c.state_flags = 0;
  //   c.timer_period = 0;
  // }
}
