#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define MAX_FILE_SIZE 10000 //max size of data.txt file

int main() {


    FILE* fp;
    // FILE* new_fp;
    /***search for names that have desired course on file***/
    fp = fopen ("data.txt", "r"); //open file in read mode
    // new_fp = fopen("aux.txt", "w"); //open aux file in write mode

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
           email[i] = '\0';
           if (strcmp(email, "gps_016@gmail.com") == 0) {
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
   printf("%s\n", response);
   fclose(fp);
   // fclose(new_fp);

   // system("cp aux.txt data1.txt");



   return 0;
}
