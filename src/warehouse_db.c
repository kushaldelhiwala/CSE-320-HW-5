#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>
#include <ctype.h>
#include <stdint.h>
#include <fcntl.h> // for open

typedef struct warehouse_struct{
	int is_valid;
	char attr_name[100];
	int thread_id;

}warehouse_struct;

int thread_track[20];
int thread_track_count;

int main(int argc, char** argv)
{
	//sigint handler
	//char* warehouse_input = *(argv + 1);
	//int warehouse_size = atoi(warehouse_input);
	int server_fifo1 = open("fifo_server1", O_RDWR);
	int client_fifo1 = open("fifo_client1", O_RDWR);

	char operation[100];
	pthread_t incoming_thread;
	char buffer1[100];
	char buffer2[100];
	
	while(1){
		read(server_fifo1, buffer1, 100*sizeof(char));
		sscanf(buffer1, "%s", operation);

		if (strcmp(operation, "Thread") == 0){
			sscanf(buffer1, "%*s %ld", &incoming_thread);
			printf("PIPE reading thread. Thread ID is %ld\n", incoming_thread);
				
		}
		
	}

	return 0;
}

