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
	/*���̳߳�ʵ�ֵķ�����ʾ����֧�ָ߲���������Reatorģʽ*/

	/*
	* @brief �̳߳�ִ���࣬��װ���߳�ִ�еĺ���
	*/
	class ThreadExecutable :public ThreadExecutableBase {
	private:
		/*
		* @brief ��������
		* @val 2 => sub reator�����д����loop��
		* @val 3 => ��ͨ�̣߳�����ҵ���߼�
		*/
		int process_channel;
		/*
		* @brief �ļ�������
		* @val 2 => sub_epollfd
		* @val 3 => process_fd
		*/
		int sub_epollfd;
		int process_fd;
		/*
		* @brief �Ƿ������ѭ������sub reatorʹ��
		*/
		bool* p_is_stop;


	public:
		/*
		* @brief ���캯��
		*/
		ThreadExecutable(int _channel, int _epollfd, int _fd, bool* _p = nullptr) :
			process_channel(_channel), sub_epollfd(_epollfd),
			process_fd(_fd), p_is_stop(_p) {}

		/*
		* @brief ��������
		*/
		~ThreadExecutable() {}

		/*
		* @brief ��д���߳�ִ�к���
		*/
		virtual int process() override {
			if (process_channel == 2) {
				/*sub_reactor�߼�����*/
				assert(sub_epollfd >0 && p_is_stop != nullptr);

				// ��ȡ�̳߳�ʵ��
				ThreadPool<ThreadExecutable>& thread_pool = ThreadPool<ThreadExecutable>::get_instance();
				// ��ȡ��ʱ������ʵ��
				TimerUtils& timer_utils = TimerUtils::get_instance();

				epoll_event events[core::MAX_EVENT_NUMBER];  /*�����¼���*/
				while (!(*p_is_stop)) {
					// epoll�����¼�
					int events_number = epoll_wait(sub_epollfd, events, core::MAX_EVENT_NUMBER, core::EXPIRED_TIME*1000);
					if ((events_number < 0) && (errno != EINTR)) {
						core::unix_error("error: couldn't listen epoll table!\n");
						break;
					}

					for (int i = 0; i < events_number; ++i) {
						printf("another epoll event list handling...\n");
						int event_fd = events[i].data.fd;

						if (events[i].events & EPOLLIN) {
							// �ɶ�
							ThreadExecutable task(3, sub_epollfd, event_fd);  // ����ҵ���߼���������
							thread_pool.execute(task);  // ���̳߳�ִ��
						}
						if (events[i].events & EPOLLOUT) {
							// ��д����δʵ�֣�
						}
						if (events[i].events & (EPOLLERR | EPOLLHUP)) {
							// �쳣
							printf("close sockfd %d\n", event_fd);
							core::EpollUtils::epoll_remove(sub_epollfd, event_fd);  // �Ƴ��������ر�
						}
						if ((events[i].events & EPOLLIN) 
							&& !(events[i].events & (EPOLLERR | EPOLLHUP))) {
							// �ɶ��ҷ��쳣����Ӷ�ʱ��
							TimerRecord timer(time(NULL), event_fd, sub_epollfd);
							timer_utils.add_timer(timer);
						}
					}	
					timer_utils.tick();
				}				
			}
			else if (process_channel == 3) {
				/*normal thread�߼�����*/
				assert(sub_epollfd > 0 && process_fd > 0);

				// ����ͻ��˵�����
				if (WebServerUtils::doit(process_fd) == -1) {
					printf("close sockfd %d\n", process_fd);
					core::EpollUtils::epoll_remove(sub_epollfd, process_fd);  // �Ƴ��������ر�
				}
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

	// ����epoll�¼�������
	int epollfd;  /*epoll�¼�������������*/
	if ((epollfd = epoll_create(100)) == -1) {
		printf("error: couldn't creat epoll table!\n");
		return -1;
	}

	// ����sub_epoll�¼�������
	int sub_epollfd;  /*sub_epoll�¼�������������*/
	if ((sub_epollfd = epoll_create(100)) == -1) {
		printf("error: couldn't creat sub epoll table!\n");
		return -1;
	}

	// ���źż�������epoll��ͳһ�¼�Դ
	core::SignalUtils sig_utils(epollfd);

	// �����������������epoll��ͳһ�¼�Դ
	core::EpollUtils::epoll_add(epollfd, listen_fd);

	epoll_event events[core::MAX_EVENT_NUMBER];  /*�����¼���*/
	static bool is_stop = false;  /*�Ƿ������ѭ���������sub_reactor����ѭ��*/

	// ��ȡ�̳߳�ʵ��
	ThreadPool<ThreadExecutable>& thread_pool = ThreadPool<ThreadExecutable>::get_instance(4);
	// ��ȡ��ʱ������ʵ��
	TimerUtils& timer_utils = TimerUtils::get_instance();

	/*����sub_reactor*/
	ThreadExecutable task(2, sub_epollfd, -1, &is_stop);
	thread_pool.execute(task);

	while (!is_stop) {
		// epoll�����¼�
		int events_number = epoll_wait(epollfd, events, core::MAX_EVENT_NUMBER, -1);
		if ((events_number < 0) && (errno != EINTR)) {
			core::unix_error("error: couldn't listen epoll table!\n");
			break;
		}

		for (int i = 0; i < events_number; ++i) {
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
				// �Ϳͻ��˽�������
				struct sockaddr_in clientaddr;  /*�ͻ��˵�ַ�Ͷ˿�*/
				socklen_t clientaddr_len = sizeof(clientaddr); /*�ṹ��С*/
				
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
				// �����ӵ�fd�ŵ�sub_epollfd�м���
				core::EpollUtils::epoll_add(sub_epollfd, connect_fd);
				// ��Ӷ�ʱ��
				TimerRecord timer(time(NULL), connect_fd, sub_epollfd);
				timer_utils.add_timer(timer);
			}
		}
	}

	// ���������Ǵ�ָ�룬��Դ���������߳��ڶ������̳߳����п���
	// ������̲߳���Ҫ�ȴ���������������ֹ
	//thread_pool.wait();

	printf("main thread end.\n");
#endif

	return 0;
}
#endif

