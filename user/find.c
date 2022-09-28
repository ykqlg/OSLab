#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void
find(char *path, char *file)
{   
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, 0)) < 0){
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch(st.type){
        case T_DIR:
            if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
            {
                fprintf(2, "find: directory'path too long\n");
                close(fd);
                return;
            }
            // 将绝对路径path复制到缓冲区以便后面多次操作
            strcpy(buf, path);
            // 利用指针p对buf进行操作，即在末端加入/，以供后面循环遍历时再更新更深的路径给buf
            p = buf+strlen(buf);
            *p++ = '/';
             //从fd中一个个读取文件中的内容信息
            while (read(fd, &de, sizeof(de)) == sizeof(de))
            {
                if(de.inum == 0)
                    continue;
                // 不要递归进入"." 和 ".."
                if (strcmp(de.name, ".")==0 || strcmp(de.name, "..")==0)
                    continue;
                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = 0;
                if(stat(buf, &st) < 0)
                {
                    fprintf(2, "find: stat failed %s\n", buf);
                    continue;
                }
                // 若是文件目录类型，进入递归
                if (st.type == T_DIR)
                {
                    find(buf, file);
                }
                // 若是文件类型，则与目标文件名file进行匹配判断
                else if (st.type == T_FILE && strcmp(de.name, file)==0)
                {
                    printf("%s\n", buf);
                } 
            }
            break;
        case T_FILE:
            fprintf(2, "find: %s is not a directory\n", path);
            close(fd);
            return;
        case T_DEVICE:
            fprintf(2, "find: %s is not a directory\n", path);
            close(fd);
            return;
        default:
            fprintf(2, "find: %s is not a directory\n", path);
            close(fd);
            return;
    }
   
}

int
main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(2, "find's argc is wrong\n");
        exit(1);
    }
    find(argv[1], argv[2]);
    exit(0);
}