// sample_a.c - Feature module A
#include <stdio.h>
#include <string.h>

void processInput(char *input) {
    char buffer[256];
    strcpy(buffer, input);   // banned: strcpy
    printf("Processing: %s\n", buffer);
    // TODO: add validation here
}

int compute(int x, int y) {
    if (x < 0) goto error;  // banned: goto
    return x + y;
error:
    return -1;
}
