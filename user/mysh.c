//Thamchareonkit Wikrom 72075605
//Shell with support for redirection and piping

#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define BUFSIZE 256
#define MAX_ARGC 10
#define MAX_ARGSIZE 10
#define MAX_CMDC 10 // maximum count for cmd that are chained together

struct cmd {
    int argc;
    char operator;
    char* argv[MAX_ARGC]; 
}; 

//header since this function would be used recursively 
void runcmd(struct cmd[MAX_CMDC], int, int);

int myfork() {
    int pid;

    if((pid = fork()) == -1) {
        fprintf(2, "mysh: fork failed\n");
    }
    return pid;
}


//naive implementation of dup2, prone to racing condition
int mydup2(int fd, int fd2) {
    close(fd2);
    return dup(fd);
}

int getline(char *buf, int bufsize) {
    write(2, "mysh $ ", 8);
    memset(buf, 0, bufsize);
    gets(buf, bufsize);

    if(buf[0] == 0) //EOF
        return -1;

    return 0;
}

//get arg until space
int getarg(char *buf, char *arg, int argsize) {
    //EOL
    if(buf[0]==0 || buf[0]=='\n')
        return 1;

    if(buf[0]==';' || buf[0]=='>' || buf[0]=='|')
        return 2;
    
    //remove starting spaces
    while(buf[0]==' ') {
        memmove(buf, buf+1, strlen(buf));
    }

    for(int i = 0; i < argsize; i++) {
        if(buf[0]==' ' || buf[0]=='\n' || buf[0]==0) {
            //remove it from the buffer before returning
            arg[i] = '\0'; 
            memmove(buf, buf+1, strlen(buf));
            break;
        }
        else if(buf[0]==';' || buf[0]=='>' || buf[0]=='|')
            return 2;

        arg[i] = buf[0];
        memmove(buf, buf+1, strlen(buf));
    }
    return 0;
}

int getargv(char *buf, struct cmd *cmd_ptr, int max_argc, int argsize) {
    cmd_ptr->operator = 0; //clear operator just in case
    
    int argc = 0; 
    int retval;

    //EOL
    if(buf[0]==0 || buf[0]=='\n')
        return 1;

    for(int i=0; i<max_argc; i++) {
        cmd_ptr->argv[i] = malloc(argsize+1);

        //break early since we reach EOL
        if((retval = getarg(buf, cmd_ptr->argv[i], argsize)) == 1) {
            free(cmd_ptr->argv[i]);
            cmd_ptr->argv[i] = 0;
            break;
        }
        //get the operator
        else if(retval == 2) {
            cmd_ptr->operator = buf[0];
            memmove(buf, buf+1, strlen(buf));
            break;
        }
        argc++;
    }
    cmd_ptr->argc = argc;
    return 0;
}

// somehow this doesn't fully clear the argv array...
// causing some bugs with the output of commands running after

void free_argv(char **argv, int argc) {
    for (int i=0; i < argc; i++) {
        free(argv[i]);
        argv[i] = 0;
    }
}

void clear_cmd(struct cmd *cmd_ptr) {
    free_argv(cmd_ptr->argv, cmd_ptr->argc);
    cmd_ptr->argc = 0;
    cmd_ptr->operator = 0;
}

int check_special_cmd(char** argv) {
    int retval = 0;

    if(!strcmp(argv[0], "exit")) {
        exit(0);
    }

    else if(!strcmp(argv[0], "cd")) {
        //cd needs to be called by the parent
        if((retval = chdir(argv[1])) < 0)
            fprintf(2, "mysh: cd to %s failed\n", argv[1]);
    }
    return retval;
}

int cmdexec(struct cmd cmd) {
    if(exec(cmd.argv[0], cmd.argv) != 0) {
        fprintf(2, "mysh: exec %s failed\n", cmd.argv[0]);
        return 1;
    }    
    return 0;
}

void runcmd(struct cmd cmd_chain[MAX_CMDC], int cmdc, int curr_index) {
    if(curr_index >= cmdc) {
        return;
    }

    int file;
    int pipe_fd[2];

    switch(cmd_chain[curr_index].operator) {
        case '>':
            //output to file
            if(curr_index+1 >= cmdc) {
                fprintf(2, "mysh: no output path specified\n");
                return;
            }

            if((file = open(cmd_chain[curr_index+1].argv[0], O_WRONLY | O_CREATE | O_TRUNC)) < 0) {
                fprintf(2, "mysh: open file failed\n");
                return;
            }

            if(myfork() == 0) {
                mydup2(file, 1);
                cmdexec(cmd_chain[curr_index]);
            }
            wait(0);
            close(file);

            runcmd(cmd_chain, cmdc, curr_index+2);
            break;

        case '|':
            //pipe to another command
            if(pipe(pipe_fd) < 0) {
                fprintf(2, "mysh: pipe failed\n");
                return;
            }

            if(myfork() == 0) {
                //close read since we only need write
                close(pipe_fd[0]);

                //redirection of stdout
                mydup2(pipe_fd[1],1);
                close(pipe_fd[1]);

                cmdexec(cmd_chain[curr_index]);

            }

            if(myfork() == 0) {
                //close write since we only need read
                close(pipe_fd[1]);

                //redirection of stdin
                mydup2(pipe_fd[0],0);
                close(pipe_fd[0]);
                
                runcmd(cmd_chain, cmdc, curr_index+1);
            }

            close(pipe_fd[0]);
            close(pipe_fd[1]);
            while(wait(0) > 0);
            break;

        default:
            if(myfork() == 0) {
                cmdexec(cmd_chain[curr_index]);
            }
            wait(0);

            runcmd(cmd_chain, cmdc, curr_index+1);
            break;
    }
}

int main(void)
{
    static char buf[BUFSIZE];
    
    struct cmd cmd_chain[MAX_CMDC];
    int cmdc;

    while(getline(buf, sizeof(buf)) == 0) {

        //get all the commands
        cmdc = 0;
        for(int i = 0; i < MAX_CMDC; i++) {
            if(getargv(buf, &cmd_chain[i], MAX_ARGC, MAX_ARGSIZE) == 1) {
                cmdc = i;
                break;
            }
        }

        //run all the commands
        if(myfork() == 0)
            runcmd(cmd_chain, cmdc, 0);
        wait(0);

        for(int k = 0; k < cmdc; k++) {
            clear_cmd(&cmd_chain[k]);
        }
    }
    exit(0);
}