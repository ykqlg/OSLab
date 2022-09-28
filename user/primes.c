#include "kernel/types.h"
#include "user.h"

void get_fd_0(int p[2]){
    close(0);
    dup(p[0]);
    close(p[0]);
    close(p[1]);
}

void get_fd_1(int p[2]){
    close(1);
    dup(p[1]);
    close(p[0]);
    close(p[1]);
}

void prime(){
    int p[2];
    int first,later;
    int pid_child;
    
    if(read(0,&first,4)){
        printf("prime %d\n",first);
        pipe(p);
        if((pid_child = fork()) == 0){
            get_fd_1(p);
            while(read(0,&later,4)){
                if(later % first != 0){
                    write(1,&later,4);
                }
            }
            exit(0);
        }
        else{
            wait(0);
            get_fd_0(p);
            prime();
        }
    }
    exit(0);


}

int main(int argc,char *argv[]){
    int pid_child;
    int p[2];
    pipe(p);
    if((pid_child = fork())==0){
        // printf("yes?");
        get_fd_1(p);
        for(int i = 2;i<36;i++){
            write(1,&i,4);
        }
        exit(0);
    }
    else{
        wait(0);
        get_fd_0(p);
        prime();
    }
    exit(0); 
}

