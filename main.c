#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>


int main(int argc, char **argv){

    int cpid, ret;
    char *cmd;

    while (1){
		cmd = readline("> ");
		cpid = fork();
		if (cmd == NULL) break;
		if (cpid ==0){
			//Child
			ret=execlp(cmd, cmd, NULL);
			if (ret ==-1){
				printf("Bad command\n");
			}
		}
		wait(NULL);
    }
    printf("done\n");
    exit(1);
}