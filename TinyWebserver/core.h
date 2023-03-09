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
	* @brief ��ӡunix����
	* @param msg => ������Ϣ
	*/
	void unix_error(char* msg);

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
}