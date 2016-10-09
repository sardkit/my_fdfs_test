#ifndef UPLOAD_OP_H
#define UPLOAD_OP_H

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fdfs_client.h>


/**
 * @brief  将文件上传至存储服务器
 * 
 * @param conf_filename         配置文件路径 
 * @param local_filename        上传文件路径
 * @param out_id        		存储后的文件id //out
 *
 * @returns   
 *   0 成功  	非0 失败
 */
int upload(const char *conf_filename, const char *local_filename, char file_id[128]);

#endif