#include "base_io.h"
#include "buffer_io.h"
#include "process_pool.h"
#include "thread_pool.h"
#include "web_server.h"
#include "smart_pointer.h"
#include "timer.h"

#include <chrono>
#include <iostream>

#define SERVER

#ifdef SERVER
int main() {
#if 1
	/*用线程池实现的服务器示例，支持高并发，主从Reator模式*/

	/*
	* @brief 线程池执行类，封装子线程执行的函数
	*/
	class ThreadExecutable :public ThreadExecutableBase {
	private:
		/*
		* @brief 处理类型
		* @val 2 => sub reator处理读写请求（loop）
		* @val 3 => 普通线程，处理业务逻辑
		*/
		int process_channel;
		/*
		* @brief 文件描述符
		* @val 2 => sub_epollfd
		* @val 3 => process_fd
		*/
		int sub_epollfd;
		int process_fd;
		/*
		* @brief 是否结束主循环，仅sub reator使用
		*/
		bool* p_is_stop;


	public:
		/*
		* @brief 构造函数
		*/
		ThreadExecutable(int _channel, int _epollfd, int _fd, bool* _p = nullptr) :
			process_channel(_channel), sub_epollfd(_epollfd),
			process_fd(_fd), p_is_stop(_p) {}

		/*
		* @brief 析构函数
		*/
		~ThreadExecutable() {}

		/*
		* @brief 重写子线程执行函数
		*/
		virtual int process() override {
			if (process_channel == 2) {
				/*sub_reactor逻辑处理*/
				assert(sub_epollfd >0 && p_is_stop != nullptr);

				// 获取线程池实例
				ThreadPool<ThreadExecutable>& thread_pool = ThreadPool<ThreadExecutable>::get_instance();
				// 获取定时器管理实例
				TimerUtils& timer_utils = TimerUtils::get_instance();

				epoll_event events[core::MAX_EVENT_NUMBER];  /*监听事件表*/
				while (!(*p_is_stop)) {
					// epoll监听事件
					int events_number = epoll_wait(sub_epollfd, events, core::MAX_EVENT_NUMBER, core::EXPIRED_TIME*1000);
					if ((events_number < 0) && (errno != EINTR)) {
						core::unix_error("error: couldn't listen epoll table!\n");
						break;
					}

					for (int i = 0; i < events_number; ++i) {
						printf("another epoll event list handling...\n");
						int event_fd = events[i].data.fd;

						if (events[i].events & EPOLLIN) {
							// 可读
							ThreadExecutable task(3, sub_epollfd, event_fd);  // 创建业务逻辑处理任务
							thread_pool.execute(task);  // 用线程池执行
						}
						if (events[i].events & EPOLLOUT) {
							// 可写（暂未实现）
						}
						if (events[i].events & (EPOLLERR | EPOLLHUP)) {
							// 异常
							printf("close sockfd %d\n", event_fd);
							core::EpollUtils::epoll_remove(sub_epollfd, event_fd);  // 移除监听并关闭
						}
						if ((events[i].events & EPOLLIN) 
							&& !(events[i].events & (EPOLLERR | EPOLLHUP))) {
							// 可读且非异常，添加定时器
							TimerRecord timer(time(NULL), event_fd, sub_epollfd);
							timer_utils.add_timer(timer);
						}
					}	
					timer_utils.tick();
				}				
			}
			else if (process_channel == 3) {
				/*normal thread逻辑处理*/
				assert(sub_epollfd > 0 && process_fd > 0);

				// 处理客户端的请求
				if (WebServerUtils::doit(process_fd) == -1) {
					printf("close sockfd %d\n", process_fd);
					core::EpollUtils::epoll_remove(sub_epollfd, process_fd);  // 移除监听并关闭
				}
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

	// 创建epoll事件监听表
	int epollfd;  /*epoll事件监听表描述符*/
	if ((epollfd = epoll_create(100)) == -1) {
		printf("error: couldn't creat epoll table!\n");
		return -1;
	}

	// 创建sub_epoll事件监听表
	int sub_epollfd;  /*sub_epoll事件监听表描述符*/
	if ((sub_epollfd = epoll_create(100)) == -1) {
		printf("error: couldn't creat sub epoll table!\n");
		return -1;
	}

	// 将信号监听加入epoll，统一事件源
	core::SignalUtils sig_utils(epollfd);

	// 将连接请求监听加入epoll，统一事件源
	core::EpollUtils::epoll_add(epollfd, listen_fd);

	epoll_event events[core::MAX_EVENT_NUMBER];  /*监听事件表*/
	static bool is_stop = false;  /*是否结束主循环，亦包括sub_reactor的主循环*/

	// 获取线程池实例
	ThreadPool<ThreadExecutable>& thread_pool = ThreadPool<ThreadExecutable>::get_instance(4);
	// 获取定时器管理实例
	TimerUtils& timer_utils = TimerUtils::get_instance();

	/*创建sub_reactor*/
	ThreadExecutable task(2, sub_epollfd, -1, &is_stop);
	thread_pool.execute(task);

	while (!is_stop) {
		// epoll监听事件
		int events_number = epoll_wait(epollfd, events, core::MAX_EVENT_NUMBER, -1);
		if ((events_number < 0) && (errno != EINTR)) {
			core::unix_error("error: couldn't listen epoll table!\n");
			break;
		}

		for (int i = 0; i < events_number; ++i) {
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
							printf("SIGINT signal caught!\n");
							break;
						}
						case SIGPIPE:
						{
							printf("SIGPIPE signal caught!\n");
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
				// 和客户端建立连接
				struct sockaddr_in clientaddr;  /*客户端地址和端口*/
				socklen_t clientaddr_len = sizeof(clientaddr); /*结构大小*/
				
				int connect_fd = accept(listen_fd,
					reinterpret_cast<struct sockaddr*>(&clientaddr),
					&clientaddr_len);
				if (connect_fd == -1) {
					printf("error: couldn't accept connect fd.\n");
					return -1;
				}
				else {
					printf("another connect is open.\n");
				}
				// 将连接的fd放到sub_epollfd中监听
				core::EpollUtils::epoll_add(sub_epollfd, connect_fd);
				// 添加定时器
				TimerRecord timer(time(NULL), connect_fd, sub_epollfd);
				timer_utils.add_timer(timer);
			}
		}
	}

	// 由于任务不是传指针，资源不是在主线程内而是在线程池中有拷贝
	// 因此主线程不需要等待任务队列完成再终止
	//thread_pool.wait();

	printf("main thread end.\n");
#endif

	return 0;
}
#endif

