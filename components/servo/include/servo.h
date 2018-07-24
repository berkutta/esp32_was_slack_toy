#ifndef _SERVO_H_
#define _SERVO_H_

#define servo_low 50
#define servo_high 150

void servo_init(void);

void servo_set(uint8_t position);

void servo_run(int amount, int minimum, int maximum, int delay);

#endif
