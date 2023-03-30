#include "web_server.h"

bool WebServerUtils::parse_uri(const std::string uri, std::string& filename, std::string& cgi_args)
{
	/* 
	* HTTP�������£�
	* GET http://host:port/cgi_bin/app?param1&param2... HTTP/1.1
	* URI�������£�
	* /cgi-bin/app?param1&param2...
	* 
	* ���農̬���ݵ���Ŀ¼�������ĵ�ǰĿ¼������ִ���ļ�����Ŀ¼��./cgi_bin
	* �κκ��ַ���cgi_bin��uri���ᱻ��Ϊ�ǶԶ�̬���ݵ�����
	* Ĭ�ϵ��ļ�����./home.html 
	*/

	std::string::size_type idx = uri.find("cgi_bin");

#ifdef DEBUG
	printf("uri: %s, %d\n", uri.c_str(), idx);
#endif // DEBUG

	filename = "./content";
	if (idx == std::string::npos) {
		// uri�в�����"cgi-bin"��ֱ�ӷ���Ĭ���ļ���
		cgi_args = "";
				
		filename += uri;
		if (uri[uri.length() - 1] == '/') {
			filename += "home.html";
		}

#ifdef DEBUG
		printf("static cgi_args: %s\n", cgi_args.c_str());
		printf("static filename: %s\n", filename.c_str());
#endif // DEBUG

		// ��̬����
		return true;
	}
	else {
		// uri��"cgi-bin"�����ض�̬����
		idx = uri.find('?');
		if (idx == std::string::npos) {
			// uri�в����в���
			cgi_args = "";
		}
		else {
			// ��uri��'?'��Ĳ��ָ�cgi_args
			cgi_args += uri.substr(idx + 1);
		}

		filename += uri.substr(0, idx);

#ifdef DEBUG
		printf("dynamic cgi_args: %s\n", cgi_args.c_str());
		printf("dynamic filename: %s\n", filename.c_str());
#endif // DEBUG

		// ��̬����
		return false;
	}
}

void WebServerUtils::client_error(int fd, std::string cause, std::string errnum, std::string shortmsg, std::string longmsg)
{
	char buffer[core::MAX_LINE];  /*д������*/
	std::string respond_body;  /*��Ӧ������Ϣ��*/

	// ������Ӧ������Ϣ��
	respond_body += "<html><title>Tiny Error</title>";
	respond_body += "<body bgcolor=""ffffff"">\r\n";
	respond_body += (errnum + ": " + shortmsg + "\r\n");
	respond_body += ("<p>" + longmsg + ": " + cause + "\r\n");
	respond_body += "<hr><em>The Tiny Web Server.</em>\r\n";

	// ���HTTP��Ӧ����״̬�е��ͻ���
	sprintf(buffer, "HTTP/1.0 %s %s\r\n", errnum.c_str(), shortmsg.c_str());
	BaseIO::written_n(fd, buffer, strlen(buffer));
	// ���HTTP��Ӧ������Ϣͷ���ͻ���
	sprintf(buffer, "Content-type: text/html\r\n");
	BaseIO::written_n(fd, buffer, strlen(buffer));
	sprintf(buffer, "Content-length: %d\r\n\r\n", respond_body.length());
	BaseIO::written_n(fd, buffer, strlen(buffer));
	// ���HTTP��Ӧ������Ϣ�嵽�ͻ���
	sprintf(buffer, "%s", respond_body.c_str());
	BaseIO::written_n(fd, buffer, strlen(buffer));
}

void WebServerUtils::read_request_head(buffer_t* bp)
{
	char buffer[core::MAX_LINE];  /*��������*/

	// ��������������Ϣͷ
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
	std::string filetype;  /*�ļ�����*/
	// ��ȡ�ļ�����
	get_filetype(filename, filetype);

	std::string respond_head;
	respond_head += "HTTP/1.0 200 OK\r\n";
	respond_head += "Server: Tiny Web Server\r\n";
	respond_head += "Content-length: " + std::to_string(filesize) + "\r\n";
	respond_head += "Content-type: " + filetype + "\r\n\r\n";

	// ������Ӧ������Ϣͷ���ͻ���
	char buffer[core::MAX_LINE];  /*д������*/
	sprintf(buffer, "%s", respond_head.c_str());
	BaseIO::written_n(fd, buffer, strlen(buffer));

	// �򿪾�̬�ļ�
	int file_fd = -1;
	if ((file_fd = open(filename.c_str(), O_RDONLY, 0)) == -1) {
		core::unix_error("Open file error");
		exit(1);
	}
	// ����̬�ļ�ӳ�䵽�����ڴ�
	char* filep = static_cast<char*>(
		mmap(0, filesize, PROT_READ, MAP_PRIVATE, file_fd, 0));
	if (filep == reinterpret_cast<char*>(-1)) {
		core::unix_error("Mmap file error");
		exit(1);
	}
	// �رվ�̬�ļ�
	if (close(file_fd) == -1) {
		core::unix_error("Close file error");
		exit(1);
	}

	// ������Ӧ������Ϣ�壨����̬�ļ������ͻ���
	BaseIO::written_n(fd, filep, filesize);

	// �ͷŹ����ڴ�
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

	// ������Ӧ������Ϣͷ��һ���ָ��ͻ���
	char buffer[core::MAX_LINE];  /*д������*/
	sprintf(buffer, "%s", respond_head.c_str());
	BaseIO::written_n(fd, buffer, strlen(buffer));

	if (fork() == 0) {
		// ִ��CGI������ӽ��̣����û��������������
		setenv("QUERY_STRING", cgi_args.c_str(), 1);
		// ����׼����ض���fd���������̴�ӡ����Ļ�����ݾͻᷢ�͵��ͻ���
		if (dup2(fd, STDOUT_FILENO) == -1) {
			core::unix_error("Dup2 error");
			exit(1);
		}
		// ���¼����ӽ��̣����ǳ��ں�����û������ַ�ռ�
		char* empty_list[] = { NULL };
		if (execve(filename.c_str(), empty_list, environ) == -1) {
			core::unix_error("Execve CGI program error");
			exit(1);
		}
	}

	// �ȴ��ӽ���ִ��CGI�������
	if (wait(NULL) == -1) {
		core::unix_error("Wait CGI program error");
		exit(1);
	}
	return;
}

void WebServerUtils::doit(int fd)
{
	char buffer[core::MAX_LINE];  /*��������*/
	char method[core::MAX_LINE];  /*HTTP����*/
	char uri[core::MAX_LINE];  /*HTTPuri*/
	char version[core::MAX_LINE];  /*HTTP�汾*/
	
	std::string filename;  /*������ļ�·��*/
	std::string cgi_args;  /*����Ĳ���*/

	buffer_t in_b(fd, 1);  /*��װfd���������*/

	// ��fd�ж���HTTP������
	BufferIO::buffer_read_line(&in_b, buffer, core::MAX_LINE);
#ifdef DEBUG
	printf("request head: %s\n", buffer);
	printf("request buffer: %s\n", in_b.buffer_ptr);
#endif // DEBUG

	// ����HTTP������
	sscanf(buffer, "%s %s %s", method, uri, version);
	if (strcasecmp(method, "GET")) {
		// ����GET����
		client_error(fd, method, "501", 
			"Not Implemented", "Tiny does not implement this method");
		return;
	}
	// ����HTTP��������Ϣͷ
	//read_request_head(&in_b);
	// GET����û����������Ϣ�壬���账��

	// ����URI
	bool is_static = parse_uri(uri, filename, cgi_args);
#ifdef DEBUG
	printf("request uri: %s\n", uri);
	printf("request filename: %s\n", filename.c_str());
	printf("request cgi_args: %s\n", cgi_args.c_str());
#endif // DEBUG

	struct stat stat_buffer;
	if (stat(filename.c_str(), &stat_buffer) == -1) {
		// ��ȡ�ļ�״̬ʧ�ܣ�Ҳ�����޴��ļ�
		client_error(fd, filename, "404", "Not found",
			"Tiny couldn't find this file");
		return;
	}

	if (is_static) {
		if (!S_ISREG(stat_buffer.st_mode) || !(S_IRUSR & stat_buffer.st_mode)) {
			// S_ISREG�Ƿ���һ�������ļ���S_IRUSR�Ƿ��ܶ�����ļ�
			client_error(fd, filename, "403", "Forbidden",
				"Tiny couldn't read the file");
			return;
		}
		// ��ͻ��˷��;�̬�ļ�
		serve_static(fd, filename, stat_buffer.st_size);
	}
	else {
		if (!S_ISREG(stat_buffer.st_mode) || !(S_IXUSR & stat_buffer.st_mode)) {
			// S_ISREG�Ƿ���һ�������ļ���S_IXUSR�Ƿ���ִ������ļ�
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

	// ����socket������
	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		return -1;
	}

	// ���ñ��ص�ַ�Ͷ˿ںſ�������
	int optval = 1;
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR,
		(const void*)&optval, sizeof(optval)) == -1) {
		return -1;
	}

	// ���÷�������ַ�Ͷ˿�
	struct sockaddr_in serveraddr;
	bzero(reinterpret_cast<char*>(&serveraddr), sizeof(serveraddr));  /*���ṹ����*/
	serveraddr.sin_family = AF_INET;  /*��ַ��*/
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);  /*IPv4��ַ*/
	serveraddr.sin_port = htons((unsigned short)port);  /*�˿ں�*/

	// ����������ַ�Ͷ˿ڰ󶨵�socket������
	if (bind(listen_fd,
		reinterpret_cast<struct sockaddr*>(&serveraddr),
		sizeof(serveraddr)) == -1) {
		return -1;
	}

	// ��socket������ת��Ϊlisten������
	if (listen(listen_fd, core::MAX_LISTENQ) == -1) {
		return -1;
	}

	return listen_fd;
}
