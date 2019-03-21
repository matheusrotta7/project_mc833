/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "3490" // the port client will be connecting to

#define MAXDATASIZE 2000 // max number of bytes we can get at once
#define MAX_NAME_SIZE 200

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
	    perror("recv");
	    exit(1);
	}

	buf[numbytes] = '\0';

	printf("client: received '%s'\n",buf);
	while (1) {
		char choice;
		scanf("%c", &choice);
		char response[1];
		response[0] = choice;
		int len = strlen(response);
		if (send(sockfd, response, len, 0) == -1) {
			perror("send");
		}
		if (choice == '2') {
			//client decided to add new user
			//server will ask the new user's name
			//let's receive the message:
			if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
			    perror("recv");
			    exit(1);
			}

			buf[numbytes] = '\0';

			printf("client: received '%s'\n",buf);

			// char* username = malloc(MAX_NAME_SIZE);
			char username[MAX_NAME_SIZE];
			char username_aux[MAX_NAME_SIZE];
			char next;
			int first = 1;
			int i = 0;
			while ((next = getchar()) != '\n') { //we lose the first char here
				scanf("%s", username_aux);
				int j = 0;
				int cur = i;
		
				for (; j < strlen(username_aux); j++, i++) {
					username[i+1] = username[i]; //shift name to the right
				}
				// cur = i;
				username[cur] = next; //restore char that was used to probe
			}
			// gets(username);
			printf("%s\n", username);
			len = strlen(username);
			if (send(sockfd, username, len, 0) == -1) {
				perror("send");
			}

		}

	}

	close(sockfd);

	return 0;
}
