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
	int physical_address;
	int is_occupied2;

}level_two;

typedef struct level_one{
	level_two pointSecond[4];
	int is_occupied1;
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

	for (int i = 0; i < 16; i++){
		main_array[i].is_occupied1 = 0;
		
		for (int j = 0; j < 4; j++){
			main_array[i].pointSecond[j].is_occupied2 = 0;
		}
	}	
	
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
			if (start_control == 1){
				fprintf(stderr, "You cannot start multiple times\n");
			}
			else{
				pthread_t thread_id; 
				pthread_create(&thread_id, NULL, shell_start, NULL);
				char buf1 [100];
					
				sprintf(buf1, "Thread %ld", thread_id);
				printf("Writing to Server:  %s\n", buf1);
				write(server_fifo, buf1,100*sizeof(char)); 	
				start_control++;	
			}
		}
		else if (strcmp(array[0], "alloc") == 0){
			int i = 0;
			int j = 0;

			while (i < 16){
				if (main_array[i].is_occupied1 == 0){	
					while (j < 4){
						if(main_array[i].pointSecond[j].is_occupied2 == 0){
							char buff2[100];
							char buff3[100];
							int warehouse_index;

							sprintf(buff2, "Alloc");
							printf("Writing alloc to server...\n");
							write(server_fifo, buff2, 100*sizeof(char));
							read(client_fifo, buff3, 100*sizeof(char));

							sscanf(buff3, "%d", &warehouse_index);
							printf("Message from server...%d\n", warehouse_index);
						
							main_array[i].pointSecond[j].physical_address = warehouse_index;
							main_array[i].pointSecond[j].is_occupied2 = 1;
							
							if (j == 3){
								main_array[i].is_occupied1 = 1;
	
							}
							break;
						}	
						j++;
					}
					break;
				}
				i++;
			}	   			
		}

		else if (strcmp(array[0], "dealloc") == 0){
			int dealloc_id;
			
			sscanf(input_line, "%d", &dealloc_id);
			
			if (dealloc_id < 0){
				fprintf(stderr, "Dealloc ID is invalid");
			}
			
			dealloc_id = dealloc_id << 26;
			int dealloc_temp = dealloc_id;
			dealloc_temp &= 3;
			int second_level = dealloc_temp;


			dealloc_temp = dealloc_id;
			
				
					
	


		}
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
