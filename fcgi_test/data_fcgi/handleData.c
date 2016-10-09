#include "handleData.h"
#include <ctype.h>

/**
 * @brief  得到目标值字符串首地址 如果失败, 返回"null"
 */
char* memstr(char* full_data, int full_data_len, char* substr) 
{ 
    if (full_data == NULL || full_data_len <= 0 || substr == NULL) { 
        return NULL; 
    } 

    if (*substr == '\0') { 
        return NULL; 
    } 

    int sublen = strlen(substr); 

    int i; 
    char* cur = full_data; 
    int last_possible = full_data_len - sublen + 1; 
    for (i = 0; i < last_possible; i++) { 
        if (*cur == *substr) { 
            //assert(full_data_len - i >= sublen);  
            if (memcmp(cur, substr, sublen) == 0) { 
                //found  
                return cur; 
            } 
        } 
        cur++; 
    } 

    return NULL; 
} 


/**
 * @brief  得到文件后缀字符串 如果非法文件后缀,返回"null"
 */
int get_file_suffix(const char *file_name, char *suffix)
{
    const char *p = file_name;
    int len = 0;
    const char *q=NULL;
    const char *k= NULL;

    if (p == NULL) {
        return -1;
    }

    q = p;

    while (*q != '\0') {
        q++;
    }

    k = q;
    while (*k != '.' && k != p) {
        k--;
    }

    if (*k == '.') {
        k++;
        len = q - k;

        if (len != 0) {
            strncpy(suffix, k, len);
            suffix[len] = '\0';
        }
        else {
            strncpy(suffix, "null", 5);
        }
    }
    else {
        strncpy(suffix, "null", 5);
    }

    return 0;
}     

/**
 * @brief  解析url query 类似 abc=123&bbb=456 字符串
 *          传入一个key,得到相应的value
 * @returns   
 *          0 成功, -1 失败
 */
int query_parse_key_value(const char *query, const char *key, 
							char *value, int *value_len_p)
{
    char *temp = NULL;
    char *end = NULL;
    int value_len =0;


    //找到是否有key
    temp = strstr(query, key);
    if (temp == NULL) {
        //LOG(UTIL_LOG_MODULE, UTIL_LOG_PROC, "Can not find key %s in query\n", key);

        return -1;
    }

    temp+=strlen(key);//=
    temp++;//value


    //get value
    end = temp;

    while ('\0' != *end && '#' != *end && '&' != *end ) {
        end++;
    }

    value_len = end-temp;

    strncpy(value, temp, value_len);
    value[value_len] ='\0';

    if (value_len_p != NULL) {
        *value_len_p = value_len;
    }

    return 0;
}

/**
 * @brief  去掉一个字符串两边的空白字符
 *
 * @param inbuf确保inbuf可修改
 *
 * @returns   
 *   0 成功  	-1 失败
 */
int trim_space(char *inbuf)
{
    int i = 0;
    int j = strlen(inbuf) - 1;

    char *str = inbuf;

    int count = 0;

    if (str == NULL ) {
        //LOG(UTIL_LOG_MODULE, UTIL_LOG_PROC, "inbuf   == NULL\n");
        return -1;
    }


    while (isspace(str[i]) && str[i] != '\0') {
        i++;
    }

    while (isspace(str[j]) && j > i) {
        j--;
    }

    count = j - i + 1;

    strncpy(inbuf, str + i, count);

    inbuf[count] = '\0';

    return 0;
}


/**
 * @brief  得到本行的指定字段数据 字段号从1开始
 *
 * @param line_head        本行首地址
 * @param col              字段号 
 * @param value            得到的数据
 * @param max_len        数据最大长度
 * @param with_quote    该字段是否有“”包括 1 是 其他否
 *
 * @returns   
 *        得到的数据
 */
char* get_value_by_col(char *line_head, int col, char *value, 
						int max_len, char with_quote)
{
    char *pos = NULL;        
    char *value_head = NULL;
    int tmp_col = 0;
    int value_len = 0;


    if (col < 1) {
        fprintf(stderr, "col must >= 1\n");
        goto END;
    }

    /* 预留 \0 空间 */
    --max_len;

    /* 第一个字段 */
    if (col == 1) {
        for (pos = line_head, value_head = line_head; *pos != '|' && *(pos-1) != '\\'; ++pos);
        if (with_quote == 1) {
            value_len = ((pos - line_head - 2) > max_len) ? max_len: (pos-line_head-2);
            memcpy(value, line_head+1, value_len);    
        }
        else {
            value_len = ((pos - line_head) > max_len) ? max_len: (pos-line_head);
            memcpy(value, line_head, value_len);    
        }
        value[value_len] = '\0';
        goto END;
    }

    /* 二到最后字段 */
    for (pos = line_head, value_head = line_head; ; ++pos) {
        if (*pos == '|' && *(pos-1) != '\\')  { //|| *pos == '\n'
            ++tmp_col; pos++;
            if (tmp_col == col) {
                if (with_quote == 1) {
                    value_len = ((pos-value_head-3)>max_len) ? max_len: (pos-value_head-3);
                    memcpy(value, value_head+1, value_len);
                }
                else {
                    value_len = ((pos-value_head-1)>max_len) ? max_len: (pos-value_head-1);
                    memcpy(value, value_head, value_len);
                }
                value[value_len] = '\0';
                goto END;
            }
            value_head = pos+1;
            if (*pos == '\n') {
                break;
            }
        }
    }

    /* col 超出字段范围 */
    fprintf(stderr, "expend colum !\n");
    value = NULL;

END:
    return value;
}

/**
 * @brief  取代源字符串中指定字符串
 *
 * @param strSrc      		 源字符串
 * @param strFind            需要替换的子串 
 * @param strReplace         替换目标字符串
 */
void str_replace(char* strSrc, char* strFind, char* strReplace)
{
    while (*strSrc != '\0')
    {
        if (*strSrc == *strFind)
        {
            if (strncmp(strSrc, strFind, strlen(strFind)) == 0)
            {
                int i = 0;
                char *q = NULL;
                char *p = NULL;
                char *repl = NULL;
                int lastLen = 0;

                i = strlen(strFind);
                q = strSrc+i;
                p = q;//p、q均指向剩余字符串的首地址
                repl = strReplace;

                while (*q++ != '\0')
                    lastLen++;
                char* temp = malloc(lastLen+1); //临时开辟一段内存保存剩下的字符串,防止内存覆盖
                int k = 0;
                for (k = 0; k < lastLen; k++)
                {
                    *(temp+k) = *(p+k);
                }
                *(temp+lastLen) = '\0';
                while (*repl != '\0')
                {
                    *strSrc++ = *repl++;
                }
                p = strSrc;
                char* pTemp = temp;//回收动态开辟内存
                while (*pTemp != '\0')
                {
                    *p++ = *pTemp++;
                }
                free(temp);
                *p = '\0';
            }
            else
                strSrc++;
        }
        else
            strSrc++;
    }
}

/**
 * @brief  获得目标文件名, 文件长度, 及数据在buf中的起始位置
 *
 * @param file_buf      源字符串
 * @param bufLen 		需要替换的子串 
 * @param mfile_name     目标文件名 //out
 * @param mbegin         数据起始位置 //out
 * @param mlength        文件大小 //out
 *
 * @returns   
 *   0 成功  	非0 失败
 */
int get_file_info(const char *file_buf, size_t bufLen, char mfile_name[256], 
					char** mbegin, size_t *mlength)
{
	int ret = 0;
	
	char *begin = (char*)file_buf;
	char *end = begin + bufLen;
	char *p = begin;
	int len = bufLen;
	
	char boundary[256] = {0};
	char content_text[256] = {0};

    //get boundary
    p = strstr(begin, "\r\n");
    if (p == NULL) {
        printf("wrong no boundary!\n");
		ret = -1;
        goto END;
    }

    strncpy(boundary, begin, p-begin);
    boundary[p-begin] = '\0';
    //printf("boundary: [%s]\n", boundary);

    p+=2;//\r\n
    //已经处理了p-begin的长度
    len -= (p-begin);

    //get content text head
    begin = p;

    p = strstr(begin, "\r\n");
    if(p == NULL) {
        printf("ERROR: get context text error, no filename?\n");
        ret = -2;
        goto END;
    }
    strncpy(content_text, begin, p-begin);
    content_text[p-begin] = '\0';
    //printf("content_text: [%s]\n", content_text);

    p+=2;//\r\n
    len -= (p-begin);

    //get filename
    // filename="123123.png"          
    char *q = begin;
    q = strstr(begin, "filename=");

    q += strlen("filename=");
    q++;

    char *k = strchr(q, '"');
    strncpy(mfile_name, q, k-q);
    mfile_name[k-q] = '\0';

    trim_space(mfile_name);
    //printf("filename: [%s]\n", filename);

    //get file start location
    begin = p;     
    p = strstr(begin, "\r\n");
    p+=4;//\r\n\r\n
    len -= (p-begin);

    begin = p;  // now begin -->file's begin
	
    //find file's end
    p = memstr(begin, len, boundary);
    if (p == NULL) {
        p = end-2;    //\r\n
    }
    else {
        p = p -2;//\r\n
    }
	
END:
	*mbegin = begin;
	*mlength = p-begin;
	return ret;
}

/**
 * @brief  获得目标文件名, 文件长度, 及数据在buf中的起始位置
 * 
 * @param mbegin         数据起始位置 
 * @param mlength        文件大小
 * @param mfile_name     目标文件名
 *
 * @returns   
 *   0 成功  	非0 失败
 */
int write_to_file(const char *begin, size_t length, const char *filename)
{
	int ret = 0;
	
	int fd = 0;
    fd = open(filename, O_CREAT|O_WRONLY, 0644);
    if (fd < 0) 
	{
        printf("open %s error\n", filename);
		ret = -1;
		goto END;
    }

    ftruncate(fd, length);
    write(fd, begin, length);    
	
END:
	close(fd);
	return ret;
}
