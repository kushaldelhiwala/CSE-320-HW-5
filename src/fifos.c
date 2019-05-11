#include <stdio.h> 
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(int argc, char** argv){
	int fd1;
	int fd2; 
	int fd3;
	int fd4;
	int fd5;
	int fd6;
	int fd7;
	int fd8;
	
	fd1 = mkfifo("fifo_client1", 0777); 
	fd2 = mkfifo("fifo_server1", 0777);
	fd3 = mkfifo("fifo_client2", 0777);
	fd4 = mkfifo("fifo_server2", 0777);
	fd5 = mkfifo("fifo_client3", 0777);
	fd6 = mkfifo("fifo_server3", 0777);
	fd7 = mkfifo("fifo_client4", 0777);
	fd8 = mkfifo("fifo_server4", 0777);

	if (fd1 < 0 || fd3 < 0 || fd5 < 0 || fd7 < 0){
		fprintf(stderr, "Error in creating client fifo's\n");
		exit(-1);
	}

	if (fd2 < 0 || fd4 < 0 || fd6 < 0 || fd8 < 0){
		fprintf(stderr, "Error in creating server fifo's\n");
		exit(-1);
	}

	printf("Successful\n");
	return 0;

}
