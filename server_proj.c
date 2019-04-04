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

#define MAX_FILE_SIZE 10000 //max size of data.txt file

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
		char* welcome_msg = "Choose one of the following options: \n1: list all people that have graduated in a certain course\n2: add new profile\n3: end activities\n4:list all skills from people that live in a certain city\n5: add new experience to a certain user\n6:show user's experience by providing their email\n7: list all info from all profiles\n8: show user's complete info by providing their email";
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
						printf("User chose to list all people in a given course\n");
						//so we must receive the desired course from the client
						if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
						    perror("recv");
						    exit(1);
						}

						buf[numbytes] = '\0';

						printf("server: received desired course '%s'\n", buf);

						/***search for names that have desired course on file***/
					    fp = fopen ("data.txt", "r"); //open file in read mode

					    char aux[100];
						char matches[20][200];
						int num_of_matches = 0;
						char name[500];
						char course[500];

					    while (fscanf(fp, "%s", aux) != EOF) {

					        if (strcmp(aux, "Completo:") == 0) {
					            int i = 0;
					            char next;
					            fscanf(fp, "%c", &next); //get preceding blank space
								fscanf(fp, "%c", &next); //this gets first char
								while (next != '\n') {
									name[i++] = next;
									fscanf(fp, "%c", &next);
								}
								name[i] = '\0';
					        }
					        else if (strcmp(aux, "Acadêmica:") == 0) {
					            int i = 0;
					            char next;
					            fscanf(fp, "%c", &next); ////get preceding blank space
								fscanf(fp, "%c", &next); //this gets first char
								while (next != '\n') {
									course[i++] = next;
									fscanf(fp, "%c", &next);
								}
								course[i] = '\0';
					            if (strcmp(course, buf) == 0) {
					                printf("server found name %s in %s course\n", name, course);
									strcpy(matches[num_of_matches++], name);
					            }
					        }
					    }

						char response[1000];
						int cur;
						if (num_of_matches == 0) {
							sprintf(response, "No entries found for %s course", course);
						}
						else {
							int i;
							cur = 0;
							for (i = 0; i < num_of_matches; i++) {

								int string_len = strlen(matches[i]);
								int j, k;
								for (j = 0, k = cur; j < string_len; j++, k++) {
									response[k] = matches[i][j]; //copy first name to response
								}
								cur = k;
								response[cur++] = '\n';
							}
						}

						response[cur] = '\0';
						// printf("%s\n", response);
						len = strlen(response);
						//send list of names in response to client
						if (send(new_fd, response, len, 0) == -1) {
							perror("send");
						}

						fclose(fp);

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
					else if (buf[0] == '4') {
						printf("User chose to list all skills from profiles in a certain city\n");
						//so we must receive the desired city from the client
						if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
						    perror("recv");
						    exit(1);
						}

						buf[numbytes] = '\0';

						printf("server: received desired city '%s'\n", buf);

						/***search for names that have desired course on file***/
					    fp = fopen ("data.txt", "r"); //open file in read mode

					    char aux[100];
						char matches[20][200];
						int num_of_matches = 0;
						char city[500];
						char skills[500];

						int correct_residence = 0;
					    while (fscanf(fp, "%s", aux) != EOF) {

					        if (correct_residence && strcmp(aux, "Habilidades:") == 0) {
					            int i = 0;
					            char next;
					            fscanf(fp, "%c", &next); //get preceding blank space
								fscanf(fp, "%c", &next); //this gets first char
								while (next != '\n') {
									skills[i++] = next;
									fscanf(fp, "%c", &next);
								}
								skills[i] = '\0';
								printf("server found skills %s in %s city\n", skills, city);
								strcpy(matches[num_of_matches++], skills);
					        }
					        if (strcmp(aux, "Residência:") == 0) {
								correct_residence = 0;
					            int i = 0;
					            char next;
					            fscanf(fp, "%c", &next); ////get preceding blank space
								fscanf(fp, "%c", &next); //this gets first char
								while (next != '\n') {
									city[i++] = next;
									fscanf(fp, "%c", &next);
								}
								city[i] = '\0';
					            if (strcmp(city, buf) == 0) {
									correct_residence = 1;
									// printf("server found name %s in %s course\n", name, course);
									// strcpy(matches[num_of_matches++], name);
					            }
					        }

					    }

						char response[1000];
						int cur;
						if (num_of_matches == 0) {
							sprintf(response, "No entries found for %s city", city);
						}
						else {
							int i;
							cur = 0;
							for (i = 0; i < num_of_matches; i++) {

								int string_len = strlen(matches[i]);
								int j, k;
								for (j = 0, k = cur; j < string_len; j++, k++) {
									response[k] = matches[i][j];
								}
								cur = k;
								response[cur++] = '\n';
							}
						}

						response[cur] = '\0';
						// printf("%s\n", response);
						len = strlen(response);
						//send list of skills in response to client
						if (send(new_fd, response, len, 0) == -1) {
							perror("send");
						}

						fclose(fp);
					}
                    else if (buf[0] == '5') {
                        //client chose to add new experience to certain user
                        //we must receive the user and the experience
                        char user[200];
                        char experience[700];

                        //receive user from client
						if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
						    perror("recv");
						    exit(1);
						}

						buf[numbytes] = '\0';

						printf("server: received user '%s'\n",buf);
                        strcpy(user, buf);

                        //receive new experience from client
						if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
						    perror("recv");
						    exit(1);
						}

						buf[numbytes] = '\0';

						printf("server: received experience '%s'\n",buf);
                        strcpy(experience, buf);




                        //-----------LOGIC TO ADD EXP ON FILE DATA.TXT-------------//
                        FILE* new_fp;
                        /***search for names that have desired course on file***/
                        fp = fopen ("data.txt", "r"); //open file in read mode
                        new_fp = fopen("aux.txt", "w"); //open aux file in write mode

                        char aux[100];
                        char name[500];

                        int correct_name = 0;
                        while (fscanf(fp, "%s", aux) != EOF) {
                            // fprintf(new_fp, "%s", aux);
                            if (strcmp(aux, "Email:") == 0)
                                fprintf(new_fp, "\n\n%s ", aux);

                            else if (strcmp(aux, "Nome") == 0)
                                fprintf(new_fp, "\n%s ", aux);

                            else if (strcmp(aux, "Residência:") == 0)
                                fprintf(new_fp, "%s ", aux);

                            else if (strcmp(aux, "Formação") == 0)
                                fprintf(new_fp, "\n%s ", aux);

                            else if (strcmp(aux, "Acadêmica:") == 0)
                                fprintf(new_fp, "%s ", aux);

                            else if (strcmp(aux, "Habilidades:") == 0)
                                fprintf(new_fp, "\n%s ", aux);

                            else if (strcmp(aux, "Completo:") == 0) {
                                fprintf(new_fp, "%s", aux);
                                correct_name = 0;
                                int i = 0;
                                char next;
                                fscanf(fp, "%c", &next); //get preceding blank space
                                fprintf(new_fp, "%c", next);
                                fscanf(fp, "%c", &next); //this gets first char
                                fprintf(new_fp, "%c", next);
                                while (next != '\n') {
                                    name[i++] = next;
                                    fscanf(fp, "%c", &next);
                                    fprintf(new_fp, "%c", next);
                                }
                                name[i] = '\0';

                                if (strcmp(name, user) == 0) {
                                    correct_name = 1;
                                }

                            }
                            else if (correct_name && strcmp(aux, "Experiência:") == 0) {

                                fprintf(new_fp, "\n%s", aux);

                                char c1, c2;
                                int cur_num = 0;
                                while (fscanf(fp, "%c%c", &c1, &c2) != EOF) {

                                    if (c1 == '\n' && c2 == '\n') {
                                        fprintf(new_fp, "\n\t\t\t (%d) %s\n", ++cur_num, experience);
                                        break;
                                    }
                                    else if ((c1 == '('  && c2 >= 48 && c2 <= 57) || (c2 == ')' && c1 >= 48 && c2 <= 57)) {
                                        cur_num++;
                                        fprintf(new_fp, "%c%c", c1, c2);
                                    }
                                    else {
                                        fprintf(new_fp, "%c%c", c1, c2);
                                    }

                                }
                            }
                            else if (strcmp(aux, "Experiência:") == 0) {
                                fprintf(new_fp, "\n%s ", aux);
                            }
                            else {
                                fprintf(new_fp, "%s ", aux);
                            }
                        }

                        //we must continue copying stuff from old file to new file
                        char next;
                        while (fscanf(fp, "%c", &next) != EOF) {
                            fprintf(new_fp, "%c", next);
                        }

                        fclose(fp);
                        fclose(new_fp);

                        system("cp aux.txt data.txt"); //system call to use bash commands (was simpler :)
                        //---END OF LOGIC LOGIC TO ADD EXP ON FILE DATA.TXT -------//
                    }
                    else if (buf[0] == '6') {

                        fp = fopen("data.txt", "r");
                        printf("Client chose to show user's experience by providing their email\n");
						//so we must receive the desired email from the client
						if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
						    perror("recv");
						    exit(1);
						}

						buf[numbytes] = '\0';

						printf("server: received desired email '%s'\n", buf);
                        char response[1000];
                        char aux[100];
                        char email[200];
                        char c1, c2;
                        int cur_ind = 0;

                        while (fscanf(fp, "%s", aux) != EOF) {

                           if (strcmp(aux, "Email:") == 0) {
                               int i = 0;
                               char next;
                               fscanf(fp, "%c", &next); //get preceding blank space
                               fscanf(fp, "%c", &next); //this gets first char
                               while (next != '\n') {
                                   email[i++] = next;
                                   fscanf(fp, "%c", &next);
                               }
                               email[i] = '\0'; //I put -1 because there was a space char after the email, causing an off by one mistake
                               if (strcmp(email, buf) == 0) {
                                   while (fscanf(fp, "%s", aux) != EOF) {
                                       if (strcmp(aux, "Experiência:") == 0) {
                                           while (fscanf(fp, "%c%c", &c1, &c2) != EOF) {

                                               if ((c1 == '\n' && c2 == '\n') || (c1 == '\n' && c2 == 'E')) {
                                                   goto respond_to_client;
                                               }
                                               else {
                                                   response[cur_ind++] = c1;
                                                   response[cur_ind++] = c2;
                                               }

                                           }
                                       }
                                   }
                               }
                           }

                       }
                       respond_to_client:
                       response[cur_ind++] = '\0';

                       len = strlen(response);
                       //send list of skills in response to client
                       if (send(new_fd, response, len, 0) == -1) {
                           perror("send");
                       }

                       fclose(fp);

                    }
                    else if (buf[0] == '7') {
                        //client chose to list all info from all profiles
                        //so basically send to client whole data.txt file
                        fp = fopen("data.txt", "r");
                        char response[50000];
                        char next;
                        int cur_ind = 0;

                        char aux[200], str[200];
                        char jpgs[10][200];
                        int cur_str_ind = 0;
                        while(fscanf(fp, "%s", aux) != EOF) {
                            if (strcmp(aux, "Foto:") == 0) {

                                fscanf(fp, "%s", str);
                                strcpy(jpgs[cur_str_ind++], str);
                                printf("%s\n", jpgs[cur_str_ind-1]);
                            }
                        }

                        FILE* fp1 = fopen("data.txt", "r");

                        while(fscanf(fp1, "%c", &next) != EOF) {
                            response[cur_ind++] = next;
                        }
                        response[cur_ind] = '\0';
                        len = strlen(response);
                        //send all info in response to client
                        if (send(new_fd, response, len, 0) == -1) {
                            perror("send");
                        }

                        char c = (char) (cur_str_ind + 48);
                        printf("num_of_it will be sent as %c\n", c);
                        response[0] = c;
                        response[1] = '\0';
                        if (send(new_fd, response, len, 0) == -1) {
                            perror("send");
                        }
                        int k;
                        for (k = 0; k < cur_str_ind; k++) {

                            //first send file name
                            len = strlen(jpgs[k]);
                            if (send(new_fd, jpgs[k], len, 0) == -1) {
                                perror("send");
                            }

                            //then start send jpg logic
                            fp = fopen(jpgs[k], "r");
                                //Get Picture Size
                            printf("Getting Picture Size\n");
                            // FILE *picture;
                            // picture = fopen(argv[1], "r");
                            int size;
                            fseek(fp, 0, SEEK_END);
                            size = ftell(fp);
                            fseek(fp, 0, SEEK_SET);

                            //Send Picture Size
                            printf("Sending Picture Size\n");
                            write(new_fd, &size, sizeof(size));


                            //Send Picture as Byte Array
                            printf("Sending Picture as Byte Array\n");
                            char send_buffer[size];
                            while(!feof(fp)) {
                                fread(send_buffer, 1, sizeof(send_buffer), fp);
                                write(new_fd, send_buffer, sizeof(send_buffer));
                                // system("sleep 1");
                                bzero(send_buffer, sizeof(send_buffer));
                            }
                        }

                        fclose(fp);
                        fclose(fp1);

                    }
                    else if (buf[0] == '8') {
                        fp = fopen("data.txt", "r");
                        printf("Client chose to show user's whole info by providing their email\n");
						//so we must receive the desired email from the client
						if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
						    perror("recv");
						    exit(1);
						}

						buf[numbytes] = '\0';

						printf("server: received desired email '%s'\n", buf);
                        char response[2000];
                        int cur_ind = 0;
                        char aux[100];
                        char email[200];
                        char c1, c2;

                        while (fscanf(fp, "%s", aux) != EOF) {
                           // response[0] = '\0'; //resetting reponse
                           if (strcmp(aux, "Email:") == 0) {
                               cur_ind = 0;
                               response[cur_ind++] = 'E';
                               response[cur_ind++] = 'm';
                               response[cur_ind++] = 'a';
                               response[cur_ind++] = 'i';
                               response[cur_ind++] = 'l';
                               response[cur_ind++] = ':';
                               // response[cur_ind++] = ' ';
                               int i = 0;
                               char next;
                               fscanf(fp, "%c", &next); //get preceding blank space
                               response[cur_ind++] = next;

                               fscanf(fp, "%c", &next); //this gets first char
                               response[cur_ind++] = next;

                               while (next != '\n') {
                                   email[i++] = next;
                                   fscanf(fp, "%c", &next);
                                   response[cur_ind++] = next;

                               }
                               email[i] = '\0'; //I put -1 because there was a space char after the email, causing an off by one mistake
                               if (strcmp(email, buf) == 0) {
                                   while (fscanf(fp, "%c%c", &c1, &c2) != EOF) {

                                       if ((c1 == '\n' && c2 == '\n') || (c1 == '\n' && c2 == 'E')) {
                                           goto respond_to_client_8;
                                       }
                                       else {
                                           response[cur_ind++] = c1;
                                           response[cur_ind++] = c2;
                                       }

                                   }
                               }
                           }

                       }
                       respond_to_client_8:
                       response[cur_ind++] = '\0';

                       len = strlen(response);
                       //send list of skills in response to client
                       if (send(new_fd, response, len, 0) == -1) {
                           perror("send");
                       }

                       fclose(fp);

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
