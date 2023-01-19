#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "parser.c"

int main(int argc, char **argv){

    int cpid, ret;
    char *cmd;
	char **parsedcmd;

    while (cmd = readline("> ")) {

		parsedcmd = parse(cmd, ' ');
		cpid = fork();
		if (cpid ==0){
			//Child
			ret=execvp(parsedcmd[0], parsedcmd);
			if (ret ==-1){
				printf("Bad command\n");
			}
		} else{
			wait((int *)NULL);
		}
    }
    printf("done\n");
    exit(1);
}