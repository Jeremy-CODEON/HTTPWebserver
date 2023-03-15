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
}
