## Description:

A proxy server is created to receive client's request and forward them to the host. 
It then forwards the host's response to the client.

Its listening socket accepts simultaneous connections. 

It does error handling for non-GET methods, non-http protocol and requests for hosts that
are blocked by calling listen(sockfd, 20) and fork() to create multiple processes. 

Zombie children are reaped by calling "signal(SIGCHLD,SIG_IGN)" so that parent does not
expect children to send their exit status. When children send the exit status and
wait for parent's reply before they terminate, they become zombies. 
zombies

## To run: 

1) In command line, type: make 

2) start the server: ./ProxyServer <Config file i.e. proxy_config>

3) Open the browser and change network preferences to proxy server: <servername> <portno>

4) To quit, type Control+C in the command line. 

5) To clean the object files, type: clean  
        

 
