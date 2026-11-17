#ifndef RELAY_PUMP_H
#define RELAY_PUMP_H

#include <stdbool.h>

void relay_pump_init(void);
void relay_pump_set(bool on);
bool relay_pump_is_on(void);

#endif
