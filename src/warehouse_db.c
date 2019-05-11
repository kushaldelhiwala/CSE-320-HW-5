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
void handler1(int sig);
char *strlwr(char *str);
int total_collections;
warehouse_struct warehouse[10000];
int warehouse_size; 

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
		exit(1);
	}
 
	int server_fifo1 = open("fifo_server1", O_RDWR);
	int client_fifo1 = open("fifo_client1", O_RDWR);
	int server_fifo2 = open("fifo_server2", O_RDWR);
	int client_fifo2 = open("fifo_client2", O_RDWR);
	int server_fifo3 = open("fifo_server3", O_RDWR);
	int client_fifo3 = open("fifo_client3", O_RDWR);
	//int server_fifo4 = open("fifo_server4", O_RDWR);
	//int client_fifo4 = open("fifo_client4", O_RDWR);
	
	signal(SIGINT, handler1);
	char operation[100];
	pthread_t incoming_thread;
	char buffer1[100];
	char buffer2[100];
	fd_set set1;
	fd_set set2;
	int revtal;
	FD_ZERO(&set1);
	FD_SET(server_fifo1, &set1);
	FD_SET(server_fifo2, &set1);
	FD_SET(server_fifo3, &set1);
	//FD_SET(server_fifo4, &set1);

	while(1){
		set2 = set1;
		revtal = select(server_fifo3+1, &set2, NULL, NULL, NULL);
		if (revtal > 0){
			if (FD_ISSET(server_fifo1, &set2)){
				read(server_fifo1, buffer1, 100*sizeof(char));
				printf("%s\n", buffer1);
				sscanf(buffer1, "%s", operation);
				sleep(1);

				if (strcmp(operation, "Thread") == 0){
					sscanf(buffer1, "%*s %ld", &incoming_thread);
					printf("PIPE reading thread. Thread ID is %ld from Client 1\n", incoming_thread);
					thread_track[thread_track_count] = incoming_thread;
					thread_track_count++;		
				}
			
				if (strcmp(operation, "Alloc") == 0){
					printf("Recieved Alloc Message from Client 1!\n");
					for (int i = 0; i < warehouse_size; i++){
						if (warehouse[i].is_valid == 0){
							char buf3 [100];
							sprintf(buf3, "%d", i);
							printf("Writing this to client 1...%s\n", buf3);
							write(client_fifo1, buf3, 100*sizeof(char));
							warehouse[i].is_valid = 1;
							warehouse[i].thread_id = thread_track[0];
							printf("Warehouse Thread ID: %ld (Client 1)\n", warehouse[i].thread_id);
							break;
						}
					} 	
				}

				if (strcmp(operation, "Dealloc") == 0){
					int dealloc_id;
					printf("Recieved Dealloc Message from Client 1!\n");
					sscanf(buffer1, "%*s %d", &dealloc_id);
					if (warehouse[dealloc_id].is_valid == 1){
						warehouse[dealloc_id].is_valid = 0;
						printf("%d has been removed from database (Client 1)\n", dealloc_id);
					}
					else{
						fprintf(stderr, "Error in dealloc. Space was not allocated (Client 1)\n");	
					}
				}
	
				if (strcmp(operation, "Read") == 0){
					int read_id;
					char buff3[100];
					printf("Recieved Read Message! (Client 1)\n");
					sscanf(buffer1, "%*s %d", &read_id);
					if (warehouse[read_id].is_valid == 1){
						sprintf(buff3, "%s", warehouse[read_id].attr_name);
						printf("Writing to client 1...\n");
						write(client_fifo1, buff3, 100*sizeof(char));
					}
					else{
						fprintf(stderr, "Read failed due to invalid address (Client 1)\n");
					}
					fflush(stdout);
				}
		
				if (strcmp(operation, "Store") == 0){
					int store_id; 
					char buff4[100];
					printf("Recieved Store Message! (Client 1)\n");
					sscanf(buffer1, "%*s %d \"%[^\"]\"", &store_id, buff4);
					printf("Store ID: %d\n", store_id);
					printf("Buffer: %s\n", buff4);
					if (warehouse[store_id].is_valid == 1){
						strcpy(warehouse[store_id].attr_name, buff4);
						printf("Store Complete!...stored %s Client 1\n", warehouse[store_id].attr_name);
					}
		
					else{
						fprintf(stderr, "Error in Storing. Warehouse not allocated Client 1\n");
					}
					fflush(stdout);
				}
	
				if (strcmp(operation, "Close") == 0){
					pthread_t thread_to_delete;
					char buff3[100];
					printf("Recieved Close Message Client 1!\n");
					sscanf(buffer1, "%*s %ld", &thread_to_delete);
			
					for (int i = 0; i < 10000; i++){
						if (warehouse[i].thread_id == thread_to_delete){
							warehouse[i].thread_id = 0;
							warehouse[i].is_valid = 0;
							strcpy(warehouse[i].attr_name, "EMPTY");
						}
					}
				}	
			}
			
			if (FD_ISSET(server_fifo2, &set2)){
                                read(server_fifo2, buffer1, 100*sizeof(char));
                                sscanf(buffer1, "%s", operation);
                                sleep(1);

                                if (strcmp(operation, "Thread") == 0){
                                        sscanf(buffer1, "%*s %ld", &incoming_thread);
                                        printf("PIPE reading thread. Thread ID is %ld Client 2\n", incoming_thread);
                                        thread_track[thread_track_count] = incoming_thread;
                                        thread_track_count++;           
                                }

                                 if (strcmp(operation, "Alloc") == 0){
                                        printf("Recieved Alloc Message from Client 2!\n");
                                        for (int i = 0; i < warehouse_size; i++){
                                                if (warehouse[i].is_valid == 0){
                                                        char buf3 [100];
                                                        sprintf(buf3, "%d", i);
                                                        printf("Writing this to client 2...%s\n", buf3);
                                                        write(client_fifo2, buf3, 100*sizeof(char));
                                                        warehouse[i].is_valid = 1;
                                                        warehouse[i].thread_id = thread_track[1];
                                                        printf("Warehouse Thread ID: %ld\n", warehouse[i].thread_id);
                                                        break;
                                                }
                                        }       
                                }

                                if (strcmp(operation, "Dealloc") == 0){
                                        int dealloc_id;
                                        printf("Recieved Dealloc Message from Client 2!\n");
                                        sscanf(buffer1, "%*s %d", &dealloc_id);
                                        if (warehouse[dealloc_id].is_valid == 1){
                                                warehouse[dealloc_id].is_valid = 0;
                                                printf("%d has been removed from database\n", dealloc_id);
                                        }
                                        else{
                                                fprintf(stderr, "Error in dealloc. Space was not allocated\n"); 
                                        }
                                }

                                if (strcmp(operation, "Read") == 0){
                                        int read_id;
                                        char buff3[100];
                                        printf("Recieved Read Message from Client 2!\n");
                                        sscanf(buffer1, "%*s %d", &read_id);
                                        if (warehouse[read_id].is_valid == 1){
                                                sprintf(buff3, "%s", warehouse[read_id].attr_name);
                                                printf("Writing to client...\n");
                                                write(client_fifo2, buff3, 100*sizeof(char));
                                        }
                                        else{
                                                fprintf(stderr, "Read failed due to invalid address\n");
                                        }
                                        fflush(stdout);
                                }

                                if (strcmp(operation, "Store") == 0){
                                        int store_id; 
                                        char buff4[100];
                                        printf("Recieved Store Message from Client 2!\n");
                                        sscanf(buffer1, "%*s %d \"%[^\"]\"", &store_id, buff4);
                                        printf("Store ID: %d\n", store_id);
                                        printf("Buffer: %s\n", buff4);
                                        if (warehouse[store_id].is_valid == 1){
                                                strcpy(warehouse[store_id].attr_name, buff4);
                                                printf("Store Complete!...stored %s\n", warehouse[store_id].attr_name);
                                        }
                
                                        else{
                                                fprintf(stderr, "Error in Storing. Warehouse not allocated\n");
                                        }
                                        fflush(stdout);
                                }
        
                                if (strcmp(operation, "Close") == 0){
                                        pthread_t thread_to_delete;
                                        char buff3[100];
                                        printf("Recieved Close Message from Client 2!\n");
                                        sscanf(buffer1, "%*s %ld", &thread_to_delete);
                        
                                        for (int i = 0; i < 10000; i++){
                                                if (warehouse[i].thread_id == thread_to_delete){
                                                        warehouse[i].thread_id = 0;
                                                        warehouse[i].is_valid = 0;
                                                        strcpy(warehouse[i].attr_name, "EMPTY");
                                                }
                                        }
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
			printf("Temp Var: %d\n", temp_var);
			if (temp_var == -1){
				printf("Client\tID\n");
				for (int i = 0; i < thread_track_count; i++){
					printf("%d\t", i+1);
					printf("%ld\n", thread_track[i]);
				}	
			}

			else{
				printf("List of Entries by Client %ld\n", temp_id);
				for (int i = 0; i < 10000; i++){
					if (warehouse[i].thread_id == temp_id && warehouse[i].is_valid == 1){
						printf("%s\n", warehouse[i].attr_name);
					}
				}				
			}
			break;
		}

		if (strcmp(array[0], "dump") == 0){
			for (int i = 0; i < warehouse_size; i++){
				printf("Warehouse Record ID: %ld\t", warehouse[i].thread_id);
				printf("Warehouse Record Name: %s\t", warehouse[i].attr_name);
				printf("Warehouse Record Valid: %d\n", warehouse[i].is_valid);
			}
			break;

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
