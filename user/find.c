#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void find(char *path, char *target) {
    int fd;
    if((fd = open(path, 0)) < 0) // RDONLY，打不开
    {
        printf("Error: cannot open %s\n", path);
        return;
    }
    struct stat st; // file status
    if(fstat(fd, &st) < 0)
    {
        printf("Error: cannot stat %s\n", path);
        close(fd);
        return;
    }
    
    struct dirent directory_entry; // directory entry
    char path_tmp[512], *p;
    if(st.type == T_FILE){
        if(strcmp(path + strlen(path) - strlen(target), target) == 0) // 文件名匹配
            printf("%s\n", path);
    } else if(st.type == T_DIR){
        if(strlen(path) + 1 + DIRSIZ + 1 > sizeof(path_tmp)){
            printf("Error: path too long\n");
            exit(0);
        }
        strcpy(path_tmp, path);
        p = path_tmp + strlen(path_tmp);
        *p++ = '/';
        while(read(fd, &directory_entry, sizeof(directory_entry)) == sizeof(directory_entry)) {
            if(directory_entry.inum == 0) continue; // 未使用
            memcpy(p, directory_entry.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if(strcmp(path_tmp + strlen(path_tmp) - 2, "/.") != 0 && strcmp(path_tmp + strlen(path_tmp) - 3, "/..") != 0) {
                find(path_tmp, target);
            }
        }
    }
    close(fd);
}

int main(int argc, char *argv[]) {
    char target[512];
    target[0] = '/';
    strcpy(target+1, argv[2]);
    find(argv[1], target);
    exit(0);
}