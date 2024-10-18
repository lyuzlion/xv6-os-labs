#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char* argv[]) {

	int pipe1[2]; // 子进程向父进程写
	int pipe2[2]; // 父进程向子进程写

	pipe(pipe1);
	pipe(pipe2);

	int pid = fork();
	if (pid < 0) {
		fprintf(2, "fork error.");
		exit(-1);
	}
	
	char data = '5';

	if (pid == 0) { // 子进程

		if (read(pipe2[0], &data, sizeof(char)) != sizeof(char)) {
			fprintf(2, "child read error.");
			exit(-1);
		} else {
			fprintf(1, "%d: received ping\n", getpid());
		}

		if (write(pipe1[1], &data, sizeof(char)) != sizeof(char)) {
			fprintf(2, "child write error.");
			exit(-1);
		}

		exit(0);
	} else { // 父进程
		if (write(pipe2[1], &data, sizeof(char)) != sizeof(char)) {
			fprintf(2, "parent write error.");
			exit(1);
		}
		// 等待子进程写
		if (read(pipe1[0], &data, sizeof(char)) != sizeof(char)) {
			fprintf(2, "parent read error.");
			exit(-1);
		} else {
			fprintf(1, "%d: received pong\n", getpid());
		}

		wait(0);
		exit(0);
	}
}

