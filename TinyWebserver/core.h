#pragma once

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <errno.h>

#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>

#define DEBUG

namespace core {
	/*
	* @brief �쳣��
	*/
	class UserException :public std::exception {
	private:
		std::string msg;  /*�쳣��Ϣ*/
	public:
		/*
		* @brief �쳣�๹�캯��
		*/
		UserException(char* _msg) :msg(_msg) {};

		/*
		* @brief �����쳣��Ϣ
		*/
		const char* what() const throw() override{
			return msg.c_str();
		}
	};

	/*
	* @brief ��ӡunix����
	* @param msg => ������Ϣ
	*/
	void unix_error(char* msg);

	/*
	* @brief ��ӡuser����
	* @param msg => ������Ϣ
	*/
	void user_error(char* msg);

	/*
	* @brief IO��д��������С
	*/
	constexpr unsigned long IO_BUFFERSIZE = 120;

	/*
	* @brief �����̳ش�С
	*/
	constexpr unsigned int MAX_PROCESS_NUMBER = 8;

	/*
	* @brief ���epoll��·���ü����¼���
	*/
	constexpr unsigned int MAX_EVENT_NUMBER = 16;

	/*
	* @brief ����̳߳ش�С
	*/
	constexpr unsigned int MAX_THREAD_NUMBER = 8;

	/*
	* @brief ����̳߳ؿɴ�����������
	*/
	constexpr unsigned int MAX_TASK_NUMBER = 16;

	/*
	* @brief epoll������
	*/
	class EpollUtils {
	public:
		/*
		* @brief �����ļ�������������
		* @param fd => �ļ�������
		* @retval old_option => ��ѡ��
		*/
		static int set_nonblocking(int fd);

		/*
		* @brief Ϊepoll�����µļ����ļ�
		* @param
		*/
		static void epoll_add(int epollfd, int fd);

		/*
		* @brief Ϊepoll�Ƴ������ļ�
		* @param
		*/
		static void epoll_remove(int epollfd, int fd);
	};

	/*
	* @brief signal������
	*/
	class SignalUtils {
	public:
		/*
		* @brief Ϊ�źŰ��źŴ�����
		* @param sig => �ź�
		* @param handler => �źŴ�����
		* @param restart => �Ƿ����µ��ñ����ź���ֹ��ϵͳ����
		*/
		static void sig_add_handler(int sig, void(*handler)(int), bool restart = true);
	};
}