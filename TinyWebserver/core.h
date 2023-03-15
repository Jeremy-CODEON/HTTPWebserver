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
	* @brief 异常类
	*/
	class UserException :public std::exception {
	private:
		std::string msg;  /*异常信息*/
	public:
		/*
		* @brief 异常类构造函数
		*/
		UserException(char* _msg) :msg(_msg) {};

		/*
		* @brief 返回异常信息
		*/
		const char* what() const throw() override{
			return msg.c_str();
		}
	};

	/*
	* @brief 打印unix错误
	* @param msg => 错误信息
	*/
	void unix_error(char* msg);

	/*
	* @brief 打印user错误
	* @param msg => 错误信息
	*/
	void user_error(char* msg);

	/*
	* @brief IO读写缓冲区大小
	*/
	constexpr unsigned long IO_BUFFERSIZE = 120;

	/*
	* @brief 最大进程池大小
	*/
	constexpr unsigned int MAX_PROCESS_NUMBER = 8;

	/*
	* @brief 最大epoll多路复用监听事件数
	*/
	constexpr unsigned int MAX_EVENT_NUMBER = 16;

	/*
	* @brief 最大线程池大小
	*/
	constexpr unsigned int MAX_THREAD_NUMBER = 8;

	/*
	* @brief 最大线程池可处理任务数量
	*/
	constexpr unsigned int MAX_TASK_NUMBER = 16;

	/*
	* @brief epoll工具类
	*/
	class EpollUtils {
	public:
		/*
		* @brief 设置文件描述符非阻塞
		* @param fd => 文件描述符
		* @retval old_option => 旧选项
		*/
		static int set_nonblocking(int fd);

		/*
		* @brief 为epoll增加新的监听文件
		* @param
		*/
		static void epoll_add(int epollfd, int fd);

		/*
		* @brief 为epoll移除监听文件
		* @param
		*/
		static void epoll_remove(int epollfd, int fd);
	};

	/*
	* @brief signal工具类
	*/
	class SignalUtils {
	public:
		/*
		* @brief 为信号绑定信号处理函数
		* @param sig => 信号
		* @param handler => 信号处理函数
		* @param restart => 是否重新调用被该信号终止的系统调用
		*/
		static void sig_add_handler(int sig, void(*handler)(int), bool restart = true);
	};
}