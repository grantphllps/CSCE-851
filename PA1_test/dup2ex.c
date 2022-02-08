/****************************************************************
 *
 * Author: Justin Bradley
 * Title: dup2ex.c
 * Date: Tuesday, February  5, 2019
 * Description: Example for dup2()
 *
 * Command line parameters: name of file
 *
 * What are the main file descriptors?
 * 0 = stdin (STDIN_FILENO)
 * 1 = stdout (STDOUT_FILENO)
 * 2 = stderr (STDERR_FILENO)
 *
 ****************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
		int pid, status;
		int fd;	/* new file descriptor */

		if (argc != 2) {
				fprintf(stderr, "usage: %s output_file\n", argv[0]);
				exit(1);
		}
		if ((fd = open(argv[1], O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0) {
				perror(argv[1]);	/* open failed */
				exit(1);
		}

		printf("This goes to the standard output.\n");
		printf("Now the standard output will go to \"%s\".\n", argv[1]);

		/* this new file will become the standard output */
		/* standard output is file descriptor 1, so we use dup2 */
		/* to copy the new file descriptor onto file descriptor 1 */
		/* dup2 will close the current standard output */

		dup2(fd, STDOUT_FILENO);
		close(fd);

		printf("This goes to the standard output too.\n");
		printf("And now any time I print to stdout, it goes to fd\n");
		exit(0);
}
