#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]){
    if (argc < 2) {
        fprintf(2, "usage: xargs <command> [args...]\n");
        exit(1);
    }
    int arg_cnt = 0;
    int cur = 0;
    char c;
    char buffer[32];
    char *p = buffer;
    char *arg_list[32];
    for(int i = 1; i < argc; i++){
        arg_list[arg_cnt++] = argv[i]; // 保存参数，包括要执行的命令以及命令的参数等。
    }
    while(read(0, &c, sizeof(c)) > 0){
        if(c == '\n'){
            buffer[cur] = 0;
            arg_list[arg_cnt++] = p;

            p = buffer;
            cur = 0;
            arg_list[arg_cnt] = 0;
            arg_cnt = argc - 1; // 记得要保留命令行里传入的参数

            if(fork() == 0){
                exec(argv[1], arg_list);
            }
            wait(0);
        }else if(c == ' ') {
            buffer[cur++] = 0;
            arg_list[arg_cnt++] = p;
            p = &buffer[cur];
        }else {
            buffer[cur++] = c;
        }
    }
    exit(0);
}