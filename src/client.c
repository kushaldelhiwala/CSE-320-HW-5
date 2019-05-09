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

int client; 
int thread_close = 1;
char *strlwr(char *str);
void *shell_start(void *vargp);

typedef struct level_two{
	int two_occupied; 
	int physical_address;

}level_two;

typedef struct level_one{
	int one_occupied;
	level_two pointSecond[4];

}level_one;


int main(int argc, char** argv)
{

	char input_line[255];
	char* found;
	char* array[100];
	char command[255];
	char input_find[255];
	int exit_flag = 0;
	level_one main_array[16];	
	
	int server_fifo;
	int client_fifo;
	int start_control = 0;

	server_fifo = open("fifo_server1", O_RDWR);
	client_fifo = open("fifo_client1", O_RDWR);
		
	do{
		printf("shell> ");
		fgets(input_line, 255, stdin);
		strtok(input_line, "\n");
		
		strcpy(input_find, input_line);	
		found = strtok(input_find, " ");
		array[0] = strdup(found);
		array[1] = NULL;
		array[0] = strlwr(array[0]);

		if (strcmp(array[0], "start") == 0){
			pthread_t thread_id; 
			pthread_create(&thread_id, NULL, shell_start, NULL);
			printf("THREAD ID CLIENT: %ld\n", thread_id);
			char buf1 [100];
		
			sprintf(buf1, "Thread %ld", thread_id);
			printf("BUFFER 1: %s\n", buf1);
			write(server_fifo, buf1,100*sizeof(char)); 		
		}
		else if (strcmp(array[0], "alloc") == 0){
			   					

		}
		else if (strcmp(array[0], "dealloc") == 0){}
		else if (strcmp(array[0], "read") == 0){}
		else if (strcmp(array[0], "store") == 0){}
		else if (strcmp(array[0], "close") == 0){
			thread_close = 0;
		}
		else if (strcmp(array[0], "infotab") == 0){}
		else if (strcmp(array[0], "exit") == 0){
			exit_flag = 1;
		}
		else{}



	} while (exit_flag == 0);
	
	close(server_fifo);
	close(client_fifo);
	return 0;
}

void *shell_start(void* vargp)
{
	int found; 

	while(1){
		if (thread_close == 0){
			pthread_exit(NULL);
		}

	}	

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
