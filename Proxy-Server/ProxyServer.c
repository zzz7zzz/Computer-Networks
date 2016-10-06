#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <netdb.h> 
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>

static const int BUFFER_SIZE_SMALL = 256;

/*
 * print error messages 
 */
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

/* 
 * get port number from config file
 */ 
int getPortFromConfig(char **argv) 
{
	FILE * config_file = fopen(*(argv+1),"r");
	char buffer[BUFFER_SIZE_SMALL];
	int portNo = 0; 
	
	if (config_file == NULL) 
	{
		error("Config file is missing");
	}

	while (fgets(buffer,BUFFER_SIZE_SMALL,config_file) != NULL)
	{
		char *firstTok;
		
		firstTok = strtok(buffer, " ");
		
		if ((strstr(firstTok, "port") != NULL)
			|| (strstr(firstTok, "Port") != NULL)) 
		{
			portNo = atoi(strtok(NULL, " "));
			fclose(config_file);
			return portNo;
		}			
		bzero(buffer,BUFFER_SIZE_SMALL);
	} 

	fclose(config_file);
	return portNo;
}

/* 
 * check if website is in list of blocked websites in config file
 * return 0 if false, return 1 if true.
 */
int isBlocked(char **argv, char *host)
{
	FILE *config_file = fopen(*(argv+1),"r");
	char buffer[BUFFER_SIZE_SMALL];
	int result = 0; 
	
	if (config_file == NULL) 
	{
		error("Config file is missing");
	}
	
	while (fgets(buffer,BUFFER_SIZE_SMALL,config_file) != NULL)
	{
		char firstTok[10];
		char blockedWebsite[30];
		
		sscanf(buffer, "%s %s", firstTok, blockedWebsite);

		if ((strcmp("block", firstTok) == 0)
			|| (strcmp("Block", firstTok) == 0)) 
		{
			char *nowwwurl = NULL; 
			char blockedWebsite_copy[30];
			strncpy(blockedWebsite_copy, blockedWebsite, 30);
			nowwwurl = strtok(blockedWebsite_copy, "www");

			if (nowwwurl != NULL && 
			strstr(host, nowwwurl) != NULL)
			{
				result = 1;
				break;
			} 
			else if (strstr(host, blockedWebsite) != NULL)
			{
				result = 1;
				break;
			}
			
		}			
		bzero(buffer,BUFFER_SIZE_SMALL);

	} 
	fclose(config_file);
	return result;
}

/*
 * 0: not https protocol. 1: https. 
 */
int ishttps(char request[])
{
	if (strstr(request, "https:"))
	{
		return 1;
	}
	
	return 0;
}

/*
 * wrap error responses in html
 */
void errormessage(char *addedhtml, char *errorresponse)
{
	//char httpver[10], status[5], descr[20];
	char *httpver_ptr, *status_ptr, *descr_ptr;
	char errorresponse_copy[strlen(errorresponse)];
	
	strcpy(errorresponse_copy,errorresponse);
	
	strcat(errorresponse_copy, "^]");
	httpver_ptr = strtok(errorresponse_copy, " ");
	status_ptr = strtok(NULL, " ");
	descr_ptr = strtok(NULL, "^]");
	strcpy(addedhtml, errorresponse);
	//strcat(addedhtml, "\r\n\r\n");
	strcat(addedhtml, "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">");
	strcat(addedhtml, "<html><head>\n<title>");
	strcat(addedhtml, status_ptr);
	strcat(addedhtml, " ");
	strcat(addedhtml, descr_ptr);
	strcat(addedhtml, "</title>\n</head><body>\n<h1>");
	strcat(addedhtml, descr_ptr);
	strcat(addedhtml, "</h1>\n<p>Request message does not meet assignment requirement.</p>");
	strcat(addedhtml, "<hr>\n</body></html>\r\n\r\n");

	return;	
}

// <!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 2.0//EN">
// <html><head>
// <title>404 Not Found</title>
// </head><body>
// <h1>Not Found</h1>
// <p>The requested URL /~xingyan was not found on this server.</p>
// <hr>
// <address>Apache/2.2.3 (Red Hat) Server at csug.rochester.edu Port 80</address>
// </body></html>


/* 
 * get connecting socket from server
 */
int getserversockfd(char host[], int portno)
{	
	int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
   
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(host);

    if (server == NULL) 
    {
        error("ERROR, no such host\n");
        exit(0);
    }
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *) server->h_addr, 
         (char *) &serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    //printf("IP address is %s\n", inet_ntoa(serv_addr.sin_addr));
    if (connect(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    return sockfd;
}


int main(int argc, char **argv)
{
	/* create fields for server socket and processes */
	int sockfd, newsockfd, portno;
	socklen_t clilen; // stores size of address for client. used for accept call
	struct sockaddr_in serv_addr, cli_addr;
	int pid = 0;
	
	/* check user commands */
	if (argc < 2) {
	 error("ERROR, no config file provided in command line\n");
	 exit(1);
	}
	
	/* create socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0); // ipv4, read in stream or chunk, os decides tcp or udp
	if (sockfd < 0) 
		error("ERROR opening socket");
	
	/* set server address - ip address and port number */	
	bzero((char *) &serv_addr, sizeof(serv_addr)); 
	portno = getPortFromConfig(argv);
	if (portno == 0)
	{
		error("ERROR no port no listed in proxy config file");
	}
	//printf("Proxy port no is %d\n", portno);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY; // ip address of the machine in which the server is running
	
	/* bind socket to server */
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		  error("ERROR on binding");
	
	/* listening for client connection */
	if (listen(sockfd, 30) < 0)
		error("ERROR on listen");
	
	/* sets up clilen */
	clilen = sizeof(cli_addr);

	while (1) 
	{
		/* accepts client connection */
		 newsockfd = accept(sockfd, 
					 (struct sockaddr *) &cli_addr, 
					 &clilen);
		 if (newsockfd < 0) 
			  error("ERROR on accept");
		
		/* parent does not expect child process to send exit status. 
		 * child does not become zombie. 
		 */
		signal(SIGCHLD,SIG_IGN); 
		
		/* create a process */
		pid = fork();
		if (pid < 0) 
			error("ERROR on fork");

		
		/* check if pid is child process */
		if (pid == 0) 
		{
			/* child code */ 
			char method[10], address[BUFFER_SIZE_SMALL], httpver[10], 
			host[BUFFER_SIZE_SMALL], path[BUFFER_SIZE_SMALL];
			char buffer[BUFFER_SIZE_SMALL];
			char *strtok_ptr = NULL;
			int serverportno = 80;
			int clienterrorflag = 0;
			int serversockfd;

			/* close connecting socket as it is duplicate of that in parent program */
			//close(sockfd); 

			/* read client request from socket */
			bzero(buffer,BUFFER_SIZE_SMALL);
			if (read(newsockfd,buffer,BUFFER_SIZE_SMALL) < 0)
				error("ERROR reading from socket");
			
			//printf("Client request is \n%s\n\n", buffer);
			
			/* parse request line */
			sscanf(buffer,"%s %s %s", method, address, httpver);
			
			/* if request line meets assignment requirements */
			if ((strncmp(method,"GET",3) == 0) &&
			((strncmp(httpver,"HTTP/1.0",8) == 0)||(strncmp(httpver,"HTTP/1.1",8) == 0)) &&
			(strncmp(address,"http://",7) == 0) &&
			(strstr(address,"connect.") == NULL))
			{					
				/* get host */
				int portflag = 0;
				int i; 
				for(i=8; i < strlen(address); i++)
				{
					if (address[i]==':')
					{
						portflag = 1;
						break;
					}
				}
				
				char address_temp[BUFFER_SIZE_SMALL];
				strcpy(address_temp, address);
				strtok_ptr = strtok(address_temp,"//");
				
				if (portflag == 1)
				{
					strtok_ptr = strtok(NULL, ":");
				} 
				else
				{
					strtok_ptr = strtok(NULL,"/"); 
				}
				
				strncpy(host, strtok_ptr, strlen(strtok_ptr));
				//printf("Host is %s\n", host);
				
				/* is it a blocked url? */
				if (isBlocked(argv, host) == 1)
				{
					//printf("Host %s is blocked\n", host);
					bzero(buffer,BUFFER_SIZE_SMALL);
					strcpy(buffer, "HTTP/1.1 403 Forbidden\r\n\r\n");
					clienterrorflag = 1;
				} 
				else 
				{
				
					/* port number */
					if (portflag == 1)
					{
						serverportno = atoi(strtok(NULL, "/"));
					}

					/* get path */
					bzero(address_temp, strlen(address_temp));
					strcpy(address_temp, address);
					strcat(address_temp, "^]");
					strtok_ptr = strtok(address_temp, "//");
					strtok_ptr = strtok(NULL, "/");
					bzero(path, strlen(path));
					if ((strtok_ptr = strtok(NULL, "^]")) != NULL)
					{
						strncpy(path, strtok_ptr, strlen(strtok_ptr));
						
					} 
					//printf("Path is %s\nPort is %d\n\n", path, serverportno);
				}			
			}
			else 
			{
				clienterrorflag = 1;
			}

			/* handle invalid requests */
			if (clienterrorflag == 1)
			{
				// default response
				bzero(buffer,BUFFER_SIZE_SMALL);
				strcpy(buffer, httpver);
				strcat(buffer, " 400 Bad Request\r\n\r\n");

				/* issue quit from browser to quit proxy server */
				if (strstr(method, "QUIT") == 0
					|| strstr(method, "quit") == 0)
				{
					bzero(buffer,BUFFER_SIZE_SMALL);
					strcpy(buffer, httpver);
					strcat(buffer, " 408 Request Timeout\r\n\r\n");
				}
				
				/* non-get method */
				if (strncmp(method, "GET", 3) != 0)
				{
					bzero(buffer,BUFFER_SIZE_SMALL);
					strcpy(buffer, httpver);
					strcat(buffer, " 405 Method Not Allowed\r\n\r\n");
					
				} 
				
				/* https */
				if (strstr(address, "https") != NULL)
				{
					bzero(buffer,BUFFER_SIZE_SMALL);
					strcpy(buffer, httpver);
					strcat(buffer, " 403 Forbidden\r\n\r\n");
			
				}
				
				if (strstr(address, "connect.") != NULL)
				{
					bzero(buffer,BUFFER_SIZE_SMALL);
					strcpy(buffer, httpver);
					strcat(buffer, " 403 Forbidden\r\n\r\n");
				}
				char errorstr[1000];

				errormessage(errorstr, buffer);
				
				//printf("Error -----\n%s\n", errorstr);
				/* send response to client */
				if (write(newsockfd,errorstr,255) < 0)
					error("ERROR writing to socket");
			
				close(newsockfd);

				exit(0);	
			}
			/* get server socket */
			serversockfd = getserversockfd(host, serverportno);

			/* create request message */
			bzero(buffer, BUFFER_SIZE_SMALL); // why does this corrupt the path?
			
			sprintf(buffer, "%s /%s %s\r\n", method, path, httpver);

			strcat(buffer, "Host: ");
			strcat(buffer, host);
			strcat(buffer, "\r\nConnection: close \r\n\r\n");
			
			//printf("\nSending Request ------\n%s", buffer);
			int n = write(serversockfd, buffer, BUFFER_SIZE_SMALL);
    		
    		if (n < 0) 
         		error("ERROR writing to socket");
			
			bzero(buffer,BUFFER_SIZE_SMALL);
			int j = 0;
			//printf("\nResponse from server\n");
    		while ((j = read(serversockfd, buffer, BUFFER_SIZE_SMALL)) > 0)
    		{
    			//printf("%s\n", buffer);
    			
    			/* send response to client-proxy socket */
				if (write(newsockfd,buffer,255) < 0)
					error("ERROR writing to socket");
				bzero(buffer,BUFFER_SIZE_SMALL);
    		}

    		if (j < 0) 
         		error("ERROR reading from socket");

			close(serversockfd);
			close(newsockfd);
			close(sockfd); 
			/* terminate child process */
			exit(0);
		}
		else 
		{
			/* parent code */
			close(newsockfd); 
		}


	}
	close(sockfd); 
	return 0; 
}

/*
 * bug: finding out that strtok changes its input string and that it treats input char * ptr as const char * ptr
 * So it rejects char * ptr because it can't insert \0 into the input string. accepts char arrays. 
 * Sol: create duplicate char[] from char pointers and create temp array. 
 */

