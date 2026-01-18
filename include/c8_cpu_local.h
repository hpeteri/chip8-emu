#ifndef C8_CPU_LOCAL_H
#define C8_CPU_LOCAL_H

#include "c8_inttypes.h"

struct c8_cpu;

static void c8_draw(
    struct c8_cpu* p_cpu,
    uint8_t x,
    uint8_t y,
    uint8_t n);

static void c8_reg_dump(
    struct c8_cpu* p_cpu,
    uint8_t x);

static void c8_reg_load(
    struct c8_cpu* p_cpu,
    uint8_t x);


#endif
