#ifndef __MD5_H__
#define __MD5_H__

#ifdef __cplusplus
	extern "C" {
#endif

/* -------------------------------------------*/
/**
 * @brief  生成md5密码
 *
 * @param initial_msg	源字符串
 * @param initial_len	字符长度
 * @param digest		输出目标(16位长的十六进制)
 *
 * @returns   
 *			成功返回链接句柄 
 *			失败返回NULL
 */
/* -------------------------------------------*/
void md5(const uint8_t *initial_msg, size_t initial_len, uint8_t *digest);

#ifdef __cplusplus
	}
#endif

#endif
