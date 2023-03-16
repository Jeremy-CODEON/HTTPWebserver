#pragma once

#include "core.h"

class ThreadExecutableBase;
class ThreadMeta;

template<typename T>
class ThreadPool;

template<typename U>
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
template<typename U>
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
	static U& get_instance(int thread_n = 4) {
		static U instance(thread_n);
		return instance;
	}
};

/*
* @brief 线程池类
*/
template<typename T>
class ThreadPool :public SingletonBase<ThreadPool<T>>{
private:
	const int MAX_THREAD_NUMBER = core::MAX_THREAD_NUMBER;  /*最大子线程数量*/
	const int MAX_TASK_NUMBER = core::MAX_TASK_NUMBER;  /*最大任务数量*/

	static ThreadMeta* sub_thread_metas;  /*子线程元数据表*/

	std::queue<T> tasks_queue;  /*任务队列*/
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
	bool execute(T& task);

	/*
	* @brief 等待工作队列中的任务全部完成
	*/
	void wait();
};

template<typename T>
ThreadMeta* ThreadPool<T>::sub_thread_metas = nullptr;

template<typename T>
ThreadPool<T>::ThreadPool(int thread_n) :
	sub_thread_n(thread_n), is_stop(false)
{
	assert((sub_thread_n > 0) && (sub_thread_n < MAX_THREAD_NUMBER));

	// 新建子线程元数据表
	sub_thread_metas = new ThreadMeta[sub_thread_n];

	for (int i = 0; i < sub_thread_n; ++i) {
		// 创建子线程
		sub_thread_metas[i].thread_ptr = std::make_shared<std::thread>(thread_run, this);
		sub_thread_metas[i].sub_tid = sub_thread_metas[i].thread_ptr->get_id();

		// 分离子线程，主要是为了尽量减少子线程和主线程之间的依赖
		sub_thread_metas[i].thread_ptr->detach();
	}

	/*初始工作子线程数量*/
	working_thread_num.store(0);
}

template<typename T>
ThreadPool<T>::~ThreadPool()
{
#ifdef DEBUG
	printf("working thread num: %d\n", working_thread_num.load());
#endif // DEBUG

	if (sub_thread_metas != nullptr) {
		delete[]sub_thread_metas;
	}

	// 声明主线程结束
	is_stop = true;
	// 通知各个线程结束
	consumer_cv.notify_all();

#ifdef DEBUG
	printf("thread pool has destroyed.\n");
#endif // DEBUG
}

template<typename T>
bool ThreadPool<T>::execute(T& task)
{
	// 先竞争锁再将任务添加到任务队列，避免有线程正在使用任务队列
	std::unique_lock<std::mutex> guard(tasks_queue_mutex);
	if (tasks_queue.size() >= MAX_TASK_NUMBER) {
		return false;
	}
	tasks_queue.push(task);
	// 用条件变量对象通知所有线程竞争消费
	consumer_cv.notify_all();

	return true;
}

template<typename T>
void ThreadPool<T>::thread_run(ThreadPool* thread_pool)
{
	while (!thread_pool->is_stop) {
		// 从任务队列中取任务
		// 如果主线程已经将thread_pool资源析构了，则guard会抛出异常，自动终止这个线程
		std::unique_lock<std::mutex> guard(thread_pool->tasks_queue_mutex);

		while (!thread_pool->is_stop && (thread_pool->tasks_queue.size() <= 0)) {
			// 防止虚假唤醒
			thread_pool->consumer_cv.wait(guard);
		}
		//thread_pool->consumer_cv.wait(guard, [&]() {
		//	// 防止虚假唤醒
		//	return thread_pool->tasks_queue.size() > 0;
		//	});

		if (thread_pool->tasks_queue.empty()) {
			// 任务队列为空
			guard.unlock();
			continue;
		}

		// 增加工作的子线程数量
		thread_pool->working_thread_num.fetch_add(1);

		T task = thread_pool->tasks_queue.front();
		thread_pool->tasks_queue.pop();

		// 获取任务后释放锁
		guard.unlock();

#ifdef DEBUG
		printf("tasks remain: %d\n", thread_pool->tasks_queue.size());
#endif // DEBUG

		// 执行任务，多态执行
		task.process();

		// 减少工作的子线程数量
		thread_pool->working_thread_num.fetch_sub(1);
		// 用条件变量对象通知生产者已处理完毕
		thread_pool->producer_cv.notify_all();
	}

#ifdef DEBUG
	printf("thread %d ends.\n", std::this_thread::get_id());
#endif // DEBUG

}

template<typename T>
void ThreadPool<T>::wait()
{
	// 先竞争锁再析构资源，避免有线程正在使用任务队列
	std::unique_lock<std::mutex> guard(tasks_queue_mutex);
	while ((working_thread_num.load() > 0) || (tasks_queue.size() > 0)) {
		// 仍有子线程在工作，或者任务队列不为空
		producer_cv.wait(guard);
	}
	return;
}
