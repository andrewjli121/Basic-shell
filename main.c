#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "parser.c"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv){

    int cpid_l, cpid_r, ret, fd;
	int ps_flag = 0;
    char *cmd;
	char **parsedcmd;
	char **ps_parse;
	int pfd[2];

    while (cmd = readline("> ")) {
		ps_flag = 0;
		int p = 0;
		parsedcmd = parse(cmd, ' ', &ps_parse, &ps_flag);
		if(ps_flag) {
			int i = 0;
			while(ps_parse[i] != NULL) {
				char* op = ps_parse[i];
				char* file = ps_parse[i+1];
				if(strcmp(op, "<") == 0) {
					cpid_l = fork();
					if(cpid_l == 0) {
						fclose(fopen(file, "a"));
						fd = open(file, O_RDONLY);
						dup2(fd, 0);
						ret = execvp(parsedcmd[0], parsedcmd);
						if (ret ==-1){
							fprintf(stderr, "Bad command\n");
							break;
						}
					} else {
						wait((int *)NULL);
					}
				}else if(strcmp(op, ">") == 0) {
					cpid_l = fork();
					if(cpid_l == 0) {
						fclose(fopen(file, "a"));
						fd = open(file, O_WRONLY);
						dup2(fd, 1);
						ret = execvp(parsedcmd[0], parsedcmd);
						if (ret ==-1){
							fprintf(stderr, "Bad command\n");
							exit(1);
						}
					} else {
						wait((int *)NULL);
					}
				} else if(strcmp(op, "2>") == 0) {
					cpid_l = fork();
					if(cpid_l == 0) {
						fclose(fopen(file, "a"));
						fd = open(file, O_WRONLY);
						dup2(fd, 2);
						ret = execvp(parsedcmd[0], parsedcmd);
						if (ret ==-1){
							fprintf(stderr, "Bad command\n");
							exit(1);
						}
					} else {
						wait((int *)NULL);
					}
				} else if(strcmp(op, "|") == 0) {
					pipe(pfd);
					cpid_l = fork();
					if(cpid_l == 0) {
						close(pfd[0]);
						dup2(pfd[1],1);
						close(pfd[1]);
						ret = execvp(parsedcmd[0], parsedcmd);
						if (ret ==-1){
							fprintf(stderr, "Bad command\n");
							exit(1);
						}
					}
					cpid_r = fork();
					if(cpid_r == 0) {
						char* command = ps_parse[i+1];
						close(pfd[1]);
						dup2(pfd[0],0);
						close(pfd[0]);
						ret = execlp(command, command, NULL);
						if (ret ==-1){
							fprintf(stderr, "Bad command\n");
							exit(1);
						}
					}
					close(pfd[0]);
					close(pfd[1]);
					wait(NULL);
					wait(NULL);
				}
				
				i += 2;
			}
		} else {
			cpid_l = fork();
			if (cpid_l ==0){
				//Child
				ret=execvp(parsedcmd[0], parsedcmd);
				if (ret ==-1){
					printf("Bad command\n");
					break;
				}
			} else{
				wait((int *)NULL);
			}
		}

    }
    exit(1);
}