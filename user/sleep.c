#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(2, "usage: sleep <number of ticks>\n"); // tick: 两次定时器中断的时间间隔。先进入到user/printf.c然后调用系统调用
		exit(-1);
	}
	sleep(atoi(argv[1])); // sleep 的实现位于 kernel/proc.c 该文件主要实现了进程管理相关的工作
	exit(0);	
}

