#pragma once

#include "core.h"

class ThreadExecutableBase;
class ThreadMeta;

template<typename T>
class ThreadPool;

template<typename U>
class SingletonBase;

/*
* @brief ִ���������
*/
class ThreadExecutableBase {
public:
	virtual int process() = 0;
};

/*
* @brief �߳�Ԫ������
*/
class ThreadMeta {
public:
	std::thread::id sub_tid;  /*���߳�id*/
	std::shared_ptr<std::thread> thread_ptr;  /*���̶߳���ָ��*/

public:
	/*
	* @brief ���̵߳��õ��޲ι��캯��
	* @param
	*/
	ThreadMeta() :sub_tid(-1) {};
};

/*
* @brief ����ģʽ����
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
	* @brief ��ȡʵ����̬����
	* @retval instance => ����ģʽʵ��
	*/
	static U& get_instance(int thread_n = 4) {
		static U instance(thread_n);
		return instance;
	}
};

/*
* @brief �̳߳���
*/
template<typename T>
class ThreadPool :public SingletonBase<ThreadPool<T>>{
private:
	const int MAX_THREAD_NUMBER = core::MAX_THREAD_NUMBER;  /*������߳�����*/
	const int MAX_TASK_NUMBER = core::MAX_TASK_NUMBER;  /*�����������*/

	static ThreadMeta* sub_thread_metas;  /*���߳�Ԫ���ݱ�*/

	std::queue<T> tasks_queue;  /*�������*/
	std::mutex tasks_queue_mutex;  /*������л�����*/
	std::condition_variable consumer_cv;  /*��������������*/
	std::condition_variable producer_cv;  /*��������������*/

	int sub_thread_n;  /*���߳�����*/
	std::atomic<int> working_thread_num;  /*���ڹ��������߳�����*/

	/*
	* @brief ���߳��Ƿ��ѽ���������������̲߳�����ʹ�����߳���Դ
	*        ��Ϊinstanceֻ��������߳�����������
	*/
	bool is_stop;	

private:
	/*
	* @brief �߳���������	
	*/
	static void thread_run(ThreadPool* thread_pool);

public:
	/*
	* @brief �̳߳ع��캯��
	*/
	ThreadPool(int thread_n);

	/*
	* @brief �̳߳���������
	*/
	~ThreadPool();

	/*
	* @brief �����߳�ִ��
	* @retval true => �ɹ������������
	* @retval false => �����������
	*/
	bool execute(T& task);

	/*
	* @brief �ȴ����������е�����ȫ�����
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

	// �½����߳�Ԫ���ݱ�
	sub_thread_metas = new ThreadMeta[sub_thread_n];

	for (int i = 0; i < sub_thread_n; ++i) {
		// �������߳�
		sub_thread_metas[i].thread_ptr = std::make_shared<std::thread>(thread_run, this);
		sub_thread_metas[i].sub_tid = sub_thread_metas[i].thread_ptr->get_id();

		// �������̣߳���Ҫ��Ϊ�˾����������̺߳����߳�֮�������
		sub_thread_metas[i].thread_ptr->detach();
	}

	/*��ʼ�������߳�����*/
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

	// �������߳̽���
	is_stop = true;
	// ֪ͨ�����߳̽���
	consumer_cv.notify_all();

#ifdef DEBUG
	printf("thread pool has destroyed.\n");
#endif // DEBUG
}

template<typename T>
bool ThreadPool<T>::execute(T& task)
{
	// �Ⱦ������ٽ�������ӵ�������У��������߳�����ʹ���������
	std::unique_lock<std::mutex> guard(tasks_queue_mutex);
	if (tasks_queue.size() >= MAX_TASK_NUMBER) {
		return false;
	}
	tasks_queue.push(task);
	// ��������������֪ͨ�����߳̾�������
	consumer_cv.notify_all();

	return true;
}

template<typename T>
void ThreadPool<T>::thread_run(ThreadPool* thread_pool)
{
	while (!thread_pool->is_stop) {
		// �����������ȡ����
		// ������߳��Ѿ���thread_pool��Դ�����ˣ���guard���׳��쳣���Զ���ֹ����߳�
		std::unique_lock<std::mutex> guard(thread_pool->tasks_queue_mutex);

		while (!thread_pool->is_stop && (thread_pool->tasks_queue.size() <= 0)) {
			// ��ֹ��ٻ���
			thread_pool->consumer_cv.wait(guard);
		}
		//thread_pool->consumer_cv.wait(guard, [&]() {
		//	// ��ֹ��ٻ���
		//	return thread_pool->tasks_queue.size() > 0;
		//	});

		if (thread_pool->tasks_queue.empty()) {
			// �������Ϊ��
			guard.unlock();
			continue;
		}

		// ���ӹ��������߳�����
		thread_pool->working_thread_num.fetch_add(1);

		T task = thread_pool->tasks_queue.front();
		thread_pool->tasks_queue.pop();

		// ��ȡ������ͷ���
		guard.unlock();

#ifdef DEBUG
		printf("tasks remain: %d\n", thread_pool->tasks_queue.size());
#endif // DEBUG

		// ִ�����񣬶�ִ̬��
		task.process();

		// ���ٹ��������߳�����
		thread_pool->working_thread_num.fetch_sub(1);
		// ��������������֪ͨ�������Ѵ������
		thread_pool->producer_cv.notify_all();
	}

#ifdef DEBUG
	printf("thread %d ends.\n", std::this_thread::get_id());
#endif // DEBUG

}

template<typename T>
void ThreadPool<T>::wait()
{
	// �Ⱦ�������������Դ���������߳�����ʹ���������
	std::unique_lock<std::mutex> guard(tasks_queue_mutex);
	while ((working_thread_num.load() > 0) || (tasks_queue.size() > 0)) {
		// �������߳��ڹ���������������в�Ϊ��
		producer_cv.wait(guard);
	}
	return;
}
