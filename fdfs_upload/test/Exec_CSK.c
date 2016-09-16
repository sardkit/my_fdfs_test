#include "make_log.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
	LOG("test", "demo",  "main begin\n");
	signal(SIGPIPE, SIG_IGN); //忽略管道破裂
	if(argc !=2)
	{
		LOG("test", "demo",  "argc is not 2\n");
		return -1;
	}

	int pfd[2] = {0};
	int pid = 0;

	pipe(pfd);

	pid = fork();
	if(0 == pid)
	{
		LOG("test", "demo",  "child process begin\n");
		close(pfd[0]);
		dup2(pfd[1], 1);  //重定向标准输出
		sleep(1);
		LOG("test", "demo", "child process end\n");
		execlp("fdfs_upload_file", "fdfs_upload_file", "/etc/fdfs/client.conf", argv[1], NULL);  //执行exec
	}
	else
	{
		LOG("test", "demo",  "origin process begin\n");
		close(pfd[1]);
		char buf[1024] = {0};
		read(pfd[0], buf, 1024);
		printf("data : %s\n", buf);
		LOG("test", "demo",  "data : %s\n", buf);

		LOG("test", "demo",  "origin process end\n");
	}
	wait(NULL);

	LOG("test", "demo",  "main end\n");
	return 0;
}
