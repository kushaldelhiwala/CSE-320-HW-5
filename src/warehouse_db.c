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
	pthread_t thread_id;

}warehouse_struct;

int thread_track[20];
int thread_track_count;
void handler1(int sig);
char *strlwr(char *str);
int total_collections;
warehouse_struct warehouse[10000];

int main(int argc, char** argv)
{
	//signal(SIGINT, handler1);
	char* warehouse_input = *(argv + 1);
	int warehouse_size = atoi(warehouse_input);
	printf("Warehouse Size: %d\n", warehouse_size);
	signal(SIGINT, handler1);
	
	if (warehouse_size > 10000){
		printf("Warehouse size is too big. Please try again.\n");
		exit(1);
	}
 
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
			thread_track[thread_track_count] = incoming_thread;
			thread_track_count++;		
		}
		
		if (strcmp(operation, "Alloc") == 0){
			printf("Recieved Alloc Message!\n");
			for (int i = 0; i < warehouse_size; i++){
				if (warehouse[i].is_valid == 0){
					char buf3 [100];
					sprintf(buf3, "%d", i);
					printf("Writing this to client...%s\n", buf3);
					write(client_fifo1, buf3, 100*sizeof(char));
					warehouse[i].is_valid = 1;
					warehouse[i].thread_id = thread_track[0];
					break;
				}
			} 	
		}
		
	}

	return 0;
}

void handler1(int sig)
{
	int exit_flag = 0;
	char input_line[255];
	char* found;
	char* array[100];
	char command[255];
	char input_find[255];	

	do{
		sleep(1);
		printf("shell> ");
		fgets(input_line, 255, stdin);
		strtok(input_line, "\n");
		
		strcpy(input_find, input_line);	
		found = strtok(input_find, " ");
		array[0] = strdup(found);
		array[1] = NULL;
		array[0] = strlwr(array[0]);


		if (strcmp(array[0], "list") == 0){
				
	
		}
	
		if (strcmp(array[0], "exit") == 0){
			exit_flag = 1;
			exit(0);
		}
		

	} while(exit_flag == 0);


}
char *strlwr(char *str)
{
	unsigned char *p = (unsigned char *)str;

	while (*p) {
		*p = tolower((unsigned char)*p);
		p++;
  	}

  	return str;
}
