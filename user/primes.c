#include "kernel/types.h"
#include "user/user.h"

void dfs(int L[2]) { 
    int p;
    read(L[0], &p, sizeof(p));
    
    if(p == -1) exit(0);
    printf("prime %d\n", p);
    
    int R[2]; 
    pipe(R);
    
    int pid = fork();
    if(pid == 0) { // 由子进程执行递归
        close(R[1]);
        close(L[0]);
        dfs(R); 
    } else { // 由父进程向子进程传递数据
        close(R[0]); 
        int buf;
        while(read(L[0], &buf, sizeof(buf)) && buf != -1) {
            if(buf % p != 0) // 埃筛，把p的倍数筛掉
                write(R[1], &buf, sizeof(buf)); //让它的子进程继续处理
        }
        buf = -1;
        write(R[1], &buf, sizeof(buf));
        wait(0);
    }
    exit(0);
}

int main(int argc, char **argv) {
    int p[2];
    pipe(p);
    
    int pid = fork();
    if(pid == 0) {
        close(p[1]);  // 子进程，关闭写端
        dfs(p);
        exit(0);
    } else {
        close(p[0]); // 父进程，关闭读端
        for(int i = 2;i <= 35;i++) write(p[1], &i, sizeof(int));
        int buf = -1;
        write(p[1], &buf, sizeof(buf));
        wait(0);
        exit(0);
    }
}