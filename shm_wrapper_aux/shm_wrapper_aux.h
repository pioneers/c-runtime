#ifndef SHM_WRAPPER_AUX_H
#define SHM_WRAPPER_AUX_H

#include <stdio.h>                         //for i/o
#include <stdlib.h>                        //for standard utility functions (exit, sleep)
#include <stdint.h>						   //for standard integer types
#include <sys/time.h>                      //for measuring time
#include <sys/types.h>                     //for sem_t and other standard system types
#include <sys/stat.h>                      //for some of the flags that are used (the mode constants)
#include <fcntl.h>                         //for flags used for opening and closing files (O_* constants)
#include <unistd.h>                        //for standard symbolic constants
#include <semaphore.h>                     //for semaphores
#include <sys/mman.h>                      //for posix shared memory
#include "../logger/logger.h"              //for logger (TODO: consider removing relative pathname in include)
#include "../runtime_util/runtime_util.h"  //for runtime constants (TODO: consider removing relative pathname in include)

#define NUM_DESC_FIELDS 6                   //number of fields in the robot description
#define NUM_GAMEPAD_BUTTONS 17              //number of gamepad buttons

//enumerated names for the buttons on the gamepad
typedef enum gp_buttons {
	A_BUTTON, B_BUTTON, X_BUTTON, Y_BUTTON, L_BUMPER, R_BUMPER, L_TRIGGER, R_TRIGGER,
	BACK_BUTTON, START_BUTTON, L_STICK, R_STICK, UP_DPAD, DOWN_DPAD, LEFT_DPAD, RIGHT_DPAD, XBOX_BUTTON
} gp_button_t;

//enumerated names for the joystick params of the gamepad
typedef enum gp_joysticks {
	X_LEFT_JOYSTICK, Y_LEFT_JOYSTICK, X_RIGHT_JOYSTICK, Y_RIGHT_JOYSTICK
} gp_joystick_t;

//enumerated names for the different values the robot description fields can take on
typedef enum robot_desc_vals {
	ISSUE, NOMINAL,             //values for robot.state
	IDLE, AUTO, TELEOP,         //values for robot.run_mode
	CONNECTED, DISCONNECTED,    //values for robot.dawn, robot.shepherd, robot.gamepad
	BLUE, GOLD                  //values for robot.team
} robot_desc_val_t;

//enumerated names for the fields in the robot description
typedef enum robot_descs {
	STATE, RUN_MODE, DAWN, SHEPHERD, GAMEPAD, TEAM
} robot_desc_field_t;

// ******************************************* UTILITY FUNCTIONS ****************************************** //

void print_robot_desc ();

void print_gamepad_state ();

// ******************************************* WRAPPER FUNCTIONS ****************************************** //

/*
Call this function from every process that wants to use the auxiliary shared memory wrapper
Should be called before any other action happens
The network handler process is responsible for initializing the shared memory blocks
	- process: one of DEV_HANDLER, EXECUTOR, NET_HANDLER to specify which process this function is
		being called from
No return value.
*/
void shm_aux_init (process_t process);

/*
Call this function if process no longer wishes to connect to auxiliary shared memory wrapper
No other actions will work after this function is called
Newtork handler is responsible for marking shared memory blocks and semaphores for destruction after all detach
	- process: one of DEV_HANDLER, EXECUTOR, NET_HANDLER to specify which process this function is
		being called from
No return value.
*/
void shm_aux_stop (process_t process);

/*
This function reads the specified field.
Blocks on the robot description semaphore.
	- process: one of DEV_HANDLER, EXECUTOR, NET_HANDLER to specify which process this function is
		being called from
	- field: one of the robot_desc_val_t's defined above to read from
Returns one of the robot_desc_val_t's defined above that is the current value of that field.
*/
robot_desc_val_t robot_desc_read (process_t process, robot_desc_field_t field);

/*
This function writes the specified value into the specified field.
Blocks on the robot description semaphore.
	- process: one of DEV_HANDLER, EXECUTOR, NET_HANDLER to specify which process this function is
		being called from
	- field: one of the robot_desc_val_t's defined above to write val to
	- val: one of the robot_desc_vals defined above to write to the specified field
No return value.
*/
void robot_desc_write (process_t process, robot_desc_field_t field, robot_desc_val_t val);

/*
This function reads the current state of the gamepad to the provided pointers.
Blocks on both the gamepad semaphore and device description semaphore (to check if gamepad connected).
	- pressed_buttons: pointer to 32-bit bitmap to which the current button bitmap state will be read into
	- joystick_vals: array of 4 floats to which the current joystick states will be read into
No return value.
*/
void gamepad_read (process_t process, uint32_t *pressed_buttons, float *joystick_vals);

/*
This function writes the given state of the gamepad to shared memory.
Blocks on both the gamepad semaphore and device description semaphore (to check if gamepad connected).
	- pressed_buttons: a 32-bit bitmap that corresponds to which buttons are currently pressed
		(only the first 17 bits used, since there are 17 buttons)
	- joystick_vals: array of 4 floats that contain the values to write to the joystick
No return value.
*/
void gamepad_write (process_t process, uint32_t pressed_buttons, float *joystick_vals);

#endif