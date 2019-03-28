/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10	 // how many pending connections queue will hold

#define MAXDATASIZE 2000 // max number of bytes we can get at once

// typedef struct userinfo {
//
// 	char email[200];
// 	char full_name[300];
// 	char city[100];
// 	char major[200];
// 	char skills[300];
// 	char skills[10][300];
// 	int next_exp;
//
// } userinfo;

void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;


	/*****stuff I added*****/
	FILE* fp;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);
		char* welcome_msg = "Choose one of the following options: \n1: list all people that have graduated in a certain course\n2: add new profile\n3: end activities\n";
		int len = strlen(welcome_msg);
		if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener
			if (send(new_fd, welcome_msg, len, 0) == -1) {
				perror("send");
			}

			int numbytes;
			char buf[MAXDATASIZE];

			while (1) {
				if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
				    perror("recv");
				    exit(1);
				}
				if (1) {
					if (buf[0] == '1') {
						printf("Barney, Panda, Pellegrino, ...\n");
					}
					else if (buf[0] == '2') {
						//***start logic to add new user, ask stuff from client
						printf("Client chose to add new user\n");
						welcome_msg = "Please inform the user's info (email, full name, photo (not yet implemented), residence, bachelor's degree, skills and one experience)";
						len = strlen(welcome_msg);
						if (send(new_fd, welcome_msg, len, 0) == -1) {
							perror("send");
						}


						/***file manipulation part***/
						fp = fopen ("data.txt", "a"); //open file in append mode


						//receive email from client
						if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
						    perror("recv");
						    exit(1);
						}

						buf[numbytes] = '\0';

						printf("server: received email '%s'\n",buf);
						fprintf(fp, "Email: %s\n", buf);

						//receive name from client
						if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
						    perror("recv");
						    exit(1);
						}

						buf[numbytes] = '\0';

						printf("server: received name '%s'\n",buf);
						fprintf(fp, "Nome Completo: %s\n", buf);

						// //receive photo from client
						// if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
						//     perror("recv");
						//     exit(1);
						// }
						//
						// buf[numbytes] = '\0';
						//
						// printf("server: received '%s'\n",buf);

						//receive residence from client
						if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
						    perror("recv");
						    exit(1);
						}

						buf[numbytes] = '\0';

						printf("server: received residence'%s'\n",buf);
						fprintf(fp, "Residência: %s\n", buf);

						//receive major from client
						if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
						    perror("recv");
						    exit(1);
						}

						buf[numbytes] = '\0';

						printf("server: received major '%s'\n",buf);
						fprintf(fp, "Formação Acadêmica: %s\n", buf);

						//receive skills from client
						if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
						    perror("recv");
						    exit(1);
						}

						buf[numbytes] = '\0';

						printf("server: received skills '%s'\n",buf);
						fprintf(fp, "Habilidades: %s\n", buf);

						//receive one experience from client
						if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
						    perror("recv");
						    exit(1);
						}

						buf[numbytes] = '\0';

						printf("server: received one experience '%s'\n",buf);
						fprintf(fp, "Experiência: (1) %s\n\n\n", buf); //adicionar mais via opções do menu



						fclose(fp);

						// printf("New user added: %s\n", buf);
					}
					else if (buf[0] == '3') {
						//end connection
						break;
					}
				}
				else {
					// printf("invalid client input\n");
				}
			}
			buf[numbytes] = '\0';


			close(new_fd);
			exit(0);
		}
		close(new_fd);  // parent doesn't need this
	}

	return 0;
}
