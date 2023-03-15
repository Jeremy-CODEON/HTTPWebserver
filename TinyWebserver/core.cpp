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
	// ��þ�ѡ��
	int old_option = fcntl(fd, F_GETFL);
	// ������ѡ����ļ�����������Ϊ������
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	// ���ؾ�ѡ��
	return old_option;
}

void core::EpollUtils::epoll_add(int epollfd, int fd)
{
	assert(epollfd != -1 && fd > 0);

	epoll_event event;
	event.data.fd = fd;
	// �¼���������Ϊ�����ݿɶ��ұ�Ե����
	event.events = EPOLLIN | EPOLLET;
	// ��fdע�ᵽepollfd��
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	// ���ý��̼������ļ�������������
	set_nonblocking(fd);
}

void core::EpollUtils::epoll_remove(int epollfd, int fd)
{
	assert(epollfd != -1 && fd > 0);

	// ��fd��epollfd���Ƴ�
	epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
	// �ر��ļ�
	close(fd);
}

void core::SignalUtils::sig_add_handler(int sig, void(*handler)(int), bool restart)
{
	struct sigaction sa;
	// ��ʼ���ṹ
	memset(&sa, '\0', sizeof(sa));

	// ���źŴ�����
	sa.sa_handler = handler;
	if (restart) {
		sa.sa_flags |= SA_RESTART;
	}
	// ���������ź�Ϊ�����źţ���ֹ�ڴ����������ж�
	sigfillset(&sa.sa_mask);
	// ��sa����sig
	if (sigaction(sig, &sa, nullptr) == -1) {
		core::unix_error("Signal add handler error");
		exit(1);
	}
}
