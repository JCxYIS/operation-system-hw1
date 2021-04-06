#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>

int main(void)
{	
	printf("GETPID=%d\n", getpid());

	// input mode!
	char mode = -1;
	while( !(mode == 'c' || mode == 'p') )
	{
		printf("Should child or parent run first? [c/p]: ");
		scanf("%c", &mode);
	}

	// let's fork!
	pid_t pid = fork();
    char *parmList[] = {"firefox", "http://140.115.51.157:8763/fork/", NULL};
	int exit_code;	
   	switch(pid)
   	{
		case -1: // -1 is uncool...
			perror("OH NO!!! BAD FORK!!!!!");
			return 1;

		case 0: // 0 is child (new fork)
			if(mode == 'p')
   				waitpid(getppid(), &exit_code, 0); // wait the parent process done

			printf("I'm Child!  PID=%d | GETPID=%d | PARENT_RETURN_CODE=%d\n", pid, getppid(), exit_code);
            for(int i = 3; i > 0; i--)
            {
                printf("    Wait %d seconds...\n", i);
                sleep(1);
            }
			exit(0);
			break;

		default: // >0 is parent (original process)			
			if(mode == 'c')
   				waitpid(pid, &exit_code, 0); // wait the child process done

			printf("I'm Parent! PID=%d | GETPID=%d | CHILD_RETURN_CODE=%d\n", pid, getpid(), exit_code);
            printf("    Open Browser!\n");
            execvp("/usr/bin/firefox", parmList);
			exit(0); // will never come here
			break;
	}

	return 0;
}
