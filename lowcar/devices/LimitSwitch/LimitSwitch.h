#ifndef LIMIT_SWITCH_H
#define LIMIT_SWITCH_H

#include "Device.h"
#include "defs.h"

class LimitSwitch : public Device {
public:
	//constructs a LimitSwitch; simply calls generic Device constructor with device type and year
	LimitSwitch ();

	//overridden functions from Device class; see descriptions in Device.h
	virtual size_t device_read (uint8_t param, uint8_t *data_buf);
	virtual void device_enable ();

private:
	const static int NUM_SWITCHES; //number of switches (one switch per pin) on a limit switch
	const static uint8_t pins[]; //pins that the limit switch reads data from (defined in defs.h)
};

#endif
