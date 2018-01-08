#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>

int64_t CLOCK_now();
int64_t CLOCK_time_passed(const int64_t time_ms);
unsigned int CLOCK_random_seed();

#endif
