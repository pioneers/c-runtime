#ifndef UTIL_H
#define UTIL_H

// ***************************** DEFINED CONSTANTS ************************** //

//enumerate names of processes
enum processes { DEV_HANDLER, EXECUTOR, NET_HANDLER };

#define MAX_DEVICES 32 //maximum number of connected devices
#define MAX_PARAMS 32 //maximum number of parameters per connected device

// ******************************* CUSTOM STRUCTS ************************** //

//hold a single param value
typedef struct param {
	int p_i;       //data if int
	float p_f;     //data if float
	uint8_t p_b;   //data if bool
} param_val_t;

//holds the device identification information of a single device
typedef struct dev_id {
	uint16_t type;
	uint8_t year;
	uint64_t uid;
} dev_id_t;

#endif