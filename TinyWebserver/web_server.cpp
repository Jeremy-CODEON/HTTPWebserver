#include "web_server.h"

bool WebServerUtils::parse_uri(const std::string uri, std::string& filename, std::string& cgi_args)
{
	/* 
	* HTTP请求如下：
	* GET http://host:port/cgi_bin/app?param1&param2... HTTP/1.1
	* URI部分如下：
	* /cgi-bin/app?param1&param2...
	* 
	* 假设静态内容的主目录就是它的当前目录，而可执行文件的主目录是./cgi_bin
	* 任何含字符串cgi_bin的uri都会被认为是对动态内容的请求
	* 默认的文件名是./home.html 
	*/

	std::string::size_type idx = uri.find("cgi_bin");

#ifdef DEBUG
	printf("uri: %s, %d\n", uri.c_str(), idx);
#endif // DEBUG

	filename = "./content";
	if (idx == std::string::npos) {
		// uri中不存在"cgi-bin"，直接返回默认文件名
		cgi_args = "";
				
		filename += uri;
		if (uri[uri.length() - 1] == '/') {
			filename += "home.html";
		}

#ifdef DEBUG
		printf("static cgi_args: %s\n", cgi_args.c_str());
		printf("static filename: %s\n", filename.c_str());
#endif // DEBUG

		// 静态内容
		return true;
	}
	else {
		// uri含"cgi-bin"，返回动态内容
		idx = uri.find('?');
		if (idx == std::string::npos) {
			// uri中不含有参数
			cgi_args = "";
		}
		else {
			// 把uri中'?'后的部分给cgi_args
			cgi_args += uri.substr(idx + 1);
		}

		filename += uri.substr(0, idx);

#ifdef DEBUG
		printf("dynamic cgi_args: %s\n", cgi_args.c_str());
		printf("dynamic filename: %s\n", filename.c_str());
#endif // DEBUG

		// 动态内容
		return false;
	}
}

void WebServerUtils::client_error(int fd, std::string cause, std::string errnum, std::string shortmsg, std::string longmsg)
{
	char buffer[core::MAX_LINE];  /*写缓冲区*/
	std::string respond_body;  /*响应报文消息体*/

	// 构建响应报文消息体
	respond_body += "<html><title>Tiny Error</title>";
	respond_body += "<body bgcolor=""ffffff"">\r\n";
	respond_body += (errnum + ": " + shortmsg + "\r\n");
	respond_body += ("<p>" + longmsg + ": " + cause + "\r\n");
	respond_body += "<hr><em>The Tiny Web Server.</em>\r\n";

	// 输出HTTP响应报文状态行到客户端
	sprintf(buffer, "HTTP/1.0 %s %s\r\n", errnum.c_str(), shortmsg.c_str());
	BaseIO::written_n(fd, buffer, strlen(buffer));
	// 输出HTTP响应报文消息头到客户端
	sprintf(buffer, "Content-type: text/html\r\n");
	BaseIO::written_n(fd, buffer, strlen(buffer));
	sprintf(buffer, "Content-length: %d\r\n\r\n", respond_body.length());
	BaseIO::written_n(fd, buffer, strlen(buffer));
	// 输出HTTP响应报文消息体到客户端
	sprintf(buffer, "%s", respond_body.c_str());
	BaseIO::written_n(fd, buffer, strlen(buffer));
}

void WebServerUtils::read_request_head(buffer_t* bp)
{
	char buffer[core::MAX_LINE];  /*读缓冲区*/

	// 仅读入请求报文消息头
	BufferIO::buffer_read_line(bp, buffer, core::MAX_LINE);
	printf("buffer: %s\n", buffer);
	while (strcmp(buffer, "\r\n")) {
#ifdef DEBUG
		printf("waiting request head...\n");
#endif // DEBUG
		BufferIO::buffer_read_line(bp, buffer, core::MAX_LINE);
#ifdef DEBUG
		printf("buffer: %s\n", buffer);
		printf("getting request head...\n");
#endif // DEBUG

	}
	printf("read request head end\n");

	return;
}

void WebServerUtils::get_filetype(const std::string& filename, std::string& filetype)
{
	if (filename.find(".html") != std::string::npos) {
		filetype = "text/html";
	}
	else if (filename.find(".gif") != std::string::npos) {
		filetype = "image/gif";
	}
	else if (filename.find(".jpg") != std::string::npos) {
		filetype = "image/jpeg";
	}
	else {
		filetype = "text/plain";
	}
}

void WebServerUtils::serve_static(int fd, std::string filename, int filesize)
{
	std::string filetype;  /*文件类型*/
	// 获取文件类型
	get_filetype(filename, filetype);

	std::string respond_head;
	respond_head += "HTTP/1.0 200 OK\r\n";
	respond_head += "Server: Tiny Web Server\r\n";
	respond_head += "Content-length: " + std::to_string(filesize) + "\r\n";
	respond_head += "Content-type: " + filetype + "\r\n\r\n";

	// 发送响应报文消息头给客户端
	char buffer[core::MAX_LINE];  /*写缓冲区*/
	sprintf(buffer, "%s", respond_head.c_str());
	BaseIO::written_n(fd, buffer, strlen(buffer));

	// 打开静态文件
	int file_fd = -1;
	if ((file_fd = open(filename.c_str(), O_RDONLY, 0)) == -1) {
		core::unix_error("Open file error");
		exit(1);
	}
	// 将静态文件映射到共享内存
	char* filep = static_cast<char*>(
		mmap(0, filesize, PROT_READ, MAP_PRIVATE, file_fd, 0));
	if (filep == reinterpret_cast<char*>(-1)) {
		core::unix_error("Mmap file error");
		exit(1);
	}
	// 关闭静态文件
	if (close(file_fd) == -1) {
		core::unix_error("Close file error");
		exit(1);
	}

	// 发送响应报文消息体（即静态文件）给客户端
	BaseIO::written_n(fd, filep, filesize);

	// 释放共享内存
	if (munmap(filep, filesize) == -1) {
		core::unix_error("Munmap file error");
		exit(1);
	}
}

void WebServerUtils::serve_dynamic(int fd, std::string filename, std::string cgi_args)
{
	std::string respond_head;
	respond_head += "HTTP/1.0 200 OK\r\n";
	respond_head += "Server: Tiny Web Server\r\n";

	// 发送响应报文消息头的一部分给客户端
	char buffer[core::MAX_LINE];  /*写缓冲区*/
	sprintf(buffer, "%s", respond_head.c_str());
	BaseIO::written_n(fd, buffer, strlen(buffer));

	if (fork() == 0) {
		// 执行CGI程序的子进程，设置环境变量传入参数
		setenv("QUERY_STRING", cgi_args.c_str(), 1);
		// 将标准输出重定向到fd，这样进程打印到屏幕的内容就会发送到客户端
		if (dup2(fd, STDOUT_FILENO) == -1) {
			core::unix_error("Dup2 error");
			exit(1);
		}
		// 重新加载子进程，覆盖除内核外的用户虚拟地址空间
		char* empty_list[] = { NULL };
		if (execve(filename.c_str(), empty_list, environ) == -1) {
			core::unix_error("Execve CGI program error");
			exit(1);
		}
	}

	// 等待子进程执行CGI程序完毕
	if (wait(NULL) == -1) {
		core::unix_error("Wait CGI program error");
		exit(1);
	}
	return;
}

void WebServerUtils::doit(int fd)
{
	char buffer[core::MAX_LINE];  /*读缓冲区*/
	char method[core::MAX_LINE];  /*HTTP方法*/
	char uri[core::MAX_LINE];  /*HTTPuri*/
	char version[core::MAX_LINE];  /*HTTP版本*/
	
	std::string filename;  /*请求的文件路径*/
	std::string cgi_args;  /*请求的参数*/

	buffer_t in_b(fd, 1);  /*封装fd读缓冲对象*/

	// 从fd中读入HTTP请求行
	BufferIO::buffer_read_line(&in_b, buffer, core::MAX_LINE);
#ifdef DEBUG
	printf("request head: %s\n", buffer);
	printf("request buffer: %s\n", in_b.buffer_ptr);
#endif // DEBUG

	// 解析HTTP请求行
	sscanf(buffer, "%s %s %s", method, uri, version);
	if (strcasecmp(method, "GET")) {
		// 不是GET请求
		client_error(fd, method, "501", 
			"Not Implemented", "Tiny does not implement this method");
		return;
	}
	// 读入HTTP请求报文消息头
	//read_request_head(&in_b);
	// GET方法没有请求报文消息体，无需处理

	// 解析URI
	bool is_static = parse_uri(uri, filename, cgi_args);
#ifdef DEBUG
	printf("request uri: %s\n", uri);
	printf("request filename: %s\n", filename.c_str());
	printf("request cgi_args: %s\n", cgi_args.c_str());
#endif // DEBUG

	struct stat stat_buffer;
	if (stat(filename.c_str(), &stat_buffer) == -1) {
		// 获取文件状态失败，也就是无此文件
		client_error(fd, filename, "404", "Not found",
			"Tiny couldn't find this file");
		return;
	}

	if (is_static) {
		if (!S_ISREG(stat_buffer.st_mode) || !(S_IRUSR & stat_buffer.st_mode)) {
			// S_ISREG是否是一个常规文件，S_IRUSR是否能读这个文件
			client_error(fd, filename, "403", "Forbidden",
				"Tiny couldn't read the file");
			return;
		}
		// 向客户端发送静态文件
		serve_static(fd, filename, stat_buffer.st_size);
	}
	else {
		if (!S_ISREG(stat_buffer.st_mode) || !(S_IXUSR & stat_buffer.st_mode)) {
			// S_ISREG是否是一个常规文件，S_IXUSR是否能执行这个文件
			client_error(fd, filename, "403", "Forbidden",
				"Tiny couldn't run the CGI program");
			return;
		}
		serve_dynamic(fd, filename, cgi_args.c_str());
	}
}

int WebServerUtils::open_listenfd(int port)
{
	int listen_fd;

	// 建立socket描述符
	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		return -1;
	}

	// 设置本地地址和端口号可以重用
	int optval = 1;
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR,
		(const void*)&optval, sizeof(optval)) == -1) {
		return -1;
	}

	// 配置服务器地址和端口
	struct sockaddr_in serveraddr;
	bzero(reinterpret_cast<char*>(&serveraddr), sizeof(serveraddr));  /*将结构清零*/
	serveraddr.sin_family = AF_INET;  /*地址簇*/
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);  /*IPv4地址*/
	serveraddr.sin_port = htons((unsigned short)port);  /*端口号*/

	// 将服务器地址和端口绑定到socket描述符
	if (bind(listen_fd,
		reinterpret_cast<struct sockaddr*>(&serveraddr),
		sizeof(serveraddr)) == -1) {
		return -1;
	}

	// 将socket描述符转换为listen描述符
	if (listen(listen_fd, core::MAX_LISTENQ) == -1) {
		return -1;
	}

	return listen_fd;
}
