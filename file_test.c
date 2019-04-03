#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define MAX_FILE_SIZE 10000 //max size of data.txt file

int main() {


    FILE* fp;
    FILE* new_fp;
    /***search for names that have desired course on file***/
    fp = fopen ("data1.txt", "r+"); //open file in read mode
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

            if (strcmp(name, "Gabriel Pellegrino da Silva") == 0) {
                correct_name = 1;
            }

        }
        else if (correct_name && strcmp(aux, "Experiência:") == 0) {

            fprintf(new_fp, "\n%s", aux);

            char c1, c2;
            int cur_num = 0;
            while (fscanf(fp, "%c%c", &c1, &c2) != EOF) {

                if (c1 == '\n' && c2 == '\n') {
                    fprintf(new_fp, "\n\t\t\t (%d) Essa é a nova experiência do GABRIE\n", ++cur_num);
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

    system("cp aux.txt data1.txt");



    return 0;
}
