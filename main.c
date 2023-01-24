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
    char *cmd;
	char **parsedcmd;
	char **ps_parse;
	pid_t pfd[2];
	int* pgid = malloc(20 * sizeof(int));
	char*** job_desc = malloc(20 * sizeof(char**));
	char** run_stop = malloc(20 * sizeof(char*));
	int job_i = 0;

    while (cmd = readline("# ")) {
		int ps_flag = 0;
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

				//Jobs
				for(int i = 0; ps_parse[i] != NULL; i++) {
					if(strcmp(ps_parse[i], "&") == 0) {
						setpgid(0,0);
						for(int i = job_i+1; i > 0; i++) {
							pgid[i] = pgid[i-1];
							job_desc[i] = job_desc[i-1];
							run_stop[i] = run_stop[i-1];
						}
						pgid[0] = getpid();
						run_stop[0] = "Running"; //CHANGEEEEEEEEEEEEEEE
						int x = 0;
						for(int i = 0; parsedcmd[i] != NULL; i++) {
							job_desc[0][i] = strcpy(job_desc[0][i], parsedcmd[i]);
							x++;
						}
						for(int i = 0; ps_parse[i] != NULL; i++) {
							job_desc[0][x] = strcpy(job_desc[0][x], ps_parse[i]);
							x++;
						}
						job_i++;
					}
				}

				//File redirection
				cpid_l = fork();
				if(cpid_l == 0) {
					while(ps_parse[i] != NULL) {

						char* op = ps_parse[i];
						char* file = ps_parse[i+1];
						i++;
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
				if(strcmp(parsedcmd[0], "fg") == 0) {
					//tcsetpgrp();
				}

				if(strcmp(parsedcmd[0], "jobs") == 0) {
					for(int i = 0; i < job_i; i++) {
						printf("[%i]", i+1);
						printf("-\t");	//change this
						printf("%s\t\t", run_stop[i]);
						for(int j = 0; job_desc[i][j]; j++) {
							printf("%s ", job_desc[i][j]);
						}
						printf("\n");
					}
					break;
				}

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