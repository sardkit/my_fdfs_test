/*
 * echo.c --
 *
 *	Produce a page containing all FastCGI inputs
 *
 *
 * Copyright (c) 1996 Open Market, Inc.
 *
 * See the file "LICENSE.TERMS" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */
#ifndef lint
static const char rcsid[] = "$Id: echo.c,v 1.5 1999/07/28 00:29:37 roberts Exp $";
#endif /* not lint */

#include "fcgi_config.h"

#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef _WIN32
#include <process.h>
#else
extern char **environ;
#endif

#include <time.h>
#include "fcgi_stdio.h"
#include "handleData.h"
#include "upload_op.h"
#include "redis_op.h"
#include "make_log.h"

#define TAG 		"CGI"
#define FILENAME 	"echo"
#define FIELD_ID_SIZE             128            /* redis hash表field域字段长度 */
#define VALUES_ID_SIZE           1024			/* redis        value域字段长度 */


static void PrintEnv(char *label, char **envp)
{
    printf("%s:<br>\n<pre>\n", label);
    for ( ; *envp != NULL; envp++) {
        printf("%s\n", *envp);
    }
    printf("</pre><p>\n");
}

int write_to_database(char (*file_id)[FIELD_ID_SIZE], char (*file_name)[VALUES_ID_SIZE])
{
	int ret = 0;
	LOG(TAG, FILENAME, "write_to_database begin----");
	redisContext *conn = rop_connectdb_nopwd("127.0.0.1", "6379");
	if(conn == NULL)
	{
		LOG(TAG, FILENAME, "rop_connectdb_nopwd error;");
		return -1;
	}
	LOG(TAG, FILENAME, "rop_connectdb_nopwd end----");
	
	ret = rop_hash_set_append(conn, "FILE_INFO_HASHE", file_id, 
								file_name, 1);
	if(ret != 0)
	{
		LOG(TAG, FILENAME, "rop_hash_set_append error;");
		goto END;
	}
	LOG(TAG, FILENAME, "rop_hash_set_append end----");
	
	char mfile_id[VALUES_ID_SIZE];
	memcpy(mfile_id, file_id[0], FIELD_ID_SIZE);
	ret = rop_zset_increment_append(conn, "FILE_HOT_ZSET", &mfile_id, 1);
	if(ret != 0)
	{
		LOG(TAG, FILENAME, "rop_zset_increment_append error;");
		goto END;
	}
	LOG(TAG, FILENAME, "rop_hash_set_append end----");
	
END:
	rop_disconnect(conn);
	LOG(TAG, FILENAME, "write_to_database end----");
	return ret;
}

int main ()
{
	char *file_buf = NULL;
    char filename[256] = {0};
	
    char fdfs_file_path[256] = {0};
    char fdfs_file_stat_buf[256] = {0};
    char fdfs_file_host_name[30] = {0};
    char fdfs_file_url[512] = {0};
	
	int count = 0;

    while (FCGI_Accept() >= 0) {
        char *contentLength = getenv("CONTENT_LENGTH");
        int len;

	printf("Content-type: text/html\r\n"
	    "\r\n"
	    "<title>FastCGI echo</title>"
	    "<h1>FastCGI echo</h1>\n"
            "Request number %d,  Process ID: %d<p>\n", ++count, getpid());

        if (contentLength != NULL) {
            len = strtol(contentLength, NULL, 10);
        }
        else {
            len = 0;
        }

        if (len <= 0) {
	    printf("No data from standard input.<p>\n");
        }
        else {
            int i, ch;
			char *filebegin = NULL;
            size_t filelen = 0;
			
			char *p;
			int ret = 0;
			
			file_buf = malloc(len + 1);
			if (file_buf == NULL) {
                printf("malloc error! file size is to big!\n");
                return -1;
            }
			

			p = file_buf; 
			printf("Standard input:<br>\n<pre>\n");
            for (i = 0; i < len; i++) {
                if ((ch = getchar()) < 0) {
                    printf("Error: Not enough bytes received on standard input<p>\n");
                    break;
				}
                //putchar(ch);
				*p++ = ch;
            }
			
			ret = get_file_info(file_buf, (size_t)len, filename, &filebegin, &filelen);
			
			if(ret == 0)
			{
				char file_id[FIELD_ID_SIZE] = {0};
				char file_info[VALUES_ID_SIZE] = {0};
				
				//生成本地文件
				write_to_file(filebegin, filelen, filename);
				
				//upload_op操作, 存入fdfs存储Storage
				ret = upload("/etc/fdfs/client.conf", filename, file_id);
				unlink(filename); //删除文件
				LOG(TAG, FILENAME, "Storage to fdfs successfully!----;");
				
				//redis_op操作, 存入redis
				char suffix[32];
				get_file_suffix(filename, suffix);
				
				time_t timep;  time(&timep);  //获取当前时间
				struct tm *tm_now = localtime(&timep);
				char time[64] = {0};
				//格式化提取当前时间
				sprintf (time, "%d-%d-%d %d:%d:%d", tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday, 
							tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec); 
				sprintf(file_info, "http://192.168.6.164/%s||%s||%s||%s||%s", 
								file_id, suffix, filename, time, "user");
				LOG(TAG, FILENAME, "file_info is: %s;", file_info);
				ret = write_to_database(&file_id, &file_info);
				
				if(ret == 0)
					printf("handle successfully! \n");
			}
			else
			{
				printf("handle failed! Please try again.\n");
			}
			
            printf("\n</pre><p>\n");
		
		END:
            memset(filename, 0, 256);
			
            memset(fdfs_file_path, 0, 256);
            memset(fdfs_file_stat_buf, 0, 256);
            memset(fdfs_file_host_name, 0, 30);
            memset(fdfs_file_url, 0, 512);

            free(file_buf);
        }
    } /* while */

    return 0;
}
