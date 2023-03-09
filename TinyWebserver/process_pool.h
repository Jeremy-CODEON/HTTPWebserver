#pragma once

#include "core.h"

/*
* @brief ���̳�ִ���������
*/
class ProcessExecutableBase {
public:
	virtual int process() = 0;
};

/*
* @brief ����Ԫ����
*/
class ProcessMeta {
public:
	pid_t sub_pid;  /*�ӽ���PID��������-1*/
	int sub_pipefd[2];  /*���ӽ��̵�ͨ�Źܵ�����[0]��[1]*/

public:
	/*
	* @brief �����̵��޲ι��캯��
	* @param
	*/
	ProcessMeta() :sub_pid(-1) {};
};

/*
* @brief ���̳�
*/
template<typename T>
class ProcessPool {
private:
	const int MAX_PROCESS_NUMBER = core::MAX_PROCESS_NUMBER;  /*����ӽ�������*/
	const int MAX_EVENT_NUMBER = core::MAX_EVENT_NUMBER;  /*epoll��������*/
	
	static ProcessPool* instance;  /*���̳�ʵ��*/
	static ProcessMeta* sub_process_metas;  /*�ӽ���Ԫ���ݱ�*/

	int sub_process_n;  /*�ӽ�������*/
	int sub_alive_process_n;  /*����ӽ�������*/
	int sub_index;  /*�ӽ�����ţ�������-1*/

	static int sig_pipefd[2];  /*�ں�����̵��źŹܵ����ں�[0]����[1]*/
	int epollfd;  /*epoll�ں��¼���*/

	bool is_stop;  /*�Ƿ���ֹ����ѭ��*/

private:
	/*
	* @brief ���̳ع��캯��
	* @param process_n => ���̳��ӽ�������
	*/
	ProcessPool(int process_n);

	/*
	* @brief ���̳ظ��ƹ��캯����δʵ�֣�
	*/
	ProcessPool(const ProcessPool& obj) {};

	/*
	* @brief ���̳���������
	*/
	~ProcessPool();	

	/*
	* @brief ��������������
	*/
	void run_parent();

	/*
	* @brief �ӽ�����������
	*/
	void run_child();

	/*
	* @brief �������̳�
	*/
	void run();

private:
	/*
	* @brief �����ļ�������������
	* @param fd => �ļ�������
	* @retval old_option => ��ѡ��
	*/
	int set_nonblocking(int fd);

	/*
	* @brief Ϊepoll�����µļ����ļ�
	* @param
	*/
	void epoll_add(int epollfd, int fd);

	/*
	* @brief Ϊepoll�Ƴ������ļ�
	* @param
	*/
	void epoll_remove(int epollfd, int fd);

private:
	/*
	* @brief �źŴ�����
	* @param sig => �ź�
	*/
	static void sig_handler(int sig);

	/*
	* @brief Ϊ�źŰ��źŴ�����
	* @param sig => �ź�
	* @param handler => �źŴ�����
	* @param restart => �Ƿ����µ��ñ����ź���ֹ��ϵͳ����
	*/
	void sig_add_handler(int sig, void(*handler)(int), bool restart = true);

	/*
	* @brief �źų�ʼ������
	*/
	void sig_init();

private:
	/*
	* @brief �����Ѿ���ֹ���ӽ��̣���������ִ��
	* @retval wait_num  => ���յ��ӽ�������
	*/
	int check_sub_process();

	/*
	* @brief ����һ���ӽ��̣���δ����Դƽ�⣩����������ִ��
	* @retval sub_process_index  => �ӽ������
	* @retval -1 => error
	*/
	int get_sub_process();

public:
	/*
	* @brief ��õ���ģʽ�µ�ʵ��
	* @param process_n => ���̳��ӽ�������
	*/
	static ProcessPool* get_instance(int process_n = 4);	

	/* 
	* @brief ���ӽ���ִ�У���������ִ��
	* @retval 0 => success
	* @retval -1 => error
	*/
	int execute(T obj);

	/*
	* @brief ���������ӽ��̣���������ִ��
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


	// �½��ӽ���Ԫ���ݱ�
	sub_process_metas = new ProcessMeta[sub_process_n];

	for (int i = 0; i < sub_process_n; ++i) {
		// �������ӽ��̵Ĺܵ�ͨ��
		if (socketpair(PF_UNIX, SOCK_STREAM, 0,
			sub_process_metas[i].sub_pipefd) == -1)
		{
			core::unix_error("Process pool pipe create error");
			exit(1);
		}

		// �����ӽ���
		sub_process_metas[i].sub_pid = fork();
		if (sub_process_metas[i].sub_pid > 0) {
#ifdef DEBUG
			printf("%d child process pid is %d\n", i, sub_process_metas[i].sub_pid);
#endif // DEBUG

			// �����̹رչܵ�[1]��
			if (close(sub_process_metas[i].sub_pipefd[1]) == -1) {
				core::unix_error("Process pool pipe parent close error");
				exit(1);
			}
			// ���������ӽ���
			continue;
		}
		else {
			// �ӽ��̹رչܵ�[0]��
			if (close(sub_process_metas[i].sub_pipefd[0]) == -1) {
				core::unix_error("Process pool pipe child close error");
				exit(1);
			}
			// ��¼��ǰ���̵����
			sub_index = i;
			// ��ֹ�����ӽ���
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
	// ����epoll�¼�������
	if ((epollfd = epoll_create(100)) == -1) {
		core::unix_error("Run parent epoll create error");
		exit(1);
	}

	// ��ʼ���ź�ͳһ�¼�Դ
	sig_init();

#if 0
	epoll_event events[MAX_EVENT_NUMBER];
	while (!is_stop) {
		// ��������ѭ��
		int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, 10);
		if ((number < 0) && (errno != EINTR)) {
			core::unix_error("Run parent epoll listen error");
			break;
		}

		for (int i = 0; i < number; ++i) {
			int event_fd = events[i].data.fd;
			if ((event_fd == sig_pipefd[1]) && (events[i].events & EPOLLIN)) {   /*�źŹܵ��ҿɶ�*/
				char signals[1024];
				// �����ź�
				int sig_num = recv(sig_pipefd[1], signals, sizeof(signals), 0);
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
								for (int k = 0; k < sub_process_n; ++k) {
									if (sub_process_metas[k].sub_pid == _pid) {
#ifdef DEBUG
										printf("child process %d join.\n", k);
#endif // DEBUG
										// �رո��˵Ĺܵ�
										close(sub_process_metas[k].sub_pipefd[0]);
										// ��Ԫ�����е��ӽ���PID��¼Ϊ-1
										sub_process_metas[k].sub_pid = -1;
										// ����ӽ�������-1
										--sub_alive_process_n;
									}
								}
							}
							if (sub_alive_process_n == 0) {
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
				// �������¼�
			}
		}
	}

#endif

	// �ر�epoll�¼�������
	close(epollfd);
}

template<typename T>
void ProcessPool<T>::run_child() {
	// ����epoll�¼�������
	if ((epollfd = epoll_create(100)) == -1) {
		core::unix_error("Run child epoll create error");
		exit(1);
	}

	// ��ʼ���ź�ͳһ�¼�Դ
	sig_init();

	// �͸����̵�ͨ�Źܵ�
	int sub_pipefd = sub_process_metas[sub_index].sub_pipefd[1];
	// ������ͨ�Źܵ�
	epoll_add(epollfd, sub_pipefd);

	epoll_event events[MAX_EVENT_NUMBER];
	while (!is_stop) {
		// �ӽ�����ѭ��
		int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
		if ((number < 0) && (errno != EINTR)) {
			core::unix_error("Run child epoll listen error");
			break;
		}

		for (int i = 0; i < number; ++i) {
			int event_fd = events[i].data.fd;
			if ((event_fd == sig_pipefd[1]) && (events[i].events & EPOLLIN)) {  /*�źŹܵ��ҿɶ�*/
				char signals[1024];
				// �����ź�
				int sig_num = recv(sig_pipefd[1], signals, sizeof(signals), 0);
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
							break;
						}
						case SIGTERM:  /*�����ֹ����������ֹ*/
						case SIGINT:  /*�����жϣ���������ֹ*/
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
			else if ((event_fd == sub_pipefd) && (events[i].events & EPOLLIN)) {  /*���ӹܵ��ҿɶ�*/
				T obj;
				int recv_n = recv(event_fd, reinterpret_cast<char*>(&obj), sizeof(obj), 0);
#ifdef DEBUG
				printf("child recv_n: %d bytes.\n", recv_n);
#endif // DEBUG

				if (((recv_n < 0) && (errno != EAGAIN)) || recv_n == 0) {
					// ��������
					core::unix_error("Run child recv error");
					continue;
				}
				else {
					// ����ҵ���߼�
					printf("child process handling msg: %d bytes.\n", recv_n);
					obj.process();
				}
			}
			else {
				// �������¼�
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
	// ��þ�ѡ��
	int old_option = fcntl(fd, F_GETFL);
	// ������ѡ����ļ�����������Ϊ������
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	// ���ؾ�ѡ��
	return old_option;
}

template<typename T>
void ProcessPool<T>::epoll_add(int epollfd, int fd) {
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

template<typename T>
void ProcessPool<T>::epoll_remove(int epollfd, int fd) {
	assert(epollfd != -1 && fd > 0);

	// ��fd��epollfd���Ƴ�
	epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
	// �ر��ļ�
	close(fd);
}

template<typename T>
void ProcessPool<T>::sig_handler(int sig) {
	int save_errno = errno;
	// ���ں˷��͸����̵���Ϣ����Ҫ������źţ���1�ֽ�
	int msg = sig;
	// ͨ���ܵ����͸�����
	send(sig_pipefd[0], reinterpret_cast<char*>(&msg), 1, 0);
	// �ָ�������Ϣ
	errno = save_errno;
}

template<typename T>
void ProcessPool<T>::sig_add_handler(int sig, void(*handler)(int), bool restart) {

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

template<typename T>
void ProcessPool<T>::sig_init()
{
	// �����ں˺ͽ��̵�ͨ�Źܵ�
	if (socketpair(PF_UNIX, SOCK_STREAM, 0, sig_pipefd) == -1) {
		core::unix_error("Run parent/child signal pipe create error");
		exit(1);
	}

	// �����ں�д�Ĺܵ�������������
	set_nonblocking(sig_pipefd[0]);
	// epoll�������̶��Ĺܵ���������ͳһ�¼�Դ
	epoll_add(epollfd, sig_pipefd[1]);

	// Ϊ�����źŰ��źŴ�����
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
		// �ӽ��̲��ܴ����ӽ���
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
				// �رո��˵Ĺܵ�
				close(sub_process_metas[i].sub_pipefd[0]);
				// ��Ԫ�����е��ӽ���PID��¼Ϊ-1
				sub_process_metas[i].sub_pid = -1;
				// ����ӽ�������-1
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
		// �ӽ��̲��ܴ����ӽ���
		return 0;
	}

	// ���´����ӽ���
	check_sub_process();

	// �����ѯ�����ӽ���
	int rand_begin = rand() % sub_process_n;
	for (int i = rand_begin; i < rand_begin + sub_process_n; ++i) {
		if (sub_process_metas[i % sub_process_n].sub_pid != -1) {
			// ��ѯ�ɹ�
			return (i % sub_process_n);
		}
	}
	return -1;
}

template<typename T>
int ProcessPool<T>::execute(T obj) {
	if (sub_index != -1) {
		// �ӽ��̲��ܴ����ӽ���
		return 0;
	}

	int sub_process_index = get_sub_process();

#ifdef DEBUG
	printf("selected process index: %d\n", sub_process_index);
#endif // DEBUG

	// ���ӽ��̵�ͨ�Źܵ�
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

	// ���ӽ��̷�����ֹ�ź�
	for (int i = 0; i < sub_process_n; ++i) {
		int _pid = sub_process_metas[i].sub_pid;
		if (_pid != -1) {
			if (kill(_pid, SIGTERM) == -1) {
				return -1;
			}
		}
	}

	// �����ӽ���
	check_sub_process();
	return 0;
}

