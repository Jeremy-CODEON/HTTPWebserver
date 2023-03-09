#pragma once

#include "core.h"

/*
* @brief 进程元数据
*/
class ProcessMeta {
public:
	pid_t sub_pid;  /*子进程PID，父进程-1*/
	int sub_pipefd[2];  /*父子进程的通信管道，父[0]子[1]*/

public:
	/*
	* @brief 父进程的无参构造函数
	* @param
	*/
	ProcessMeta() :sub_pid(-1) {};
};

/*
* @brief 进程池执行类抽象类
*/
class ProcessExecutableBase {
public:
	virtual int process() = 0;
};

/*
* @brief 进程池执行类，封装子进程执行的函数
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
* @brief 进程池
*/
class ProcessPool {
private:
	const int MAX_PROCESS_NUMBER = core::MAX_PROCESS_NUMBER;  /*最大子进程数量*/
	const int MAX_EVENT_NUMBER = core::MAX_EVENT_NUMBER;  /*epoll最大监听数*/
	
	static ProcessPool* instance;  /*进程池实例*/
	static ProcessMeta* sub_process_metas;  /*子进程元数据表*/

	int sub_process_n;  /*子进程数量*/
	int sub_alive_process_n;  /*存活子进程数量*/
	int sub_index;  /*子进程序号，父进程-1*/

	static int sig_pipefd[2];  /*内核与进程的信号管道，内核[0]进程[1]*/
	int epollfd;  /*epoll内核事件表*/

	bool is_stop;  /*是否终止进程循环*/

private:
	/*
	* @brief 进程池构造函数
	* @param process_n => 进程池子进程数量
	*/
	ProcessPool(int process_n);

	/*
	* @brief 进程池复制构造函数（未实现）
	*/
	ProcessPool(const ProcessPool& obj) {};

	/*
	* @brief 进程池析构函数
	*/
	~ProcessPool();	

	/*
	* @brief 父进程启动函数
	*/
	void run_parent();

	/*
	* @brief 子进程启动函数
	*/
	void run_child();

	/*
	* @brief 启动进程池
	*/
	void run();

private:
	/*
	* @brief 设置文件描述符非阻塞
	* @param fd => 文件描述符
	* @retval old_option => 旧选项
	*/
	int set_nonblocking(int fd);

	/*
	* @brief 为epoll增加新的监听文件
	* @param
	*/
	void epoll_add(int epollfd, int fd);

	/*
	* @brief 为epoll移除监听文件
	* @param
	*/
	void epoll_remove(int epollfd, int fd);

private:
	/*
	* @brief 信号处理函数
	* @param sig => 信号
	*/
	static void sig_handler(int sig);

	/*
	* @brief 为信号绑定信号处理函数
	* @param sig => 信号
	* @param handler => 信号处理函数
	* @param restart => 是否重新调用被该信号终止的系统调用
	*/
	void sig_add_handler(int sig, void(*handler)(int), bool restart = true);

	/*
	* @brief 信号初始化设置
	*/
	void sig_init();

private:
	/*
	* @brief 回收已经终止的子进程，仅父进程执行
	* @retval wait_num  => 回收的子进程数量
	*/
	int check_sub_process();

	/*
	* @brief 分配一个子进程（尚未作资源平衡），仅父进程执行
	* @retval sub_process_index  => 子进程序号
	* @retval -1 => error
	*/
	int get_sub_process();

public:
	/*
	* @brief 获得单例模式下的实例
	* @param process_n => 进程池子进程数量
	*/
	static ProcessPool* get_instance(int process_n = 4);	

	/* 
	* @brief 用子进程执行，仅父进程执行
	* @retval 0 => success
	* @retval -1 => error
	*/
	int execute(ProcessExecutable obj);

	/*
	* @brief 结束所有子进程，仅父进程执行
	* @retval 0 => success
	* @retval -1 => error
	*/
	int destory();
};



