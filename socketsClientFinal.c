#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

int const myRouterNumber = 0;
int otherRouterNumber;
int const numberOfNodes = 4;//Number of neighboring routers
int buffer[4];

//initialize global connection variables
int returnVal;
//for connection to router 1
int socketFD1;
struct sockaddr_in serv_addr1;
struct hostent *server1;
//for connection to router 2
int socketFD2;
struct sockaddr_in serv_addr2;
struct hostent *server2;
//for connection to router 3
int socketFD3;
struct sockaddr_in serv_addr3;
struct hostent *server3;

//initialize table values
//infinity = 1000000000
int DistanceVectors[][4] = {
	{0, 1, 3, 7}, //myLinkCost
	{1000000000, 0, 1000000000, 1000000000},
	{1000000004, 1000000005, 0, 1000000006},
	{1000000007, 1000000008, 1000000009, 0}
};

int arraySame(int a[], int b[]){
	int i;
	for (i=0;i<numberOfNodes;i++){
		if (a[i] != b[i]){
			return 0;
		}
	}
	return 1;
}

int copyArray(int src[], int dest[]){
	int i;
	for (i=0; i<numberOfNodes; i++){
		dest[i] = src[i];
	}
}

int printIntArray(int arr[]){
	int i;
	for (i=0; i < numberOfNodes; i++){
		printf("%d\n", arr[i]);
	}
}

int findMin(int arr[]){
	int min = 1000000000;
	int i;
	for(i=0; i<numberOfNodes; i++){
		if(arr[i] < min){
			min = arr[i];
		}
	}
	return min;
}

int bellmanFordEquation(int srcRouterNum,  int destRouterNum){

	int minArray[numberOfNodes];

	int i;
	for (i=0; i<numberOfNodes; i++){
			int Cxy = DistanceVectors[srcRouterNum][i];
			int Dxy = DistanceVectors[i][destRouterNum];

			minArray[i] = Cxy + Dxy;
	}
	int min = findMin(minArray);

	return min;
}

void initializeConnection(char *host, int port, int *socketFD, struct hostent *server, struct sockaddr_in *serv_addr){
	*socketFD = socket(AF_INET, SOCK_STREAM, 0);
	//printf("after socket()\n");
	if (*socketFD < 0) {
		printf("Failed to open socket!\n");
		exit(0);
	}
	server = gethostbyname(host);
	if (server == NULL) {
        printf("Error, cannot find host %s\n", host);
        exit(0);
    }
    //printf("after server\n");
    bzero((char *) serv_addr, sizeof(*serv_addr));
    //printf("after bzero\n");
    //set fields in serv_addr struct
    serv_addr->sin_family = AF_INET;
    //printf("after AF_INET\n");
    bcopy( (char *)server->h_addr, (char *) &serv_addr->sin_addr.s_addr, server->h_length);
    //printf("after bcopy\n");
    serv_addr->sin_port = htons(port);


    //Try to connect
    int connectReturnVal = connect(*socketFD,serv_addr,sizeof(*serv_addr));

    if (connectReturnVal < 0){
    	printf("Error! Couldn't connect to server! Exiting with error code %d\n", connectReturnVal);
        exit(0);
    }
    else{
    	printf("Successfully connected to %s on port %d\n", host, port);
    }
}

void sendRouterNumber(int socketN, int routerNum){
	returnVal = write(socketN, &routerNum, 4);
	 if (returnVal < 0){
		printf("Error sending Router Number to socket! Exiting...\n");
		exit(0);
	}
	else{
		//printf("Successfully sent Router Number: %d!\n", myRouterNumber);
		//printf("Sent %d bytes\n", returnVal);
	}
}

void sendDistanceVector(int socketN){
	returnVal = write(socketN, DistanceVectors[myRouterNumber], 16);//255
    if (returnVal < 0){
		printf("Error sending Distance Vector to socket! Exiting...\n");
		exit(0);
	}
	else{
		//printf("Successfully sent Distance Vector!\n");
		//printf("Sent %d bytes\n", returnVal);
	}
	//printf("Awaiting reply...\n");
}

void readRouterNumber(int socketN){
	returnVal = read(socketN, &otherRouterNumber, 4);
	if (returnVal < 0){
		printf("Error recieving Router Number from socket! Exiting...\n");
		exit(0);
	}
	else{
		//printf("Successfully got Router Number from router %d\n", otherRouterNumber);
		//printf("Read %d bytes\n", returnVal);
	}
}

void readDistanceVector(int socketN, int savedRNum){
	returnVal = read(socketN, buffer, 16);
    //printf("After Read for DV... routerNum: %d\n", savedRNum);
    otherRouterNumber = savedRNum;//save correct router number value to variable
    if (returnVal < 0){
		printf("Error reading from socket! Exiting...\n");
		exit(0);
	}
	else{
		//printf("Successfully got Distance Vector from router %d!\n", otherRouterNumber);
		//printf("Read %d bytes\n", returnVal);
	}
}


int main(int argc, char *argv[]){

	if (argc < 3){
		printf("Need a host name as argument 1 and port number as argument 2.");
		exit(0);
	}

	char *hostName = argv[1];
	int portNumber = atoi(argv[2]);
	

	printf("hostName: %s, portNumber: %d\n", hostName, portNumber);

	//FINAL EDIT POINT*
	//change "hostName"s to respective ip addresses
	
	//connect to router 1
	initializeConnection(hostName, portNumber, &socketFD1, server1, &serv_addr1);
	//connect to router 2
	initializeConnection(hostName, (portNumber+1), &socketFD2, server2, &serv_addr2);
	//connecto to router 3
	initializeConnection(hostName, (portNumber+2), &socketFD3, server3, &serv_addr3);
    //Successfully connected!!!
    //printf("Sleeping for 7 seconds\n");
    //printf("socketFD1: %d, socketFD2: %d, socketFD3: %d\n", socketFD1, socketFD2, socketFD3);
	//sleep(7);

	int dvUpdated = 1;
	int count = 0;

	while(dvUpdated){

		printf("===========================Loop %d===========================\n", count);
		count++;
		//exchange with router 1
    	sendRouterNumber(socketFD1, myRouterNumber);
		sendDistanceVector(socketFD1);
		readRouterNumber(socketFD1);
		//printf("socketFD1: %d, socketFD2: %d, socketFD3: %d\n", socketFD1, socketFD2, socketFD3);
		//save value from being overwritten
		int routerNum = otherRouterNumber;
		readDistanceVector(socketFD1, routerNum);
		//copies recieved array (distance vector) to this router's table
		copyArray(buffer, DistanceVectors[1]);//otherRouterNumber

//*/
		//exchange with router 2
    	sendRouterNumber(socketFD2, myRouterNumber);
		sendDistanceVector(socketFD2);
		readRouterNumber(socketFD2);
		//save value from being overwritten
		int routerNum2 = otherRouterNumber;
		readDistanceVector(socketFD2, routerNum2);
		//copies recieved array (distance vector) to this router's table
		copyArray(buffer, DistanceVectors[2]);//otherRouterNumber

	
		//exchange with router 3
    	sendRouterNumber(socketFD3, myRouterNumber);
		sendDistanceVector(socketFD3);
		readRouterNumber(socketFD3);
		//save value from being overwritten
		int routerNum3 = otherRouterNumber;
		readDistanceVector(socketFD3, routerNum3);
		//copies recieved array (distance vector) to this router's table
		copyArray(buffer, DistanceVectors[3]);//otherRouterNumber*/

		printf("Printing my Distance Vector Table Before Bellman Ford Update...\n");
		printIntArray(DistanceVectors[myRouterNumber]);
	
		//printf("Router %d's Distance Vector before updating\n", myRouterNumber);
		//printIntArray(DistanceVectors[myRouterNumber]);

		//perform bellman ford update
		printf("Updating Distance Vector using bellman ford...\n");

		int temp[4];
		copyArray(DistanceVectors[myRouterNumber], temp);

		//loop used to perform bellman ford for entire table
		int i;
		for(i=numberOfNodes; i>=0; i--){
			if(DistanceVectors[myRouterNumber][i] != myRouterNumber){//cannot beat 0
				//updates this routers DV
				DistanceVectors[myRouterNumber][i] = bellmanFordEquation(myRouterNumber, i);//least cost from myRouterNumber to i
			}
		}


		//printf("Printing Router %d's Distance Vector after update\n", myRouterNumber);
		//printIntArray(DistanceVectors[myRouterNumber]);
		printf("Printing Updated Distance Vector...\n");
		printIntArray(DistanceVectors[myRouterNumber]);

		//check if table updated or same
		int result = arraySame(temp, DistanceVectors[myRouterNumber]);

		//if updated, sent updated dv[] to to all other servers and repeat

		if(result){//if not updated (no change)
			dvUpdated = 0;
			//printf("Arrays are the same! result =  %d\n", result);
			printf("Distance Vector was NOT updated...\n");
		}
		else{//if any change
			dvUpdated = 1;
			//printf("Arrays are not the same... result = %d\n", result);
			printf("Distance Vector was updated!\n");
		}

		//dvUpdated = 0;
	}//end while

	printf("Closing Connection...\n");

	return 0;
}