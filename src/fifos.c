#include <stdio.h> 
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(int argc, char** argv){
	int fd1;
	int fd2; 
	
	fd1 = mkfifo("fifo_client1", 0777); 
	fd2 = mkfifo("fifo_server1", 0777);

	if (fd1 < 0 || fd2 < 0){
		fprintf(stderr, "Error in creating client fifo's\n");
		exit(-1);
	}

	printf("Successful\n");
	return 0;

}
