#include "base_io.h"
#include "buffer_io.h"
#include "process_pool.h"
#include "thread_pool.h"
#include "web_server.h"

#include <chrono>

int main() {
#if 0
	printf("Hello world!\n");
#endif

#if 0
	/*基本IO使用示例*/

	char c;
	while (BaseIO::read_n(STDIN_FILENO, &c, 1) != 0) {
		BaseIO::written_n(STDOUT_FILENO, &c, 1);
	}
#endif

#if 0
	/*缓冲区IO函数使用示例*/

	std::string in_filename("/home/jeremy_wsl/projects/TinyWebserver/test/in.txt");
	std::string out_filename("/home/jeremy_wsl/projects/TinyWebserver/test/out.txt");

	// 打开文件，构建缓冲区
	buffer_t *in_bp = BufferIO::buffer_open(in_filename.c_str(), O_RDONLY);
	if (in_bp != nullptr) {
		printf("open in file successfully.\n");
	}
	buffer_t* out_bp = BufferIO::buffer_open(out_filename.c_str(), O_WRONLY);
	if (out_bp != nullptr) {
		printf("open out file successfully.\n");
	}

	constexpr int N = 100;
	char content[N + 1];
	int in_bn = 0, out_bn = 0;

	// 定义函数指针别名
	using read_fn = ssize_t(*)(buffer_t*, void*, size_t);
	//typedef ssize_t(*read_fn)(buffer_t*, void*, size_t);
	//read_fn fnp = BufferIO::buffer_read;
	//read_fn fnp = BufferIO::buffer_read_n;
	read_fn fnp = BufferIO::buffer_read_line;

	// 从文件中读入到content
	while ((in_bn = fnp(in_bp, content, N)) > 0)
	{
		content[in_bn] = '\0';
		printf("\nanother read, content:\n");
		printf("%s\n", content);
		printf("another read end.\n");

		// 从content写出到文件
		out_bn = BufferIO::buffer_written_n(out_bp, content, in_bn);
		printf("another write %d bytes.\n", out_bn);
	}
	//out_bn = BufferIO::buffer_written_flush(out_bp);
	//printf("another write %d bytes.\n", out_bn);

	// 关闭文件
	if (!BufferIO::buffer_close(in_bp)) {
		printf("close in file successfully.\n");
	}
	if (!BufferIO::buffer_close(out_bp)) {
		printf("close out file successfully.\n");
	}
#endif

#if 0
	/*进程池使用示例1*/
	ProcessPool& process_pool = ProcessPool::get_instance(4);
	//printf("main instance get.\n");


	// 启动进程池
	//process_pool->run();

	ProcessExecutable obj;
	for (int i = 0; i < 10; ++i) {
		process_pool.execute(obj);
	}

	//process_pool->destory();
#endif

#if 0
	/*进程池使用示例2*/

	/*
	* @brief 进程池执行类，封装子进程执行的函数
	*/
	class ProcessExecutable :public ProcessExecutableBase {
	private:
		char* content;

		void print_content() {
			content = "Hello world!";
			printf("process task: %s\n", content);
		}
	public:
		virtual int process() override {
			print_content();
		}
	};	

	// 获取进程池实例
	ProcessPool<ProcessExecutable>& process_pool = ProcessPool<ProcessExecutable>::get_instance(4);
	// 启动进程池
	//process_pool->run();

	ProcessExecutable obj;
	for (int i = 0; i < 10; ++i) {
		process_pool.execute(obj);
	}

	// 销毁子进程
	//process_pool->destory();
#endif

#if 0
	class A{};
	std::shared_ptr<int> shared_null = nullptr;      // 可以将shared_ptr初始化为nullptr
	std::unique_ptr<int> unique_null = nullptr;      // 可以将unique_ptr初始化为nullptr
	//std::weak_ptr<int> weak_null = nullptr;        // 不能将weak_ptr初始化为nullptr

	std::shared_ptr<int> shared_obj = std::make_shared<int>();             // 可以用new基本类型初始化shared_ptr（推荐）
	//std::shared_ptr<int> shared_obj(new int);                            // 可以用new基本类型初始化shared_ptr
	//std::shared_ptr<int> shared_obj = std::shared_ptr<int>(new int);           // 可以用new基本类型初始化shared_ptr（和上一个写法等价）
	std::unique_ptr<int> unique_obj(new int);                              // 可以用new基本类型初始化unique_ptr
	//std::unique_ptr<int> unique_obj = std::unique_ptr<int>(new int);           // 可以用new基本类型初始化unique_ptr（和上一个写法等价）
	//std::weak_ptr<int> weak_obj(new int);                                // 不能用new基本类型初始化weak_ptr

	std::shared_ptr<A> shared_A = std::make_shared<A>();                   // 可以用对象指针初始化shared_ptr（推荐）
	//std::shared_ptr<A> shared_A(new A);                                  // 可以用对象指针初始化shared_ptr
	//std::shared_ptr<A> shared_A = std::shared_ptr<A>(new A);             // 可以用对象指针初始化shared_ptr（和上一个写法等价）
	std::unique_ptr<A> unique_A(new A);                                    // 可以用对象指针初始化unique_ptr
	//std::unique_ptr<A> unique_A = std::unique_ptr<A>(new A);             // 可以用对象指针初始化unique_ptr（和上一个写法等价）
	//std::weak_ptr<A> weak_A(new A);                                      // 不能用对象指针初始化weak_ptr

	std::shared_ptr<int> shared_p1(shared_obj);                            // 可以用shared_ptr初始化shared_ptr
	std::shared_ptr<int> shared_p2(std::move(unique_obj));                 // 可以用unique_ptr初始化shared_ptr，unique_ptr自动置空
	std::unique_ptr<int> unique_p(std::move(unique_obj));                  // 只能用unique_ptr初始化unique_ptr
	std::weak_ptr<int> weak_p1(shared_obj);                                // 可以用shared_ptr初始化weak_ptr
	//std::weak_ptr<int> weak_p2(unique_obj);                              // 不能用unique_ptr初始化weak_ptr
	std::weak_ptr<int> weak_p3(weak_p1);                                   // 可以用weak_ptr初始化weak_ptr
	std::shared_ptr<int> shared_p3(weak_p1);                               // 可以用weak_ptr初始化shared_ptr


	shared_null = shared_obj;                                              // 可以用shared_ptr赋值给shared_ptr
	shared_null = std::move(unique_obj);                                   // 可以用unique_ptr赋值给shared_ptr，unique_ptr自动置空
	//shared_null = weak_p1;                                               // 不能用weak_ptr赋值给shared_ptr
	unique_null = std::move(unique_obj);                                   // 只能用unique_ptr赋值给unique_ptr，前一个unique_ptr自动置空
	weak_p3 = shared_obj;                                                  // 可以用shared_ptr赋值给weak_ptr
	//weak_p3 = std::move(unique_obj);                                     // 不能用unique_ptr赋值给weak_ptr
	weak_p3 = weak_p1;                                                     // 可以用weak_ptr赋值给weak_ptr

	shared_null = nullptr;                            // 可以将nullptr赋值给shared_ptr
	unique_null = nullptr;                            // 可以将nullptr赋值给unique_ptr
    //weak_p1 = nullptr;                              // 不能将nullptr赋值给weak_ptr

	int num = 4;
	std::shared_ptr<int> shared_int(&num);                              // 可以用普通指针初始化shared_ptr
	std::unique_ptr<int> unique_int(&num);                              // 可以用普通指针初始化unique_ptr
	//std::weak_ptr<int> weak_int(&num);                                // 不能用普通指针初始化weak_ptr
	
	//shared_null = &num;                                               // 不能直接将普通指针赋值给shared_ptr
	//unique_null = &num;                                               // 不能直接将普通指针赋值给unique_ptr
	//weak_p1 = &num;                                                   // 不能直接将普通指针赋值给weak_ptr

	shared_null = std::make_shared<int>(num);                           // 可以将封装后的普通指针赋值给shared_ptr
	unique_null = std::unique_ptr<int>(&num);                           // 可以将封装后的普通指针赋值给unique_ptr
	//weak_p1 = std::weak_ptr<int>(&num);                               // 不能将封装后的普通指针赋值给weak_ptr
#endif

#if 0
	/*线程池使用示例*/

	/*
	* @brief 线程池执行类，封装子线程执行的函数
	*/
	class ThreadExecutable :public ThreadExecutableBase {
	private:
		char* content;

		void print_content() {
			content = "Hello world!";
			printf("thread task: %s\n", content);
		}
	public:
		virtual int process() override {
			print_content();
		}
	};

	// 获取线程池实例
	ThreadPool& thread_pool = ThreadPool::get_instance(4);

	ThreadExecutable obj;
	for (int i = 0; i < 10; ++i) {
		thread_pool.execute(&obj);
	}

	// 由于任务只传指针，资源都在主线程内而不是整个进程生命
	// 因此主线程需要等待任务队列完成再终止
	thread_pool.wait();

	printf("main thread end.\n");
#endif

#if 0
	/*CGI程序，可用于服务器调用返回动态内容*/

	char* buf, * p;
	char arg1[core::MAX_LINE], arg2[core::MAX_LINE], content[core::MAX_LINE];
	int n1 = 0, n2 = 0;

	// 提取两个参数
	if ((buf = getenv("QUERY_STRING")) != NULL) {
		p = strchr(buf, '&');
		*p = '\0';
		strcpy(arg1, buf);
		strcpy(arg2, p + 1);
		n1 = atoi(arg1);
		n2 = atoi(arg2);
	}

	// 构建响应消息体
	sprintf(content, "Welcome to add.com: ");
	sprintf(content, "%sTHE Internet addition portal.\r\n<p>", content);
	sprintf(content, "%sThe answer is %d + %d = %d\r\n<p>",
		content, n1, n2, n1 + n2);
	sprintf(content, "%sThanks for visiting!\r\n", content);

	// 发送余下的响应消息头
	printf("Content-length: %d\r\n", strlen(content));
	printf("Content-type: text/html\r\n\r\n");
	printf("%s", content);
	fflush(stdout);

#endif	

#if 0
	/*单线程服务器示例*/

	printf("Tiny server is running!\n");

	int port = 8080;  /*服务器端口*/
	// 获取端口socket描述符
	int listen_fd = WebServerUtils::open_listenfd(port);
	if (listen_fd == -1) {
		printf("error: couldn't get listen_fd!\n");
		return -1;
	}

	struct sockaddr_in clientaddr;  /*客户端地址和端口*/
	socklen_t clientaddr_len = sizeof(clientaddr); /*结构大小*/
	// 可能发生connect_fd的客户端已经关闭但服务器仍尝试发送数据给客户端
	// 从而引发Broken pipe异常
	// 需要通过接收SIGPIPE信号来处理该异常，避免服务器终止
	while (true) {
		int connect_fd = accept(listen_fd,
			reinterpret_cast<struct sockaddr*>(&clientaddr),
			&clientaddr_len);
		if (connect_fd == -1) {
			printf("error: couldn't accept connect fd.\n");
			continue;
		}
		else {
			printf("another connect is open.\n");
		}
		WebServerUtils::doit(connect_fd);
		if (close(connect_fd) == -1) {
			printf("error: couldn't close connect fd.\n");
		}
		else {
			printf("another connect is closed.\n");
		}
	}

	// 静态内容访问：http://localhost:8080/home.html
	// 动态内容访问：http://localhost:8080/cgi_bin/adder?15&2
#endif

#if 0
	/*
	* 测试基类指针实现是会导致栈上数据的重写而引发难以发现的错误	
	*/

	/*
	* @brief 线程池执行类，封装子线程执行的函数
	*/
	class ThreadExecutable :public ThreadExecutableBase {
	private:
		int test;
		int listen_fd;  /*监听socket描述符*/
		int connect_fd;  /*连接socket描述符*/

	public:
		/*
		* @brief 构造函数
		*/
		ThreadExecutable(int _fd, int _t) : listen_fd(_fd), connect_fd(-1), test(_t) {};

		/*
		* @brief 析构函数
		*/
		~ThreadExecutable() {
			printf("main: ThreadExecutable destories.\n");
		}

		/*
		* @brief 重写子线程执行函数
		*/
		virtual int process() override {
			printf("thread: process begins.\n");

			// 暂停2秒
			std::this_thread::sleep_for(std::chrono::seconds(5));

			printf("thread %d\n", test);

			struct sockaddr_in clientaddr;  /*客户端地址和端口*/
			socklen_t clientaddr_len = sizeof(clientaddr); /*结构大小*/

			// 和客户端建立连接
			connect_fd = accept(listen_fd,
				reinterpret_cast<struct sockaddr*>(&clientaddr),
				&clientaddr_len);
			if (connect_fd == -1) {
				printf("error: couldn't accept connect fd.\n");
				return -1;
			}
			else {
				printf("another connect is open.\n");
			}

			// 处理客户端的请求
			WebServerUtils::doit(connect_fd);
			if (close(connect_fd) == -1) {
				printf("error: couldn't close connect fd.\n");
			}
			else {
				printf("another connect is closed.\n");
			}

			printf("thread: process ends.\n");
			return 0;
		}
	};

	printf("Tiny server is running!\n");

	// 获取线程池实例
	ThreadPool<ThreadExecutable>& thread_pool = ThreadPool<ThreadExecutable>::get_instance(4);

	int port = 8080;  /*服务器端口*/
	// 获取服务器端口socket描述符
	int listen_fd = WebServerUtils::open_listenfd(port);
	if (listen_fd == -1) {
		printf("error: couldn't get listen_fd!\n");
		return -1;
	}

	// 创建epoll事件监听表
	int epollfd;  /*epoll事件监听表描述符*/
	if ((epollfd = epoll_create(100)) == -1) {
		printf("error: couldn't creat epoll table!\n");
		return -1;
	}

	// 将信号监听加入epoll，统一事件源
	core::SignalUtils sig_utils(epollfd);

	// 将连接请求监听加入epoll，统一事件源
	core::EpollUtils::epoll_add(epollfd, listen_fd);

	epoll_event events[core::MAX_EVENT_NUMBER];  /*监听事件表*/
	int is_stop = false;  /*是否结束主循环*/
	int cnt = 0;
	while (!is_stop) {
		// epoll监听事件
		int number = epoll_wait(epollfd, events, core::MAX_EVENT_NUMBER, -1);
		if ((number < 0) && (errno != EINTR)) {
			core::unix_error("error: couldn't listen epoll table!\n");
			break;
		}

		for (int i = 0; i < number; ++i) {
#ifdef DEBUG
			printf("events %d handling...\n", i);
#endif // DEBUG
			void* tmp_ptr;
			ThreadExecutable tmp_task(listen_fd, 99999);
			int event_fd = events[i].data.fd;
			if ((event_fd == sig_utils.sig_pipefd[1]) && (events[i].events & EPOLLIN)) {   
				/*事件来源于信号管道且可读*/
				char signals[1024];
				// 接收信号
				int sig_num = recv(sig_utils.sig_pipefd[1], signals, sizeof(signals), 0);
				if (sig_num == -1) {
					// 信号接收失败
					continue;
				}
				else {
					// 处理信号
					for (int j = 0; j < sig_num; ++j) {
						switch (signals[j]) {
						case SIGCHLD:  /*回收子进程*/
						{
							pid_t _pid;
							int _stat;
							while ((_pid = waitpid(-1, &_stat, WNOHANG)) > 0) {
								// 循环回收子进程
							}
							break;
						}
						case SIGTERM:  /*软件终止，父进程终止*/
						case SIGINT:  /*键盘中断，父进程终止*/
						{
							is_stop = true;
							break;
						}
						case SIGPIPE:
						{
							break;
						}
						default:
						{
							break;
						}
						}
					}
				}
			}
			else if (event_fd == listen_fd ) {
				/*事件来源于socket请求*/
				ThreadExecutable task(listen_fd, cnt++);
				tmp_ptr = (void*)&task;
				thread_pool.execute(task);
			}
			//memset(tmp_ptr,1,sizeof(ThreadExecutable));
			memcpy(tmp_ptr, &tmp_task, sizeof(ThreadExecutable));
			printf("%d\n", sizeof(ThreadExecutable));
		}
	}

	// 由于任务只传指针，资源都在主线程内而不是整个进程生命
	// 因此主线程需要等待任务队列完成再终止
	thread_pool.wait();

	printf("main thread end.\n");
#endif

#if 1
	/*用线程池实现的服务器示例*/

	/*
	* @brief 线程池执行类，封装子线程执行的函数
	*/
	class ThreadExecutable :public ThreadExecutableBase {
	private:
		int listen_fd;  /*监听socket描述符*/
		int connect_fd;  /*连接socket描述符*/

	public:
		/*
		* @brief 构造函数
		*/
		ThreadExecutable(int _fd) : listen_fd(_fd), connect_fd(-1) {};

		/*
		* @brief 析构函数
		*/
		~ThreadExecutable() {};

		/*
		* @brief 重写子线程执行函数
		*/
		virtual int process() override {
			struct sockaddr_in clientaddr;  /*客户端地址和端口*/
			socklen_t clientaddr_len = sizeof(clientaddr); /*结构大小*/

			// 和客户端建立连接
			connect_fd = accept(listen_fd,
				reinterpret_cast<struct sockaddr*>(&clientaddr),
				&clientaddr_len);
			if (connect_fd == -1) {
				printf("error: couldn't accept connect fd.\n");
				return -1;
			}
			else {
				printf("another connect is open.\n");
			}

			// 处理客户端的请求
			WebServerUtils::doit(connect_fd);
			if (close(connect_fd) == -1) {
				printf("error: couldn't close connect fd.\n");
			}
			else {
				printf("another connect is closed.\n");
			}
			return 0;
		}
	};

	printf("Tiny server is running!\n");

	int port = 8080;  /*服务器端口*/
	// 获取服务器端口socket描述符
	// 最好是在创建子线程之前完成对listen_fd的构建，但由于内核共享，所以在创建之后对线程也可见
	int listen_fd = WebServerUtils::open_listenfd(port);
	if (listen_fd == -1) {
		printf("error: couldn't get listen_fd!\n");
		return -1;
	}

	// 获取线程池实例
	ThreadPool<ThreadExecutable>& thread_pool = ThreadPool<ThreadExecutable>::get_instance(4);

	// 创建epoll事件监听表
	int epollfd;  /*epoll事件监听表描述符*/
	if ((epollfd = epoll_create(100)) == -1) {
		printf("error: couldn't creat epoll table!\n");
		return -1;
	}

	// 将信号监听加入epoll，统一事件源
	core::SignalUtils sig_utils(epollfd);

	// 将连接请求监听加入epoll，统一事件源
	core::EpollUtils::epoll_add(epollfd, listen_fd);

	epoll_event events[core::MAX_EVENT_NUMBER];  /*监听事件表*/
	int is_stop = false;  /*是否结束主循环*/
	while (!is_stop) {
		// epoll监听事件
		int number = epoll_wait(epollfd, events, core::MAX_EVENT_NUMBER, -1);
		if ((number < 0) && (errno != EINTR)) {
			core::unix_error("error: couldn't listen epoll table!\n");
			break;
		}

		for (int i = 0; i < number; ++i) {
			int event_fd = events[i].data.fd;
			if ((event_fd == sig_utils.sig_pipefd[1]) && (events[i].events & EPOLLIN)) {
				/*事件来源于信号管道且可读*/
				char signals[1024];
				// 接收信号
				int sig_num = recv(sig_utils.sig_pipefd[1], signals, sizeof(signals), 0);
				if (sig_num == -1) {
					// 信号接收失败
					continue;
				}
				else {
					// 处理信号
					for (int j = 0; j < sig_num; ++j) {
						switch (signals[j]) {
						case SIGCHLD:  /*回收子进程*/
						{
							pid_t _pid;
							int _stat;
							while ((_pid = waitpid(-1, &_stat, WNOHANG)) > 0) {
								// 循环回收子进程
							}
							break;
						}
						case SIGTERM:  /*软件终止，父进程终止*/
						case SIGINT:  /*键盘中断，父进程终止*/
						{
							is_stop = true;
							break;
						}
						case SIGPIPE:
						{
							break;
						}
						default:
						{
							break;
						}
						}
					}
				}
			}
			else if (event_fd == listen_fd) {
				/*事件来源于socket请求*/
				ThreadExecutable task(listen_fd);
				thread_pool.execute(task);
			}
		}
	}

	// 由于任务只传指针，资源都在主线程内而不是整个进程生命
	// 因此主线程需要等待任务队列完成再终止
	//thread_pool.wait();

	printf("main thread end.\n");
#endif

#if 0
	/*用进程池实现的服务器示例*/

	/*
	* @brief 进程池执行类，封装子进程执行的函数
	*/
	class ProcessExecutable :public ProcessExecutableBase {
	public:
		int listen_fd;  /*监听socket描述符*/
		int connect_fd;  /*连接socket描述符*/

	public:
		/*
		* @brief 构造函数
		*/
		ProcessExecutable() : listen_fd(-1), connect_fd(-1) {};

		/*
		* @brief 有参构造函数
		*/
		ProcessExecutable(int _fd) : listen_fd(_fd), connect_fd(-1) {};

		/*
		* @brief 析构函数
		*/
		~ProcessExecutable() {};

		/*
		* @brief 重写子线程执行函数
		*/
		virtual int process() override {
			struct sockaddr_in clientaddr;  /*客户端地址和端口*/
			socklen_t clientaddr_len = sizeof(clientaddr); /*结构大小*/

			printf("sub process listen_fd: %d\n", listen_fd);

			// 和客户端建立连接
			connect_fd = accept(listen_fd,
				reinterpret_cast<struct sockaddr*>(&clientaddr),
				&clientaddr_len);
			if (connect_fd == -1) {
				printf("error: couldn't accept connect fd.\n");
				return -1;
			}
			else {
				printf("another connect is open.\n");
			}

			// 处理客户端的请求
			WebServerUtils::doit(connect_fd);
			if (close(connect_fd) == -1) {
				printf("error: couldn't close connect fd.\n");
			}
			else {
				printf("another connect is closed.\n");
			}
			return 0;
		}
	};

	printf("Tiny server is running!\n");

	int port = 8080;  /*服务器端口*/
	// 获取服务器端口socket描述符
	// 必须在创建子进程之前完成对listen_fd的构建
	int listen_fd = WebServerUtils::open_listenfd(port);
	if (listen_fd == -1) {
		printf("error: couldn't get listen_fd!\n");
		return -1;
	}
#ifdef DEBUG
	printf("listen_fd: %d\n", listen_fd);
#endif // DEBUG	

	// 获取线程池实例
	ProcessPool<ProcessExecutable>& process_pool = ProcessPool<ProcessExecutable>::get_instance(4);

	if (process_pool.sub_index == -1) {
		/*主进程才做*/			

		// 创建epoll事件监听表
		int epollfd;  /*epoll事件监听表描述符*/
		if ((epollfd = epoll_create(100)) == -1) {
			printf("error: couldn't creat epoll table!\n");
			return -1;
		}

		// 将信号监听加入epoll，统一事件源
		core::SignalUtils sig_utils(epollfd);

		// 将连接请求监听加入epoll，统一事件源
		core::EpollUtils::epoll_add(epollfd, listen_fd);

		epoll_event events[core::MAX_EVENT_NUMBER];  /*监听事件表*/
		int is_stop = false;  /*是否结束主循环*/
		while (!is_stop) {
			// epoll监听事件
			int number = epoll_wait(epollfd, events, core::MAX_EVENT_NUMBER, -1);
			if ((number < 0) && (errno != EINTR)) {
				core::unix_error("error: couldn't listen epoll table!\n");
				break;
			}

			for (int i = 0; i < number; ++i) {
				int event_fd = events[i].data.fd;
				if ((event_fd == sig_utils.sig_pipefd[1]) && (events[i].events & EPOLLIN)) {
					/*事件来源于信号管道且可读*/
					char signals[1024];
					// 接收信号
					int sig_num = recv(sig_utils.sig_pipefd[1], signals, sizeof(signals), 0);
					if (sig_num == -1) {
						// 信号接收失败
						continue;
					}
					else {
						// 处理信号
						for (int j = 0; j < sig_num; ++j) {
							switch (signals[j]) {
							case SIGCHLD:  /*回收子进程*/
							{
								pid_t _pid;
								int _stat;
								while ((_pid = waitpid(-1, &_stat, WNOHANG)) > 0) {
									// 寻找该子进程对应的元数据
									for (int k = 0; k < process_pool.sub_process_n; ++k) {
										if (process_pool.sub_process_metas[k].sub_pid == _pid) {
#ifdef DEBUG
											printf("child process %d join.\n", k);
#endif // DEBUG
											// 关闭父端的管道
											close(process_pool.sub_process_metas[k].sub_pipefd[0]);
											// 将元数据中的子进程PID记录为-1
											process_pool.sub_process_metas[k].sub_pid = -1;
											// 存活子进程数量-1
											--process_pool.sub_alive_process_n;
										}
									}
								}
								if (process_pool.sub_alive_process_n == 0) {
									// 所有子进程都退出了
									is_stop = true;
								}
								break;
							}
							case SIGTERM:  /*软件终止，父进程终止*/
							case SIGINT:  /*键盘中断，父进程终止*/
							{
#ifdef DEBUG
								printf("kill all child processes.\n");
#endif // DEBUG
								for (int k = 0; k < process_pool.sub_process_n; ++k) {
									int _pid = process_pool.sub_process_metas[k].sub_pid;
									if (_pid != -1) {
										kill(_pid, SIGTERM);
									}
								}
								is_stop = true;
								break;
							}
							default:
							{
								break;
							}
							}
						}
					}
				}
				else if (event_fd == listen_fd) {
					/*事件来源于socket请求*/
					ProcessExecutable task(listen_fd);
					process_pool.execute(task);
				}
			}
		}

		printf("main thread end.\n");
	}

#endif

	return 0;
}