#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

void sighandler(int signum) {
    if(signum == 2) {
        //SIGINT
        
    } else if(signum == 18) {
        //SIGTSTP
    } else if(signum == 20) {
        //SIGCHLD
    }
}