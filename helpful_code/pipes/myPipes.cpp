#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

int main() {
    int writeEnd = 1;
    int readEnd = 0;
    int fdp[2];
    int status = 0;
    pipe(fdp);

    int pid = fork();

    if (pid == 0) {
        //The Child

        //Create the command arguments
        const char **args = new const char * [4];
        args[0] = "ls";
        args[1] = "-l";
        args[3] = NULL;


        //Close the read end
        close(fdp[readEnd]);

        //Redirect the output to the write end of the pipe
        dup2(fdp[writeEnd],STDOUT_FILENO);

        //close the write end
        close(fdp[writeEnd]);

        //execute the command
        execvp("ls", (char **)args);

        //set the exit status
        exit(0);


    }
    else {
        //The Parent

        //close the write end of the pipe
        close(fdp[writeEnd]);

        //New Child Process
        int pid_2 = fork();

        if (pid_2 == 0) {
            //Create the command
            const char **args = new const char * [4];
            args[0] = "grep";
            args[1] = "pipes";
            args[3] = NULL;

            //Redirect the input of command
            dup2(fdp[readEnd],STDIN_FILENO);

            //close the readEnd of the Pipe
            close(fdp[readEnd]);

            //execute the command
            execvp("grep", (char **)args);

            exit(0);
            }

        else {
            //Still the Parent
            
        int waitforpid = wait(&status);
        waitforpid = wait(&status);
        }

    }

}