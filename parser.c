#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct job_list {
    int jobid;
    int status; //0 stopped, 1 running, 2 done
    char** desc;
    struct job_list *nextptr;
    int cpid_l;
    int cpid_r;
    pid_t rv;
};

void check_ps(char* str, char delim, char* saveptr, char*** ps_parse,int** ps_flag);

char** parse(char* str, char delim, char*** ps_parse, int* ps_flag)  {
    int length = strlen(str);
    char** result = malloc(length * sizeof(char*));
    char* saveptr;
    char* subtoken;
    char** ps_result;
    int i = 0;
    for(str; ; str = NULL) {
        subtoken = strtok_r(str, &delim, &saveptr);
        length--;
        if(subtoken == NULL) break;
        if(strcmp(subtoken, "<")==0 || strcmp(subtoken, ">")==0 || strcmp(subtoken, "|") ==0 || strcmp(subtoken, "2>")==0 || strcmp(subtoken, "&") == 0) {
            ps_result = malloc(length * sizeof(char*));
            *ps_result = subtoken;
            if(strcmp(subtoken, "|") == 0) {
                *ps_flag = 2;
            } else {
                *ps_flag = 1;
            }
            check_ps(str, delim, saveptr, &ps_result, &ps_flag);
            *ps_parse = ps_result;
            break;
        }
        result[i] = subtoken;
        i++;
    }
    result[i] = NULL;
    return result;
}

void check_ps(char* str, char delim, char* saveptr, char*** ps_result, int** ps_flag) {
    int i = 1;
    char* subtoken;
    for(str; ; str = NULL) {
        subtoken = strtok_r(str, &delim, &saveptr);
        if(subtoken == NULL) break;
        if(strcmp(subtoken, "|") == 0) {
            **ps_flag = 2;
        }
        (*ps_result)[i] = subtoken;
        i++;
    }
    (*ps_result)[i] = NULL;
    return;
}