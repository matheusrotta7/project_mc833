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
#include <sys/time.h>

#include <arpa/inet.h>

#define PORT "3490" // the port client will be connecting to



#define PICBUFFER 10000 //size of the max chunk of data that is part of whole pic to be sent
#define PICBUFFER_SEND 5000 //size of the max chunk of data that is part of whole pic to be sent


#define MAXDATASIZE 10000 // max number of bytes we can get at once
#define MAX_NAME_SIZE 500

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
    struct timeval tv1, tv2;
	int sockfd, numbytes;
	char buf[MAXDATASIZE+1];
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
		char response[2];
		response[0] = choice;
        response[1] = '\0'; //this is so necessary! Some compilers fix the lack of \0, some don't
		int len = strlen(response);

		if (send(sockfd, response, len, 0) == -1) {
			perror("send");
		}
		if (choice == '2') {
			//client decided to add new user
			//server will ask the new user's info
			//let's receive the message:
			if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
			    perror("recv");
			    exit(1);
			}

			buf[numbytes] = '\0';

			printf("client: received '%s'\n",buf);

			char buf[MAX_NAME_SIZE];
			char next;
			int i = 0;
			scanf("%c", &next); //this one grabs annoying newline (this only happens here)
			/****get email from stdin****/
			scanf("%c", &next); //this gets first char
			while (next != '\n') {
				buf[i++] = next;
				scanf("%c", &next);
			}
			buf[i] = '\0';
			/****************************/
			len = strlen(buf);
			//send email to server
			if (send(sockfd, buf, len, 0) == -1) {
				perror("send");
			}

			/****get name from stdin****/
			i = 0;
			scanf("%c", &next); //this gets first char
			while (next != '\n') {
				buf[i++] = next;
				scanf("%c", &next);
			}
			buf[i] = '\0';
			/****************************/
			len = strlen(buf);
			//send name to server
			if (send(sockfd, buf, len, 0) == -1) {
				perror("send");
			}

            //-----------photo logic------------------//
            i = 0;
			scanf("%c", &next); //this gets first char
			while (next != '\n') {
				buf[i++] = next;
				scanf("%c", &next);
			}
			buf[i] = '\0';
			/****************************/
			len = strlen(buf);
            printf("I AM SENDING THIS FILENAME: %s\n", buf);
			//send photo filename to server
			if (send(sockfd, buf, len, 0) == -1) {
				perror("send");
			}

            /*now we have to send the actual file to server*/
            //--------------begin send jpg logic:******/////
            FILE* fp = fopen(buf, "rb");
            char response[PICBUFFER_SEND+1];
            int cur_ind = 0;
            int j;
            int k;
            int len;
            // char next;

            //we must know how many iterations will be made and send it to client
            //let's measure file size and then divide it by PICBUFFER
            // size_t file_size = 0;
            // while (fscanf(fp, "%c", &next) != EOF) {
            //     file_size++;
            // }

            // obtain file size:
            size_t file_size;
            fseek (fp , 0 , SEEK_END);
            file_size = ftell (fp);
            rewind (fp);


            int num_of_it = file_size/PICBUFFER_SEND + ((file_size%PICBUFFER_SEND) != 0); //formulinha fofinha :) por no relatório uma breve explicação pq ficou legal
            printf("num_of_it: %d\n", num_of_it);
            sprintf(response, "%d", num_of_it);
            len = strlen(response);
            //send num_of_it to client
            if (send(sockfd, response, len, 0) == -1) {
                perror("send");
            }
            system("sleep 0.3");



            for (j = 0; j < num_of_it; j++) {
                printf("iteration no.: %d\n", j);
                // cur_ind = 0;
                size_t result;
                result = fread (response, 1, PICBUFFER_SEND, fp);

                printf("result size: %lu\n", result);
                printf("before send\n");

                //send 10000 size chunk
                if (send(sockfd, response, result, 0) == -1) {
                    // perror("send");
                }
                system("sleep 0.5");
                // printf("after send, before sleep\n");
                // printf("after sleep\n");

            }
            fclose(fp);
            //--------------end send jpg logic:********/////
            //-------end of photo logic------------------//


			/****get residence from stdin****/
			i = 0;
			scanf("%c", &next); //this gets first char
			while (next != '\n') {
				buf[i++] = next;
				scanf("%c", &next);
			}
			buf[i] = '\0';
			/****************************/
			len = strlen(buf);
			//send residence to server
			if (send(sockfd, buf, len, 0) == -1) {
				perror("send");
			}

			/****get major from stdin****/
			i = 0;
			scanf("%c", &next); //this gets first char
			while (next != '\n') {
				buf[i++] = next;
				scanf("%c", &next);
			}
			buf[i] = '\0';
			/****************************/
			len = strlen(buf);
			//send major to server
			if (send(sockfd, buf, len, 0) == -1) {
				perror("send");
			}

			/****get skills from stdin****/
			i = 0;
			scanf("%c", &next); //this gets first char
			while (next != '\n') {
				buf[i++] = next;
				scanf("%c", &next);
			}
			buf[i] = '\0';
			/****************************/
			len = strlen(buf);
			//send skills to server
			if (send(sockfd, buf, len, 0) == -1) {
				perror("send");
			}

			/****get one experience from stdin****/
			i = 0;
			scanf("%c", &next); //this gets first char
			while (next != '\n') {
				buf[i++] = next;
				scanf("%c", &next);
			}
			buf[i] = '\0';
			/****************************/
			len = strlen(buf);
			//send one experience to server
			if (send(sockfd, buf, len, 0) == -1) {
				perror("send");
			}

		}
		else if (choice == '1') {
			//user chose to list all people that majored in a certain course
			//we need to know the course, mate, let's ask them
			int i;
			char next;
			printf("Enter desired course: \n");
			/****get desired course from stdin****/
			i = 0;
			scanf("%c", &next); //this gets annoying newline
			scanf("%c", &next); //this gets first char
			while (next != '\n') {
				buf[i++] = next;
				scanf("%c", &next);
			}
			buf[i] = '\0';
			/****************************/
			len = strlen(buf);
			//send desired course to server
			if (send(sockfd, buf, len, 0) == -1) {
				perror("send");
			}

			//now client must receive answer from server:
			if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
			    perror("recv");
			    exit(1);
			}

			buf[numbytes] = '\0';

			printf("\nclient: received the following names:\n");
			printf("%s\n", buf);

		}
		else if (choice == '4') {
			//user chose to list all skills in a certain city
			//we need to know the city, mate, let's ask them
			int i;
			char next;
			printf("Enter desired city: \n");
			/****get desired city from stdin****/
			i = 0;
			scanf("%c", &next); //this gets annoying newline
			scanf("%c", &next); //this gets first char
			while (next != '\n') {
				buf[i++] = next;
				scanf("%c", &next);
			}
			buf[i] = '\0';
			/****************************/
			len = strlen(buf);
			//send desired course to server
            gettimeofday(&tv1, NULL);
			if (send(sockfd, buf, len, 0) == -1) {
				perror("send");
			}

			//now client must receive answer from server:
			if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
			    perror("recv");
			    exit(1);
			}
            gettimeofday(&tv2, NULL);

			buf[numbytes] = '\0';

			printf("\nclient: received the following skills:\n");
			printf("%s\n", buf);
            printf("TOTAL TIME ON CLIENT: %ld\n", tv2.tv_usec - tv1.tv_usec);
		}
        else if (choice == '5') {
            //user decided to add new experience to a certain person (by name)
            int i;
			char next;
			printf("Enter desired name: \n");
			/****get desired name from stdin****/
			i = 0;
			scanf("%c", &next); //this gets annoying newline
			scanf("%c", &next); //this gets first char
			while (next != '\n') {
				buf[i++] = next;
				scanf("%c", &next);
			}
			buf[i] = '\0';
			/****************************/
			len = strlen(buf);
			//send desired course to server
			if (send(sockfd, buf, len, 0) == -1) {
				perror("send");
			}

            printf("Enter desired experience: \n");
			/****get desired name from stdin****/
			i = 0;
			// scanf("%c", &next); //this gets annoying newline
			scanf("%c", &next); //this gets first char
			while (next != '\n') {
				buf[i++] = next;
				scanf("%c", &next);
			}
			buf[i] = '\0';
			/****************************/
			len = strlen(buf);
			//send desired course to server
			if (send(sockfd, buf, len, 0) == -1) {
				perror("send");
			}
            //client does not need to receive any response from user


        }
        else if (choice == '6') {
            //user chose to show user's experience by providing their email
			//we need to know the email, mate, let's ask them
			int i;
			char next;
			printf("Enter desired email: \n");
			/****get desired city from stdin****/
			i = 0;
			scanf("%c", &next); //this gets annoying newline
			scanf("%c", &next); //this gets first char
			while (next != '\n') {
				buf[i++] = next;
				scanf("%c", &next);
			}
			buf[i] = '\0';
			/****************************/
			len = strlen(buf);
			//send desired email to server
			if (send(sockfd, buf, len, 0) == -1) {
				perror("send");
			}

			//now client must receive answer from server:
			if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
			    perror("recv");
			    exit(1);
			}

			buf[numbytes] = '\0';

			printf("\nclient: received the following experience:\n");
			printf("%s\n", buf);
        }
        else if (choice == '7') {
            //now client must receive all info from server:
			if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
			    perror("recv");
			    exit(1);
			}

			buf[numbytes] = '\0';

			printf("\nclient: received this info from all profiles:\n");
			printf("%s\n", buf);

            //receive num_of_photos
            if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
			    perror("recv");
			    exit(1);
			}
            buf[numbytes] = '\0';
            // printf("\n\n%c", buf[0]);
            int num_of_photos;
            // num_of_photos -= 48;
            sscanf(buf, "%d", &num_of_photos);
            printf("num_of_photos on client: %d\n", num_of_photos);
            int i;
            for (i = 0; i < num_of_photos; i++) {
                //first get filename
                if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
    			    perror("recv");
    			    exit(1);
    			}
                buf[numbytes] = '\0';
                printf("NUMBYTES RECEIVED FOR FILENAME: %d\n", numbytes);
                // printf("%s\n", buf);
                // buf[0] = '@';

                printf("NAME OF CURRENT FILE: %s\n", buf);

                //then start logic to receive jpg:
                FILE* fp;
                fp = fopen(buf, "wb");

                //receive num_of_it for this current photo
                if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
                    perror("recv");
                    exit(1);
                }

                buf[numbytes] = '\0';
                printf("BUFFER OF NUM_OF_IT RECEIVED FROM SERVER %s\n", buf);
                int num_of_it;
                sscanf(buf, "%d", &num_of_it);
                int k;
                printf("num_of_it on client: %d\n", num_of_it);

                for (k = 0; k < num_of_it; k++) {
                    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
                        perror("recv");
                        exit(1);
                    }

                    buf[numbytes] = '\0';
                    printf("numbytes in iteration %d: %d\n", k, numbytes);
                    int l;
                    // int len = strlen(buf);
                    fwrite(buf, 1, numbytes, fp);
                }
                fclose(fp);

                gettimeofday(&tv2, NULL);
                printf("TOTAL TIME: %ld\n", tv2.tv_usec - tv1.tv_usec);
            }
        }
        else if (choice == '8') {
            //user chose to show user's whole info by providing their email
			//we need to know the email, mate, let's ask them
			int i;
			char next;
			printf("Enter desired email: \n");
			/****get desired city from stdin****/
			i = 0;
			scanf("%c", &next); //this gets annoying newline
			scanf("%c", &next); //this gets first char
			while (next != '\n') {
				buf[i++] = next;
				scanf("%c", &next);
			}
			buf[i] = '\0';
			/****************************/
			len = strlen(buf);
			//send desired email to server
			if (send(sockfd, buf, len, 0) == -1) {
				perror("send");
			}

			//now client must receive answer from server:
			if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
			    perror("recv");
			    exit(1);
			}

			buf[numbytes] = '\0';

			printf("\nclient: received the following info:\n");
			printf("%s\n", buf);
        }

	}

	close(sockfd);

	return 0;
}
