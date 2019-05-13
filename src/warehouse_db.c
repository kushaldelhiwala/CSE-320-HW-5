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
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
//buffers to NULL
typedef struct warehouse_struct{
	int is_valid;
	char attr_name[100];
	pthread_t thread_id;

}warehouse_struct;

pthread_t thread_track[20];
int thread_track_count = 0;
pid_t processes[20];
void handler1(int sig);
char *strlwr(char *str);
int total_collections = 0;
warehouse_struct warehouse[10000];
int warehouse_size = 0;
int server_fifo1 = 0;
int client_fifo1 = 0;
int server_fifo2 = 0;
int client_fifo2 = 0;
int server_fifo3 = 0;
int client_fifo3 = 0;
int server_fifo4 = 0;
int client_fifo4 = 0; 
void red ();
void yellow();
void green();
void blue();
void reset();

int main(int argc, char** argv)
{	
	if (argc != 2){
		printf("You need to pass in warehouse size\n");
		return -1;
	}
	
	char *warehouse_input = *(argv + 1);
	warehouse_size = atoi(warehouse_input);	

	if (warehouse_size > 10000){
		printf("Warehouse size is too big. Please try again.\n");
		return -1;
	}
	
	for (int i = 0; i < warehouse_size; i++){
		warehouse[i].is_valid = 0;
		warehouse[i].thread_id = 0;
		strcpy(warehouse[i].attr_name, "");
	}
	
	for (int i = 0; i < 20; i++){
		processes[i] = 0;
	}

	for (int i = 0; i < 20; i++){
		thread_track[i] = 0;
	}
 
	server_fifo1 = open("fifo_server1", O_RDWR);
	client_fifo1 = open("fifo_client1", O_RDWR);
	server_fifo2 = open("fifo_server2", O_RDWR);
	client_fifo2 = open("fifo_client2", O_RDWR);
	server_fifo3 = open("fifo_server3", O_RDWR);
	client_fifo3 = open("fifo_client3", O_RDWR);
	server_fifo4 = open("fifo_server4", O_RDWR);
	client_fifo4 = open("fifo_client4", O_RDWR);
	
	signal(SIGINT, handler1);
	char operation[100] = {0};
	pthread_t incoming_thread = 0;
	char buffer1[100] = {0};
	char buffer2[100] = {0};
	fd_set set1;
	fd_set set2;
	int revtal = 0;
	FD_ZERO(&set1);
	FD_SET(server_fifo1, &set1);
	FD_SET(server_fifo2, &set1);
	FD_SET(server_fifo3, &set1);
	FD_SET(server_fifo4, &set1);

	while(1){
		set2 = set1;
		revtal = select(server_fifo4+1, &set2, NULL, NULL, NULL);

		if (revtal > 0){
			if (FD_ISSET(server_fifo1, &set2)){
				read(server_fifo1, buffer1, 100*sizeof(char));
				sscanf(buffer1, "%s", operation);
				sleep(1);
				red();

				if (strcmp(operation, "Thread") == 0){
					pid_t process1 = 0;
					sscanf(buffer1, "%*s %ld %d", &incoming_thread, &process1);
					printf("Started Session with Client 1. Thread ID is %ld\n", incoming_thread);
					thread_track[0] = incoming_thread;
					thread_track_count++;
					processes[0] = process1;
				}
			
				if (strcmp(operation, "Alloc") == 0){
					printf("Recieved Alloc Message from Client 1!\n");
					for (int i = 0; i < warehouse_size; i++){
						if (warehouse[i].is_valid == 0){
							char buf3 [100] = {0};
							sprintf(buf3, "%d", i);
							write(client_fifo1, buf3, 100*sizeof(char));
							warehouse[i].is_valid = 1;
							warehouse[i].thread_id = thread_track[0];
							printf("Space Allocated at %d for Client 1\n", i);
							break;
						}
					} 	
				}

				if (strcmp(operation, "Dealloc") == 0){
					int dealloc_id = 0;
					printf("Recieved Dealloc Message from Client 1!\n");
					sscanf(buffer1, "%*s %d", &dealloc_id);
					printf("Dealloc ID: %d\n", dealloc_id);
					if (warehouse[dealloc_id].is_valid == 1){
						warehouse[dealloc_id].is_valid = 0;
						strcpy(warehouse[dealloc_id].attr_name, "");
						warehouse[dealloc_id].thread_id = 0;
						printf("%d has been removed from database (Client 1)\n", dealloc_id);
					}
					else{
						fprintf(stderr, "Error in dealloc. Space was not allocated (Client 1)\n");	
					}
				}
	
				if (strcmp(operation, "Read") == 0){
					int read_id = 0;
					char buff3[100] = {0};
					printf("Recieved Read Message from Client 1\n");
					sscanf(buffer1, "%*s %d", &read_id);
					if (warehouse[read_id].is_valid == 1){
						sprintf(buff3, "%s", warehouse[read_id].attr_name);
						printf("Sending value to client 1...\n");
						write(client_fifo1, buff3, 100*sizeof(char));
					}
					else{
						fprintf(stderr, "Read failed due to invalid address (Client 1)\n");
					}
					fflush(stdout);
				}
		
				if (strcmp(operation, "Store") == 0){
					int store_id = 0;
					char buff4[100] = {0};
					char buff5[100] = {0};
					printf("Recieved Store Message from Client 1\n");
					sscanf(buffer1, "%*s %d \"%[^\"]\"", &store_id, buff4);
					if (warehouse[store_id].is_valid == 1){
						strcpy(warehouse[store_id].attr_name, buff4);
						printf("Stored %s from Client 1 at Index %d\n", warehouse[store_id].attr_name, store_id);
						sprintf(buff5, "Store Successful\n");
						write(client_fifo1, buff5, 100*sizeof(char));
					}
		
					else{
						fprintf(stderr, "Error in Storing. Warehouse not allocated (Client 1)\n");
					}
					fflush(stdout);
				}
	
				if (strcmp(operation, "Close") == 0){
					pthread_t thread_to_delete = 0;
					char buff3[100] = {0};
					printf("Recieved Close Message from Client 1!\n");
					sscanf(buffer1, "%*s %ld", &thread_to_delete);
			
					for (int i = 0; i < 10000; i++){
						if (warehouse[i].thread_id == thread_to_delete){
							warehouse[i].thread_id = 0;
							warehouse[i].is_valid = 0;
							strcpy(warehouse[i].attr_name, "");
						}
					}
					thread_track[0] = 0;
				}
				
				if (strcmp(operation, "Exit") == 0){
					pthread_t thread_to_delete = 0;
					char buf6[100] = {0};
					printf("Recieved Exit Message from Client 1!\n");
					sscanf(buffer1, "%*s %ld", &thread_to_delete);				       
	
					for (int i = 0; i < 10000; i++){
                                                 if (warehouse[i].thread_id == thread_to_delete){
                                                         warehouse[i].thread_id = 0;
                                                         warehouse[i].is_valid = 0;
                                                         strcpy(warehouse[i].attr_name, "");
                                                 }
                                         }
                                         thread_track[0] = 0;
					 processes[0] = 0;
	
				}	
			}
			
			if (FD_ISSET(server_fifo2, &set2)){
                                read(server_fifo2, buffer1, 100*sizeof(char));
                                sscanf(buffer1, "%s", operation);
                                sleep(1);
				blue();

                                if (strcmp(operation, "Thread") == 0){
					pid_t process1 = 0;
                                        sscanf(buffer1, "%*s %ld %d", &incoming_thread, &process1);
					printf("Started Session with Client 2. Thread ID is %ld\n", incoming_thread);
                                        thread_track[1] = incoming_thread;
                                        thread_track_count++; 
					processes[1] = process1;
                                }

                                 if (strcmp(operation, "Alloc") == 0){
                                        printf("Recieved Alloc Message from Client 2!\n");
                                        for (int i = 0; i < warehouse_size; i++){
                                                if (warehouse[i].is_valid == 0){
                                                        char buf3 [100];
                                                        sprintf(buf3, "%d", i);
                                                        write(client_fifo2, buf3, 100*sizeof(char));
                                                        warehouse[i].is_valid = 1;
                                                        warehouse[i].thread_id = thread_track[1];
							printf("Space Allocated at %d for Client 2\n", i);
                                                        break;
                                                }
                                        }       
                                }

                                if (strcmp(operation, "Dealloc") == 0){
                                        int dealloc_id = 0;
                                        printf("Recieved Dealloc Message from Client 2!\n");
                                        sscanf(buffer1, "%*s %d", &dealloc_id);
                                        if (warehouse[dealloc_id].is_valid == 1){
                                                warehouse[dealloc_id].is_valid = 0;
                                                printf("%d has been removed from database (Client 2)\n", dealloc_id);
                                        }
                                        else{
                                                fprintf(stderr, "Error in dealloc. Space was not allocated (Client 2)\n"); 
                                        }
                                }

                                if (strcmp(operation, "Read") == 0){
                                        int read_id = 0;
                                        char buff3[100] = {0};
                                        printf("Recieved Read Message from Client 2!\n");
                                        sscanf(buffer1, "%*s %d", &read_id);
                                        if (warehouse[read_id].is_valid == 1){
                                                sprintf(buff3, "%s", warehouse[read_id].attr_name);
                                                printf("Sending value to client 2...\n");
                                                write(client_fifo2, buff3, 100*sizeof(char));
                                        }
                                        else{
                                                fprintf(stderr, "Read failed due to invalid address (Client 2)\n");
                                        }
                                        fflush(stdout);
                                }

                                if (strcmp(operation, "Store") == 0){
                                        int store_id = 0;
                                        char buff4[100] = {0};
					char buff5[100] = {0};
                                        printf("Recieved Store Message from Client 2!\n");
                                        sscanf(buffer1, "%*s %d \"%[^\"]\"", &store_id, buff4);
                                        if (warehouse[store_id].is_valid == 1){
                                                strcpy(warehouse[store_id].attr_name, buff4);
						printf("Stored %s from Client 2 at Index %d\n", warehouse[store_id].attr_name, store_id);
						sprintf(buff5, "Store Successful\n");
						write(client_fifo2, buff5, 100*sizeof(char));
                                        }
                
                                        else{
                                                fprintf(stderr, "Error in Storing. Warehouse not allocated (Client 2)\n");
                                        }
                                        fflush(stdout);
                                }
        
                                if (strcmp(operation, "Close") == 0){
                                        pthread_t thread_to_delete = 0;
                                        char buff3[100] = {0};
                                        printf("Recieved Close Message from Client 2!\n");
                                        sscanf(buffer1, "%*s %ld", &thread_to_delete);
                        
                                        for (int i = 0; i < 10000; i++){
                                                if (warehouse[i].thread_id == thread_to_delete){
                                                        warehouse[i].thread_id = 0;
                                                        warehouse[i].is_valid = 0;
                                                        strcpy(warehouse[i].attr_name, "");
                                                }
                                        }
					thread_track[1] = 0;
                                }
				
				if (strcmp(operation, "Exit") == 0){
                                         pthread_t thread_to_delete = 0;
                                         pid_t process_to_delete = 0;
                                         char buf6[100] = {0};
                                         printf("Recieved Exit Message from Client 2!\n");
                                         sscanf(buffer1, "%*s %ld", &thread_to_delete); 

                                         for (int i = 0; i < 10000; i++){
                                                  if (warehouse[i].thread_id == thread_to_delete){
                                                          warehouse[i].thread_id = 0;
                                                          warehouse[i].is_valid = 0;
                                                          strcpy(warehouse[i].attr_name, "");
                                                  }
                                          }
                                          thread_track[1] = 0;
                                          processes[1] = 0;
 
                                 }
       
                        }
		
			if (FD_ISSET(server_fifo3, &set2)){
                        	read(server_fifo3, buffer1, 100*sizeof(char));
                                sscanf(buffer1, "%s", operation);
                                sleep(1);
				green();

                                if (strcmp(operation, "Thread") == 0){
					pid_t process1 = 0;
                                        sscanf(buffer1, "%*s %ld %d", &incoming_thread, &process1);
					printf("Started Session with Client 3. Thread ID is %ld\n", incoming_thread);
                                        thread_track[2] = incoming_thread;
                                        thread_track_count++; 
					processes[2] = process1;
                                }

                                 if (strcmp(operation, "Alloc") == 0){
                                        printf("Recieved Alloc Message from Client 3!\n");
                                        for (int i = 0; i < warehouse_size; i++){
                                                if (warehouse[i].is_valid == 0){
                                                        char buf3 [100];
                                                        sprintf(buf3, "%d", i);
                                                        write(client_fifo3, buf3, 100*sizeof(char));
                                                        warehouse[i].is_valid = 1;
                                                        warehouse[i].thread_id = thread_track[2];
							printf("Space Allocated at %d for Client 3\n", i);
                                                        break;
                                                }
                                        }       
                                }

                                if (strcmp(operation, "Dealloc") == 0){
                                        int dealloc_id = 0;
                                        printf("Recieved Dealloc Message from Client 3!\n");
                                        sscanf(buffer1, "%*s %d", &dealloc_id);
                                        if (warehouse[dealloc_id].is_valid == 1){
                                                warehouse[dealloc_id].is_valid = 0;
                                                printf("%d has been removed from database (Client 3)\n", dealloc_id);
                                        }
                                        else{
                                                fprintf(stderr, "Error in dealloc. Space was not allocated (Client 3)\n"); 
                                        }
                                }

                                if (strcmp(operation, "Read") == 0){
                                        int read_id = 0;
                                        char buff3[100] = {0};
                                        printf("Recieved Read Message from Client 3!\n");
                                        sscanf(buffer1, "%*s %d", &read_id);
                                        if (warehouse[read_id].is_valid == 1){
                                                sprintf(buff3, "%s", warehouse[read_id].attr_name);
                                                printf("Sending value to client 3...\n");
                                                write(client_fifo3, buff3, 100*sizeof(char));
                                        }
                                        else{
                                                fprintf(stderr, "Read failed due to invalid address (Client 3)\n");
                                        }
                                        fflush(stdout);
                                }

                                if (strcmp(operation, "Store") == 0){
                                        int store_id = 0;
                                        char buff4[100] = {0};
					char buff5[100] = {0};
                                        printf("Recieved Store Message from Client 3!\n");
                                        sscanf(buffer1, "%*s %d \"%[^\"]\"", &store_id, buff4);
                                        if (warehouse[store_id].is_valid == 1){
                                                strcpy(warehouse[store_id].attr_name, buff4);
						printf("Stored %s from Client 3 at Index %d\n", warehouse[store_id].attr_name, store_id);
						sprintf(buff5, "Store Successful\n");
						write(client_fifo3, buff5, 100*sizeof(char));
                                        }
                
                                        else{
                                                fprintf(stderr, "Error in Storing. Warehouse not allocated (Client 3)\n");
                                        }
                                        fflush(stdout);
                                }
        
                                if (strcmp(operation, "Close") == 0){
                                        pthread_t thread_to_delete = 0;
                                        char buff3[100] = {0};
                                        printf("Recieved Close Message from Client 3!\n");
                                        sscanf(buffer1, "%*s %ld", &thread_to_delete);
                        
                                        for (int i = 0; i < 10000; i++){
                                                if (warehouse[i].thread_id == thread_to_delete){
                                                        warehouse[i].thread_id = 0;
                                                        warehouse[i].is_valid = 0;
                                                        strcpy(warehouse[i].attr_name, "");
                                                }
                                        }
					thread_track[2] = 0;
                                }
				
				if (strcmp(operation, "Exit") == 0){
                                         pthread_t thread_to_delete = 0;
                                         char buf6[100];
                                         printf("Recieved Exit Message from Client 1!\n");
                                	 sscanf(buffer1, "%*s %ld", &thread_to_delete); 
 
                                         for (int i = 0; i < 10000; i++){
                                                  if (warehouse[i].thread_id == thread_to_delete){
                                                          warehouse[i].thread_id = 0;
                                                          warehouse[i].is_valid = 0;
                                                          strcpy(warehouse[i].attr_name, "");
                                                  }
                                          }
                                          thread_track[2] = 0;
                                          processes[2] = 0;
 
                                 }
       
                        }
			if (FD_ISSET(server_fifo4, &set2)){
                                read(server_fifo4, buffer1, 100*sizeof(char));
                                sscanf(buffer1, "%s", operation);
                                sleep(1);
				yellow();

                                if (strcmp(operation, "Thread") == 0){
					pid_t process1 = 0;
                                        sscanf(buffer1, "%*s %ld %d", &incoming_thread, &process1);
                                        printf("Started Session with Client 4. Thread ID is %ld\n", incoming_thread);
                                        thread_track[3] = incoming_thread;
                                        thread_track_count++;
					processes[3] = process1;
                                }
                        
                                if (strcmp(operation, "Alloc") == 0){
                                        printf("Recieved Alloc Message from Client 4!\n");
                                        for (int i = 0; i < warehouse_size; i++){
                                                if (warehouse[i].is_valid == 0){
                                                        char buf3 [100] = {0};
                                                        sprintf(buf3, "%d", i);
                                                        write(client_fifo4, buf3, 100*sizeof(char));
                                                        warehouse[i].is_valid = 1;
                                                        warehouse[i].thread_id = thread_track[3];
                                                        printf("Space Allocated at %d for Client 4\n", i);
                                                        break;
                                                }
                                        }       
                                }

                                if (strcmp(operation, "Dealloc") == 0){
                                        int dealloc_id = 0;
                                        printf("Recieved Dealloc Message from Client 4!\n");
                                        sscanf(buffer1, "%*s %d", &dealloc_id);
                                        if (warehouse[dealloc_id].is_valid == 1){
                                                warehouse[dealloc_id].is_valid = 0;
                                                printf("%d has been removed from database (Client 4)\n", dealloc_id);
                                        }
                                        else{
                                                fprintf(stderr, "Error in dealloc. Space was not allocated (Client 4)\n");      
                                        }
                                }

                                  if (strcmp(operation, "Read") == 0){
                                        int read_id = 0;
                                        char buff3[100] = {0};
                                        printf("Recieved Read Message from Client 4\n");
                                        sscanf(buffer1, "%*s %d", &read_id);
                                        if (warehouse[read_id].is_valid == 1){
                                                sprintf(buff3, "%s", warehouse[read_id].attr_name);
                                                printf("Sending value to client 4...\n");
                                                write(client_fifo4, buff3, 100*sizeof(char));
                                        }
                                        else{
                                                fprintf(stderr, "Read failed due to invalid address (Client 4)\n");
                                        }
                                        fflush(stdout);
                                }

                                 if (strcmp(operation, "Store") == 0){
                                        int store_id = 0;
                                        char buff4[100] = {0};
                                        char buff5[100] = {0};
                                        printf("Recieved Store Message from Client 4\n");
                                        sscanf(buffer1, "%*s %d \"%[^\"]\"", &store_id, buff4);
                                        if (warehouse[store_id].is_valid == 1){
                                                strcpy(warehouse[store_id].attr_name, buff4);
                                                printf("Stored %s from Client 4 at Index %d\n", warehouse[store_id].attr_name, store_id);
                                                sprintf(buff5, "Store Successful\n");
                                                write(client_fifo4, buff5, 100*sizeof(char));
                                        }
                
                                        else{
                                                fprintf(stderr, "Error in Storing. Warehouse not allocated (Client 4)\n");
                                        }
                                        fflush(stdout);
                                }
        
                                if (strcmp(operation, "Close") == 0){
                                        pthread_t thread_to_delete = 0;
                                        char buff3[100] = {0};
                                        printf("Recieved Close Message from Client 4!\n");
                                        sscanf(buffer1, "%*s %ld", &thread_to_delete);
                        
                                        for (int i = 0; i < 10000; i++){
                                                if (warehouse[i].thread_id == thread_to_delete){
                                                        warehouse[i].thread_id = 0;
                                                        warehouse[i].is_valid = 0;
                                                        strcpy(warehouse[i].attr_name, "");
                                                }
                                        }
                                        thread_track[3] = 0;
                                }

				if (strcmp(operation, "Exit") == 0){
                                         pthread_t thread_to_delete = 0;
                                         char buf6[100] = {0};
                                         printf("Recieved Exit Message from Client 1!\n");
                                	 sscanf(buffer1, "%*s %ld", &thread_to_delete);  
                                         for (int i = 0; i < 10000; i++){
                                                  if (warehouse[i].thread_id == thread_to_delete){
                                                          warehouse[i].thread_id = 0;
                                                          warehouse[i].is_valid = 0;
                                                          strcpy(warehouse[i].attr_name, "");
                                                  }
                                          }
                                          thread_track[3] = 0;
                                          processes[3] = 0;
 
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
	reset();

	do{
		sleep(1);
		printf("\n");
		printf("shell> ");
		fgets(input_line, 255, stdin);
		strtok(input_line, "\n");
		
		strcpy(input_find, input_line);	
		found = strtok(input_find, " ");
		array[0] = strdup(found);
		array[1] = NULL;
		array[0] = strlwr(array[0]);


		if (strcmp(array[0], "list") == 0){
			pthread_t temp_id;
			int temp_var;
			temp_var = sscanf(input_line, "%*s %ld", &temp_id);
			if (temp_var == -1){
				printf("Client\tID\n");
				for (int i = 0; i < thread_track_count; i++){
					if (thread_track[i] > 0){
						printf("%d\t", i+1);
						printf("%ld\n", thread_track[i]);
					}
				}
			}

			else{
				printf("List of Entries by Client %ld\n", temp_id);
				printf("ID\t\t\tAttribute Name\t\tIs Valid\n");
				for (int i = 0; i < warehouse_size; i++){
					if (warehouse[i].thread_id == temp_id && warehouse[i].is_valid == 1){
						printf("%ld\t\t", warehouse[i].thread_id);
						printf("%s\t\t", warehouse[i].attr_name);
						printf("%d\n", warehouse[i].is_valid);
					}
				}				
			}
			 fflush(stdout);
			free(array[0]);
			break;
		}

		if (strcmp(array[0], "dump") == 0){
			for (int i = 0; i < warehouse_size; i++){
				printf("Index: %d\t", i);
				printf("Warehouse Record ID: %ld\t", warehouse[i].thread_id);
				printf("Warehouse Record Name: %s\t", warehouse[i].attr_name);
				printf("Warehouse Record Valid: %d\n", warehouse[i].is_valid);
			}
			free(array[0]);
			break;

		}
	
		if (strcmp(array[0], "exit") == 0){
			if (processes[0] > 0){
				kill(processes[0], SIGTERM); 
			}
			if (processes[1] > 0){
				kill(processes[1], SIGTERM);
			}
			if (processes[2] > 0){
				kill(processes[2], SIGTERM);
			}
			if (processes[3] > 0){
				kill(processes[3], SIGTERM);
			}
			close(server_fifo1);
			close(client_fifo1);
			close(server_fifo2);
			close(client_fifo2);
			close(server_fifo3);
			close(client_fifo3);
			close(server_fifo4);
			close(client_fifo4);
			free(array[0]);
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

void red () {
	fprintf(stderr, "\033[1;31m");
	printf("\033[1;31m");
}

void yellow () {
	fprintf(stderr, "\033[1;33m");
	printf("\033[1;33m");
}

void green(){
	fprintf(stderr, "\033[1;32m");
	printf("\033[1;32m");
}

void blue(){
	fprintf(stderr, "\033[1;34m");
	printf("\033[1;34m");
}

void reset () {
	fprintf(stderr, "\033[0m");
	printf("\033[0m");
}
