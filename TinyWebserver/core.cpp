#include "core.h"

void core::unix_error(char* msg)
{
	fprintf(stderr, "%s: %s\n", msg, strerror(errno));
}

void core::user_error(char* msg)
{
	fprintf(stderr, "%s\n", msg);
	throw UserException(msg);
}

/*
* 以下是EpollUtils类实现
*/

int core::EpollUtils::set_nonblocking(int fd)
{
	// 获得旧选项
	int old_option = fcntl(fd, F_GETFL);
	// 设置新选项，将文件描述符设置为非阻塞
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	// 返回旧选项
	return old_option;
}

void core::EpollUtils::epoll_add(int epollfd, int fd)
{
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

void core::EpollUtils::epoll_remove(int epollfd, int fd)
{
	assert(epollfd != -1 && fd > 0);

	// 将fd从epollfd上移除
	epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
	// 关闭文件
	close(fd);
}

/* 
* 以下是SignalUtils类实现
*/

int core::SignalUtils::sig_pipefd[2] = { -1, -1 };

void core::SignalUtils::sig_handler(int sig)
{
	int save_errno = errno;
	// 从内核发送给进程的消息就是要处理的信号，仅1字节
	int msg = sig;
	// 通过管道发送给进程
	send(sig_pipefd[0], reinterpret_cast<char*>(&msg), 1, 0);
	// 恢复错误信息
	errno = save_errno;
}

void core::SignalUtils::sig_add_handler(int sig, void(*handler)(int), bool restart)
{
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
#ifdef DEBUG
	//printf("signal set successfully!\n");
#endif // DEBUG

}

void core::SignalUtils::sig_init()
{
	// 建立内核和进程的通信管道
	if (socketpair(PF_UNIX, SOCK_STREAM, 0, sig_pipefd) == -1) {
		core::unix_error("Run parent/child signal pipe create error");
		exit(1);
	}

	// 设置内核写的管道描述符非阻塞
	core::EpollUtils::set_nonblocking(sig_pipefd[0]);
	// epoll监听进程读的管道描述符，统一事件源
	core::EpollUtils::epoll_add(epollfd, sig_pipefd[1]);

	// 为常见信号绑定信号处理函数
	sig_add_handler(SIGCHLD, sig_handler);
	sig_add_handler(SIGTERM, sig_handler);
	sig_add_handler(SIGINT, sig_handler);
	sig_add_handler(SIGPIPE, sig_handler);
}

core::SignalUtils::SignalUtils(int _fd) :epollfd(_fd)
{
	sig_init();
}
