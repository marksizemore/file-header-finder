#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>

#include "headers.h"

/* reads in 512 bytes of the disk image at a time, typically this is
   the size of a sector. Executes the 'file' command in a child process
   to identify the file type and therefore locate the sector containing
   the file header */
void
find_headers(char *image_path)
{
	//open our disk image to be processed
	int image_fd = open(image_path, O_RDONLY);
	uint8_t sector_data[512];
	
	int bytes;	
	int i = 0;
	//read in 512 bytes at a time, traditionally this is the size of a sector
	while(bytes = read(image_fd, sector_data, 512) > 0)
	{
		
		char *file_info = malloc(512);
		int pipe_fd[2];
		
		pipe(pipe_fd);

		//create new file in working directory, pass as arg to execl
		int sector_fd = open("sector", O_RDWR | O_CREAT, 0777);
		
		/*write the sectore data to a hard disk file so we can 
		execute the 'file' command on it*/
		write(sector_fd, sector_data, 512);

		int pid = fork();
		if(pid == 0)
		{
			/*close the read end of the pipe, it won't be needed in 
			the child process*/
			close(pipe_fd[0]);

			dup2(pipe_fd[1], 1);//redirects stdout to the pipe
			execlp("file", "file", "sector", (char *) NULL);
		}
		read(pipe_fd[0], file_info, 512);
		close(pipe_fd[0]);
		
		//trim the first part of the string which lists the file name
		int j;
		for(j = 0; j < 8; j++)
		{
			file_info++;
		}
		
		char compare[4];
		
		strncpy(compare, file_info, 4);
		
		/*if the file command indicates that this sector is binary data, 
		 do not display, we are only looking for headers*/
		if(strncmp(compare, "data", 4) != 0)
		{
			printf("sector %d: %s", i, file_info);
		}

		i++;
	}
	//delete the last sector file which is in the current directory
	file_cleanup();
}

/* the last sector is still written to a hard disk file in the current
 * directory, this function deletes it */
void
file_cleanup()
{
	//run the 'rm' command in a child process, no need to send data to the parent
	int pid = fork();
	if(pid == 0)
	{
		execlp("rm", "rm", "sector", (char *) NULL);
	}
}
