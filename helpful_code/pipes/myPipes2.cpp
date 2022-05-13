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

        std::cout << "Child Process for ls" << std::endl;
        //Create the command arguments
        const char **args = new const char * [4];
        args[0] = "ls";
        args[1] = NULL;


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

        std::cout << "Child Process for cat" << std::endl;

        //close the write end of the pipe
        close(fdp[writeEnd]);

        int fdp2[2];
        pipe(fdp2);

        //New Child Process
        pid = fork();

        if (pid == 0) {
            //Create the command
            const char **args = new const char * [4];
            args[0] = "cat";
            args[1] = NULL;

            close(fdp2[readEnd]);

            //Redirect the input of command
            dup2(fdp[readEnd],STDIN_FILENO);
            dup2(fdp2[writeEnd],STDOUT_FILENO);

            //close the readEnd of the Pipe
            close(fdp[readEnd]);
            close(fdp2[writeEnd]);

            //execute the command
            execvp("cat", (char **)args);

            exit(0);
            }

        else {
            //Still the Parent

            std::cout << "Child Process for grep" << std::endl;

            close(fdp2[writeEnd]);
            pid = fork();

            if (pid == 0) {
                //Create the command
                const char **args = new const char * [4];
                args[0] = "grep";
                args[1] = "pipes";
                args[3] = NULL;

                dup2(fdp2[readEnd],STDIN_FILENO);

                close(fdp2[readEnd]);
                execvp("grep", (char **)args);

                exit(0);


            }

            
        int waitforpid = wait(&status);
        waitforpid = wait(&status);
        waitforpid = wait(&status);
        }

    }

}