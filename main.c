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
struct job_list* head = NULL;

void sighandler(int s) {
	if(s == SIGTSTP) {
		tcsetpgrp(0,0);
	}
	struct job_list* curr = head;
	struct job_list* prev;
	while(curr) {
		if(curr->status == 2) {

		}
		prev = curr;
		curr = curr->nextptr;
	}
}

int main(int argc, char **argv){

    int cpid_l, cpid_r, ret, fd, fd2;
    char *cmd;
	char **parsedcmd;
	char **ps_parse;
	pid_t pfd[2];
	signal(SIGINT, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
    while (cmd = readline("# ")) {
		int ps_flag = 0;
		ps_flag = 0;
		int p = 0;

		parsedcmd = parse(cmd, ' ', &ps_parse, &ps_flag);
		signal(SIGCHLD, &sighandler);
		if(ps_flag != 0) {
			int i = 0;
			if(ps_flag == 2) {
				//pipe
				char* op = ps_parse[i];
				char* file = ps_parse[i+1];
				pipe(pfd);
				cpid_l = fork();
				if(cpid_l == 0) {
					signal(SIGINT, SIG_DFL);
					signal(SIGTTIN, SIG_DFL);
					signal(SIGTSTP, SIG_DFL);
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
					signal(SIGINT, SIG_DFL);
					signal(SIGTTIN, SIG_DFL);
					signal(SIGTSTP, SIG_DFL);
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

				//File redirection
				//LL Job adding
				for(int i = 0; ps_parse[i] != NULL; i++) {
					if(strcmp(ps_parse[i], "&") == 0) {
						setpgid(0,0);
						struct job_list *job = (struct job_list*) malloc(sizeof(struct job_list));
						job->status = 1;
						job->cpid_l = cpid_l;
						job->desc = parsedcmd;
						int j = 0;
						while(job->desc[j] != NULL) {
							j++;
						}
						job->desc[j] = ps_parse[0];
						job->nextptr = NULL;
						int count = 0;
						struct job_list* hold = head;
						while(head != NULL) {
							count++;
							head = head->nextptr;
						}
						head = hold;
						job->jobid = count + 1;
						if(head != NULL) {
							while(head->nextptr != NULL) {
								head = head->nextptr;
							}
							head->nextptr = job;
							head = hold;
						} else {
							head = job;
						}
						
					}
				}

				cpid_l = fork();
				if(cpid_l == 0) {
					signal(SIGINT, SIG_DFL);
					signal(SIGTTIN, SIG_DFL);
					signal(SIGTSTP, SIG_DFL);
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
		} else {	//Normal commands
			if(strcmp(parsedcmd[0], "fg") == 0) {
				struct job_list* top = head;
				head = head->nextptr;
				tcsetpgrp(0, top->cpid_l);
				kill(top->cpid_l, SIGCONT);
				for(int i = 0; strcmp(top->desc[i], "&") != 0; i++) {
					printf("%s ", top->desc[i]);
				}
				printf("\n");
				waitpid(-1 * top->cpid_l, &(top->status), WUNTRACED);
			} else if(strcmp(parsedcmd[0], "jobs") == 0) {
				struct job_list* hold = head;
				while(head != NULL) {
					printf("[%i] ", head->jobid);
					if(head == hold) {
						printf("+ ");
					}
					else {
						printf("- ");
					}
					if(head->status == 0) {
						printf("Stopped\t");
					}
					else if(head->status == 1) {
						printf("Running\t");
					}
					for(int i = 0; strcmp(head->desc[i], "&") != 0; i++) {
						printf("%s ", head->desc[i]);
					}
					printf("\n");
					fflush(stdout);
					head = head->nextptr;
				}
				head = hold;
			} else if(strcmp(parsedcmd[0], "bg") == 0) {
				struct job_list* top = head;
				while(top) {
					if(top->status == 0) {
						kill(top->cpid_l, SIGCONT);
						for(int i = 0; strcmp(top->desc[i], "&") != 0; i++) {
							printf("%s ", top->desc[i]);
						}
						printf("\n");
						waitpid(-1 * top->cpid_l, &(top->status), WNOHANG);
						break;
					}
					top = top->nextptr;
				}			
			} else {
				cpid_l = fork();
				if (cpid_l == 0){
					//Child
					signal(SIGINT, SIG_DFL);
					signal(SIGTTIN, SIG_DFL);
					signal(SIGTSTP, SIG_DFL);

					setpgid(0,0);
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

    }
    exit(1);
}