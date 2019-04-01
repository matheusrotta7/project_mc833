#include<stdio.h>
#include<string.h>

#define MAX_FILE_SIZE 10000 //max size of data.txt file

int main() {


    FILE* fp;
    /***search for names that have desired course on file***/
    fp = fopen ("data1.txt", "r+"); //open file in read mode

    char aux[100];
    char matches[20][200];
    int num_of_matches = 0;
    char city[500];
    char skills[500];

    int correct_residence = 0;
    while (fscanf(fp, "%s", &aux) != EOF) {

        if (correct_residence && strcmp(aux, "Completo:") == 0) {
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
            if (strcmp(city, "Campinas") == 0) {
                correct_residence = 1;
                // goto skip_reset;
                // printf("server found name %s in %s course\n", name, course);
                // strcpy(matches[num_of_matches++], name);
            }
        }
        // int meaning_of_life = 42;
        // skip_reset:
        // meaning_of_life = 42;
    }







    fclose(fp);


    return 0;
}
