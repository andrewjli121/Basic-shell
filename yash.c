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
int status = 0;
void sighandler(int s) {
	struct job_list* curr = head;
	struct job_list* prev;
	while(curr) {
		pid_t rv = waitpid(curr->cpid_l, &status, WNOHANG);
		if(rv && WIFEXITED(status)) {
			curr->status = 2;
		}
		curr = curr->nextptr;
	}
}

int main(int argc, char **argv){

    int cpid_l, cpid_r, ret, fd, fd2;
    char *cmd;
	pid_t rv;
	pid_t p_pid = getpid();
	char **parsedcmd;
	char **ps_parse;
	pid_t pfd[2];
	signal(SIGINT, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGCHLD, &sighandler);
    while (cmd = readline("# ")) {

		int ps_flag = 0;
		ps_flag = 0;
		int background = 0;
		parsedcmd = parse(cmd, ' ', &ps_parse, &ps_flag);
		if(*parsedcmd == NULL) continue;
		if(ps_flag != 0) {
			int pp_index = 0;
			if(ps_flag == 2) {
				//pipe
				char* op = ps_parse[pp_index];
				char* file = ps_parse[pp_index+1];
				pipe(pfd);
				cpid_l = fork();
				for(int i = 0; ps_parse[i] != NULL; i++) {
					if(strcmp(ps_parse[i], "&") == 0) {
						struct job_list *job = (struct job_list*) malloc(sizeof(struct job_list));
						job->status = 1;
						job->cpid_l = cpid_l;
						int leng1 = 0;
						while(parsedcmd[leng1] != NULL) {
							leng1++;
						}
						int leng2 = 0;
						while(ps_parse[leng2] != NULL) {
							leng2++;
						}
						int total_length = leng1 + leng2 + 1;
						job->desc = malloc(total_length * sizeof(char*));
						for(int k = 0; parsedcmd[k] != NULL; k++) {
							job->desc[k] = parsedcmd[k];
						}
						for(int k = 0; ps_parse[k] != NULL; k++) {
							job->desc[k+leng1] = ps_parse[k];
						}
						job->desc[total_length-1] = NULL;
						job->nextptr = NULL;
						int count = 0;
						struct job_list* hold = head;
						while(head != NULL) {
							count = head->jobid;
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
						background = 1;
						break;
					}
				}
				setpgid(cpid_l, cpid_l);
				if(!background) {
					tcsetpgrp(0, cpid_l);
				}
				if(cpid_l == 0) {
					signal(SIGINT, SIG_DFL);
					signal(SIGTTIN, SIG_DFL);
					signal(SIGTSTP, SIG_DFL);
					signal(SIGTTOU, SIG_IGN);
					close(pfd[0]);
					dup2(pfd[1],1);
					close(pfd[1]);
					while(strcmp(ps_parse[pp_index], "|") != 0 && strcmp(ps_parse[pp_index+1], "|") != 0) {
						char* op = ps_parse[pp_index];
						char* file = ps_parse[pp_index+1];
						if(strcmp(op, "<") == 0) {
							fclose(fopen(file, "a"));
							fd = open(file, O_RDONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
							dup2(fd, 0);
						}else if(strcmp(op, ">") == 0) {
							fclose(fopen(file, "a"));
							fd = open(file, O_WRONLY | O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
							dup2(fd, 1);
						} else if(strcmp(op, "2>") == 0) {
							fclose(fopen(file, "a"));
							fd = open(file, O_WRONLY | O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
							dup2(fd, 2);
						}
						pp_index += 2;
					}
					ret = execvp(parsedcmd[0], parsedcmd);
					if (ret ==-1){
						exit(1);
					}
				}
				cpid_r = fork();
				setpgid(cpid_r, cpid_l);
				if(cpid_r == 0) {
					signal(SIGINT, SIG_DFL);
					signal(SIGTTIN, SIG_DFL);
					signal(SIGTSTP, SIG_DFL);
					while(strcmp(ps_parse[pp_index], "|") != 0) {
						pp_index++;
					}
					int command = pp_index + 1;
					pp_index++;
					while(ps_parse[pp_index] && strcmp(ps_parse[pp_index], "<") != 0 && strcmp(ps_parse[pp_index], ">") != 0 
						&& strcmp(ps_parse[pp_index], "2>") != 0 && strcmp(ps_parse[pp_index], "|") != 0 && strcmp(ps_parse[pp_index], "&") != 0) {
						pp_index++;
					}
					int end_good = pp_index;
					close(pfd[1]);
					dup2(pfd[0],0);
					close(pfd[0]);
					while(ps_parse[pp_index] && ps_parse[pp_index+1] && strcmp(ps_parse[pp_index],"&") != 0) {
						char* op = ps_parse[pp_index];
						char* file = ps_parse[pp_index+1];
						if(strcmp(op, "<") == 0) {
							fclose(fopen(file, "a"));
							fd2 = open(file, O_RDONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
							dup2(fd2, 0);
						}else if(strcmp(op, ">") == 0) {
							fclose(fopen(file, "a"));
							fd2 = open(file, O_WRONLY | O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
							dup2(fd2, 1);
						} else if(strcmp(op, "2>") == 0) {
							fclose(fopen(file, "a"));
							fd2 = open(file, O_WRONLY | O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
							dup2(fd2, 2);
						}
						pp_index += 2;
					}
					ps_parse[end_good] = NULL;
					ret = execvp(ps_parse[command], (ps_parse + command));
					if (ret ==-1){
						//fprintf(stderr, "Bad command\n");
						exit(1);
					}
				}
				close(pfd[0]);
				close(pfd[1]);
				if(!background) {
					waitpid(-1 * cpid_l, &status, WUNTRACED);
				} else {
					waitpid(-1 * cpid_l, &status, WNOHANG);
				}
				tcsetpgrp(0, p_pid);
				if(!background && WIFSTOPPED(status)) {
					struct job_list *job = (struct job_list*) malloc(sizeof(struct job_list));
					job->status = 0;
					job->cpid_l = cpid_l;
					job->desc = parsedcmd;
					job->nextptr = NULL;
					int count = 0;
					struct job_list* hold = head;
					while(head != NULL) {
						count = head->jobid;
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
			} else {
				//no pipe

				//File redirection
				//LL Job adding
				cpid_l = fork();
				for(int i = 0; ps_parse[i] != NULL; i++) {
					if(strcmp(ps_parse[i], "&") == 0) {
						struct job_list *job = (struct job_list*) malloc(sizeof(struct job_list));
						job->status = 1;
						job->cpid_l = cpid_l;
						int leng1 = 0;
						while(parsedcmd[leng1] != NULL) {
							leng1++;
						}
						int leng2 = 0;
						while(ps_parse[leng2] != NULL) {
							leng2++;
						}
						int total_length = leng1 + leng2 + 1;
						job->desc = malloc(total_length * sizeof(char*));
						for(int k = 0; parsedcmd[k] != NULL; k++) {
							job->desc[k] = parsedcmd[k];
						}
						for(int k = 0; ps_parse[k] != NULL; k++) {
							job->desc[k+leng1] = ps_parse[k];
						}
						job->desc[total_length-1] = NULL;
						job->nextptr = NULL;
						int count = 0;
						struct job_list* hold = head;
						while(head != NULL) {
							count = head->jobid;
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
						background = 1;
						break;
					}
				}
				setpgid(cpid_l, cpid_l);
				if(!background) {
					tcsetpgrp(0, cpid_l);
				}
				if(cpid_l == 0) {
					signal(SIGINT, SIG_DFL);
					signal(SIGTTIN, SIG_DFL);
					signal(SIGTSTP, SIG_DFL);
					while(ps_parse[pp_index] && ps_parse[pp_index+1]) {

						char* op = ps_parse[pp_index];
						char* file = ps_parse[pp_index+1];
						if(strcmp(op, "<") == 0) {
							fclose(fopen(file, "a"));
							fd = open(file, O_RDONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
							dup2(fd, 0);
						}else if(strcmp(op, ">") == 0) {
							fclose(fopen(file, "a"));
							fd = open(file, O_WRONLY | O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
							dup2(fd, 1);
						} else if(strcmp(op, "2>") == 0) {
							fclose(fopen(file, "a"));
							fd = open(file, O_WRONLY | O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
							dup2(fd, 2);
						}
						
						pp_index += 2;
					}
					ret = execvp(parsedcmd[0], parsedcmd);
					if (ret ==-1){
						exit(1);
					}
				} else {
					if(background) {
						rv = waitpid(-1 * cpid_l, &status, WNOHANG);
					} else {
						waitpid(-1 * cpid_l, &status, WUNTRACED);
						tcsetpgrp(0, p_pid);
					}
					if(rv && background && WIFEXITED(status)) {
						printf("WIFEXITED CALLED %d", WEXITSTATUS(status)); 
						fflush(stdout);
						struct job_list* top = head;
						while(top->cpid_l != cpid_l) {
							top = top->nextptr;
						}	
						top->status = 2;
					}
					if(!background && WIFSTOPPED(status)) {
						struct job_list *job = (struct job_list*) malloc(sizeof(struct job_list));
						job->status = 0;
						job->cpid_l = cpid_l;
						int leng1 = 0;
						while(parsedcmd[leng1] != NULL) {
							leng1++;
						}
						int leng2 = 0;
						while(ps_parse[leng2] != NULL) {
							leng2++;
						}
						int total_length = leng1 + leng2 + 1;
						job->desc = malloc(total_length * sizeof(char*));
						for(int k = 0; parsedcmd[k] != NULL; k++) {
							job->desc[k] = parsedcmd[k];
						}
						for(int k = 0; ps_parse[k] != NULL; k++) {
							job->desc[k+leng1] = ps_parse[k];
						}
						job->desc[total_length-1] = NULL;
						job->nextptr = NULL;
						int count = 0;
						struct job_list* hold = head;
						while(head != NULL) {
							count = head->jobid;
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
			}
		} else {	//Normal commands
			if(strcmp(parsedcmd[0], "fg") == 0) {
				struct job_list* hold = head;
				struct job_list* top = head;
				while(head->nextptr) {
					top = head;
					head = head->nextptr;
				}
				if(head) {
					tcsetpgrp(1, head->cpid_l);
					kill(head->cpid_l, SIGCONT);
					for(int i = 0; head->desc[i] != NULL; i++) {
						printf("%s ", head->desc[i]);
					}
					printf("\n");
					waitpid(-1 * head->cpid_l, &status, WUNTRACED);
					if(WIFSTOPPED(status)) {
						top->nextptr = head;
						head = hold;
					}
					tcsetpgrp(1, p_pid);
				}
				head = hold;
				continue;
			} else if(strcmp(parsedcmd[0], "jobs") == 0) {
				struct job_list* curr = head;
				struct job_list* prev;
				while(curr) {
					if(head == curr && curr->status == 2) {
						head = head->nextptr;
						printf("[%i] ", curr->jobid);
						printf("- ");
						printf("Done\t");
						for(int i = 0; curr->desc[i] != NULL; i++) {
							printf("%s ", curr->desc[i]);
						}
						printf("\n");
						fflush(stdout);
						free(curr->desc);
						free(curr);
						continue;
					}
					prev = curr;
					curr = curr->nextptr;
					if(curr && curr->status == 2) {
						prev->nextptr = curr->nextptr;
						printf("[%i] ", curr->jobid);
						printf("- ");
						printf("Done\t");
						for(int i = 0; curr->desc[i] != NULL; i++) {
							printf("%s ", curr->desc[i]);
						}
						printf("\n");
						fflush(stdout);
						free(curr->desc);
						free(curr);
					}
				}

				struct job_list* hold = head;
				while(head != NULL) {
					printf("[%i] ", head->jobid);
					if(!(head->nextptr)) {
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
					} else if(head->status == 2) {
						printf("Done\t");
					}
					for(int i = 0; head->desc[i] != NULL; i++) {
						printf("%s ", head->desc[i]);
					}
					printf("\n");
					fflush(stdout);
					head = head->nextptr;
				}
				head = hold;
				continue;
			} else if(strcmp(parsedcmd[0], "bg") == 0) {
				struct job_list* top = head;
				while(top->nextptr) {
					top = top->nextptr;
				}
				while(top) {
						kill(top->cpid_l, SIGCONT);
						printf("[%i]", top->jobid);
						printf("+ ");
						for(int i = 0; top->desc[i] != NULL; i++) {
							printf("%s ", top->desc[i]);
						}
						printf("&");
						printf("\n");
						waitpid(-1 * top->cpid_l, &status, WNOHANG);
						break;
				}
				continue;			
			} else {
				cpid_l = fork();
				setpgid(cpid_l, cpid_l);
				tcsetpgrp(0, cpid_l);
				if (cpid_l == 0){
					//Child
					signal(SIGINT, SIG_DFL);
					signal(SIGTTIN, SIG_DFL);
					signal(SIGTSTP, SIG_DFL);
					signal(SIGTTOU, SIG_IGN);
					ret=execvp(parsedcmd[0], parsedcmd);
					if (ret ==-1){
						//fprintf(stderr, "Bad command\n");
						break;
					}
				} else {
					waitpid(-1 * cpid_l, &status, WUNTRACED);
					tcsetpgrp(0, p_pid);
					if(WIFSTOPPED(status)) {
						struct job_list *job = (struct job_list*) malloc(sizeof(struct job_list));
						job->status = 0;
						job->cpid_l = cpid_l;
						job->desc = parsedcmd;
						job->nextptr = NULL;
						int count = 0;
						struct job_list* hold = head;
						while(head != NULL) {
							count = head->jobid;
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
			}
		}
    }
    exit(1);
}