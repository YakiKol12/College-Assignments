#include "chatServer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/select.h>

static int end_server = 0;

void intHandler(int);
int isNumber(const char*);

int main (int argc, char *argv[]) {
	
	signal(SIGINT, intHandler);
   
	conn_pool_t* pool = malloc(sizeof(conn_pool_t));
	init_pool(pool);
   
	/*************************************************************/
	/* Create an AF_INET stream socket to receive incoming       */
	/* connections on                                            */
	/*************************************************************/
	int ws;
	if((ws = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket failed");
		free(pool);
		close(ws);
		exit(EXIT_FAILURE);
	}

	/*************************************************************/
	/* Set socket to be nonblocking. All of the sockets for      */
	/* the incoming connections will also be nonblocking since   */
	/* they will inherit that state from the listening socket.   */
	/*************************************************************/
	int on = 1;
	int rc = ioctl(ws, (int)FIONBIO, (char*)&on);
	if(rc != 0) {
		perror("ioctl failed");
		free(pool);
		close(ws);
		exit(EXIT_FAILURE);
	}

	/*************************************************************/
	/* Bind the socket                                           */
	/*************************************************************/
	struct sockaddr_in srv;
	srv.sin_family = AF_INET;
	srv.sin_addr.s_addr = htonl(INADDR_ANY);

	if(argc != 2) {
		printf("Usage: server <port>\n");
		free(pool);
		close(ws);
		exit(EXIT_FAILURE);
	}

	if(argv[1] == NULL || isNumber(argv[1]) == 0) {
		printf("Usage: server <port>\n");
		free(pool);
		close(ws);
		exit(EXIT_FAILURE);
	}
	int port = atoi(argv[1]);
	if(port < 1 || port > 65536) {
		printf("Usage: server <port>");
		free(pool);
		close(ws);
		exit(EXIT_FAILURE);
	}
	srv.sin_port = htons(port);

	if(bind(ws, (struct sockaddr*) &srv, sizeof(srv)) < 0) {
		perror("bind failed");
		free(pool);
		close(ws);
		exit(EXIT_FAILURE);
	}

	/*************************************************************/
	/* Set the listen back log                                   */
	/*************************************************************/
	if(listen(ws, 5) < 0) {
		perror("listen failed");
		free(pool);
		close(ws);
		exit(EXIT_FAILURE);
	}

	/*************************************************************/
	/* Initialize fd_sets  			                             */
	/*************************************************************/
	pool->maxfd = ws;
	FD_SET(ws, &pool->read_set);

	/*************************************************************/
	/* Loop waiting for incoming connects, for incoming data or  */
	/* to write data, on any of the connected sockets.           */
	/*************************************************************/
	do {
		/**********************************************************/
		/* Copy the master fd_set over to the working fd_set.     */
		/**********************************************************/
		pool->ready_read_set = pool->read_set;
		pool->ready_write_set = pool->write_set;
		/**********************************************************/
		/* Call select() 										  */
		/**********************************************************/
		printf("Waiting on select()...\nMaxFd %d\n", pool->maxfd);
		pool->nready = select(pool->maxfd + 1, &pool->ready_read_set,
			 &pool->ready_write_set, NULL, NULL);
		if(pool->nready < 0) {
			break;
		}
		
		/**********************************************************/
		/* One or more descriptors are readable or writable.      */
		/* Need to determine which ones they are.                 */
		/**********************************************************/
		int count = 0;
		for (int i = 3; i <= pool->maxfd && count < pool->nready; i++) {
			/* Each time a ready descriptor is found, one less has  */
			/* to be looked for.  This is being done so that we     */
			/* can stop looking at the working set once we have     */
			/* found all of the descriptors that were ready         */
				
			/*******************************************************/
			/* Check to see if this descriptor is ready for read   */
			/*******************************************************/
			if (FD_ISSET(i, &pool->ready_read_set)) {
				count++;
				/***************************************************/
				/* A descriptor was found that was readable		   */
				/* if this is the listening socket, accept one      */
				/* incoming connection that is queued up on the     */
				/*  listening socket before we loop back and call   */
				/* select again. 						            */
				/****************************************************/
				if(i == ws) {
                	int newFd = accept(ws, NULL, NULL);
					if(add_conn(newFd, pool) == -1) {
						close(newFd);
						continue;
					}
					printf("New incoming connection on sd %d\n", newFd);
				}
				/****************************************************/
				/* If this is not the listening socket, an 			*/
				/* existing connection must be readable				*/
				/* Receive incoming data his socket             */
				/****************************************************/
				else {
					printf("Descriptor %d is readable\n", i);
					char buffer[BUFFER_SIZE] = "";
					int readBytes = read(i, buffer, sizeof(buffer));
					if(readBytes < 0) {
						continue;
					}
					/* If the connection has been closed by client 		*/
					/* remove the connection (remove_conn(...))    		*/
					if(readBytes == 0) {
						printf("Connection closed for sd %d\n", i);
						remove_conn(i, pool);
						if(pool->maxfd < ws) {
							pool->maxfd = ws;
						}
						continue;
					}
					/**********************************************/
					/* Data was received, add msg to all other    */
					/* connectios					  			  */
					/**********************************************/
					printf("%d bytes received from sd %d\n", readBytes, i);
					add_msg(i, buffer, readBytes, pool);
				}
				
			} /* End of if (FD_ISSET()) */
			/*******************************************************/
			/* Check to see if this descriptor is ready for write  */
			/*******************************************************/
			if (FD_ISSET(i, &pool->ready_write_set)) {
				count++;
				/* try to write all msgs in queue to sd */
				write_to_client(i, pool);
		 	}
		 /*******************************************************/
		 
		 
      } /* End of loop through selectable descriptors */

   } while (end_server == 0);

	/*************************************************************/
	/* If we are here, Control-C was typed,						 */
	/* clean up all open connections					         */
	/*************************************************************/
	conn_t* conn = pool->conn_head;
	conn_t* next = NULL;
	while(conn != NULL) {
		next = conn->next;
		remove_conn(conn->fd, pool);
		conn = next;
	}
	FD_ZERO(&pool->read_set);
	FD_ZERO(&pool->ready_read_set);
	FD_ZERO(&pool->write_set);
	FD_ZERO(&pool->ready_write_set);
	free(pool);
	pool = NULL;	
	close(ws);
	return 0;
}


int init_pool(conn_pool_t* pool) {
	//initialized all fields
	pool->maxfd = 0;
	pool->nready = 0;
	FD_ZERO(&pool->read_set);
	FD_ZERO(&pool->ready_read_set);
	FD_ZERO(&pool->write_set);
	FD_ZERO(&pool->ready_write_set);
	pool->conn_head = NULL;
	pool->nr_conns = 0;
	return 0;
}

int add_conn(int sd, conn_pool_t* pool) {
	/*
	 * 1. allocate connection and init fields
	 * 2. add connection to pool
	 * */
	conn_t *conn = (conn_t*)malloc(sizeof(conn_t));
	if(conn == NULL) {
		return -1;
	}
	conn->fd = sd;
	conn->write_msg_head = NULL;
	conn->write_msg_tail = NULL;
	conn->next = NULL;
	conn->prev = NULL;

	if(pool->conn_head == NULL) {
		pool->conn_head = conn;
	}
	else {
		conn->next = pool->conn_head;
		pool->conn_head->prev = conn;
		pool->conn_head = conn;
	}

	if(sd > pool->maxfd) {
		pool->maxfd = sd;
	}

	pool->nr_conns++;
	FD_SET(sd, &pool->read_set);
	
	return 0;
}

int remove_conn(int sd, conn_pool_t* pool) {
	printf("removing connection with sd %d \n", sd);
	/*
	* 1. remove connection from pool 
	* 2. deallocate connection 
	* 3. remove from sets 
	* 4. update max_fd if needed 
	*/

	conn_t *conn = pool->conn_head;
	while(conn != NULL) {
		if(conn->fd == sd)
			break;
		conn = conn->next;
	}
	// remove conn from pool
	if(conn == NULL) {
		return 0;
	}
	if(conn->prev == NULL) {
		pool->conn_head = conn->next;
		if(conn->next != NULL) {
			conn->next->prev = conn->prev;
		}
	}
	else if(conn->next == NULL) {
		conn->prev->next = conn->next;	
	}
	else {
		conn->prev->next = conn->next;
		conn->next->prev = conn->prev;
	}

	// free conn msg list
	msg_t* current = conn->write_msg_head;
	msg_t* next = NULL;
	
	while (current != NULL) {
		next = current->next;
		if(current->message != NULL) {
			free(current->message);
		}
		free(current);
		current = next;
	}
	conn->write_msg_head = NULL;
	conn->write_msg_tail = NULL;

	// update maxFd
	if(sd == pool->maxfd) {
		pool->maxfd = 0;
		conn_t* connection = pool->conn_head;
		while(connection != NULL) {
			if(connection->fd > pool->maxfd)
				pool->maxfd = connection->fd;
			connection = connection->next;
		}
	}

	pool->nr_conns--;
	FD_CLR(sd, &pool->read_set);
	FD_CLR(sd, &pool->write_set);
	close(sd);
	free(conn);
	return 0;
}

int add_msg(int sd,char* buffer,int len,conn_pool_t* pool) {
	/*
	 * 1. add msg_t to write queue of all other connections 
	 * 2. set each fd to check if ready to write 
	 */
	conn_t *conn = pool->conn_head;
	while(conn != NULL) {
		if(conn->fd == sd) {
			conn = conn->next;
			continue;
		}

		msg_t *msg = (msg_t*)malloc(sizeof(msg_t));
		if(msg == NULL) {
			return -1;
		}
		msg->message = (char*)malloc(sizeof(char) * len + 1);
		if(msg->message == NULL) {
			return -1;
		}
		strncpy(msg->message, buffer, len);
		msg->message[len] = '\0';
		msg->size = len;
		msg->prev = NULL;
		msg->next = NULL;

		if(conn->write_msg_head == NULL) {
			conn->write_msg_head = msg;
			conn->write_msg_tail = msg;
		}
		else {
			conn->write_msg_tail->next = msg;
			msg->prev = conn->write_msg_tail;
			conn->write_msg_tail = msg;
		}
		
		FD_SET(conn->fd, &pool->write_set);
		conn = conn->next;
	}
	return 0;
}

int write_to_client(int sd,conn_pool_t* pool) {
	/*
	 * 1. write all msgs in queue 
	 * 2. deallocate each writen msg 
	 * 3. if all msgs were writen successfully, there is nothing else to write to this fd...
	 */
	conn_t *conn = pool->conn_head;
	while(conn != NULL) {
		if(conn->fd == sd)
			break;
		conn = conn->next;
	}

	if(conn == NULL) {
		return -1;
	}

	msg_t* msg = conn->write_msg_head;
	msg_t* next = NULL;
	while(msg != NULL) {
		next = msg->next;
		write(sd, msg->message, msg->size);
		free(msg->message);
		free(msg);
		msg = next;
	}
	conn->write_msg_head = NULL;
	conn->write_msg_tail = NULL;

	FD_CLR(sd, &pool->write_set);
	return 0;
}

void intHandler(int SIG_INT) {
	/* use a flag to end_server to break the main loop */
	end_server = -1;
}

// This method checks if a given string represent a number
int isNumber(const char *str) {
    int i = strlen(str);
    while (i--) {
        if (str[i] >= '0' && str[i] <= '9')
            continue;
        return 0;
    }
    return 1;
}
