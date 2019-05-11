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
int thread_close;
char *strlwr(char *str);
void *shell_start(void *vargp);
pthread_t thread_id;

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
	if (argc != 2){
		printf("You need to specify a unique client number (1-4)\n");
		return -1;
	}
	
	char *client_input = *(argv + 1);
	client = atoi(client_input);
	
	char input_line[255];
	char* found;
	char* array[100];
	char command[255];
	char input_find[255];
	int exit_flag = 0;
	level_one main_array[16];
	thread_close = 1;

	for (int i = 0; i < 16; i++){
		main_array[i].is_occupied1 = 0;
		
		for (int j = 0; j < 4; j++){
			main_array[i].pointSecond[j].is_occupied2 = 0;
			main_array[i].pointSecond[j].physical_address = -1;
		}
	}	
	
	int server_fifo;
	int client_fifo;
	int start_control = 0;
	
	if (client == 1){
		server_fifo = open("fifo_server1", O_RDWR);
		client_fifo = open("fifo_client1", O_RDWR);
	}

	else if (client == 2){
		server_fifo = open("fifo_server2", O_RDWR);
		client_fifo = open("fifo_client2", O_RDWR);
	}

	else if (client == 3){
		server_fifo = open("fifo_server3", O_RDWR);
		client_fifo = open("fifo_client3", O_RDWR);
	}

	else if (client == 4){
		server_fifo = open("fifo_server4", O_RDWR);
		client_fifo = open("fifo_client4", O_RDWR);
	}	
	else{
		fprintf(stderr, "Please enter a FIFO from 1-4\n");
	}

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
				pthread_create(&thread_id, NULL, shell_start, NULL);
				char buf1 [100];
					
				sprintf(buf1, "Thread %ld", thread_id);
				printf("Session is now active\n");
				write(server_fifo, buf1,100*sizeof(char)); 	
				start_control++;	
			}
		}
		else if (strcmp(array[0], "alloc") == 0){
			if (start_control == 0){
				fprintf(stderr, "Please Start the Program First\n");
			}
			else{
				int i = 0;
				int j = 0;
				int success = 0;
				while (i < 16){
					if (main_array[i].is_occupied1 == 0){	
						while (j < 4){
							if(main_array[i].pointSecond[j].is_occupied2 == 0){
								char buff2[100];
								char buff3[100];
								int warehouse_index;

								sprintf(buff2, "Alloc");
								printf("Allocating space...\n");
								write(server_fifo, buff2, 100*sizeof(char));
								read(client_fifo, buff3, 100*sizeof(char));

								sscanf(buff3, "%d", &warehouse_index);
								int local_id = i;
								local_id = local_id << 4;
								local_id|= j;
								printf("Allocation Complete! Local ID: %d will be stored at Warehouse Index: %d\n", local_id, warehouse_index);
								success = 1;
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
				if (success == 0){
					fprintf(stderr, "You are out of space\n");
				}
			} 			
		}

		else if (strcmp(array[0], "dealloc") == 0){
			if (start_control == 0){
				fprintf(stderr, "Please Start the Program First\n");
			}
			else{
				int dealloc_id;
			
				sscanf(input_line, "%*s %d", &dealloc_id);
			
				if (dealloc_id < 0){
					fprintf(stderr, "Dealloc ID is invalid\n");
				}
				int dealloc_temp = dealloc_id;
				dealloc_temp &= 3;
				int second_level = dealloc_temp;
		
				dealloc_temp = dealloc_id;
				dealloc_temp = dealloc_temp >> 2;
			
				int first_level = dealloc_temp;

				if (main_array[first_level].pointSecond[second_level].is_occupied2 == 1){				
					main_array[first_level].pointSecond[second_level].is_occupied2 = 0;
				}
				else{
					fprintf(stderr, "This memory address was not allocated\n");
				}
				if (main_array[first_level].is_occupied1 == 1){
					main_array[first_level].is_occupied1 = 0;
				}
			
				char buff2[100];
				int warehouse_id = main_array[first_level].pointSecond[second_level].physical_address;
				sprintf(buff2, "Dealloc %d", warehouse_id);
				printf("Deallocing started\n");
				write(server_fifo, buff2, 100*sizeof(char));
				printf("Dealling complete\n");
			}						
		}
		else if (strcmp(array[0], "read") == 0){
			if (start_control == 0){
				fprintf(stderr, "Please Start the Program First\n");
			}
			else{
				int read_id;
			
				sscanf(input_line, "%*s %d", &read_id);

				if (read_id < 0){
					fprintf(stderr, "Read ID is invalid\n");
				}
	
				int read_temp = read_id;
				read_temp &= 3;
				int second_level = read_temp;

				read_temp = read_id;
				read_temp = read_temp >> 2;

				int first_level = read_temp;
			
				char buff2[100];
				char buff4[100];
				int physical_address = main_array[first_level].pointSecond[second_level].physical_address;
				sprintf(buff2, "Read %d", physical_address);
				printf("Reading from server...\n");
				write(server_fifo, buff2, 100*sizeof(char));
				read(client_fifo, buff4, 100*sizeof(char));
				printf("Art Entry at %d is: %s\n", read_id ,buff4);			
			}
		}
		else if (strcmp(array[0], "store") == 0){
			if (start_control == 0){
				fprintf(stderr, "Please Start the Program First\n");
			}
			else{
				int store_id; 
				char store_array[50];
				char temp_array[50];
				temp_array[0] = '"';
				temp_array[1] = 0;
			
				sscanf(input_line, "%*s %d \"%[^\"]\"", &store_id, store_array);	
				if (store_id < 0){
					fprintf(stderr, "Store ID is invalid\n");
				}
			
				strcat(temp_array, store_array);
				strcat(temp_array, "\"");
			
				int store_temp = store_id;
                        	store_temp &= 3;
                        	int second_level = store_temp; 
                        	store_temp = store_id;
                        	store_temp = store_temp >> 2;
                        	int first_level = store_temp;

				char buff5[100];
				char buff6[100];
				if (main_array[first_level].pointSecond[second_level].is_occupied2 == 1){
					int physical_address = main_array[first_level].pointSecond[second_level].physical_address;
					sprintf(buff5, "Store %d %s", physical_address, temp_array);
					//printf("Storing in server...%s\n", temp_array);
					printf("BUFF 5: %s\n", buff5);
					write(server_fifo, buff5, 100*sizeof(char));
					read(client_fifo, buff6, 100*sizeof(char));
					printf("%s", buff6);
				}
				else{
					fprintf(stderr, "Unoccupied store\n");
				}
			}
	
		}
		else if (strcmp(array[0], "close") == 0){
			char buf5[100];
			for (int i = 0; i < 16; i++){
				main_array[i].is_occupied1 = 0;
				for (int j = 0; j < 4; j++){
					main_array[i].pointSecond[j].is_occupied2 = 0;
					main_array[i].pointSecond[j].physical_address = -1;
				}
			}
			
			sprintf(buf5, "Close %ld", thread_id);
			write(server_fifo, buf5, 100*sizeof(char));
			thread_close = 0;
			start_control--;
			sleep(1);
			printf("Your session has been closed. Thank you!\n");
						
		}		
		else if (strcmp(array[0], "infotab") == 0){
			if (start_control == 0){
				fprintf(stderr, "Please Start the Program First\n");
			}
			else{
				char buf10[100];
				int temp1; 
				printf("First Level Table\n");
				printf("Number\t");
				printf("Is Occupied\n");
				for (int i = 0; i < 16; i++){
					printf("%d\t", i);
					printf("%d\n", main_array[i].is_occupied1);
				}

				printf("Please enter a number (0-15) to see the second level table\n");
				if (fgets(buf10, sizeof(buf10), stdin)) {
    					if (1 == sscanf(buf10, "%d", &temp1)){
						printf("Number\t");
						printf("Is Occupied\t");
						printf("Physical Address\n");
        					for (int j = 0; j < 4; j++){
							printf("%d\t", j);
							printf("%d\t\t", main_array[temp1].pointSecond[j].is_occupied2);
							printf("%d\n", main_array[temp1].pointSecond[j].physical_address);
							printf("Note: -1 is default physical address if it hasn't been set\n");
						}
    					}
				}
			}
		}
		else if (strcmp(array[0], "exit") == 0){
			exit_flag = 1;
		}
		else{
			fprintf(stderr, "Invalid command\n");
		}

	} while (exit_flag == 0);
	
	close(server_fifo);
	close(client_fifo);
	return 0;
}

void *shell_start(void* vargp)
{
	while(1){
		if (thread_close == 0){
			pthread_exit(NULL);
			sleep(1);
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
