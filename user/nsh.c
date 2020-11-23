#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"
 
void execPipe(char*argv[],int argc);
//*****************START  from sh.c *******************
#define MAXARGS 10
#define MAXWORD 30
#define MAXLINE 100
 
int getcmd(char *buf, int nbuf)
{
    memset(buf, 0, nbuf);
    gets(buf, nbuf);
    if (buf[0] == 0) // EOF
        return -1;
    return 0;
}
char whitespace[] = " \t\r\n\v";
char args[MAXARGS][MAXWORD];
//*****************END  from sh.c ******************
//首先我们先要去掉输入格式中的空格。
void setargs(char *cmd, char* argv[],int* argc)
{
    // 让argv的每一个元素都指向args的每一行
    for(int i=0;i<MAXARGS;i++){
        argv[i]=&args[i][0];
    }
    int i = 0; // 表示第i个word
    int j = 0; // 表示word中第j个char
    for (; cmd[j] != '\n' && cmd[j] != '\0'; j++)
    {
        // 每一轮循环都是找到输入的命令中的一个word，比如 echo hello world ,就是先找到echo，再找到hello和world
        // 让argv[i]分别指向他们的开头，并且将echo，hello，world后面的空格设为\0。 
        while (strchr(whitespace,cmd[j])){ //跳过之前的空格
            j++;
        }
        //将有用的参数存入argv
        argv[i++]=cmd+j;
        //只要不是空格，就j++,找到下一个空格
        while (strchr(whitespace,cmd[j])==0){
            j++;
        }
        cmd[j]='\0';
    }
    argv[i]=0;
    *argc=i;
}
 
// void runcmd(char *cmd)
void runcmd(char*argv[],int argc)
{
    for(int i=1;i<argc;i++){
        if(!strcmp(argv[i],"|")){
            // 如果遇到 | 即pipe，至少说明后面还有一个命令要执行
            execPipe(argv,argc);
        }
    }
    // 此时是仅处理一个命令：现在判断argv[1]开始，后面有没有> 
    for(int i=1;i<argc;i++){
        // 如果遇到 > ，说明需要执行输出重定向，首先需要关闭stdout
        if(!strcmp(argv[i],">")){
            if(argc != i){
                close(1);
                // 此时需要把输出重定向到后面给出的文件名对应的文件里
                open(argv[i+1],O_CREATE|O_WRONLY);
                argv[i]=0;
            }
            else{
                // 当然如果>是最后一个，那就会error
                fprintf(2,"%s\n","redirect error");
            }
        }
        if(!strcmp(argv[i],"<")){
            // 如果遇到< ,需要执行输入重定向，关闭stdin
            if(argc != i){
                close(0);
                open(argv[i+1],O_RDONLY);
                argv[i]=0;
            }
            else {
                fprintf(2,"%s\n","redirect error");
            }
        }
    }
    //exec(argv[0], argv);
}
 
void execPipe(char*argv[],int argc){
    int i=0;
    // 首先找到命令中的"|",然后把他换成'\0'
    for(;i<argc;i++){ // 从前到后，找到第一个就停止，后面都递归调用
        if(!strcmp(argv[i],"|")){
            argv[i]=0;
            break;
        }
    }
    // 先考虑最简单的情况：cat file | wc
    int pd[2];
    pipe(pd);
    if(fork()==0){
        // 子进程 //把自己的标准输出关闭,执行左边的命令 
        close(1);
        dup(pd[1]);
        close(pd[0]);
        close(pd[1]);
        // exec(argv[0],argv);
        runcmd(argv,i);
    }else{
        // 父进程 //把自己的标准输入关闭,执行右边的命令 
        close(0);
        dup(pd[0]);
        close(pd[0]);
        close(pd[1]);
        // exec(argv[i+1],argv+i+1);
        runcmd(argv+i+1,argc-i-1);
    }
}
int main()
{
    char buf[MAXLINE];
    // Read and run input commands.
    while (getcmd(buf, sizeof(buf)) >= 0)
    {
        if (fork() == 0)
        {
            char* argv[MAXARGS];
            int argc=-1;
            setargs(buf, argv,&argc);//去掉空格
            runcmd(argv,argc);
        }
        wait(0);
    }
    exit(0);
}