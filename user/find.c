#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

/*
	将路径格式化为文件名
*/
char* fmt_name(char *path){
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--);
  p++;
  memmove(buf, p, strlen(p)+1);
  return buf;
}
/*
	系统文件名与要查找的文件名，若一致，打印系统文件完整路径
*/
void eq_print(char *fileName, char *findName){
	if(strcmp(fmt_name(fileName), findName) == 0){
		printf("%s\n", fileName);
	}
}
/*
	在某路径中查找某文件
*/
void find(char *path, char *findName){
	int fd;
	struct stat st;	
	if((fd = open(path, O_RDONLY)) < 0){
		fprintf(2, "find: cannot open %s\n", path);
		return;
	}
	if(fstat(fd, &st) < 0){
		fprintf(2, "find: cannot stat %s\n", path);
		close(fd);
		return;
	}
	char buf[512], *p;	
	struct dirent de;
	switch(st.type){	
		case T_FILE:
			eq_print(path, findName);			
			break;
		case T_DIR:
			if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
				printf("find: path too long\n");
				break;
			}
			strcpy(buf, path);
			p = buf+strlen(buf);
			*p++ = '/';
			while(read(fd, &de, sizeof(de)) == sizeof(de)){
				//printf("de.name:%s, de.inum:%d\n", de.name, de.inum);
				if(de.inum == 0 || de.inum == 1 || strcmp(de.name, ".")==0 || strcmp(de.name, "..")==0)
					continue;				
				memmove(p, de.name, strlen(de.name));
				p[strlen(de.name)] = 0;
				find(buf, findName);
			}
			break;
	}
	close(fd);	
}

int main(int argc, char *argv[]){
	if(argc < 3){
		printf("find: find <path> <fileName>\n");
		exit();
	}
	find(argv[1], argv[2]);
	exit();
}





/*这里的核心代码其实就是find函数:find(path,target)

首先我们用open查看该路径指向的文件是（目录也是文件!）否存在,然后用fstat看指向文件的类型，如果是普通文件，那么直接比对名称是否相同（注意，我们比对的不是完整的路径，而是路径中最后一个元素，也就是指向的文件的文件名）

如果是目录文件（directory），那么就需要查看目录项（dirent，directory entry），拼出目录项对应的全部路径，然后递归调用find，即可

另外注意一点，为什么inum为0要跳过，因为inum为0代表该dirent是无效的，或者说没有存放文件的

为什么为0，就无效？这个要看unlink，unlink函数中，将要unlink的dirent，memset为0了*/