#ifndef RELAY_PUMP_H
#define RELAY_PUMP_H
#include <stdbool.h>

/* Relay bom tren PA4, kich muc THAP (LOW = bom ON). */
void relay_init(void);          /* bom tat sau khi init */
void relay_set(bool on);
bool relay_is_on(void);

#endif
