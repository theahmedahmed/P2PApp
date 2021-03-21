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

struct node{
	char pname[50];
	char fname[50];
	char addr[50];
	struct node *next;
};

typedef struct node node_t;

static node_t *head = NULL;

int addnode(char *peer, char *file, char *address){
	node_t *entry = malloc(sizeof(struct node));
	node_t *tmp;
	strcpy(entry->pname, peer);
	strcpy(entry->fname, file);
	strcpy(entry->addr, address);
	entry->next = NULL;
	
	if (head == NULL){
		head = entry;
	}
	else{
		for (tmp = head; tmp != NULL; tmp = tmp-> next){
			if(strcmp(tmp->pname, peer) == 0 && strcmp(tmp->fname, file) == 0){
				return 1;
			}
			if (tmp->next == NULL){
				tmp->next = entry;
				break;
			}
		}
	}
	return 0;
}

int removenode(char *peer, char *file){
	node_t *tmp;
	
	if(strcmp(head->pname, peer) == 0 && strcmp(head->fname, file) == 0){
		head = head->next;
		return 0;
	}
	
	for (tmp = head; tmp != NULL; tmp = tmp-> next){
		if(strcmp(tmp->next->pname, peer) == 0 && strcmp(tmp->next->fname, file) == 0){
			tmp->next = tmp->next->next;
			return 0;
		}
	}
	return 1;
}

void removepeer(char *peer){
	node_t *tmp;
	
	while (head != NULL && strcmp(head->pname, peer) == 0){
		head = head->next;
	}
	
	for (tmp = head; tmp != NULL; tmp = tmp-> next){
		if(strcmp(tmp->next->pname, peer) == 0){
			tmp->next = tmp->next->next;
		}
	}
}

int contsearch(char *peer, char *file, char str[]){
	node_t *tmp;
	
	strcpy(str, "S ");
	
	for (tmp = head; tmp != NULL; tmp = tmp-> next){
		if(strcmp(tmp->pname, peer) != 0 && strcmp(tmp->fname, file) == 0){
			strcat(str, tmp->addr);
			return 0;
		}
	}
	return 1;
}

void createlist(char str[]){
	node_t *tmp = head;
	
	if (head == NULL){
		strcpy(str, "List is empty");
		return;
	}
	
	strcpy(str, "");
	
	while(tmp != NULL){
		strcat(str, tmp->pname);
		strcat(str, " ");
		strcat(str, tmp->fname);
		strcat(str, "; ");
		tmp = tmp->next;
	}
}

void error(char *msg){
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    int sd, length, fromlen, n, count, check;
    struct sockaddr_in server;
    struct sockaddr_in from;
    char buf[255], buf1[255], buf2[255], buf3[255], *ptr, *delim = " ";
    node_t *tmp;

    if(argc<2){//Check that a port number has been entered in the command line args
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    sd = socket(AF_INET, SOCK_DGRAM, 0);//Create socket
    if(sd < 0){
        error("socket");
    }
    
    //Initialize the UDP server
    length = sizeof(server);
    bzero(&server, length);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(atoi(argv[1]));
    ///////////////////

    if(bind(sd, (struct sockaddr *)&server, length)<0){//Bind the server to the socket
        error("Binding");
    }
    
    fromlen = sizeof(struct sockaddr_in);

    while (1){
        bzero(buf, 255);
        printf("Ready\n");
        n = recvfrom(sd, buf, 255, 0, (struct sockaddr*)&from, &fromlen);//Receive a command from one of the peers
        if(n<0){
            error("recvfrom");
        }
        
        ptr = strtok(buf, delim);
        
        if (strcmp(ptr, "R") == 0){//Registering a new file
        	bzero(buf1, 255);
        	bzero(buf2, 255);
        	bzero(buf3, 255);
        	n = recvfrom(sd, buf1, 255, 0, (struct sockaddr*)&from, &fromlen);
        	
        	printf("Here a\n");
        	n = recvfrom(sd, buf2, 255, 0, (struct sockaddr*)&from, &fromlen);
        	printf("Here a\n");
        	n = recvfrom(sd, buf3, 255, 0, (struct sockaddr*)&from, &fromlen);
        	printf("Here a\n");
        	check = addnode(buf1, buf2, buf3);
        	
        	if (check == 1){
        		n = sendto(sd, "E Content already exists", 24, 0, (struct sockaddr*)&from, fromlen);
			if(n<0){
			    error("sendto R1");
			}
        	}
        	else{
        		n = sendto(sd, "A", 1, 0, (struct sockaddr*)&from, fromlen);
			if(n<0){
			    error("sendto R2");
			}
        	}
        }
        
        else if (strcmp(ptr, "O") == 0){//Sending the list registered content to the requesting peer
        	bzero(buf1, 255);
        	createlist(buf1);
        	n = sendto(sd, buf1, 255, 0, (struct sockaddr*)&from, fromlen);
		if(n<0){
		    error("sendto O");
		}
        }
        
        else if (strcmp(ptr, "T") == 0){//Deregistering one of the peer's files
        	bzero(buf1, 255);
        	bzero(buf2, 255);
        	n = recvfrom(sd, buf1, 255, 0, (struct sockaddr*)&from, &fromlen);
        	printf("Here b\n");
        	n = recvfrom(sd, buf2, 255, 0, (struct sockaddr*)&from, &fromlen);
        	printf("Here b\n");
        	
        	check = removenode(buf1, buf2);
        	if (check == 1){
        		n = sendto(sd, "E Content already does not exist", 32, 0, (struct sockaddr*)&from, fromlen);
			if(n<0){
			    error("sendto T1");
			}
        	}
        	else{
        		n = sendto(sd, "A", 1, 0, (struct sockaddr*)&from, fromlen);
			if(n<0){
			    error("sendto T2");
			}
        	}
        }
        
        else if (strcmp(ptr, "Q") == 0){//One of the peers is disconnecting, remove all of their content
        	ptr = strtok(NULL, delim);
        	removepeer(ptr);
        }
        
        else if (strcmp(ptr, "S") == 0){//Search for a certain file, and send back the address of the owner peer
        	bzero(buf1, 255);
        	bzero(buf2, 255);
        	n = recvfrom(sd, buf1, 255, 0, (struct sockaddr*)&from, &fromlen);
        	printf("Here c\n");
        	n = recvfrom(sd, buf2, 255, 0, (struct sockaddr*)&from, &fromlen);
        	printf("Here c\n");
        	
        	check = contsearch(buf1, buf2, buf3);
        	if (check == 1){
        		n = sendto(sd, "E Content does not exist or you are the owner of the file", 57, 0, (struct sockaddr*)&from, fromlen);
			if(n<0){
			    error("sendto S1");
			}
        	}
        	else{
        		n = sendto(sd, buf3, 255, 0, (struct sockaddr*)&from, fromlen);
			if(n<0){
			    error("sendto S2");
			}
        	}
        }
        
        bzero(buf, 255);
    }
    return 0;
}
