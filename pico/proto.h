#include <stdint.h>

int ___PROTO__clangd_common;

#pragma pack(push) // no warning!
#pragma pack(1)

#define PICOY_BODY 112
#define PICOY_LAST 1

struct PICOY_PKT {
  uint16_t packet_id;
  uint16_t packet_flags;
  uint32_t time_duration;
  uint64_t time_start;
  uint8_t data[PICOY_BODY];
};

#define KERNELY_ENABLE 1
#define KERNELY_FINISH 2

struct KERNELY_PKT {
  uint8_t state_flags;
  uint16_t timer_period;
};
#pragma pack(pop)
