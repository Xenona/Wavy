/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include <hardware/gpio.h>
#include <pico/time.h>
#include <pico/types.h>
#include <stdio.h>

bool repeating_timer_callback(__unused struct repeating_timer *t) {
  printf("Repeat at %lld\n", time_us_64());
  return true;
}

int main() {
  const uint LED_PIN = 1;
  const uint READ_PIN = 0;
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  gpio_init(READ_PIN);
  gpio_set_dir(READ_PIN, GPIO_IN);

  uint readValue;

  struct repeating_timer timer;
  // add_repeating_timer_us(-500, repeating_timer_callback, NULL, &timer);

  stdio_init_all();
  int a = 0;
  int t1, t2;
  while (true) {
    //     // printf("Hello, world!\n");
    //     // sleep_ms(1000);
    //   scanf("%d %d", &a, &b);
    //   printf("%d", a+b);

    // gpio_put(LED_PIN, 1);
    // sleep_ms(500);
    // gpio_put(LED_PIN, 0);
    // sleep_ms(500);

    // readValue = gpio_get(READ_PIN);
    // printf("Got: %d\n", readValue);

    scanf("%d", &a);

    t1 = time_us_64();
    while (a != 0) {
      printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");
      a-=512;
    }
    t2 = time_us_64();
    printf("All the stuff took %d us\n", t2-t1);

  }
}
