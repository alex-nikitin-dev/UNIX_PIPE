#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define countProcesses 10

void ArrayInitialize(int **&ps, size_t count)
{
	ps = new int *[count];
	for (size_t i = 0; i < count; i++)
		ps[i] = new int[2]{0};
}

void DeleteArray(int **&ps, size_t count)
{
	for (size_t i = 0; i < count; i++)
	{
		delete[] ps[i];
	}
	delete[] ps;
	ps = nullptr;
}

bool CreatePipes(int **ps, size_t count)
{
	for (size_t i = 0; i < count; i++)
	{
		if (pipe(ps[i]) != 0)
			return false;
	}
	return true;
}

int main()
{
	int **ps = nullptr;
	int *pid = new int[countProcesses];
	ArrayInitialize(ps, countProcesses - 1);
	if (!CreatePipes(ps, countProcesses - 1))
	{
		printf("pipe error\n");
		return 1;
	}
	int bufIndex = 0;
	char bf[120]{0};

	for (int i = 0; i < countProcesses; i++)
	{
		if ((pid[i] = fork()) == 0)
		{
			//inside in a child process

			//Close every unused pipe
			for (int j = 0; j < countProcesses - 1; j++)
			{
				if (j != i - 1 && j != i)
				{
					close(ps[j][0]);
					close(ps[j][1]);
				}
			}

			bufIndex = 0;
			bf[0] = '\0';
			//if a previos pipe exists
			if (i - 1 >= 0)
			{
				close(ps[i - 1][1]);
				dup2(ps[i - 1][0], 0);
				close(ps[i - 1][0]);
				for (bufIndex = 0; (bf[bufIndex] = getchar()) != EOF; bufIndex++)
					;
				if (bufIndex > 0)
					bf[bufIndex] = '\0';
				fprintf(stderr, "I'm process %i and read  from pipe %i some data: '%s'\n", i, i - 1, bf);
			}

			//if a next pipe exists
			if (i < countProcesses - 1)
			{
				close(ps[i][0]);
				dup2(ps[i][1], 1);
				close(ps[i][1]);
				if (strlen(bf) == 0)
				{
					printf("%i", i);
					fprintf(stderr, "I'm process %i and write to   pipe %i some data: '%i'\n", i, i, i);
				}
				else
				{
					printf("%s %i", bf, i);
					fprintf(stderr, "I'm process %i and write to   pipe %i some data: '%s %i'\n", i, i, bf, i);
				}
			}
			//if this is the last prosess
			if (i == countProcesses - 1)
			{
				printf("\n\nfrom %d: read %lu  '%s'\n\n", getpid(), strlen(bf), bf);
			}
			exit(0);
		}
	}

	//close all pipes
	for (size_t i = 0; i < countProcesses - 1; i++)
	{
		close(ps[i][0]);
		close(ps[i][1]);
	}

	//wait all processes
	for (size_t i = 0; i < countProcesses; i++)
	{
		waitpid(pid[i], NULL, 0);
	}

	DeleteArray(ps, countProcesses - 1);
	delete[] pid;
	pid = nullptr;
	return 0;
}
