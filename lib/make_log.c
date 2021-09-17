#include<stdio.h>
#include<stdarg.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<time.h>
#include<sys/stat.h>

#include"make_log.h"
#include<pthread.h>

#if 1

/*
pthread_mutex_t lock;
pthread_mutex_init(lock);

struct file_path{
    char *module_name;
    char *proc_name;
    const char *filename;
    int line;
    const char *funcname;
    char *fmt;

};

void *comm_log(void *p)
{
    pthread_mutex_lock(&lock);
    pthread_mutex_unlock(&lock);
}

int lock_file(char *module, char *proc, const char *file,
              int lines, const char *func, char *mt, ...)
{
    va_list ap;
    struct file_path path;
    path.module_name = module;
    path.proc_name = proc;
    path.filename = file;
    path.line = lines;
    path.funcname = func;
    //path.fmt  = mt;
    va_start(ap,mt);
    vsprintf(path.fmt,mt,ap);
    va_end(ap);
    pthread_mutex_init(&lock,0); 
    pthread_t comm;
    pthread_create(&comm,0,dumpmsg_to_file,&path);
    pthread_join(comm,0);
    pthread_mutex_destroy(&lock);
}*/

pthread_mutex_t ca_log_lock=PTHREAD_MUTEX_INITIALIZER;

//创建目录并写入内容
int dumpmsg_to_file(char *module_name, char *proc_name, const char *filename,
                    int line, const char *funcname, char *fmt, ...)
{
        char mesg[4096]={0};
        char buf[4096]={0};
	    char filepath[1024] = {0};
        time_t t=0;
        struct tm * now=NULL;                                                                                     
        va_list ap; 

        time(&t);                                                                                                 
        now = localtime(&t);                                       
        va_start(ap, fmt);                                                                               
        vsprintf(mesg, fmt, ap);                                                                       
        va_end(ap);                        

        snprintf(buf, 4096, "===%04d%02d%02d-%02d%02d%02d,%s[%d]=== %s\n",
                 now -> tm_year + 1900, now -> tm_mon + 1, now -> tm_mday, 
                 now -> tm_hour, now -> tm_min, now -> tm_sec,
                 funcname, line, mesg);                                     
        int ret = make_path(filepath, module_name, proc_name);
        if (ret) {
            return ret;
        }
        
        pthread_mutex_lock(&ca_log_lock);
	    out_put_file(filepath, buf);     
        pthread_mutex_unlock(&ca_log_lock);

        return 0;     
}
#endif

//写入内容
int out_put_file(char *path, char *buf)
{
	int fd;                                                                                                   
    fd = open(path, O_RDWR | O_CREAT | O_APPEND, 0755);

    if(write(fd, buf, strlen(buf)) != (int)strlen(buf)) {                                      
        fprintf(stderr, "write error!\n");                           
        close(fd);                                                                                        
    } else {                                                                                                  
        //write(fd, "\n", 1);
        close(fd);                                                                                        
    }               

	return 0;
}

//创建目录
int make_path(char *path, char *module_name, char *proc_name)
{
	time_t t;
	struct tm *now = NULL;
	char top_dir[1024] = {"."};
	char second_dir[1024] = {"./logs"};
    
	time(&t);
    now = localtime(&t);
	snprintf(path, 1024, "./logs/%s-%s.%04d-%02d-%02d.log", module_name, proc_name, now->tm_year + 1900, now->tm_mon + 1, now->tm_mday);
	
	//检查目标目录是否存在，若不存在则创建
    if(access(second_dir, 0) == -1) {
        if(access(top_dir, 0) == -1) {
            if(mkdir(top_dir, 0755) == -1) {
			fprintf(stderr, "create %s failed!\n", top_dir);
            return -1;
            }
        }
        
        if(mkdir(second_dir, 0755) == -1) {
			fprintf(stderr, "create %s failed!\n", second_dir);
            return -2;
		}
    }
	//printf("path:%s\n", path);
	return 0;
}

#if 0
//测试日志写入
int main(void)
{
	char path[1024] = {0};
	char proc_name[] = {"sys_guard"};
	char buf[] = {"12345\n"};
	make_path(path, proc_name);
	out_put_file(path, buf);
	return 0;
} 
#endif
