#pragma once

#include "core.h"

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
* @brief ���̳�ִ���������
*/
class ProcessExecutableBase {
public:
	virtual int process() = 0;
};

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
	virtual int process() {
		print_content();
	}
};

/*
* @brief ���̳�
*/
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
	int execute(ProcessExecutable obj);

	/*
	* @brief ���������ӽ��̣���������ִ��
	* @retval 0 => success
	* @retval -1 => error
	*/
	int destory();
};



