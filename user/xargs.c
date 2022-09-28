#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

#define MAXBUF 1024
#define MAXN 1024

int
main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(2, "usage: xargs command\n");
        exit(1);
    }
    char * argv_exec[MAXARG];
    int index=0;
    for (int i = 1; i < argc; i++) {
        argv_exec[index++] = argv[i];
    }
    char buf[MAXBUF];
    while(read(0, buf, MAXBUF)) {
		char new_argv[MAXBUF];
        argv_exec[index] = new_argv;

        // 获得buf中除去换行符的有效命令参数
        for(int i = 0; i <strlen(buf);i++){
            if(buf[i]!='\n'){
                new_argv[i]=buf[i];
            }
            //换行一次，表明需要执行一次命令
            else{
                // 将新的参数追加到之前的参数末端
                if(fork()==0){
                    exec(argv[1],argv_exec);
                }
                wait(0);
            }
        }
        
    }
    exit(0);
}