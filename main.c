#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "parser.c"
#include "sighandler.c"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv){

    int cpid_l, cpid_r, ret, fd, fd2;
	int ps_flag = 0;
    char *cmd;
	char **parsedcmd;
	char **ps_parse;
	int pfd[2];

    while (cmd = readline("# ")) {
		ps_flag = 0;
		int p = 0;
		parsedcmd = parse(cmd, ' ', &ps_parse, &ps_flag);
		signal(SIGINT, sighandler);
		signal(SIGTSTP, sighandler);
		signal(SIGCHLD, sighandler);
		if(ps_flag != 0) {
			int i = 0;
			
			if(ps_flag == 2) {
				//pipe
				char* op = ps_parse[i];
				char* file = ps_parse[i+1];
				pipe(pfd);
				cpid_l = fork();
				if(cpid_l == 0) {
					close(pfd[0]);
					dup2(pfd[1],1);
					close(pfd[1]);
					while(strcmp(ps_parse[i], "|") != 0) {
						char* op = ps_parse[i];
						char* file = ps_parse[i+1];
						if(strcmp(op, "<") == 0) {
							fclose(fopen(file, "a"));
							fd = open(file, O_RDONLY);
							dup2(fd, 0);
						}else if(strcmp(op, ">") == 0) {
							fclose(fopen(file, "a"));
							fd = open(file, O_WRONLY);
							dup2(fd, 1);
						} else if(strcmp(op, "2>") == 0) {
							fclose(fopen(file, "a"));
							fd = open(file, O_WRONLY);
							dup2(fd, 2);
						}
						i += 2;
					}
					ret = execvp(parsedcmd[0], parsedcmd);
					if (ret ==-1){
						//fprintf(stderr, "Bad command\n");
						exit(1);
					}
				}
				cpid_r = fork();
				if(cpid_r == 0) {
					char* command = ps_parse[i+3];
					i += 4;
					close(pfd[1]);
					dup2(pfd[0],0);
					close(pfd[0]);
					while(ps_parse[i] != NULL) {
						char* op = ps_parse[i];
						char* file = ps_parse[i+1];
						if(strcmp(op, "<") == 0) {
							fclose(fopen(file, "a"));
							fd2 = open(file, O_RDONLY);
							dup2(fd2, 0);
						}else if(strcmp(op, ">") == 0) {
							fclose(fopen(file, "a"));
							fd2 = open(file, O_WRONLY);
							dup2(fd2, 1);
						} else if(strcmp(op, "2>") == 0) {
							fclose(fopen(file, "a"));
							fd2 = open(file, O_WRONLY);
							dup2(fd2, 2);
						}
						i += 2;
					}
					printf("%s", command);
					ret = execlp(command, command, NULL);
					if (ret ==-1){
						//fprintf(stderr, "Bad command\n");
						exit(1);
					}
				}
				close(pfd[0]);
				close(pfd[1]);
				wait(NULL);
				wait(NULL);
			} else {
				//no pipe
				cpid_l = fork();
				if(cpid_l == 0) {
					while(ps_parse[i] != NULL) {
						char* op = ps_parse[i];
						char* file = ps_parse[i+1];
						if(strcmp(op, "<") == 0) {
							fclose(fopen(file, "a"));
							fd = open(file, O_RDONLY);
							dup2(fd, 0);
						}else if(strcmp(op, ">") == 0) {
							fclose(fopen(file, "a"));
							fd = open(file, O_WRONLY);
							dup2(fd, 1);
						} else if(strcmp(op, "2>") == 0) {
							fclose(fopen(file, "a"));
							fd = open(file, O_WRONLY);
							dup2(fd, 2);
						}
						
						i += 2;
					}
					ret = execvp(parsedcmd[0], parsedcmd);
					if (ret ==-1){
						exit(1);
					}
				} else {
					wait((int *)NULL);
				}
			}
		} else {
			cpid_l = fork();
			if (cpid_l ==0){
				//Child
				ret=execvp(parsedcmd[0], parsedcmd);
				if (ret ==-1){
					//fprintf(stderr, "Bad command\n");
					break;
				}
			} else{
				wait((int *)NULL);
			}
		}

    }
    exit(1);
}