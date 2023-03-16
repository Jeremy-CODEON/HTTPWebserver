#pragma once

#include "core.h"

class ThreadExecutableBase;
class ThreadMeta;
class ThreadPool;

template<typename T>
class SingletonBase;

/*
* @brief 执行类抽象类
*/
class ThreadExecutableBase {
public:
	virtual int process() = 0;
};

/*
* @brief 线程元数据类
*/
class ThreadMeta {
public:
	std::thread::id sub_tid;  /*子线程id*/
	std::shared_ptr<std::thread> thread_ptr;  /*子线程对象指针*/

public:
	/*
	* @brief 父线程调用的无参构造函数
	* @param
	*/
	ThreadMeta() :sub_tid(-1) {};
};

/*
* @brief 单例模式基类
*/
template<typename T>
class SingletonBase {
private:
	SingletonBase(const SingletonBase& rhs) {};
	SingletonBase& operator=(const SingletonBase& rhs) {};

protected:
	SingletonBase() {};
	~SingletonBase() {};

public:
	/*
	* @brief 获取实例静态函数
	* @retval instance => 单例模式实例
	*/
	static T& get_instance(int thread_n = 4) {
		static T instance(thread_n);
		return instance;
	}
};

/*
* @brief 线程池类
*/
class ThreadPool :public SingletonBase<ThreadPool>{
private:
	const int MAX_THREAD_NUMBER = core::MAX_THREAD_NUMBER;  /*最大子线程数量*/
	const int MAX_TASK_NUMBER = core::MAX_TASK_NUMBER;  /*最大任务数量*/

	static ThreadMeta* sub_thread_metas;  /*子线程元数据表*/

	std::queue<ThreadExecutableBase*> tasks_queue;  /*任务队列*/
	std::mutex tasks_queue_mutex;  /*任务队列互斥量*/
	std::condition_variable consumer_cv;  /*消费者条件变量*/
	std::condition_variable producer_cv;  /*生产者条件变量*/

	int sub_thread_n;  /*子线程数量*/
	std::atomic<int> working_thread_num;  /*正在工作的子线程数量*/

	/*
	* @brief 主线程是否已结束，如结束则子线程不能再使用主线程资源
	*        因为instance只存活在主线程生命周期中
	*/
	bool is_stop;	

private:
	/*
	* @brief 线程启动函数	
	*/
	static void thread_run(ThreadPool* thread_pool);

public:
	/*
	* @brief 线程池构造函数
	*/
	ThreadPool(int thread_n);

	/*
	* @brief 线程池析构函数
	*/
	~ThreadPool();

	/*
	* @brief 用子线程执行
	* @retval true => 成功加入任务队列
	* @retval false => 任务队列已满
	*/
	bool execute(ThreadExecutableBase* task);

	/*
	* @brief 等待工作队列中的任务全部完成
	*/
	void wait();
};


