#pragma once

#include "core.h"
#include "base_io.h"
#include "buffer_io.h"

//extern char** environ;

/*
* @brief 服务器工具类
*/
class WebServerUtils {
private:
	/*
	* @brief 解析一个HTTP的URI（只处理GET请求）
	* @param
	* @retval true => 请求静态内容
	* @retval false => 请求动态内容
	*/
	static bool parse_uri(const std::string uri,
		std::string& filename, std::string& cgi_args);

	/*
	* @brief 发送含错误信息的HTTP响应给客户端
	* @param
	*/
	static void client_error(int fd, std::string cause, std::string errnum,
		std::string shortmsg, std::string longmsg);

	/*
	* @brief 读取HTTP请求头（只读取后忽略）
	* @param
	*/
	static void read_request_head(buffer_t *bp);

	/*
	* @brief 获取文件类型
	* @param
	*/
	static void get_filetype(const std::string& filename, std::string& filetype);

	/*
	* @brief 向客户端发送HTTP响应，主体包含一个本地静态文件的内容
	* @param
	*/
	static void serve_static(int fd, std::string filename, int filesize);

	/*
	* @brief 向客户端发送HTTP响应，主体为动态内容输出
	* @param
	*/
	static void serve_dynamic(int fd, std::string filename, std::string cgi_args);

public:
	/*
	* @brief 处理客户端的一个HTTP事务请求
	* @param
	* @retval 0 => 读取成功
	* @retval -1 => 读取失败，需要关闭连接
	*/
	static int doit(int fd);

	/* 
	* @brief 打开一个监听某个端口的socket
	* @param port => 监听的端口号
	*/
	static int open_listenfd(int port);
};
