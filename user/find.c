#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void find(char *path, char *target) {
    int fd;
    if((fd = open(path, 0)) < 0) { // RDONLY，打不开
        fprintf(2, "open %s error\n", path);
        return;
    }
    struct stat st; // file status，包含文件所属的磁盘设备号、inode、文件类型、硬链接数、文件大小
    if(fstat(fd, &st) < 0) {
        fprintf(2, "get %s status error\n", path);
        close(fd);
        return;
    }
    
    struct dirent directory_entry; // directory entry，inum：inode number；name
    char path_tmp[512], *p;
    if(st.type == T_FILE){
        if(strcmp(path + strlen(path) - strlen(target), target) == 0) // 文件名匹配
            printf("%s\n", path);
    } else if(st.type == T_DIR){
        if(strlen(path) + DIRSIZ + 2 > sizeof(path_tmp)){ // '\0' + '/'
            printf("path too long error\n");
            exit(0);
        }
        strcpy(path_tmp, path);
        p = path_tmp + strlen(path_tmp);
        *p++ = '/';
        while(read(fd, &directory_entry, sizeof(directory_entry)) == sizeof(directory_entry)) {
            if(directory_entry.inum == 0) continue; // 该目录项未使用
            memcpy(p, directory_entry.name, DIRSIZ); // 把
            p[DIRSIZ] = 0;
            if(strcmp(path_tmp + strlen(path_tmp) - 2, "/.") != 0 && strcmp(path_tmp + strlen(path_tmp) - 3, "/..") != 0) { // 遇到. 和..不递归
                find(path_tmp, target);
            }
        }
    }
    close(fd);
}

int main(int argc, char *argv[]) {
    if(argc != 3) {
        fprintf(2, "usage : find <directory path> <name>\n");
        exit(-1);
    }    
    char target[512];
    target[0] = '/';
    strcpy(target + 1, argv[2]);
    find(argv[1], target);
    exit(0);
}