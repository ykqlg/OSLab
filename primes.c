#include "kernel/types.h"
#include "user.h"

void divide_part(int rfd,int wfd,int cnt){
    int new_p[2];
    pipe(new_p);
    int new_rfd = dup(new_p[0]);
    int new_wfd = dup(new_p[1]);
    close(new_p[0]);
    close(new_p[1]);
    int keep[cnt];
    int first,later;
    int pid_child;
    int newCnt = 0;
    if((pid_child = fork()) == 0){
        close(wfd);
        read(rfd,first,4);
        printf("prime %d",first);
        for(int i = 1; i<cnt ; i++){
            read(rfd,later,4)
            if(later%first != 0){
                keep[newCnt++]=later;
                close
                write(wfd,later,4);
                newCnt++;
            }
        }
        close(rfd);
        exit(0);
    }
    else{
        wait(0);
        divide_part(new_rfd,newCnt);
        close(new_rfd);
    }
    
}

int main(int argc,char* argv[]){
    int pipeNum = 10;
    int p[pipeNum][2];
    pipe(p1);
    int pid_child,pid_parent;
    pid_child1
    
    int p[2];
    pipe(p);

    if((pid_child = fork())==0){
        // for(int i = 0;i<pipeNum;i++){
        //     close(p[i][1]);
        //     read(p[i][0],buf,4);
        //     close(p[i][0]);
        //     fork()
        // }
        close(0);
        dup(p[1]);
        close(p[0]);
        for(int i = 2;i<36;i++){
            write(0,1,4);
        }
        close(p[1]);
        
    }
    else{
        int fd = dup(p[0][1]);
        close(p[0][0]);
        for(int i = 2;i<36;i++){
            write(fd,1,4);
        }
        close(p[0][1]);
        
    }
    exit(0); //确保进程退出
}