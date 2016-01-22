#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int const myRouterNumber = 2;
int otherRouterNumber;
int const numberOfNodes = 4;//Number of neighboring routers
int buffer[4];
int routerNum;

//initialize table values
//infinity = 1000000000
int DistanceVectors[][4] = {
	{0, 1000000000, 1000000000, 1000000000},
	{1000000000, 0, 1000000000, 1000000000},
	{3, 1, 0, 2},//myLinkCost
	{1000000000, 1000000000, 1000000000, 0}
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
			int Cxy = DistanceVectors[srcRouterNum][i];//From N to my node[1,2,3, or 4]
			int Dxy = DistanceVectors[i][destRouterNum];//From my node[1,2,3, or 4]

			minArray[i] = Cxy + Dxy;
	}
	int min = findMin(minArray);

	return min;
}

void readRouterNumber(int newSocketN){
	int rVal = read(newSocketN, &otherRouterNumber, 4);
	if (rVal < 0){
		//printf("Error recieving Router Number from socket! Exiting...\n");
		printf("Finished Reading, nothing else to read! Exiting\n");
		exit(0);
	}
	else{
		printf("Successfully got Router Number from router %d\n", otherRouterNumber);
	}
}

void readDistanceVector(int newSocketN){
	int rVal = read(newSocketN, buffer, 16);
	otherRouterNumber = routerNum;
	if (rVal < 0){
		printf("Error recieving Distance Vector from socket! Exiting...\n");
		exit(0);
	}
	else{
		printf("Successfully got Distance Vector from  router %d\n", otherRouterNumber);
	}
}

void sendRouterNumber(int newSocketN){
	int rVal = write(newSocketN, &myRouterNumber, 4);
	if (rVal < 0){
		printf("Error sending Router Number to socket! Exiting...\n");
		exit(0);
	}
	else{
		printf("Successfully sent Router Number to router %d!\n", otherRouterNumber);
	}
}

void sendDistanceVector(int newSocketN){
	int rVal = write(newSocketN, DistanceVectors[myRouterNumber], 16);
	if (rVal < 0){
		printf("Error writing to socket! Exiting...\n");
		exit(0);
	}
	else{
		printf("Successfully sent Distance Vector!\n");
	}
}

int main(int argc, char *argv[]){

	if (argc < 2){
		printf("Need to include a port number as a single argument, the server will listen on that port number.");
		exit(0);
	}

	printf("Starting Server...\n");

	int portNumber = atoi(argv[1]);

	int socketFD, newSocketFD, ClientAddrSize, returnVal;
	struct sockaddr_in serv_addr, cli_addr;//*/
	

	//begin listener
	socketFD = socket(AF_INET, SOCK_STREAM, 0);

	if (socketFD < 0) {
		printf("Failed to open socket!\n");
		exit(0);
	}
	//set all values in serv_addr struct to 0s
	bzero( (char*) &serv_addr, sizeof(serv_addr));

	//set values in serv_addr sockaddr struct
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portNumber);//convert port# to network byte order
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	//bind socket to an address
	int bindReturnVal = bind(socketFD, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

	if (bindReturnVal < 0){
		printf("Failed to Bind! Exiting...\n");
		exit(0);
	}

	listen(socketFD, 5);//5 is max # of 
	//end listener*/
	//initializeListener(portNumber);

	printf("Running Server...\n");





	//After connection request comes in
	ClientAddrSize = sizeof(cli_addr);
	//must make a new socket for the new incoming connection
	newSocketFD = accept(socketFD, (struct sockaddr *) &cli_addr, &ClientAddrSize);

	if (newSocketFD < 0){
		printf("Failed to accept connection! Exiting...\n");
		exit(0);
	}
	else{
		printf("Found incoming connection!\n");
	}

	//printf("Sleeping for 7 seconds\n");
	//sleep(7);

	printf("Printing Router %d's Distance Vector Before Reading\n", myRouterNumber);
	printIntArray(DistanceVectors[myRouterNumber]);

	int DVChanged = 1;
	int count = 0;

	while(DVChanged){

		printf("===========================Loop %d===========================\n", count);
		count++;

		int temp[4];
		copyArray(DistanceVectors[myRouterNumber], temp);

		//Recieving Router Number
		readRouterNumber(newSocketFD);
	
		routerNum = otherRouterNumber;
	
		//Recieving Distance Vector
		readDistanceVector(newSocketFD);
	

		copyArray(buffer, DistanceVectors[otherRouterNumber]);

		printf("Printing Recieved Vector\n");
		printIntArray(DistanceVectors[otherRouterNumber]);

		printf("Printing my Distance Vector, My Router Num: %d\n", myRouterNumber);
		printIntArray(DistanceVectors[myRouterNumber]);

		//Respond With...

		//Send Router Number
		sendRouterNumber(newSocketFD);

		//send Distance Vector to other Router
		sendDistanceVector(newSocketFD);

		printf("Router %d's Distance Vector before updating using bellman ford\n", myRouterNumber);
		printIntArray(DistanceVectors[myRouterNumber]);

		//perform bellman ford update
		printf("Updating Distance Vector using bellman ford...\n");

		//printf("number of nodes = %d\n", numberOfNodes);
		//loop used to perform bellman ford for entire table
		int i;
		for(i=0; i<numberOfNodes; i++){
			if(DistanceVectors[myRouterNumber][i] != myRouterNumber){//cannot beat 0
				//updates this routers DV
				DistanceVectors[myRouterNumber][i] = bellmanFordEquation(myRouterNumber, i);//least cost from myRouterNumber to i
			}
		}

		printf("Printing Router %d's Distance Vector after update\n", myRouterNumber);
		printIntArray(DistanceVectors[myRouterNumber]);
	}//end while

	printf("Closing Connection...\n");

	return 0;
}