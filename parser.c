#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void check_ps(char* str, char delim, char** saveptr, char** ps_parse);

char** parse(char* str, char delim, char** ps_parse, int* ps_flag)  {
    int length = strlen(str);
    char** result = malloc(length * sizeof(char*));
    char* saveptr;
    char* subtoken;
    int i = 0;
    for(str; ; str = NULL) {
        subtoken = strtok_r(str, &delim, &saveptr);
        length--;
        if(subtoken == NULL) break;
        if(strcmp(subtoken, "<")==0 || strcmp(subtoken, "<")==0 || strcmp(subtoken, "|") ==0 || strcmp(subtoken, "2<")==0) {
            ps_parse = malloc(length * sizeof(char*));
            *ps_parse = subtoken;
            check_ps(str, delim, &saveptr, ps_parse);
            *ps_flag = 1;
            break;
        }
        result[i] = subtoken;
        i++;
    }
    result[i] = NULL;
    return result;
}

void check_ps(char* str, char delim, char** saveptr, char** ps_parse) {
    int i = 1;
    char* subtoken;
    for(str; ; str = NULL) {
        subtoken = strtok_r(str, &delim, saveptr);
        if(subtoken == NULL) break;
        ps_parse[i] = subtoken;
        printf("%s\n", subtoken);
        i++;
    }
    ps_parse[i] = NULL;
    return;
}