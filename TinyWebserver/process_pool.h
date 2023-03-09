#pragma once

#include "core.h"

/*
* @brief 进程池执行类抽象类
*/
class ProcessExecutableBase {
public:
	virtual int process() = 0;
};

/*
* @brief 进程元数据
*/
class ProcessMeta {
public:
	pid_t sub_pid;  /*子进程PID，父进程-1*/
	int sub_pipefd[2];  /*父子进程的通信管道，父[0]子[1]*/

public:
	/*
	* @brief 父进程的无参构造函数
	* @param
	*/
	ProcessMeta() :sub_pid(-1) {};
};

/*
* @brief 进程池
*/
template<typename T>
class ProcessPool {
private:
	const int MAX_PROCESS_NUMBER = core::MAX_PROCESS_NUMBER;  /*最大子进程数量*/
	const int MAX_EVENT_NUMBER = core::MAX_EVENT_NUMBER;  /*epoll最大监听数*/
	
	static ProcessPool* instance;  /*进程池实例*/
	static ProcessMeta* sub_process_metas;  /*子进程元数据表*/

	int sub_process_n;  /*子进程数量*/
	int sub_alive_process_n;  /*存活子进程数量*/
	int sub_index;  /*子进程序号，父进程-1*/

	static int sig_pipefd[2];  /*内核与进程的信号管道，内核[0]进程[1]*/
	int epollfd;  /*epoll内核事件表*/

	bool is_stop;  /*是否终止进程循环*/

private:
	/*
	* @brief 进程池构造函数
	* @param process_n => 进程池子进程数量
	*/
	ProcessPool(int process_n);

	/*
	* @brief 进程池复制构造函数（未实现）
	*/
	ProcessPool(const ProcessPool& obj) {};

	/*
	* @brief 进程池析构函数
	*/
	~ProcessPool();	

	/*
	* @brief 父进程启动函数
	*/
	void run_parent();

	/*
	* @brief 子进程启动函数
	*/
	void run_child();

	/*
	* @brief 启动进程池
	*/
	void run();

private:
	/*
	* @brief 设置文件描述符非阻塞
	* @param fd => 文件描述符
	* @retval old_option => 旧选项
	*/
	int set_nonblocking(int fd);

	/*
	* @brief 为epoll增加新的监听文件
	* @param
	*/
	void epoll_add(int epollfd, int fd);

	/*
	* @brief 为epoll移除监听文件
	* @param
	*/
	void epoll_remove(int epollfd, int fd);

private:
	/*
	* @brief 信号处理函数
	* @param sig => 信号
	*/
	static void sig_handler(int sig);

	/*
	* @brief 为信号绑定信号处理函数
	* @param sig => 信号
	* @param handler => 信号处理函数
	* @param restart => 是否重新调用被该信号终止的系统调用
	*/
	void sig_add_handler(int sig, void(*handler)(int), bool restart = true);

	/*
	* @brief 信号初始化设置
	*/
	void sig_init();

private:
	/*
	* @brief 回收已经终止的子进程，仅父进程执行
	* @retval wait_num  => 回收的子进程数量
	*/
	int check_sub_process();

	/*
	* @brief 分配一个子进程（尚未作资源平衡），仅父进程执行
	* @retval sub_process_index  => 子进程序号
	* @retval -1 => error
	*/
	int get_sub_process();

public:
	/*
	* @brief 获得单例模式下的实例
	* @param process_n => 进程池子进程数量
	*/
	static ProcessPool* get_instance(int process_n = 4);	

	/* 
	* @brief 用子进程执行，仅父进程执行
	* @retval 0 => success
	* @retval -1 => error
	*/
	int execute(T obj);

	/*
	* @brief 结束所有子进程，仅父进程执行
	* @retval 0 => success
	* @retval -1 => error
	*/
	int destory();
};

template<typename T>
ProcessPool<T>* ProcessPool<T>::instance = nullptr;
template<typename T>
ProcessMeta* ProcessPool<T>::sub_process_metas = nullptr;
template<typename T>
int ProcessPool<T>::sig_pipefd[2] = { -1, -1 };

template<typename T>
ProcessPool<T>::ProcessPool(int process_n) :
	sub_process_n(process_n), sub_alive_process_n(process_n),
	sub_index(-1), epollfd(-1), is_stop(false) {
	assert((process_n > 0) && (process_n <= MAX_PROCESS_NUMBER));

#ifdef DEBUG
	printf("ProcessPool constructor\n");
#endif // DEBUG


	// 新建子进程元数据表
	sub_process_metas = new ProcessMeta[sub_process_n];

	for (int i = 0; i < sub_process_n; ++i) {
		// 建立父子进程的管道通信
		if (socketpair(PF_UNIX, SOCK_STREAM, 0,
			sub_process_metas[i].sub_pipefd) == -1)
		{
			core::unix_error("Process pool pipe create error");
			exit(1);
		}

		// 创建子进程
		sub_process_metas[i].sub_pid = fork();
		if (sub_process_metas[i].sub_pid > 0) {
#ifdef DEBUG
			printf("%d child process pid is %d\n", i, sub_process_metas[i].sub_pid);
#endif // DEBUG

			// 父进程关闭管道[1]端
			if (close(sub_process_metas[i].sub_pipefd[1]) == -1) {
				core::unix_error("Process pool pipe parent close error");
				exit(1);
			}
			// 继续创建子进程
			continue;
		}
		else {
			// 子进程关闭管道[0]端
			if (close(sub_process_metas[i].sub_pipefd[0]) == -1) {
				core::unix_error("Process pool pipe child close error");
				exit(1);
			}
			// 记录当前进程的序号
			sub_index = i;
			// 终止创建子进程
			break;
		}
	}
}

template<typename T>
ProcessPool<T>::~ProcessPool()
{
	if (instance != nullptr) {
		delete[]instance;
	}
}

template<typename T>
void ProcessPool<T>::run_parent() {
	// 创建epoll事件监听表
	if ((epollfd = epoll_create(100)) == -1) {
		core::unix_error("Run parent epoll create error");
		exit(1);
	}

	// 初始化信号统一事件源
	sig_init();

#if 0
	epoll_event events[MAX_EVENT_NUMBER];
	while (!is_stop) {
		// 父进程主循环
		int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, 10);
		if ((number < 0) && (errno != EINTR)) {
			core::unix_error("Run parent epoll listen error");
			break;
		}

		for (int i = 0; i < number; ++i) {
			int event_fd = events[i].data.fd;
			if ((event_fd == sig_pipefd[1]) && (events[i].events & EPOLLIN)) {   /*信号管道且可读*/
				char signals[1024];
				// 接收信号
				int sig_num = recv(sig_pipefd[1], signals, sizeof(signals), 0);
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
								for (int k = 0; k < sub_process_n; ++k) {
									if (sub_process_metas[k].sub_pid == _pid) {
#ifdef DEBUG
										printf("child process %d join.\n", k);
#endif // DEBUG
										// 关闭父端的管道
										close(sub_process_metas[k].sub_pipefd[0]);
										// 将元数据中的子进程PID记录为-1
										sub_process_metas[k].sub_pid = -1;
										// 存活子进程数量-1
										--sub_alive_process_n;
									}
								}
							}
							if (sub_alive_process_n == 0) {
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
							for (int k = 0; k < sub_process_n; ++k) {
								int _pid = sub_process_metas[k].sub_pid;
								if (_pid != -1) {
									kill(_pid, SIGTERM);
								}
							}
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
			else {
				// 处理别的事件
			}
		}
	}

#endif

	// 关闭epoll事件监听表
	close(epollfd);
}

template<typename T>
void ProcessPool<T>::run_child() {
	// 创建epoll事件监听表
	if ((epollfd = epoll_create(100)) == -1) {
		core::unix_error("Run child epoll create error");
		exit(1);
	}

	// 初始化信号统一事件源
	sig_init();

	// 和父进程的通信管道
	int sub_pipefd = sub_process_metas[sub_index].sub_pipefd[1];
	// 监听该通信管道
	epoll_add(epollfd, sub_pipefd);

	epoll_event events[MAX_EVENT_NUMBER];
	while (!is_stop) {
		// 子进程主循环
		int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
		if ((number < 0) && (errno != EINTR)) {
			core::unix_error("Run child epoll listen error");
			break;
		}

		for (int i = 0; i < number; ++i) {
			int event_fd = events[i].data.fd;
			if ((event_fd == sig_pipefd[1]) && (events[i].events & EPOLLIN)) {  /*信号管道且可读*/
				char signals[1024];
				// 接收信号
				int sig_num = recv(sig_pipefd[1], signals, sizeof(signals), 0);
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
							break;
						}
						case SIGTERM:  /*软件终止，父进程终止*/
						case SIGINT:  /*键盘中断，父进程终止*/
						{
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
			else if ((event_fd == sub_pipefd) && (events[i].events & EPOLLIN)) {  /*父子管道且可读*/
				T obj;
				int recv_n = recv(event_fd, reinterpret_cast<char*>(&obj), sizeof(obj), 0);
#ifdef DEBUG
				printf("child recv_n: %d bytes.\n", recv_n);
#endif // DEBUG

				if (((recv_n < 0) && (errno != EAGAIN)) || recv_n == 0) {
					// 跳过处理
					core::unix_error("Run child recv error");
					continue;
				}
				else {
					// 处理业务逻辑
					printf("child process handling msg: %d bytes.\n", recv_n);
					obj.process();
				}
			}
			else {
				// 处理别的事件
			}
		}
	}
}

template<typename T>
void ProcessPool<T>::run() {
	if (sub_index == -1) {
		run_parent();
	}
	else {
		run_child();
	}
}

template<typename T>
int ProcessPool<T>::set_nonblocking(int fd) {
	// 获得旧选项
	int old_option = fcntl(fd, F_GETFL);
	// 设置新选项，将文件描述符设置为非阻塞
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	// 返回旧选项
	return old_option;
}

template<typename T>
void ProcessPool<T>::epoll_add(int epollfd, int fd) {
	assert(epollfd != -1 && fd > 0);

	epoll_event event;
	event.data.fd = fd;
	// 事件就绪条件为：数据可读且边缘触发
	event.events = EPOLLIN | EPOLLET;
	// 将fd注册到epollfd上
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	// 设置进程监听的文件描述符非阻塞
	set_nonblocking(fd);
}

template<typename T>
void ProcessPool<T>::epoll_remove(int epollfd, int fd) {
	assert(epollfd != -1 && fd > 0);

	// 将fd从epollfd上移除
	epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
	// 关闭文件
	close(fd);
}

template<typename T>
void ProcessPool<T>::sig_handler(int sig) {
	int save_errno = errno;
	// 从内核发送给进程的消息就是要处理的信号，仅1字节
	int msg = sig;
	// 通过管道发送给进程
	send(sig_pipefd[0], reinterpret_cast<char*>(&msg), 1, 0);
	// 恢复错误信息
	errno = save_errno;
}

template<typename T>
void ProcessPool<T>::sig_add_handler(int sig, void(*handler)(int), bool restart) {

	struct sigaction sa;
	// 初始化结构
	memset(&sa, '\0', sizeof(sa));

	// 绑定信号处理函数
	sa.sa_handler = handler;
	if (restart) {
		sa.sa_flags |= SA_RESTART;
	}
	// 设置所有信号为掩码信号，禁止在处理函数引起中断
	sigfillset(&sa.sa_mask);
	// 用sa设置sig
	if (sigaction(sig, &sa, nullptr) == -1) {
		core::unix_error("Signal add handler error");
		exit(1);
	}
}

template<typename T>
void ProcessPool<T>::sig_init()
{
	// 建立内核和进程的通信管道
	if (socketpair(PF_UNIX, SOCK_STREAM, 0, sig_pipefd) == -1) {
		core::unix_error("Run parent/child signal pipe create error");
		exit(1);
	}

	// 设置内核写的管道描述符非阻塞
	set_nonblocking(sig_pipefd[0]);
	// epoll监听进程读的管道描述符，统一事件源
	epoll_add(epollfd, sig_pipefd[1]);

	// 为常见信号绑定信号处理函数
	sig_add_handler(SIGCHLD, sig_handler);
	sig_add_handler(SIGTERM, sig_handler);
	sig_add_handler(SIGINT, sig_handler);
	sig_add_handler(SIGPIPE, sig_handler);
}

template<typename T>
ProcessPool<T>* ProcessPool<T>::get_instance(int process_n)
{
	if (instance == nullptr) {
		instance = new ProcessPool(process_n);
		instance->run();
	}
	return instance;
}

template<typename T>
int ProcessPool<T>::check_sub_process() {
	if (sub_index != -1) {
		// 子进程不能处理子进程
		return 0;
	}

	int wait_num = 0;
	pid_t _pid;
	int _stat;
	while ((_pid = waitpid(-1, &_stat, WNOHANG)) > 0) {
		for (int i = 0; i < sub_process_n; ++i) {
			if (sub_process_metas[i].sub_pid == _pid) {
#ifdef DEBUG
				printf("child process %d join.\n", i);
#endif // DEBUG
				// 关闭父端的管道
				close(sub_process_metas[i].sub_pipefd[0]);
				// 将元数据中的子进程PID记录为-1
				sub_process_metas[i].sub_pid = -1;
				// 存活子进程数量-1
				--sub_alive_process_n;
				++wait_num;
			}
		}
	}
	return wait_num;
}

template<typename T>
int ProcessPool<T>::get_sub_process() {
	if (sub_index != -1) {
		// 子进程不能处理子进程
		return 0;
	}

	// 更新存活的子进程
	check_sub_process();

	// 随机轮询分配子进程
	int rand_begin = rand() % sub_process_n;
	for (int i = rand_begin; i < rand_begin + sub_process_n; ++i) {
		if (sub_process_metas[i % sub_process_n].sub_pid != -1) {
			// 轮询成功
			return (i % sub_process_n);
		}
	}
	return -1;
}

template<typename T>
int ProcessPool<T>::execute(T obj) {
	if (sub_index != -1) {
		// 子进程不能处理子进程
		return 0;
	}

	int sub_process_index = get_sub_process();

#ifdef DEBUG
	printf("selected process index: %d\n", sub_process_index);
#endif // DEBUG

	// 和子进程的通信管道
	int sub_pipefd = sub_process_metas[sub_process_index].sub_pipefd[0];
	int send_n = send(sub_pipefd, reinterpret_cast<char*>(&obj), sizeof(obj), 0);
	if (send_n == -1) {
		core::unix_error("Run parent send error");
		return -1;
	}
#ifdef DEBUG
	printf("parent process send msg: %d bytes.\n", send_n);
#endif // DEBUG

	return 0;
}

template<typename T>
int ProcessPool<T>::destory() {
	if (sub_index != -1) {
		return 0;
	}

	// 向子进程发送终止信号
	for (int i = 0; i < sub_process_n; ++i) {
		int _pid = sub_process_metas[i].sub_pid;
		if (_pid != -1) {
			if (kill(_pid, SIGTERM) == -1) {
				return -1;
			}
		}
	}

	// 回收子进程
	check_sub_process();
	return 0;
}

