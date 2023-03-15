#pragma once

#include "core.h"

class ThreadExecutableBase;
class ThreadMeta;
class ThreadPool;

template<typename T>
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
	* @brief ��ȡʵ����̬����
	* @retval instance => ����ģʽʵ��
	*/
	static T& get_instance(int thread_n = 4) {
		static T instance(thread_n);
		return instance;
	}
};

/*
* @brief �̳߳���
*/
class ThreadPool :public SingletonBase<ThreadPool>{
private:
	const int MAX_THREAD_NUMBER = core::MAX_THREAD_NUMBER;  /*������߳�����*/
	const int MAX_TASK_NUMBER = core::MAX_TASK_NUMBER;  /*�����������*/

	static ThreadMeta* sub_thread_metas;  /*���߳�Ԫ���ݱ�*/

	std::queue<ThreadExecutableBase*> tasks_queue;  /*�������*/
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
	bool execute(ThreadExecutableBase* task);

	/*
	* @brief �ȴ����������е�����ȫ�����
	*/
	void wait();
};


