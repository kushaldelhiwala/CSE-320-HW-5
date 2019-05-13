# HW 5
Good luck and don't cheat!

Edit by Kushal: 

Welcome to my HW 5! To properly run the program(s), please follow the appropriate steps, and pay attention to the various details stated. 

The total HW consists of 3 files: fifos.c, client.c, and server.c. All 3 files must be ran at some point to have proper execution of the program. 

1) Please proceed to the main directory (kdelhiwala@kushal:~/cse320/hw5-kdelhiwala$ ).
2) Once you are at the main directory, please type "make", and press enter. When you hit make, you are compiling all three of the files. 
3) The first file you want to run is "fifos". To do this, please type ./fifos. You need to run fifos first to create the fifos for the client + server communication. 
4) Next, please type ./client N. N is a number from (1-4). This will determine as to which client you are using. As per the scope of this HW, we can have up to four clients. 
	a) If you would like to add additional clients, please open another tab in your terminal, and do ./client N again, this time with a different number.
5) Once you have opened your clients, you now need to run server. Open another tab in your terminal, and type in ./server N, where N is the size of your database. N can be up to size 10,000. 
6) Now, at this point, you will have your clients + server open. The client side should have the shell open, and this shell will take various commands.
	a) start -> This starts a connection with the server 
	b) alloc -> This creates an art entry in the DB where you can store a record
	c) dealloc (ID) -> This function takes in a local ID along with "dealloc". It should remove the entry from the list of entries on the client side, and mark the record as invalid on the server
			   side. NOTE: Please enter this ID as a base 10 numerical.
	d) read (ID) -> This will read what value is stored at the local id (ID). Again, please enter this ID as a base 10 numerical. 
	e) store (ID) (Z) -> This will store some record (Z) at the id specified (ID). NOTE: please input the record between quotation marks " ".
	f) close -> This will close the current session with the server. You can start a new one by clicking "start". 
	g) exit -> This will terminate the client
7) While working with the client, you will notice that there are messages being sent to the server as well (via PIPE). On the server side there is also a Client, which you can access with ctrl+c
8) On the server side, the shell will take four commands.
	a) list -> This will list out all the client thread_id's
	b) list ID -> This will take in an ID, where ID is the TID of a specific client. When you do this, you will see all the art entries stored by this particular client. 
	c) dump -> This prints all the information about all the clients, the things they store, etc. 
	d) exit -> This allows you to exit the server. 

In general, you will be able to run the program if you follow the steps above. Below I will list some general caveats about the program you should know.


1) On the server side, we will be printing messages from all 4 clients. Client 1 messages will print in red, Client 2 in blue, Client 3 in green, Client 4 in yellow.
2) Please ensure the commands are entered in lowercase. 
3)   
