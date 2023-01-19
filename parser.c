#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char** parse(char* str, char delim)  {
    int length = strlen(str);
    char** result = malloc(length * sizeof(char*));
    char* saveptr;
    char* subtoken;
    int i = 0;
    for(str; ; str = NULL) {
        subtoken = strtok_r(str, &delim, &saveptr);
        if(subtoken == NULL) break;
        *(result+i) = subtoken;
        i++;
    }
    *(result+i) = NULL;
    return result;
}