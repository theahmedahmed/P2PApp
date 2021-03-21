#include <stdio.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/select.h>

void send_file(FILE *fp, int sd)
{
	char buf[255] = {0};
	printf("Start a\n");
	while(fgets(buf, 255, fp) != NULL){
		write(sd, buf, sizeof(buf));
	}
	printf("End b\n");
}

void write_file(int sd, char* filename)
{
	int n;
	FILE *fp;
	char buf[255];
	
	fp = fopen(filename, "w");
	
	while(1){
		n = read(sd, buf, 255);
		if (n <= 0){
			break;
		}
		fprintf(fp, "%s", buf);
		bzero(buf, 255);
	}
	fclose(fp);
}

void createserver(char *filename, char *host, int port){
	int sd, check;
	struct sockaddr_in server;
	struct hostent *hp;
	char buf[255];
	
	sd = socket(AF_INET, SOCK_STREAM, 0);

	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if (hp = gethostbyname(host))
	  bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);
	else if ( inet_aton(host, (struct in_addr *) &server.sin_addr) ){
	  fprintf(stderr, "Can't get server's address\n");
	  exit(1);
	}
	
	check = connect(sd, (struct sockaddr *)&server, sizeof(server));
	if (check == -1){
		perror("Connection error");
		exit(1);
	}
	
	write(sd, filename, sizeof(filename));
	read(sd, buf, 255);
	
	if (strcmp(buf, "no") == 0){
		printf("\nE The file does not exist.\n");
		return;
	} 
	
	write_file(sd, filename);
	
	close(sd);
}

void error(char *msg){
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
	int sd, tcpsd, clientsd, length, n, port, sendtoport;
	uint16_t sendport;
	struct sockaddr_in server, from, tcp;
	struct hostent *hp, *hp2;
	fd_set rfds, afds;
	FILE *fp;
	char buf[255], buf1[255], addrwport[100], sendportstr[10], *sendtoaddr, *ptr, *ptr2, *filename, *delim = " ";
	char buf2[255];
	char *host = "localhost";
	char *peername = "Ahmed";
	char *address = "192.168.1.145";
	int tcpport = 15017;
	
	switch(argc){
	case 2://If there is one command line arg, it is the index server port number
		port = atoi(argv[1]);
		break;
	case 3://If two, it is the index server port number, and the port for the sending TCP server
		port = atoi(argv[1]);
		tcpport = atoi(argv[2]);
		break;
	default:
		fprintf(stderr, "Usage: %s [tcpport] port\n", argv[0]);
		exit(1);
	}

	//Creating the UDP server
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sd < 0){
		error("socket");
	}
	
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if (hp = gethostbyname(host))
	  bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);
	else if ( inet_aton(host, (struct in_addr *) &server.sin_addr) ){
	  fprintf(stderr, "Can't get server's address\n");
	  exit(1);
	}
	length = sizeof(struct sockaddr_in);
	///////////////////////////////////
	
	
	//Creating the sending TCP server
	tcpsd = socket(AF_INET, SOCK_STREAM, 0);

	bzero((char *)&tcp, sizeof(struct sockaddr_in));
	tcp.sin_family = AF_INET;
	tcp.sin_port = htons(tcpport);
	tcp.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if (bind(tcpsd, (struct sockaddr *)&tcp, sizeof(tcp)) == -1){
		fprintf(stderr, "Can't bind name to socket\n");
		exit(1);
	}
	
	sendport = htons(tcp.sin_port);
	sprintf(sendportstr, "%d", sendport);
	strcpy(addrwport, address);
	strcat(addrwport, ":");
	strcat(addrwport, sendportstr);
	///////////////////////////////////
	
	switch (fork()){//Create two threads
		case -1:
			error("fork error");
	      	case 0:           //This thread is for issuing commands to the index server and receiving files from other peers
			while(1){
				printf(">");
				scanf("%s", buf);//Enter the command
				ptr = strtok(buf, delim);
				
				if (strcmp(ptr, "R") == 0){//Register a file
					n = sendto(sd, ptr, strlen(ptr), 0, &server, length);
					if (n<0){
						error("Sendto");
					}
					n = sendto(sd, peername, 5, 0, &server, length);
					printf("Enter file name: ");
					scanf("%s", buf2);
					n = sendto(sd, buf2, 255, 0, &server, length);
					n = sendto(sd, addrwport, strlen(addrwport), 0, &server, length);
					
					n = recvfrom(sd, buf, 255, 0, &from, &length);
					if (n<0){
						error("recvfrom");
					}
					write(1, buf, n);
					printf("\n");
				}
				
				else if (strcmp(ptr, "O") == 0){//Request the list of all registered content
					n = sendto(sd, buf, 255, 0, &server, length);
					if (n<0){
						error("Sendto");
					}
					n = recvfrom(sd, buf1, 255, 0, &from, &length);
					if (n<0){
						error("recvfrom");
					}
					write(1, buf1, n);
					printf("\n");
				}
				
				else if (strcmp(ptr, "T") == 0){//Deregister a file
					n = sendto(sd, ptr, strlen(ptr), 0, &server, length);
					if (n<0){
						error("Sendto");
					}
					n = sendto(sd, peername, 5, 0, &server, length);
					printf("Enter file name: ");
					scanf("%s", buf2);
					n = sendto(sd, buf2, 255, 0, &server, length);
					
					
					n = recvfrom(sd, buf, 255, 0, &from, &length);
					if (n<0){
						error("recvfrom");
					}
					write(1, buf, n);
					printf("\n");
				}
				
				else if (strcmp(ptr, "S") == 0){//Search for a file registered on the index server
					n = sendto(sd, ptr, strlen(ptr), 0, &server, length);
					if (n<0){
						error("Sendto");
					}
					n = sendto(sd, peername, 5, 0, &server, length);
					printf("Enter file name: ");
					scanf("%s", buf2);
					n = sendto(sd, buf2, 255, 0, &server, length);
					
					
					n = recvfrom(sd, buf, 255, 0, &from, &length);
					if (n<0){
						error("recvfrom");
					}
					
					ptr = strtok(buf, delim);
					if (strcmp(ptr, "E") == 0){
						printf("E Content does not exist or you are the owner of the file\n");
						continue;
					}
					
					ptr = strtok(NULL, delim);
					ptr2 = strtok(ptr, ":");
					sendtoaddr = ptr2;
					ptr2 = strtok(NULL, ":");
					sendtoport = atoi(ptr2);
					
					createserver(buf2, sendtoaddr, sendtoport);
					
					n = sendto(sd, "R", 1, 0, &server, length);
					if (n<0){
						error("Sendto");
					}
					n = sendto(sd, peername, 5, 0, &server, length);
					n = sendto(sd, buf2, 255, 0, &server, length);
					n = sendto(sd, addrwport, strlen(addrwport), 0, &server, length);
					
					n = recvfrom(sd, buf, 255, 0, &from, &length);
					if (n<0){
						error("recvfrom");
					}
					write(1, buf, n);
					printf("\n");
				}
				
				
				else if (strcmp(ptr, "quit") == 0 || strcmp(ptr, "Quit") == 0){//Deregister all content and quit
					strcpy(buf1, "Q ");
					strcat(buf1, peername);
					n = sendto(sd, buf1, 255, 0, &server, length);
					if (n<0){
						error("Sendto");
					}
					exit(1);
				}
			}
	      	default:          //This thread is for listening to the sending TCP socket for any requests for a file
			while (1){
				if(listen(tcpsd, 5)<0){
					printf("Error listening\n");
					exit(1);
				}
				
				clientsd = accept(tcpsd, NULL, NULL);
				if (clientsd < 0){
					error("accept");
				}
				
				read(clientsd, buf1, 255);
				printf("File: %s\n", buf1);
				
				fp = fopen(buf1, "r");
				if (fp == NULL){
					write(clientsd, "no", 255);
					error("Error in reading file");
				}
				write(clientsd, "yes", 255);
				send_file(fp, clientsd);
				fclose(fp);
				close(clientsd);
			}
		
	}
	return 0;
}
