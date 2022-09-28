#include "kernel/types.h"
#include "user.h"

int main(int argc,char* argv[]){
    int p1[2];
    int p2[2];
    pipe(p1);
    pipe(p2);
    char buf[4];
    int n = 4;
    int ret = fork();
    if (ret == 0) { 
        /* 子进程 */
        int pid_child = getpid();
        close(p1[0]);
        write(p1[1],"pong",n);
        close(p1[1]);

        close(p2[1]); // 关闭写端
        read(p2[0],buf , n);
        close(p2[0]); // 读取完成，关闭读端
        printf("%d: received %s\n",pid_child,buf);

        // exit(0);
    } else if (ret>0) { 
        /* 父进程 */
        int pid_parent = getpid();
        close(p2[0]); // 关闭读端
        write(p2[1], "ping", n);
        close(p2[1]); // 写入完成，关闭写端
    
        wait(0);
        close(p1[1]);
        read(p1[0],buf,n);
        close(p1[0]);
        
        printf("%d: received %s\n",pid_parent,buf);


    }
    exit(0);
}