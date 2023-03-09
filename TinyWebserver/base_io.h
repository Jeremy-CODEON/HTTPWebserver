#pragma once

#include "core.h"

/*
* @brief 基础I/O类，包含无缓冲区的I/O函数
*/
class BaseIO {
private:
	BaseIO();
	BaseIO(const BaseIO& obj) {};
	virtual ~BaseIO() {};
public:
	/* 
	* @brief 至多拷贝n个字节到内存，自动处理EOF值和中断
	* @param 
	* @retval bytes_num => success
	* @retval -1 => error
	*/
	static ssize_t read_n(int fd, void *usrbuf, size_t n);

	/*
	* @brief 从内存拷贝n个字节到文件，自动处理中断
	* @param
	* @retval bytes_num => success
	* @retval -1 => error
	*/
	static ssize_t written_n(int fd, void* usrbuf, size_t n);
};