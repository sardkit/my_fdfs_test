#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "redis_op.h"


int main(int argc, char *argv[])
{
    int ret = 0;
    //连接数据库
    redisContext *conn = NULL;

    conn = rop_connectdb_nopwd("127.0.0.1", "6379");

    //set foo  nihao
    ret = rop_set_string(conn, "foo", "你好");
    if (ret == 0) {
        printf("set succ!\n");
    }
    else {
        printf("set fail\n");
    }
	
	char getval[128] = {0};
	ret = rop_get_string(conn, "foo", getval);
    if (ret == 0) 
	{
        printf("get succ!\n result is: %s\n", getval);
    }
    else {
        printf("get fail\n");
    }
	
	char get_setval[8][VALUES_ID_SIZE];
	int get_num = 0;
	ret = rop_zset_range_list(conn, "FILE_HOT_ZSET", 0, 7, get_setval, &get_num);
	if (ret == 0) 
	{
		int i = 0;
        printf("rop_zset_range_list get succ!\n");
		char hvalue[VALUES_ID_SIZE] = {0};
		for(; i < get_num; i++)
		{
			printf("file_id: %s\n", get_setval[i]);
			rop_hget_field_value(conn, "FILE_INFO_HASHE", get_setval[i], hvalue);
			printf("\tget_val: %s\n", hvalue);
		}	
    }
    else {
        printf("rop_zset_range_list get fail\n");
    }


    //释放数据库
    rop_disconnect(conn);


	return 0;
}
