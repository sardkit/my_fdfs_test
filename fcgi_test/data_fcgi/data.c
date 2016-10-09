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
#include "redis_op.h"
#include "make_log.h"
#include "cJSON.h"

#define SUP_PNG 	"accdb|avi|bmp|css|doc|docx|eml|eps|fla|ind|jsf|mpeg|ppt|pst|settings|url|wmv|" \
					"gif|ini|midi|null|pptx|pub|tar|vsd|xls|gz|jpeg|mov|pdf|proj|rar|tiff|wav|xlsx|" \
					"html|jpg|mp3|png|psd|readme|txt|wma|zip|"

#define TAG 		"CGI"
#define FILENAME 	"data"


//组织应答报文
int reply_message(int fromId, int endId)
{
	LOG(TAG, FILENAME, "reply_message begin----");
	int ret = 0;
	
	char file_id[9][VALUES_ID_SIZE];
	char url[256];
	char type[16];
	char filename[128];
	char timep[32];
	char user[32];
	int pv = 0;
	char pic_url[128];
	
	int get_num = 0;
	
	redisContext *conn = rop_connectdb_nopwd("127.0.0.1", "6379");
	if(conn == NULL)
	{
		LOG(TAG, FILENAME, "rop_connectdb_nopwd error;");
		return -1;
	}
	LOG(TAG, FILENAME, "rop_connectdb_nopwd end----");	
	
	//查找数据
	ret = rop_zset_range_list(conn, "FILE_HOT_ZSET", fromId, endId, file_id, &get_num);
	if (ret != 0) 
	{
		LOG(TAG, FILENAME, "rop_zset_range_list error;");
		goto END;		
    }
	LOG(TAG, FILENAME, "rop_zset_range_list end----");	

    cJSON *root = cJSON_CreateObject();
    cJSON *array = cJSON_CreateArray();
	
    int i = 0;
	char hvalue[VALUES_ID_SIZE] = {0};
	for(; i < get_num; i++)
	{
        cJSON *item = cJSON_CreateObject();
        LOG(TAG, FILENAME, "file_id: %s", file_id[i]);
		rop_hget_field_value(conn, "FILE_INFO_HASHE", file_id[i], hvalue);
		
		get_value_by_col(hvalue, 1, url, 256, 0);
		get_value_by_col(hvalue, 2, type, 16, 0);
		get_value_by_col(hvalue, 3, filename, 128, 0);
		get_value_by_col(hvalue, 4, timep, 32, 0);
		get_value_by_col(hvalue, 5, user, 32, 0);
		pv = rop_zset_get_score(conn, "FILE_HOT_ZSET", file_id[i]);
        //LOG(TAG, FILENAME, "____get_val: %s,%s,%s,%s,%s,%d", url, type, filename, timep, user, pv);
        LOG(TAG, FILENAME, "get item value end----");
		
		if(strstr(SUP_PNG, type))
			sprintf(pic_url, "http://192.168.6.164/static/file_png/%s.png", type);
		else
			sprintf(pic_url, "http://192.168.6.164/static/file_png/null.png");

        //打包成json报文
        cJSON_AddStringToObject(item, "id", file_id[i]);
        cJSON_AddStringToObject(item, "kind", type);
        cJSON_AddStringToObject(item, "title_m", filename);
        cJSON_AddStringToObject(item, "title_s", user);
        cJSON_AddStringToObject(item, "descrip", timep);
        cJSON_AddStringToObject(item, "picurl_m", pic_url);
        cJSON_AddStringToObject(item, "url", url);
        cJSON_AddNumberToObject(item, "pv", pv - 1);
        cJSON_AddNumberToObject(item, "hot", 0);

        cJSON_AddItemToArray(array, item);
    }
    cJSON_AddItemToObject(root, "games", array);
    const char *out = cJSON_Print(root);
	
    LOG(TAG, FILENAME, "json_val: %s", out);
	printf("%s", out);

    if(out != NULL)
    {
        cJSON_Delete(root);
        free((void *)out);
    }
END:
	rop_disconnect(conn);
	LOG(TAG, FILENAME, "reply_message end----");
	return ret;
}


//设置fileId自增1
int file_pv_increase(char *file_id)
{
	LOG(TAG, FILENAME, "file_pv_increase begin----");
	int ret = 0;
	
	
	redisContext *conn = rop_connectdb_nopwd("127.0.0.1", "6379");
	if(conn == NULL)
	{
		LOG(TAG, FILENAME, "rop_connectdb_nopwd error;");
		return -1;
	}
	LOG(TAG, FILENAME, "rop_connectdb_nopwd end----");	
	
	//设置文件描述自增1
	ret = rop_zset_increment(conn, "FILE_HOT_ZSET", file_id);

END:
	rop_disconnect(conn);
	LOG(TAG, FILENAME, "file_pv_increase end----");
	return ret;
}


int main ()
{
    while (FCGI_Accept() >= 0) {
		
		char *query_string = getenv("QUERY_STRING");
		LOG(TAG, FILENAME, "web query_string is: %s----", query_string);
		
		printf("Content-type: text/html\r\n"
			"\r\n");
        
        if (query_string == NULL){
			LOG(TAG, FILENAME, "[ERROR] no get information obtained!!!;");
        }
        else {
			int ret = 0;
			int from_id = 0, count = 0;
			char cmd[16] = {0};
			char user[64] = {0};
			char tmpdata[16] = {0};
			
			//解析get请求数据
			ret = query_parse_key_value(query_string, "cmd", cmd, &ret);
			
			if(strcmp(cmd, "newFile") == 0)
			{
				ret = query_parse_key_value(query_string, "fromId", tmpdata, &from_id);
				from_id = atoi(tmpdata);
			
				ret = query_parse_key_value(query_string, "count", tmpdata, &count);
				count = atoi(tmpdata);
			
				ret = query_parse_key_value(query_string, "user", user, &ret);

				if(ret == 0)
					LOG(TAG, FILENAME, "get cmd data successfully!----;");
			
			
				//组织应答报文
				ret = reply_message(from_id, from_id+count-1);
			}
			else if(strcmp(cmd, "increase") == 0)
			{
				char fileId[128] = {0};
				int len = 0;
				ret = ret = query_parse_key_value(query_string, "fileId", fileId, &len);
				
				str_replace(fileId, "%2F", "/");
				
				if(ret == 0)
					LOG(TAG, FILENAME, "get cmd data successfully!--%s;", fileId);
				
				//设置fileId自增1
				ret = file_pv_increase(fileId);
			}
			
			
			if(ret == 0)
			{	
				LOG(TAG, FILENAME,"handle %s successfully!----", cmd);
			}
			else
			{
				LOG(TAG, FILENAME,"handle %s failed! Please try again.", cmd);
			}
        }
    } /* while */

    return 0;
}
