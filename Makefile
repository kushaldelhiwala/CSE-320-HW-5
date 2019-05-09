all: client server fifos

client:
	gcc src/client.c -o client -lpthread

server:
	gcc src/warehouse_db.c -o server -lpthread 

fifos: 
	gcc src/fifos.c -o fifos -lpthread 

clean:
	rm client server fifos fifo_client1 fifo_server1 
