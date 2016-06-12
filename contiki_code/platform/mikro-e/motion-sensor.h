#ifndef __MOTION_SENSOR_H__
#define __MOTION_SENSOR_H__

#include <pic32_gpio.h>

#define MOTION_PORT D
#define MOTION_PIN  0

#define MOTION_SENSOR_NAME "MotionSensor"

extern const struct sensors_sensor motion_sensor;

void motion_isr();
#endif
