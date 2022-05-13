/*
 * Example program demonstrating UNIX pipes.
 *
 * Justin Bradley
 *
 * What are the main file descriptors?
 * 0 = stdin (STDIN_FILENO)
 * 1 = stdout (STDOUT_FILENO)
 * 2 = stderr (STDERR_FILENO)
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define BUFFER_SIZE 25
#define READ_END	0
#define WRITE_END	1
#define ANY_CHILD   0

int main(void)
{
		char write_msg[BUFFER_SIZE] = "Greetings from the parent";
		char read_msg[BUFFER_SIZE];
		pid_t pid;
		int fd[2];
		int child_status;


		char new_msg[BUFFER_SIZE] = "A new message";
		char newnew_msg[BUFFER_SIZE] = "Foobar";

		/* create the pipe */
		if (pipe(fd) == -1) {
				fprintf(stderr,"Pipe failed");
				return 1;
		}

		/* now fork a child process */
		pid = fork();

		if (pid < 0) { // check for failed pipe creation
				fprintf(stderr, "Fork failed");
				return 1;
		}

		if (pid > 0) {  /* parent process */
				/* close the unused end of the pipe */
				close(fd[READ_END]);

				/* write to the pipe */
				write(fd[WRITE_END], write_msg, strlen(write_msg)+1);

				/* // write a new message */
				write(fd[WRITE_END], new_msg, strlen(new_msg)+1);

				write(fd[WRITE_END], newnew_msg, strlen(newnew_msg)+1);
				
				/* close the write end of the pipe */
				close(fd[WRITE_END]);

				waitpid(ANY_CHILD, &child_status, 0); 
		}
		else { /* child process */
				/* close the unused end of the pipe */
				close(fd[WRITE_END]);

				/* read from the pipe */
				read(fd[READ_END], read_msg, strlen(write_msg)+1);
				printf("child read: %s\n",read_msg);
		
				read(fd[READ_END], read_msg, strlen(new_msg)+1);
				printf("child read: %s\n",read_msg);

				read(fd[READ_END], read_msg, strlen(newnew_msg)+1);
				printf("child read: %s\n",read_msg);

				/* close the read end of the pipe */
				close(fd[READ_END]);
		}

		return 0;
}
