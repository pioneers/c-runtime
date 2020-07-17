#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>
#include "shm_wrapper.h"

//test process 2 for shm_wrapper. is a dummy executor

// ************************************* HELPER FUNCTIONS ******************************************** //

void sync()
{
	int dev_ix;
	uint32_t catalog;
	param_val_t params_in[MAX_PARAMS];
	param_val_t params_out[MAX_PARAMS];

	//wait until sync in tester1 connects the sync device
	while (1) {
		get_catalog(&catalog);
		if (catalog) {
			break;
		}
		usleep(1000);
	}

	params_in[0].p_b = 0;
	device_write(0, EXECUTOR, COMMAND, 1, params_in); //write a 0 to the downstream block
	params_in[0].p_b = 1;
	while (1) {
		device_write(0, EXECUTOR, DATA, 1, params_in); //write 1 to the upstream block
		device_read(0, EXECUTOR, COMMAND, 1, params_out); //wait on a 1 in the downstream block
		if (params_out[0].p_b) {
			break;
		}
		usleep(100);
	}
	//wait until sync in tester1 disconnects the device
	while (1) {
		get_catalog(&catalog);
		if (!catalog) {
			break;
		}
		usleep(1000);
	}
	sleep(1);
	printf("\tSynced; starting test!\n");
}

// *************************************************************************************************** //
//test the param bitmap and sanity check to make sure shm connection is functioning
void sanity_test()
{
	param_val_t params_in[MAX_PARAMS];
	params_in[1].p_i = 10;
	params_in[1].p_f = -0.9;
	params_in[1].p_b = 1;
	
	param_val_t params_out[MAX_PARAMS];
	
	uint32_t pmap[33];
	
	sync();
	
	print_dev_ids();
	
	//write 5 times to the command block of the device at varying times
	for (int i = 0; i < 5; i++) {
		params_in[1].p_i += 10;
		params_in[1].p_f += 0.1;
		params_in[1].p_b = 1 - params_in[1].p_b;

		device_write(0, EXECUTOR, COMMAND, 2, params_in);
		
		print_cmd_map();
		
		sleep((unsigned int) ((float) (i + 2)) / 2.0);
	}
	
	print_cmd_map();
}

// *************************************************************************************************** //
//test connecting devices in quick succession
//test failure message print when connecting too many devices
//test disconnecting and reconnecting devices on system at capacity
//test disconnecting devices in quick succession
void dev_conn_test ()
{
	sync();
	
	//process2's job is simply to monitor what's going on in the catalog
	//as devices connect and disconnect. all the work is being done from process1
	for (int i = 0; i < 7; i++) {
		print_dev_ids();
		usleep(500000); //sleep for 0.5 seconds between prints
	}
}

// *************************************************************************************************** //
//test to find approximately how many read/write operations
//can be done in a second on a device downstream block between
//exeuctor and device handler
void single_thread_load_test ()
{
	int dev_ix = -1;
	
	int count = 100; //starting number of writes to complete
	double gain = 40000;
	
	double range = 0.01; //tolerance range of time taken around 1 second for test to complete
	int counts[5]; //to hold the counts that resulted in those times
	int good_trials = 0;
	
	struct timeval start, end;
	double time_taken;
	
	uint32_t catalog = 0;
	uint32_t pmap[MAX_DEVICES + 1];
	param_val_t params_in[MAX_PARAMS];
	params_in[0].p_b = 0;
	params_in[0].p_i = 1;
	params_in[0].p_f = 2.0;
	
	sync();
	
	//wait until the two devices connect
	while (1) {
		get_catalog(&catalog);
		if ((catalog & 1) && (catalog & 2)) {
			break;
		}
	}
	
	//adjust the count until 5 trials lie within 0.01 second of 1 second
	while (good_trials < 5) {
		gettimeofday(&start, NULL);
		for (int i = 0; i < count; i++) {
			//wait until previous write has been pulled out by process1
			while (1) {
				get_cmd_map(pmap);
				if (!pmap[0]) {
					break;
				}
			}
			device_write(0, EXECUTOR, COMMAND, 1, params_in); //write into block
		}
		gettimeofday(&end, NULL);
		
		time_taken = (double)(end.tv_sec - start.tv_sec) + ((double)(end.tv_usec - start.tv_usec) / 1000000.0);
		if (time_taken > (1.0 - range) && time_taken < (1.0 + range)) {
			counts[good_trials++] = count;
		} else {
			count += (int)((1.0 - time_taken) * gain);
		}
		printf("count = %d completed in %f seconds for %f writes / second\n", count, time_taken, (double) count / time_taken);
	}
	
	//manually calculate and print the average bc laziness :P
	printf("\naverage: %f writes / second\n", (double)(counts[0] + counts[1] + counts[2] + counts[3] + counts[4]) / 5.0);
	
	//tell process1 to stop reading
	params_in[0].p_b = 1;
	device_write(1, EXECUTOR, DATA, 1, params_in);
	
	//wait for devices to disconnect
	while (1) {
		get_catalog(&catalog);
		if (!catalog) {
			break;
		}
		usleep(1000);
	}
}

// *************************************************************************************************** //
//threaded function for reading in Dual Thread Read Write Test
void *read_thread_dtrwt (void *arg)
{
	int prev_val = 0, count = 0, i = 0;
	param_val_t params_test[MAX_PARAMS];
	param_val_t params_out[MAX_PARAMS];
	
	//we are reading from the device data block
	//use the device data block on device 1 so that tester2 can signal end of test
	params_test[0].p_b = 0;
	device_write(1, EXECUTOR, DATA, 1, params_test);
	
	//use the second device device's p_b on param 0 to indicate when test is done
	//sit here pulling information from the block as fast as possible, 
	//and count how many unique values we get until tester1 says we're done
	while (1) {
		//check if time to stop
		i++;
		if (i == 1000) {
			device_read(1, EXECUTOR, DATA, 1, params_test);
			if (params_test[0].p_b) {
				break;
			}
			i = 0;
		}
		
		//pull from that block and record it if changed
		device_read(0, EXECUTOR, DATA, 1, params_out);
		if (prev_val != params_out[0].p_i) {
			prev_val = params_out[0].p_i;
			count++;
		}
	}
	printf("read_thread from tester2 pulled %d unique values from tester2 on upstream block\n", count);
	
	return NULL;
}

//threaded function for writing in Dual Thread Read Write Test
void *write_thread_dtrwt (void *arg)
{
	const int trials = 100000; //write 100,000 times to the block
	param_val_t params_test[MAX_PARAMS];
	param_val_t params_in[MAX_PARAMS];
	uint32_t pmap[MAX_DEVICES + 1];
	
	//we are writing to the device command block
	//write 100,000 times on the device command block as fast as possible
	params_in[0].p_i = 1;
	for (int i = 0; i < trials; i++) {
		(params_in[0].p_i)++;
		while (1) {
			get_cmd_map(pmap);
			if (!pmap[0]) {
				break;
			}
		}
		device_write(0, EXECUTOR, COMMAND, 1, params_in); //write into block
	}
	printf("write_thread from tester2 wrote %d values to downstream block\n", trials);
	
	//signal on the device downnstream block on device 1 so tester1 can stop reading
	params_test[0].p_b = 1;
	device_write(1, EXECUTOR, COMMAND, 1, params_test);
	
	return NULL;
}

//test reading and writing on a device's upstream and downstream block
//at the same time using two threads. from the executor, we will be
//writing to the downstream block and reading from the upstream block
void dual_thread_read_write_test ()
{
	int dev_ix = -1;
	int status;
	pthread_t read_tid, write_tid; //thread_ids for the two threads
	
	sync();
	
	printf("Beginning dual thread read write test...\n");
	
	//create threads
	if ((status = pthread_create(&read_tid, NULL, read_thread_dtrwt, NULL)) != 0) {
		printf("read pthread creation failed with exit code %d\n", status);
	}
	printf("Read thread created\n");
	
	if ((status = pthread_create(&write_tid, NULL, write_thread_dtrwt, NULL)) != 0) {
		printf("write pthread creation failed with exit code %d\n", status);
	}
	printf("Write thread created\n");
	
	//wait for the threads to finish
	pthread_join(read_tid, NULL);
	pthread_join(write_tid, NULL);
	
	sleep(1);
	
	printf("Done!\n");
}

// *************************************************************************************************** //

//test to find approximately how many read/write operations
//can be done in a second on a device downstream block between
//exeuctor and device handler, USING DEV_UID READ/WRITE FUNCTIONS
void single_thread_load_test_uid ()
{
	int dev_ix = -1;
	uint64_t dev_uid = 0;
	dev_id_t dev_ids[MAX_DEVICES];
	
	int count = 100; //starting number of writes to complete
	double gain = 40000;
	
	double range = 0.01; //tolerance range of time taken around 1 second for test to complete
	int counts[5]; //to hold the counts that resulted in those times
	int good_trials = 0;
	
	struct timeval start, end;
	double time_taken;
	
	uint32_t catalog = 0;
	uint32_t pmap[MAX_DEVICES + 1];
	param_val_t params_in[MAX_PARAMS];
	params_in[0].p_b = 0;
	params_in[0].p_i = 1;
	params_in[0].p_f = 2.0;
	
	sync();
	
	//wait until the two devices connect
	while (1) {
		get_catalog(&catalog);
		if ((catalog & 1) && (catalog & 2)) {
			break;
		}
	}
	
	get_device_identifiers(dev_ids);
	dev_uid = dev_ids[0].uid;
	
	//adjust the count until 5 trials lie within 0.01 second of 1 second
	while (good_trials < 5) {
		gettimeofday(&start, NULL);
		for (int i = 0; i < count; i++) {
			//wait until previous write has been pulled out by process1
			while (1) {
				get_cmd_map(pmap);
				if (!pmap[0]) {
					break;
				}
			}
			device_write_uid(dev_uid, EXECUTOR, COMMAND, 1, params_in); //write into block
		}
		gettimeofday(&end, NULL);
		
		time_taken = (double)(end.tv_sec - start.tv_sec) + ((double)(end.tv_usec - start.tv_usec) / 1000000.0);
		if (time_taken > (1.0 - range) && time_taken < (1.0 + range)) {
			counts[good_trials++] = count;
		} else {
			count += (int)((1.0 - time_taken) * gain);
		}
		printf("count = %d completed in %f seconds for %f writes / second using UID functions\n", count, time_taken, (double) count / time_taken);
	}
	
	//manually calculate and print the average bc laziness :P
	printf("\naverage: %f writes / second using UID functions\n", (double)(counts[0] + counts[1] + counts[2] + counts[3] + counts[4]) / 5.0);
	
	//tell process1 to stop reading
	params_in[0].p_b = 1;
	device_write(1, EXECUTOR, DATA, 1, params_in);
	
	//wait for devices to disconnect
	while (1) {
		get_catalog(&catalog);
		if (!catalog) {
			break;
		}
		usleep(1000);
	}
}

// *************************************************************************************************** //

//sanity gamepad test
void sanity_gamepad_test ()
{
	uint32_t buttons;
	float joystick_vals[4];
	
	sync();
	
	printf("Begin sanity gamepad test...\n");
	
	for (int i = 0; i < 7; i++) {
		gamepad_read(&buttons, joystick_vals);
		printf("buttons = %d\t joystick_vals = (", buttons);
		for (int j = 0; j < 4; j++) {
			printf("%f, ", joystick_vals[j]);
		}
		printf(")\n");
		usleep(500000); //sleep for half a second
	}
	printf("Done!\n\n");
}

// *************************************************************************************************** //

//sanity robot description test
void sanity_robot_desc_test ()
{
	int count = 0;
	robot_desc_val_t curr[6] = { IDLE, DISCONNECTED, DISCONNECTED, CONNECTED };
	robot_desc_val_t prev[6] = { IDLE, DISCONNECTED, DISCONNECTED, CONNECTED };
	
	sync();
	
	printf("Begin sanity robot desc test...\n");
	
	while (count < 9) {
		for (int i = 0; i < NUM_DESC_FIELDS; i++) {
			curr[i] = robot_desc_read(i);
			if (curr[i] != prev[i]) {
				printf("something has changed! new robot description:\n");
				print_robot_desc();
				prev[i] = curr[i];
				count++;
			}
		}
		usleep(100);
	}
	printf("Done!\n\n");
}

// *************************************************************************************************** //

//check that device subscriptions are working
void subscription_test ()
{
	uint64_t id0, id1, id2;
	dev_id_t dev_ids[MAX_DEVICES];
	
	sync();
	
	printf("Begin subscription test...\n");
	
	usleep(100000); //put this program out of sync with test1 loop
	
	get_device_identifiers(dev_ids);
	id0 = dev_ids[0].uid;
	id1 = dev_ids[1].uid;
	id2 = dev_ids[2].uid;
	
	for (int i = 0; i < 5; i++) {
		switch(i) {
			case 0: //sanity
				place_sub_request(id0, EXECUTOR, 1);
				break;
			case 1: //different processes
				place_sub_request(id1, NET_HANDLER, 2);
				place_sub_request(id2, EXECUTOR, 3);
				break;
			case 2: //setting the same subscription
				place_sub_request(id0, EXECUTOR, 1);
				break;
			case 3: //removing a param from subscription
				place_sub_request(id2, EXECUTOR, 1);
				break;
			case 4: //don't do anything
				break;
			default:
				break;			
		}
		sleep(1);
	}
	
	printf("Done!\n");
}

// *************************************************************************************************** //

void ctrl_c_handler (int sig_num)
{
	printf("Aborting and cleaning up\n");
	fflush(stdout);
	shm_stop(EXECUTOR);
	logger_stop(EXECUTOR);
	exit(1);
}

int main()
{	
	shm_init();
	logger_init(EXECUTOR);
	signal(SIGINT, ctrl_c_handler); //hopefully fails gracefully when pressing Ctrl-C in the terminal
	
	sanity_test();	
	
	dev_conn_test();
	
	single_thread_load_test();
	
	dual_thread_read_write_test();
	
	single_thread_load_test_uid();
	
	sanity_gamepad_test();
	
	sanity_robot_desc_test();
	
	subscription_test();
	
	return 0;
}