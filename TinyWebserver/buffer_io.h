#pragma once


#include "core.h"

/*
* @brief 缓冲区结构
*/
class Buffer {
public:
	int fd;  /*文件标识符*/
	int rw_flag;  /*读(1)写(2)标记*/
	int n_left;  /*已读或未写字节数*/
	char* buffer_ptr;  /*缓冲区已读或未写指针*/
	char* buffer_area;  /*缓冲区*/

public:
	/*
	* @brief 构造缓冲区
	* @param _fd => 文件标识符
	* @param _flag => 读写标记
	*/
	Buffer(int _fd, int _flag);

	/*
	* @brief 复制构造缓冲区
	* @param _fd => 文件标识符
	* @param _flag => 读写标记
	*/
	Buffer(const Buffer &_buff);

	/*
	* @brief 析构缓冲区
	*/
	~Buffer();
};

using buffer_t = Buffer;

/*
* @brief 缓冲I/O类，包含带缓冲区的I/O函数
*/
class BufferIO {
private:
	BufferIO() {};
	BufferIO(const BufferIO& obj) {};
	virtual ~BufferIO() {};
public:		
	/*
	* @brief 带缓存的打开文件
	* @param
	* @retval buffer_p => success
	* @retval nullptr => error
	*/
	static buffer_t* buffer_open(const char* filename, int flag, mode_t mode = 0);

	/*
	* @brief 带缓存的关闭文件
	* @param
	*/
	static int buffer_close(buffer_t* bp);

	/*
	* @brief 至多拷贝n个字节到内存，自动处理中断，缓存读
	* @param
	* @retval bytes_num => success
	* @retval 0 => EOF
	* @retval -1 => error
	*/
	static ssize_t buffer_read(buffer_t* bp, void* usrbuf, size_t n);

	/*
	* @brief 至多拷贝n个字节到内存，自动处理中断、EOF值和缓冲区不足，缓存读
	* @param
	* @retval bytes_num => success
	* @retval -1 => error
	*/
	static ssize_t buffer_read_n(buffer_t *bp, void* usrbuf, size_t n);

	/*
	* @brief 至多拷贝n个字节到内存，如果有换行符（含）则提前返回，
	*        自动处理中断、EOF值和缓冲区不足，缓存读
	* @param
	* @retval bytes_num => success
	* @retval -1 => error
	*/
	static ssize_t buffer_read_line(buffer_t* bp, void* usrbuf, size_t n);

	/*
	* @brief 从内存拷贝n个字节到文件，自动处理中断，缓存写
	* @param
	* @retval bytes_num => success
	* @retval -1 => error
	*/
	static ssize_t buffer_written(buffer_t* bp, void* usrbuf, size_t n);

	/*
	* @brief 从内存拷贝n个字节到文件，自动处理中断和缓冲区已满，缓存写
	* @param
	* @retval bytes_num => success
	* @retval -1 => error
	*/
	static ssize_t buffer_written_n(buffer_t* bp, void* usrbuf, size_t n);

	/*
	* @brief 刷新缓冲区中的全部字节到文件，自动处理中断
	* @param
	* @retval bytes_num => success
	* @retval -1 => error
	*/
	static ssize_t buffer_written_flush(buffer_t* bp);
};
