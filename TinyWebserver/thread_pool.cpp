#include "thread_pool.h"

ThreadMeta* ThreadPool::sub_thread_metas = nullptr;

ThreadPool::ThreadPool(int thread_n):
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

ThreadPool::~ThreadPool()
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

bool ThreadPool::execute(ThreadExecutableBase* task)
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

void ThreadPool::thread_run(ThreadPool *thread_pool)
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

		ThreadExecutableBase* task = thread_pool->tasks_queue.front();
		thread_pool->tasks_queue.pop();

		// ��ȡ������ͷ���
		guard.unlock();

#ifdef DEBUG
		printf("tasks remain: %d\n", thread_pool->tasks_queue.size());
#endif // DEBUG
		
		// ִ�����񣬶�ִ̬��
		assert(task != nullptr);
		task->process();

		// ���ٹ��������߳�����
		thread_pool->working_thread_num.fetch_sub(1);
		// ��������������֪ͨ�������Ѵ������
		thread_pool->producer_cv.notify_all();
	}

#ifdef DEBUG
	printf("thread %d ends.\n", std::this_thread::get_id());
#endif // DEBUG

}

void ThreadPool::wait()
{
	// �Ⱦ�������������Դ���������߳�����ʹ���������
	std::unique_lock<std::mutex> guard(tasks_queue_mutex);
	while ((working_thread_num.load() > 0) || (tasks_queue.size() > 0)) {
		// �������߳��ڹ���������������в�Ϊ��
		producer_cv.wait(guard);
	}
	return;
}
