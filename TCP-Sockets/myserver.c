/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> // contains type of socket and netint
#include <sys/socket.h>
#include <netinet/in.h> // structures for internet domain address
#include <signal.h> 
#include "hashTable.h" // my custom-made hash table

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void init() 
{
	FILE *input_file = fopen("cityzip.csv","r");
	char buffer[1024];
	
	if (input_file == 0) 
	{
		fprintf(stderr,"Cannot open input file.\n");
	}
	
	createHash();

    /* skip the first line in csv file */
	fgets(buffer, 1024, input_file);
	
	while (1)
	{

		if (fgets(buffer, 1024, input_file) != NULL)
		{
			parseDataFromCSV(buffer);
			bzero(buffer,1024);

		} else {
			break;
		}


	} 


    fclose(input_file);

}

int main(int argc, char *argv[])
{
     
     int sockfd, newsockfd, portno;
     socklen_t clilen; // stores size of address for client. used for accept call
     char buffer[256]; // to store read char
     struct sockaddr_in serv_addr, cli_addr;
     int pid = 0;
	 // int n; // contains no. of char read or written from read write calls
	
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr)); // why char and not int?
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     
	 if (listen(sockfd,5) < 0)
	 	error("ERROR on listen");
     
     clilen = sizeof(cli_addr);
     
     /* insert csv data into hashtable */
     init();
     
     while (1) 
     {
		 newsockfd = accept(sockfd, 
					 (struct sockaddr *) &cli_addr, 
					 &clilen);
		 if (newsockfd < 0) 
			  error("ERROR on accept");
		
		
		pid = fork();
		if (pid < 0) 
		{
			error("ERROR on fork");
		}
		
		if (pid == 0) // 0 means child process
		{
			 // child code
			 close(sockfd); // why
			 bzero(buffer,256);
			 if (read(newsockfd,buffer,255) < 0)
				error("ERROR reading from socket");
			 printf("Here is the message: %s\n",buffer);
			 
			 struct node *nodeResp; 
			 char *city, *state;
			 char resp[256];
			 
			 nodeResp = searchInHash(buffer);
			 
			 if (nodeResp != NULL)
			 {
				 city = nodeResp->city;
				 state = nodeResp->state;
			 
				 strcpy(resp, "City: ");
				 strcat(resp, city);
				 strcat(resp, "\n");
				 strcat(resp, "State: ");
				 strcat(resp, state);
				 strcat(resp, "\n");
			 
			 } else {
			 
			 	strcpy(resp, "Not found in csv file\n");
			 }
			 if (write(newsockfd,resp,255) < 0)
				error("ERROR writing to socket");
			 signal(SIGCHLD,SIG_IGN); // ignore child's termination to reap zombies 
			 exit(0);
		}
		else 
		{
			// parent code
			close(newsockfd);
	
		
		}

		 
     }
     close(newsockfd);
     return 0; 
}
