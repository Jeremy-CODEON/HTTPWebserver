#pragma once


#include "core.h"

/*
* @brief �������ṹ
*/
class Buffer {
public:
	int fd;  /*�ļ���ʶ��*/
	int rw_flag;  /*��(1)д(2)���*/
	int n_left;  /*�Ѷ���δд�ֽ���*/
	char* buffer_ptr;  /*�������Ѷ���δдָ��*/
	char* buffer_area;  /*������*/

public:
	/*
	* @brief ���컺����
	* @param _fd => �ļ���ʶ��
	* @param _flag => ��д���
	*/
	Buffer(int _fd, int _flag);

	/*
	* @brief ���ƹ��컺����
	* @param _fd => �ļ���ʶ��
	* @param _flag => ��д���
	*/
	Buffer(const Buffer &_buff);

	/*
	* @brief ����������
	*/
	~Buffer();
};

using buffer_t = Buffer;

/*
* @brief ����I/O�࣬��������������I/O����
*/
class BufferIO {
private:
	BufferIO() {};
	BufferIO(const BufferIO& obj) {};
	virtual ~BufferIO() {};
public:		
	/*
	* @brief ������Ĵ��ļ�
	* @param
	* @retval buffer_p => success
	* @retval nullptr => error
	*/
	static buffer_t* buffer_open(const char* filename, int flag, mode_t mode = 0);

	/*
	* @brief ������Ĺر��ļ�
	* @param
	*/
	static int buffer_close(buffer_t* bp);

	/*
	* @brief ���࿽��n���ֽڵ��ڴ棬�Զ������жϣ������
	* @param
	* @retval bytes_num => success
	* @retval 0 => EOF
	* @retval -1 => error
	*/
	static ssize_t buffer_read(buffer_t* bp, void* usrbuf, size_t n);

	/*
	* @brief ���࿽��n���ֽڵ��ڴ棬�Զ������жϡ�EOFֵ�ͻ��������㣬�����
	* @param
	* @retval bytes_num => success
	* @retval -1 => error
	*/
	static ssize_t buffer_read_n(buffer_t *bp, void* usrbuf, size_t n);

	/*
	* @brief ���࿽��n���ֽڵ��ڴ棬����л��з�����������ǰ���أ�
	*        �Զ������жϡ�EOFֵ�ͻ��������㣬�����
	* @param
	* @retval bytes_num => success
	* @retval -1 => error
	*/
	static ssize_t buffer_read_line(buffer_t* bp, void* usrbuf, size_t n);

	/*
	* @brief ���ڴ濽��n���ֽڵ��ļ����Զ������жϣ�����д
	* @param
	* @retval bytes_num => success
	* @retval -1 => error
	*/
	static ssize_t buffer_written(buffer_t* bp, void* usrbuf, size_t n);

	/*
	* @brief ���ڴ濽��n���ֽڵ��ļ����Զ������жϺͻ���������������д
	* @param
	* @retval bytes_num => success
	* @retval -1 => error
	*/
	static ssize_t buffer_written_n(buffer_t* bp, void* usrbuf, size_t n);

	/*
	* @brief ˢ�»������е�ȫ���ֽڵ��ļ����Զ������ж�
	* @param
	* @retval bytes_num => success
	* @retval -1 => error
	*/
	static ssize_t buffer_written_flush(buffer_t* bp);
};
