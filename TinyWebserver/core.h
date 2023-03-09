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
#include <iostream>

#define DEBUG

namespace core {
	/*
	* @brief 打印unix错误
	* @param msg => 错误信息
	*/
	void unix_error(char* msg);

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
}