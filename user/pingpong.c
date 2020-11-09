#include <kernel/types.h>
#include <user/user.h>
/*
 xv6可运行
 chapter01: ping pong练习程序
*/
int main(){
    //pipe1(p1)：写端父进程，读端子进程
    //pipe2(p2)；写端子进程，读端父进程
    int p1[2],p2[2];
    //来回传输的字符数组
    char buffer[64];
    //传输字符数组的长度
    long length = sizeof(buffer);
    //父进程写，子进程读的pipe
    pipe(p1);
    //子进程写，父进程读的pipe
    pipe(p2);
    //子进程
    if(fork() == 0){
        //关掉不用的p1[1]、p2[0]
        close(p1[1]);
        close(p2[0]);
		//子进程从pipe1的读端，读取字符数组
		if(read(p1[0], buffer, length) != length){
			printf("a--->b read error!");
			exit();
		}
		//打印读取到的字符数组
		printf("%d: received %s\n", getpid(),buffer);
		//子进程向pipe2的写端，写入字符数组
		if(write(p2[1], "pong", length) != length){
			printf("a<---b write error!");
			exit();
		}
        exit();
    }
    //关掉不用的p1[0]、p2[1]
    close(p1[0]);
    close(p2[1]);
	//父进程向pipe1的写端，写入字符数组
	if(write(p1[1],"ping", length) != length){
		printf("a--->b write error!");
		exit();
	}
	//父进程从pipe2的读端，读取字符数组
	if(read(p2[0], buffer, length) != length){
		printf("a<---b read error!");
		exit();
	}
	//打印读取的字符数组
	printf("%d: received %s\n", getpid(),buffer);
    //等待进程子退出
    wait();
	exit();
}
