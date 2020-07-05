#include "tcp_conn.h"

//used for creating and cleaning up TCP connection
typedef struct {
	int conn_fd;
	int challenge_fd;
    int send_logs;
    robot_desc_field_t client;
} tcp_conn_args_t;

pthread_t dawn_tid, shepherd_tid;
FILE* log_file = NULL;

struct sockaddr_un exec_addr = {AF_UNIX, CHALLENGE_SOCKET};

/*
 * Clean up memory and file descriptors before exiting from dawn connection main control loop
 * Arguments:
 *    - void *args: should be a pointer to tcp_conn_args_t populated with the listed descriptors and pointers
 */
static void tcp_conn_cleanup (void *args)
{
	tcp_conn_args_t* tcp_args = (tcp_conn_args_t *) args;

	if (close(tcp_args->conn_fd) != 0) {
		log_printf(ERROR, "Failed to close conn_fd");
		perror("close: conn_fd");
	}
	if (close(tcp_args->challenge_fd) != 0) {
		log_printf(ERROR, "Failed to close challenge_fd");
		perror("close: challenge_fd");
	}
	if (log_file != NULL) {
		if (fclose(log_file) != 0) {
			log_printf(ERROR, "Failed to close log_file");
			perror("fclose: log_file");
		}
	}
	robot_desc_write(tcp_args->client, DISCONNECTED);
	free(args);
}


/* 
 * Send a log message on the TCP connection to the client. Reads lines from the pipe until there is no more data
 * or it has read MAX_NUM_LOGS lines from the pipe, packages the message, and sends it.
 * Arguments:
 *    - int conn_fd: socket connection's file descriptor on which to write to the TCP port
 */
static void send_log_msg (int conn_fd, FILE *log_file)
{	
	char nextline[MAX_LOG_LEN];   //next log line read from FIFO pipe
	Text log_msg = TEXT__INIT;    //initialize a new Text protobuf message
	log_msg.n_payload = 0;
	log_msg.payload = malloc(MAX_NUM_LOGS * sizeof(char *));
	
	//read in log lines until there are no more to read, or we read MAX_NUM_LOGS lines
	while (log_msg.n_payload < MAX_NUM_LOGS) {
		if (fgets(nextline, MAX_LOG_LEN, log_file) != NULL) {
			log_msg.payload[log_msg.n_payload] = malloc(MAX_LOG_LEN);
			strcpy(log_msg.payload[log_msg.n_payload], nextline);
			log_msg.n_payload++;
		}
		else if (errno == EAGAIN) { // EAGAIN occurs when nothing to read
			break;
		}
		else { // Error occurred
			perror("fgets");
			log_printf(ERROR, "Error reading from log fifo\n");
			return;
		}
	}
	
	//prepare the message for sending
	uint16_t len_pb = text__get_packed_size(&log_msg);
	uint8_t* send_buf = make_buf(LOG_MSG, len_pb);
	text__pack(&log_msg, send_buf + 3);  //pack message into the rest of send_buf (starting at send_buf[3] onward)
	
	//send message on socket
	if (writen(conn_fd, send_buf, len_pb + 3) == -1) {
		perror("writen");
		log_printf(ERROR, "sending log message failed");
	}
	
	//free all allocated memory
	for (int i = 0; i < log_msg.n_payload; i++) {
		free(log_msg.payload[i]);
	}
	free(log_msg.payload);
	free(send_buf);
}


/*
 * Receives new message from client on TCP connection and processes the message.
 * Arguments:
 *    - int conn_fd: socket connection's file descriptor from which to read the message
 *    - int results_fd: file descriptor of FIFO pipe to executor to which to write challenge input data if received
 * Returns: pointer to integer in which return status will be stored
 *      0 if message received and processed
 *     -1 if message could not be parsed because Shepherd disconnected and connection closed
 *     -2 if message could not be unpacked or other error
 */
static int recv_new_msg (int conn_fd, int challenge_fd)
{
	net_msg_t msg_type;           //message type
	uint16_t len_pb;              //length of incoming serialized protobuf message
	uint8_t* buf;                 //buffer to read raw data into

	int err = parse_msg(conn_fd, &msg_type, &len_pb, &buf);
	if (err == 0) { // Means there is EOF while reading which means client disconnected
		// log_printf(DEBUG, "EOF occurred");
		return -1; 
	}
	else if (err == -1) { // Means there is some other error while reading
		return -2; 
	}
	log_printf(DEBUG,"in recv msg");
	//unpack according to message
	if (msg_type == CHALLENGE_DATA_MSG) {
		// Send to executor
		int sendlen = sendto(challenge_fd, buf, len_pb, 0, (struct sockaddr*) &exec_addr, sizeof(struct sockaddr_un));
		if (sendlen <= 0 || sendlen != len_pb) {
			perror("sendto");
			log_printf(ERROR, "Socket send to challenge_fd failed");
			return -2;
		}
		log_printf(DEBUG, "entering CHALLENGE mode. running coding challenges!");
		robot_desc_write(RUN_MODE, CHALLENGE);
	} 
	else if (msg_type == RUN_MODE_MSG) {
		log_printf(DEBUG, "Received run mode msg, len %d", len_pb);
		RunMode* run_mode_msg = run_mode__unpack(NULL, len_pb, buf);
		if (run_mode_msg == NULL) {
			log_printf(ERROR, "Cannot unpack run_mode msg");
			return -2;
		}
		
		//write the specified run mode to the RUN_MODE field of the robot description
		switch (run_mode_msg->mode) {
			case (MODE__IDLE):
				log_printf(DEBUG, "entering IDLE mode");
				robot_desc_write(RUN_MODE, IDLE);
				break;
			case (MODE__AUTO):
				log_printf(DEBUG, "entering AUTO mode");
				robot_desc_write(RUN_MODE, AUTO);
				break;
			case (MODE__TELEOP):
				log_printf(DEBUG, "entering TELEOP mode");
				robot_desc_write(RUN_MODE, TELEOP);
				break;
			case (MODE__ESTOP):
				log_printf(DEBUG, "ESTOP RECEIVED! entering IDLE mode");
				robot_desc_write(RUN_MODE, IDLE);
				break;
			default:
				log_printf(ERROR, "requested robot to enter invalid robot mode %s", run_mode_msg->mode);
				break;
		}
		run_mode__free_unpacked(run_mode_msg, NULL);
	} 
	else if (msg_type == START_POS_MSG) {
		StartPos* start_pos_msg = start_pos__unpack(NULL, len_pb, buf);
		if (start_pos_msg == NULL) {
			log_printf(ERROR, "Cannot unpack start_pos msg");
			return -2;
		}
		
		//write the specified start pos to the STARTPOS field of the robot description
		switch (start_pos_msg->pos) {
			case (POS__LEFT):
				log_printf(DEBUG, "robot is in LEFT start position");
				robot_desc_write(START_POS, LEFT);
				break;
			case (POS__RIGHT):
				log_printf(DEBUG, "robot is in RIGHT start position");
				robot_desc_write(START_POS, RIGHT);
				break;
			default:
				log_printf(WARN, "entered unknown start position");
		}
		start_pos__free_unpacked(start_pos_msg, NULL);
	} 
	else if (msg_type == DEVICE_DATA_MSG) {
		DevData* dev_data_msg = dev_data__unpack(NULL, len_pb, buf);
		if (dev_data_msg == NULL) {
			log_printf(ERROR, "Cannot unpack dev_data msg");
			return -2;
		}
		log_printf(WARN, "Received device data msg which has no implementation");
		//TODO: tell UDP suite to only send parts of data, not implemented by Dawn yet
		
		dev_data__free_unpacked(dev_data_msg, NULL);
	}
	else {
		log_printf(WARN, "unknown message type %d; shepherd should only send RUN_MODE (2), START_POS (3), or CHALLENGE_DATA (4)", msg_type);
		return -2;
	}
	free(buf);
	return 0;
}


/*
 * Main control loop for dawn connection. Sets up connection by opening up pipe to read log messages from
 * and sets up read_set for select(). Then it runs main control loop, using select() to make actions event-driven.
 * Arguments:
 *    - void *args: (always NULL)
 * Return:
 *    - NULL
 */
static void* process_tcp (void* tcp_args)
{
	tcp_conn_args_t* args = (tcp_conn_args_t*) tcp_args;
	pthread_cleanup_push(tcp_conn_cleanup, args);

	//variables used for waiting for something to do using select()
	int log_fd = fileno(log_file);
	fd_set read_set;
	int maxfd = args->challenge_fd > args->conn_fd ? args->challenge_fd : args->conn_fd;
	if (args->send_logs) {
		maxfd = log_fd > maxfd ? log_fd : maxfd;
	}
	maxfd = maxfd + 1;

	//main control loop that is responsible for sending and receiving data
	while (1) {
		//set up the read_set argument to selct()
		FD_ZERO(&read_set);
		FD_SET(args->conn_fd, &read_set);
		FD_SET(args->challenge_fd, &read_set);
		if (args->send_logs) {
			FD_SET(log_fd, &read_set);
		}
		
		//prepare to accept cancellation requests over the select
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		
		//wait for something to happen
		if (select(maxfd, &read_set, NULL, NULL, NULL) < 0) {
			log_printf(ERROR, "Failed to wait for select in control loop for client %d: %s", args->client, strerror(errno));
		}
		
		//deny all cancellation requests until the next loop
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

		//send a new log message if one is available and we want to send logs
		if (args->send_logs && FD_ISSET(log_fd, &read_set)) {
			send_log_msg(args->conn_fd, log_file);
		}

		//send challenge results if executor sent them
		if(FD_ISSET(args->challenge_fd, &read_set)) {
			// Get results from executor
			int max_size = 32 * NUM_CHALLENGES;
			uint8_t read_buf[max_size];
			int readlen = recvfrom(args->challenge_fd, read_buf, max_size, 0, NULL, NULL);
			if (readlen == max_size) {
				log_printf(WARN, "challenge_fd: Read length matches size of read buffer %d", readlen);
			}
			if (readlen <= 0) {
				perror("recvfrom");
				log_printf(ERROR, "Socket recv from challenge_fd failed");
			}

			// Send results to client
			uint8_t* send_buf = make_buf(CHALLENGE_DATA_MSG, readlen);
			memcpy(send_buf+3, read_buf, readlen); 
			if (writen(args->conn_fd, send_buf, readlen + 3) == -1) {
				perror("write");
				log_printf(ERROR, "sending challenge data message failed");
			}
		}

		//receive new message on socket if it is ready
		if (FD_ISSET(args->conn_fd, &read_set)) {
			int err = recv_new_msg(args->conn_fd, args->challenge_fd); 
			if (err == -1) { 
				log_printf(WARN, "client %d has disconnected", args->client);
				break;
			}
		}
	}
	
	//call the cleanup function
	pthread_cleanup_pop(1);
	return NULL;
}

/*
 * Start the main TCP connection control thread. Does not block.
 * Should be called when the client has requested a connection to Runtime.
 * Arguments:
 *    - int connfd: connection socket descriptor on which there is the established connection with the client
 */
void start_tcp_conn (robot_desc_field_t client, int conn_fd, int send_logs)
{
	//initialize argument to new connection thread
	tcp_conn_args_t* args = malloc(sizeof(tcp_conn_args_t));
	args->client = client;
	args->conn_fd = conn_fd;
	args->send_logs = send_logs;
	args->challenge_fd = -1;
	
	pthread_t* tid;
	if (client == DAWN) {
        tid = &dawn_tid;
    }
    else if (client == SHEPHERD) {
        tid = &shepherd_tid;
    }
    else {
        log_printf(ERROR, "Invalid TCP client %d", client);
    }

	// open challenge socket to read and write
	if ((args->challenge_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		log_printf(ERROR, "failed to create challenge socket");
		return;
	}
	struct sockaddr_un my_addr;
	memset(&my_addr, 0, sizeof(struct sockaddr_un));
	my_addr.sun_family = AF_UNIX;
	// sprintf(my_addr.sun_path, "/tmp/net_handler_%d", client);
	if (bind(args->challenge_fd, (struct sockaddr *) &my_addr, sizeof(sa_family_t)) < 0) {
        log_printf(FATAL, "challenge socket bind failed: %s", strerror(errno));
		close(args->challenge_fd);
		return;
	}

	//Open FIFO pipe for logs
	if (send_logs) {
		int log_fd;
		if ((log_fd = open(LOG_FIFO, O_RDONLY | O_NONBLOCK)) == -1) {
			perror("fifo open");
			log_printf(ERROR, "could not open log FIFO on %d", args->client);
			close(args->challenge_fd);
			return;
		}
		if ((log_file = fdopen(log_fd, "r")) == NULL) {
			perror("fdopen");
			log_printf(ERROR, "could not open log file from fd");
			close(args->challenge_fd);
			close(log_fd);
			return;
		}
	}

	//create the main control thread
	if (pthread_create(tid, NULL, process_tcp, args) != 0) {
		perror("pthread_create");
		log_printf(ERROR, "Failed to create main TCP thread for %d", client);
		return;
	}
	robot_desc_write(client, CONNECTED);
}

/*
 * Stops the TCP connection control thread with given client cleanly. May block briefly to allow
 * main control thread to finish what it's doing. Should be called right before the net_handler main loop terminates.
 */
void stop_tcp_conn (robot_desc_field_t client)
{
	pthread_t tid;
	if (client == DAWN) {
		tid = dawn_tid;
	}
	else if (client == SHEPHERD) {
		tid = shepherd_tid;
	}
	else {
		log_printf(ERROR, "Invalid TCP client %d", client);
	}

	if (pthread_cancel(tid) != 0) {
		perror("pthread_cancel");
		log_printf(ERROR, "Failed to cancel TCP client thread for %d", client);
	}
	if (pthread_join(tid, NULL) != 0) {
		perror("pthread_join");
		log_printf(ERROR, "Failed to join on TCP client thread for %d", client);
	}
}
