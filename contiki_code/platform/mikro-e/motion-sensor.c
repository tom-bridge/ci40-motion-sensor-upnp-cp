#include "motion-sensor.h"
#include <lib/sensors.h>
#include <pic32_gpio.h>
#include "button-sensor.h"

static int motion_status_value = 0;
static int _motion_value = 0;

static int motion_configure(int type, int value)
{
	switch(type)
	{
		case SENSORS_HW_INIT:
		BUTTON_IRQ_INIT(MOTION_PORT);
		return 1;

		case SENSORS_ACTIVE:
		if(value)
		{
			motion_status_value = 1;
			BUTTON_IRQ_ENABLE(MOTION_PORT, MOTION_PIN);
		}
		else
		{
			BUTTON_IRQ_DISABLE(MOTION_PORT, MOTION_PIN);
			motion_status_value = 0;
		}
		return 1;
	}
	return 0;
}

static int motion_status(int type)
{
	return motion_status_value;
}

static int motion_value(int type)
{
	return _motion_value;
}

void motion_isr()
{
	if(_motion_value == 1)
		_motion_value = 0;
	else
		_motion_value = 1;

	printf("Motion ISR, setting val: %i\n", _motion_value);
	BUTTON_CLEAR_IRQ(MOTION_PORT, MOTION_PIN);
	sensors_changed(&motion_sensor);
}

SENSORS_SENSOR(motion_sensor, MOTION_SENSOR_NAME, motion_value, motion_configure, motion_status);
