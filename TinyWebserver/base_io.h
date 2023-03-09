#pragma once

#include "core.h"

/*
* @brief ����I/O�࣬�����޻�������I/O����
*/
class BaseIO {
private:
	BaseIO();
	BaseIO(const BaseIO& obj) {};
	virtual ~BaseIO() {};
public:
	/* 
	* @brief ���࿽��n���ֽڵ��ڴ棬�Զ�����EOFֵ���ж�
	* @param 
	* @retval bytes_num => success
	* @retval -1 => error
	*/
	static ssize_t read_n(int fd, void *usrbuf, size_t n);

	/*
	* @brief ���ڴ濽��n���ֽڵ��ļ����Զ������ж�
	* @param
	* @retval bytes_num => success
	* @retval -1 => error
	*/
	static ssize_t written_n(int fd, void* usrbuf, size_t n);
};