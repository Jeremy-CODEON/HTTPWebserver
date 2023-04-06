#pragma once

#include "core.h"
#include "base_io.h"
#include "buffer_io.h"

//extern char** environ;

/*
* @brief ������������
*/
class WebServerUtils {
private:
	/*
	* @brief ����һ��HTTP��URI��ֻ����GET����
	* @param
	* @retval true => ����̬����
	* @retval false => ����̬����
	*/
	static bool parse_uri(const std::string uri,
		std::string& filename, std::string& cgi_args);

	/*
	* @brief ���ͺ�������Ϣ��HTTP��Ӧ���ͻ���
	* @param
	*/
	static void client_error(int fd, std::string cause, std::string errnum,
		std::string shortmsg, std::string longmsg);

	/*
	* @brief ��ȡHTTP����ͷ��ֻ��ȡ����ԣ�
	* @param
	*/
	static void read_request_head(buffer_t *bp);

	/*
	* @brief ��ȡ�ļ�����
	* @param
	*/
	static void get_filetype(const std::string& filename, std::string& filetype);

	/*
	* @brief ��ͻ��˷���HTTP��Ӧ���������һ�����ؾ�̬�ļ�������
	* @param
	*/
	static void serve_static(int fd, std::string filename, int filesize);

	/*
	* @brief ��ͻ��˷���HTTP��Ӧ������Ϊ��̬�������
	* @param
	*/
	static void serve_dynamic(int fd, std::string filename, std::string cgi_args);

public:
	/*
	* @brief ����ͻ��˵�һ��HTTP��������
	* @param
	* @retval 0 => ��ȡ�ɹ�
	* @retval -1 => ��ȡʧ�ܣ���Ҫ�ر�����
	*/
	static int doit(int fd);

	/* 
	* @brief ��һ������ĳ���˿ڵ�socket
	* @param port => �����Ķ˿ں�
	*/
	static int open_listenfd(int port);
};
