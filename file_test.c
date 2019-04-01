#include<stdio.h>
#include<string.h>

#define MAX_FILE_SIZE 10000 //max size of data.txt file

int main() {


    FILE* fp;
    fp = fopen ("data.txt", "r"); //open file in read mode
    
    char aux[100];
    while (fscanf(fp, "%s", &aux) != EOF) {

        char name[500];
        char course[500];
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
            if (strcmp(course, "Engenharia Elétrica") == 0) {
                printf("%s\n", name);
            }
        }
    }







    fclose(fp);


    return 0;
}
