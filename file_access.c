#include<stdio.h>
void sequentialAccess(char filename[]){
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file.\n");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
    }

    fclose(file);
}
void directAccess(char filename[], int targetLine){
    FILE *file;
    char line[256];
    long positions[1000];
    int lineNo=1;
    file=fopen(filename,"r");
    if(file==NULL){
        printf("Error opening file.\n");
        return;
    }
    while(fgets(line,sizeof(line),file)){
        positions[lineNo-1]=ftell(file);
        lineNo++;
    }
    rewind(file);
    lineNo=1;
    while(lineNo < targetLine)
    {
        fgets(line, sizeof(line), file);

        lineNo++;
    }

    printf("\n===== Direct Access =====\n");

    printf("Jumped directly to Line %d:\n", targetLine);

    printf("%s\n", line);

    fclose(file);
}
        