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

/**
 * @brief  对文件进行md5加密, 生成md5码
 * 
 * @param msg         			文件名称 
 * @param out_md5        		生成的md5码
 *
 * @returns   
 *   0 成功  	非0 失败
 */
int md5_encode(const char *msg, char out_md5[48]); 

#endif