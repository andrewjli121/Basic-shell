#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "parser.c"

int main(int argc, char **argv){

    int cpid, ret, fd;
	int ps_flag = 0;
    char *cmd;
	char **parsedcmd;
	char **ps_parse;

    while (cmd = readline("> ")) {
		ps_flag = 0;
		parsedcmd = parse(cmd, ' ', ps_parse, &ps_flag);
		if(ps_flag) {
			for(int i = 0; ps_parse[i] != NULL; i++) {
				printf("%i\n", i);
			}
		}
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