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
	/*����IOʹ��ʾ��*/

	char c;
	while (BaseIO::read_n(STDIN_FILENO, &c, 1) != 0) {
		BaseIO::written_n(STDOUT_FILENO, &c, 1);
	}
#endif

#if 0
	/*������IO����ʹ��ʾ��*/

	std::string in_filename("/home/jeremy_wsl/projects/TinyWebserver/test/in.txt");
	std::string out_filename("/home/jeremy_wsl/projects/TinyWebserver/test/out.txt");

	// ���ļ�������������
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

	// ���庯��ָ�����
	using read_fn = ssize_t(*)(buffer_t*, void*, size_t);
	//typedef ssize_t(*read_fn)(buffer_t*, void*, size_t);
	//read_fn fnp = BufferIO::buffer_read;
	//read_fn fnp = BufferIO::buffer_read_n;
	read_fn fnp = BufferIO::buffer_read_line;

	// ���ļ��ж��뵽content
	while ((in_bn = fnp(in_bp, content, N)) > 0)
	{
		content[in_bn] = '\0';
		printf("\nanother read, content:\n");
		printf("%s\n", content);
		printf("another read end.\n");

		// ��contentд�����ļ�
		out_bn = BufferIO::buffer_written_n(out_bp, content, in_bn);
		printf("another write %d bytes.\n", out_bn);
	}
	//out_bn = BufferIO::buffer_written_flush(out_bp);
	//printf("another write %d bytes.\n", out_bn);

	// �ر��ļ�
	if (!BufferIO::buffer_close(in_bp)) {
		printf("close in file successfully.\n");
	}
	if (!BufferIO::buffer_close(out_bp)) {
		printf("close out file successfully.\n");
	}
#endif

#if 0
	/*���̳�ʹ��ʾ��1*/
	ProcessPool& process_pool = ProcessPool::get_instance(4);
	//printf("main instance get.\n");


	// �������̳�
	//process_pool->run();

	ProcessExecutable obj;
	for (int i = 0; i < 10; ++i) {
		process_pool.execute(obj);
	}

	//process_pool->destory();
#endif

#if 0
	/*���̳�ʹ��ʾ��2*/

	/*
	* @brief ���̳�ִ���࣬��װ�ӽ���ִ�еĺ���
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

	// ��ȡ���̳�ʵ��
	ProcessPool<ProcessExecutable>& process_pool = ProcessPool<ProcessExecutable>::get_instance(4);
	// �������̳�
	//process_pool->run();

	ProcessExecutable obj;
	for (int i = 0; i < 10; ++i) {
		process_pool.execute(obj);
	}

	// �����ӽ���
	//process_pool->destory();
#endif

#if 0
	class A{};
	std::shared_ptr<int> shared_null = nullptr;      // ���Խ�shared_ptr��ʼ��Ϊnullptr
	std::unique_ptr<int> unique_null = nullptr;      // ���Խ�unique_ptr��ʼ��Ϊnullptr
	//std::weak_ptr<int> weak_null = nullptr;        // ���ܽ�weak_ptr��ʼ��Ϊnullptr

	std::shared_ptr<int> shared_obj = std::make_shared<int>();             // ������new�������ͳ�ʼ��shared_ptr���Ƽ���
	//std::shared_ptr<int> shared_obj(new int);                            // ������new�������ͳ�ʼ��shared_ptr
	//std::shared_ptr<int> shared_obj = std::shared_ptr<int>(new int);           // ������new�������ͳ�ʼ��shared_ptr������һ��д���ȼۣ�
	std::unique_ptr<int> unique_obj(new int);                              // ������new�������ͳ�ʼ��unique_ptr
	//std::unique_ptr<int> unique_obj = std::unique_ptr<int>(new int);           // ������new�������ͳ�ʼ��unique_ptr������һ��д���ȼۣ�
	//std::weak_ptr<int> weak_obj(new int);                                // ������new�������ͳ�ʼ��weak_ptr

	std::shared_ptr<A> shared_A = std::make_shared<A>();                   // �����ö���ָ���ʼ��shared_ptr���Ƽ���
	//std::shared_ptr<A> shared_A(new A);                                  // �����ö���ָ���ʼ��shared_ptr
	//std::shared_ptr<A> shared_A = std::shared_ptr<A>(new A);             // �����ö���ָ���ʼ��shared_ptr������һ��д���ȼۣ�
	std::unique_ptr<A> unique_A(new A);                                    // �����ö���ָ���ʼ��unique_ptr
	//std::unique_ptr<A> unique_A = std::unique_ptr<A>(new A);             // �����ö���ָ���ʼ��unique_ptr������һ��д���ȼۣ�
	//std::weak_ptr<A> weak_A(new A);                                      // �����ö���ָ���ʼ��weak_ptr

	std::shared_ptr<int> shared_p1(shared_obj);                            // ������shared_ptr��ʼ��shared_ptr
	std::shared_ptr<int> shared_p2(std::move(unique_obj));                 // ������unique_ptr��ʼ��shared_ptr��unique_ptr�Զ��ÿ�
	std::unique_ptr<int> unique_p(std::move(unique_obj));                  // ֻ����unique_ptr��ʼ��unique_ptr
	std::weak_ptr<int> weak_p1(shared_obj);                                // ������shared_ptr��ʼ��weak_ptr
	//std::weak_ptr<int> weak_p2(unique_obj);                              // ������unique_ptr��ʼ��weak_ptr
	std::weak_ptr<int> weak_p3(weak_p1);                                   // ������weak_ptr��ʼ��weak_ptr
	std::shared_ptr<int> shared_p3(weak_p1);                               // ������weak_ptr��ʼ��shared_ptr


	shared_null = shared_obj;                                              // ������shared_ptr��ֵ��shared_ptr
	shared_null = std::move(unique_obj);                                   // ������unique_ptr��ֵ��shared_ptr��unique_ptr�Զ��ÿ�
	//shared_null = weak_p1;                                               // ������weak_ptr��ֵ��shared_ptr
	unique_null = std::move(unique_obj);                                   // ֻ����unique_ptr��ֵ��unique_ptr��ǰһ��unique_ptr�Զ��ÿ�
	weak_p3 = shared_obj;                                                  // ������shared_ptr��ֵ��weak_ptr
	//weak_p3 = std::move(unique_obj);                                     // ������unique_ptr��ֵ��weak_ptr
	weak_p3 = weak_p1;                                                     // ������weak_ptr��ֵ��weak_ptr

	shared_null = nullptr;                            // ���Խ�nullptr��ֵ��shared_ptr
	unique_null = nullptr;                            // ���Խ�nullptr��ֵ��unique_ptr
    //weak_p1 = nullptr;                              // ���ܽ�nullptr��ֵ��weak_ptr

	int num = 4;
	std::shared_ptr<int> shared_int(&num);                              // ��������ָͨ���ʼ��shared_ptr
	std::unique_ptr<int> unique_int(&num);                              // ��������ָͨ���ʼ��unique_ptr
	//std::weak_ptr<int> weak_int(&num);                                // ��������ָͨ���ʼ��weak_ptr
	
	//shared_null = &num;                                               // ����ֱ�ӽ���ָͨ�븳ֵ��shared_ptr
	//unique_null = &num;                                               // ����ֱ�ӽ���ָͨ�븳ֵ��unique_ptr
	//weak_p1 = &num;                                                   // ����ֱ�ӽ���ָͨ�븳ֵ��weak_ptr

	shared_null = std::make_shared<int>(num);                           // ���Խ���װ�����ָͨ�븳ֵ��shared_ptr
	unique_null = std::unique_ptr<int>(&num);                           // ���Խ���װ�����ָͨ�븳ֵ��unique_ptr
	//weak_p1 = std::weak_ptr<int>(&num);                               // ���ܽ���װ�����ָͨ�븳ֵ��weak_ptr
#endif

#if 0
	/*�̳߳�ʹ��ʾ��*/

	/*
	* @brief �̳߳�ִ���࣬��װ���߳�ִ�еĺ���
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

	// ��ȡ�̳߳�ʵ��
	ThreadPool& thread_pool = ThreadPool::get_instance(4);

	ThreadExecutable obj;
	for (int i = 0; i < 10; ++i) {
		thread_pool.execute(&obj);
	}

	// ��������ֻ��ָ�룬��Դ�������߳��ڶ�����������������
	// ������߳���Ҫ�ȴ���������������ֹ
	thread_pool.wait();

	printf("main thread end.\n");
#endif

#if 0
	/*CGI���򣬿����ڷ��������÷��ض�̬����*/

	char* buf, * p;
	char arg1[core::MAX_LINE], arg2[core::MAX_LINE], content[core::MAX_LINE];
	int n1 = 0, n2 = 0;

	// ��ȡ��������
	if ((buf = getenv("QUERY_STRING")) != NULL) {
		p = strchr(buf, '&');
		*p = '\0';
		strcpy(arg1, buf);
		strcpy(arg2, p + 1);
		n1 = atoi(arg1);
		n2 = atoi(arg2);
	}

	// ������Ӧ��Ϣ��
	sprintf(content, "Welcome to add.com: ");
	sprintf(content, "%sTHE Internet addition portal.\r\n<p>", content);
	sprintf(content, "%sThe answer is %d + %d = %d\r\n<p>",
		content, n1, n2, n1 + n2);
	sprintf(content, "%sThanks for visiting!\r\n", content);

	// �������µ���Ӧ��Ϣͷ
	printf("Content-length: %d\r\n", strlen(content));
	printf("Content-type: text/html\r\n\r\n");
	printf("%s", content);
	fflush(stdout);

#endif	

#if 0
	/*���̷߳�����ʾ��*/

	printf("Tiny server is running!\n");

	int port = 8080;  /*�������˿�*/
	// ��ȡ�˿�socket������
	int listen_fd = WebServerUtils::open_listenfd(port);
	if (listen_fd == -1) {
		printf("error: couldn't get listen_fd!\n");
		return -1;
	}

	struct sockaddr_in clientaddr;  /*�ͻ��˵�ַ�Ͷ˿�*/
	socklen_t clientaddr_len = sizeof(clientaddr); /*�ṹ��С*/
	// ���ܷ���connect_fd�Ŀͻ����Ѿ��رյ��������Գ��Է������ݸ��ͻ���
	// �Ӷ�����Broken pipe�쳣
	// ��Ҫͨ������SIGPIPE�ź���������쳣�������������ֹ
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

	// ��̬���ݷ��ʣ�http://localhost:8080/home.html
	// ��̬���ݷ��ʣ�http://localhost:8080/cgi_bin/adder?15&2
#endif

#if 0
	/*
	* ���Ի���ָ��ʵ���ǻᵼ��ջ�����ݵ���д���������Է��ֵĴ���	
	*/

	/*
	* @brief �̳߳�ִ���࣬��װ���߳�ִ�еĺ���
	*/
	class ThreadExecutable :public ThreadExecutableBase {
	private:
		int test;
		int listen_fd;  /*����socket������*/
		int connect_fd;  /*����socket������*/

	public:
		/*
		* @brief ���캯��
		*/
		ThreadExecutable(int _fd, int _t) : listen_fd(_fd), connect_fd(-1), test(_t) {};

		/*
		* @brief ��������
		*/
		~ThreadExecutable() {
			printf("main: ThreadExecutable destories.\n");
		}

		/*
		* @brief ��д���߳�ִ�к���
		*/
		virtual int process() override {
			printf("thread: process begins.\n");

			// ��ͣ2��
			std::this_thread::sleep_for(std::chrono::seconds(5));

			printf("thread %d\n", test);

			struct sockaddr_in clientaddr;  /*�ͻ��˵�ַ�Ͷ˿�*/
			socklen_t clientaddr_len = sizeof(clientaddr); /*�ṹ��С*/

			// �Ϳͻ��˽�������
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

			// ����ͻ��˵�����
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

	// ��ȡ�̳߳�ʵ��
	ThreadPool<ThreadExecutable>& thread_pool = ThreadPool<ThreadExecutable>::get_instance(4);

	int port = 8080;  /*�������˿�*/
	// ��ȡ�������˿�socket������
	int listen_fd = WebServerUtils::open_listenfd(port);
	if (listen_fd == -1) {
		printf("error: couldn't get listen_fd!\n");
		return -1;
	}

	// ����epoll�¼�������
	int epollfd;  /*epoll�¼�������������*/
	if ((epollfd = epoll_create(100)) == -1) {
		printf("error: couldn't creat epoll table!\n");
		return -1;
	}

	// ���źż�������epoll��ͳһ�¼�Դ
	core::SignalUtils sig_utils(epollfd);

	// �����������������epoll��ͳһ�¼�Դ
	core::EpollUtils::epoll_add(epollfd, listen_fd);

	epoll_event events[core::MAX_EVENT_NUMBER];  /*�����¼���*/
	int is_stop = false;  /*�Ƿ������ѭ��*/
	int cnt = 0;
	while (!is_stop) {
		// epoll�����¼�
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
				/*�¼���Դ���źŹܵ��ҿɶ�*/
				char signals[1024];
				// �����ź�
				int sig_num = recv(sig_utils.sig_pipefd[1], signals, sizeof(signals), 0);
				if (sig_num == -1) {
					// �źŽ���ʧ��
					continue;
				}
				else {
					// �����ź�
					for (int j = 0; j < sig_num; ++j) {
						switch (signals[j]) {
						case SIGCHLD:  /*�����ӽ���*/
						{
							pid_t _pid;
							int _stat;
							while ((_pid = waitpid(-1, &_stat, WNOHANG)) > 0) {
								// ѭ�������ӽ���
							}
							break;
						}
						case SIGTERM:  /*�����ֹ����������ֹ*/
						case SIGINT:  /*�����жϣ���������ֹ*/
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
				/*�¼���Դ��socket����*/
				ThreadExecutable task(listen_fd, cnt++);
				tmp_ptr = (void*)&task;
				thread_pool.execute(task);
			}
			//memset(tmp_ptr,1,sizeof(ThreadExecutable));
			memcpy(tmp_ptr, &tmp_task, sizeof(ThreadExecutable));
			printf("%d\n", sizeof(ThreadExecutable));
		}
	}

	// ��������ֻ��ָ�룬��Դ�������߳��ڶ�����������������
	// ������߳���Ҫ�ȴ���������������ֹ
	thread_pool.wait();

	printf("main thread end.\n");
#endif

#if 1
	/*���̳߳�ʵ�ֵķ�����ʾ��*/

	/*
	* @brief �̳߳�ִ���࣬��װ���߳�ִ�еĺ���
	*/
	class ThreadExecutable :public ThreadExecutableBase {
	private:
		int listen_fd;  /*����socket������*/
		int connect_fd;  /*����socket������*/

	public:
		/*
		* @brief ���캯��
		*/
		ThreadExecutable(int _fd) : listen_fd(_fd), connect_fd(-1) {};

		/*
		* @brief ��������
		*/
		~ThreadExecutable() {};

		/*
		* @brief ��д���߳�ִ�к���
		*/
		virtual int process() override {
			struct sockaddr_in clientaddr;  /*�ͻ��˵�ַ�Ͷ˿�*/
			socklen_t clientaddr_len = sizeof(clientaddr); /*�ṹ��С*/

			// �Ϳͻ��˽�������
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

			// ����ͻ��˵�����
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

	int port = 8080;  /*�������˿�*/
	// ��ȡ�������˿�socket������
	// ������ڴ������߳�֮ǰ��ɶ�listen_fd�Ĺ������������ں˹��������ڴ���֮����߳�Ҳ�ɼ�
	int listen_fd = WebServerUtils::open_listenfd(port);
	if (listen_fd == -1) {
		printf("error: couldn't get listen_fd!\n");
		return -1;
	}

	// ��ȡ�̳߳�ʵ��
	ThreadPool<ThreadExecutable>& thread_pool = ThreadPool<ThreadExecutable>::get_instance(4);

	// ����epoll�¼�������
	int epollfd;  /*epoll�¼�������������*/
	if ((epollfd = epoll_create(100)) == -1) {
		printf("error: couldn't creat epoll table!\n");
		return -1;
	}

	// ���źż�������epoll��ͳһ�¼�Դ
	core::SignalUtils sig_utils(epollfd);

	// �����������������epoll��ͳһ�¼�Դ
	core::EpollUtils::epoll_add(epollfd, listen_fd);

	epoll_event events[core::MAX_EVENT_NUMBER];  /*�����¼���*/
	int is_stop = false;  /*�Ƿ������ѭ��*/
	while (!is_stop) {
		// epoll�����¼�
		int number = epoll_wait(epollfd, events, core::MAX_EVENT_NUMBER, -1);
		if ((number < 0) && (errno != EINTR)) {
			core::unix_error("error: couldn't listen epoll table!\n");
			break;
		}

		for (int i = 0; i < number; ++i) {
			int event_fd = events[i].data.fd;
			if ((event_fd == sig_utils.sig_pipefd[1]) && (events[i].events & EPOLLIN)) {
				/*�¼���Դ���źŹܵ��ҿɶ�*/
				char signals[1024];
				// �����ź�
				int sig_num = recv(sig_utils.sig_pipefd[1], signals, sizeof(signals), 0);
				if (sig_num == -1) {
					// �źŽ���ʧ��
					continue;
				}
				else {
					// �����ź�
					for (int j = 0; j < sig_num; ++j) {
						switch (signals[j]) {
						case SIGCHLD:  /*�����ӽ���*/
						{
							pid_t _pid;
							int _stat;
							while ((_pid = waitpid(-1, &_stat, WNOHANG)) > 0) {
								// ѭ�������ӽ���
							}
							break;
						}
						case SIGTERM:  /*�����ֹ����������ֹ*/
						case SIGINT:  /*�����жϣ���������ֹ*/
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
				/*�¼���Դ��socket����*/
				ThreadExecutable task(listen_fd);
				thread_pool.execute(task);
			}
		}
	}

	// ��������ֻ��ָ�룬��Դ�������߳��ڶ�����������������
	// ������߳���Ҫ�ȴ���������������ֹ
	//thread_pool.wait();

	printf("main thread end.\n");
#endif

#if 0
	/*�ý��̳�ʵ�ֵķ�����ʾ��*/

	/*
	* @brief ���̳�ִ���࣬��װ�ӽ���ִ�еĺ���
	*/
	class ProcessExecutable :public ProcessExecutableBase {
	public:
		int listen_fd;  /*����socket������*/
		int connect_fd;  /*����socket������*/

	public:
		/*
		* @brief ���캯��
		*/
		ProcessExecutable() : listen_fd(-1), connect_fd(-1) {};

		/*
		* @brief �вι��캯��
		*/
		ProcessExecutable(int _fd) : listen_fd(_fd), connect_fd(-1) {};

		/*
		* @brief ��������
		*/
		~ProcessExecutable() {};

		/*
		* @brief ��д���߳�ִ�к���
		*/
		virtual int process() override {
			struct sockaddr_in clientaddr;  /*�ͻ��˵�ַ�Ͷ˿�*/
			socklen_t clientaddr_len = sizeof(clientaddr); /*�ṹ��С*/

			printf("sub process listen_fd: %d\n", listen_fd);

			// �Ϳͻ��˽�������
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

			// ����ͻ��˵�����
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

	int port = 8080;  /*�������˿�*/
	// ��ȡ�������˿�socket������
	// �����ڴ����ӽ���֮ǰ��ɶ�listen_fd�Ĺ���
	int listen_fd = WebServerUtils::open_listenfd(port);
	if (listen_fd == -1) {
		printf("error: couldn't get listen_fd!\n");
		return -1;
	}
#ifdef DEBUG
	printf("listen_fd: %d\n", listen_fd);
#endif // DEBUG	

	// ��ȡ�̳߳�ʵ��
	ProcessPool<ProcessExecutable>& process_pool = ProcessPool<ProcessExecutable>::get_instance(4);

	if (process_pool.sub_index == -1) {
		/*�����̲���*/			

		// ����epoll�¼�������
		int epollfd;  /*epoll�¼�������������*/
		if ((epollfd = epoll_create(100)) == -1) {
			printf("error: couldn't creat epoll table!\n");
			return -1;
		}

		// ���źż�������epoll��ͳһ�¼�Դ
		core::SignalUtils sig_utils(epollfd);

		// �����������������epoll��ͳһ�¼�Դ
		core::EpollUtils::epoll_add(epollfd, listen_fd);

		epoll_event events[core::MAX_EVENT_NUMBER];  /*�����¼���*/
		int is_stop = false;  /*�Ƿ������ѭ��*/
		while (!is_stop) {
			// epoll�����¼�
			int number = epoll_wait(epollfd, events, core::MAX_EVENT_NUMBER, -1);
			if ((number < 0) && (errno != EINTR)) {
				core::unix_error("error: couldn't listen epoll table!\n");
				break;
			}

			for (int i = 0; i < number; ++i) {
				int event_fd = events[i].data.fd;
				if ((event_fd == sig_utils.sig_pipefd[1]) && (events[i].events & EPOLLIN)) {
					/*�¼���Դ���źŹܵ��ҿɶ�*/
					char signals[1024];
					// �����ź�
					int sig_num = recv(sig_utils.sig_pipefd[1], signals, sizeof(signals), 0);
					if (sig_num == -1) {
						// �źŽ���ʧ��
						continue;
					}
					else {
						// �����ź�
						for (int j = 0; j < sig_num; ++j) {
							switch (signals[j]) {
							case SIGCHLD:  /*�����ӽ���*/
							{
								pid_t _pid;
								int _stat;
								while ((_pid = waitpid(-1, &_stat, WNOHANG)) > 0) {
									// Ѱ�Ҹ��ӽ��̶�Ӧ��Ԫ����
									for (int k = 0; k < process_pool.sub_process_n; ++k) {
										if (process_pool.sub_process_metas[k].sub_pid == _pid) {
#ifdef DEBUG
											printf("child process %d join.\n", k);
#endif // DEBUG
											// �رո��˵Ĺܵ�
											close(process_pool.sub_process_metas[k].sub_pipefd[0]);
											// ��Ԫ�����е��ӽ���PID��¼Ϊ-1
											process_pool.sub_process_metas[k].sub_pid = -1;
											// ����ӽ�������-1
											--process_pool.sub_alive_process_n;
										}
									}
								}
								if (process_pool.sub_alive_process_n == 0) {
									// �����ӽ��̶��˳���
									is_stop = true;
								}
								break;
							}
							case SIGTERM:  /*�����ֹ����������ֹ*/
							case SIGINT:  /*�����жϣ���������ֹ*/
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
					/*�¼���Դ��socket����*/
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