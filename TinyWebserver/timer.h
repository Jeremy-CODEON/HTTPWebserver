#pragma once

#include "core.h"

/*
* @brief ��ʱ����¼��
*/
class TimerRecord {
public:
	time_t expired_time;  /*��ʱʱ��*/
	int fd;  /*socket�ļ�������*/
	int epollfd;  /*epoll�ļ�������*/
public:
	/*
	* @brief ���캯��
	* @param _time => ��ʱʱ��
	* @param _fd => socket fd
	* @param _epollfd => epoll fd
	*/
	TimerRecord(time_t _time, int _fd, int _epollfd) :
		expired_time(_time + core::EXPIRED_TIME), fd(_fd), epollfd(_epollfd) {}
	/*
	* @brief ��ʱ������
	*/
	void expired_handler() {
		printf("close sockfd %d by timer expired handler\n", fd);
		core::EpollUtils::epoll_remove(epollfd, fd);  // �Ƴ����ر�fd
	}
};

#if 0
/*
* @��ʱ���ȽϷº���
*/
class TimerRecordGreater {
public:
	bool operator()(const TimerRecord& _a, const TimerRecord& _b) {
		return _a.expired_time > _b.expired_time;
	}
};

/*
* @brief ��ʱ�������ࣨС����+��ϣ��
*/
class TimerUtils {
private:
	std::priority_queue<TimerRecord, std::vector<TimerRecord>, TimerRecordGreater> heap;  /*С����*/
	std::unordered_map<int, int> map;  /*������ϣ��{fd, count}*/
	std::mutex mutex;  /*��д������*/

	TimerUtils() = default;
	TimerUtils(const TimerUtils&) = delete;
	TimerUtils& operator=(const TimerUtils&) = delete;
	virtual ~TimerUtils() = default;
public:
	/*
	* @brief ���Ӷ�ʱ��
	*/
	void add_timer(TimerRecord& timer) {
		// ���Ӽ�¼ʱ����
		std::unique_lock<std::mutex> guard(mutex);
		heap.push(timer);
		map[timer.fd]++;
	}
	/* 
	* @brief ��Ⲣ����ʱ��ʱ��
	*/
	void tick() {
#ifdef DEBUG
		printf("another timer tick begins...\n");
#endif // DEBUG

		if (heap.empty()) {
			return;
		}
		// �����¼ʱ����
		std::unique_lock<std::mutex> guard(mutex); 
		TimerRecord timer = heap.top();
		// ��ȡ��ǰϵͳʱ��
		time_t cur_time = time(NULL); 	
		while (!heap.empty() && timer.expired_time < cur_time) {
			// �ѳ�ʱ
			heap.pop();
			map[timer.fd]--;
			
			if (map[timer.fd] == 0) {
				// ���ó�ʱ������
				timer.expired_handler();
			}
			if(!heap.empty())
			{
				timer = heap.top();
			}
		}
	}

	static TimerUtils& get_instance() {
		static TimerUtils instance;
		return instance;
	}
};
#endif


/*
* @brief ��ʱ�������ࣨ˫������+ָ���ϣ��
*/
class TimerUtils {
private:
	using iterator_type = std::list<TimerRecord>::iterator;  /*����������*/
private:
	std::list<TimerRecord> list;  /*˫������*/	
	std::unordered_map<int, iterator_type> map;  /*��λ��ϣ��{fd, list::iterator}*/
	std::mutex mutex;  /*��д������*/

	TimerUtils() = default;
	TimerUtils(const TimerUtils&) = delete;
	TimerUtils& operator=(const TimerUtils&) = delete;
	virtual ~TimerUtils() = default;
public:
	/*
	* @brief ���Ӷ�ʱ��
	*/
	void add_timer(TimerRecord& timer) {
		// ���Ӽ�¼ʱ����
		std::unique_lock<std::mutex> guard(mutex);
		if (map.find(timer.fd) == map.end() || map[timer.fd] == list.end()) {
			// ˫�������в����ڸ�fd��ֱ����ĩβ����
			list.push_back(timer);
			map[timer.fd] = std::prev(list.end());
		}
		else {
			// ˫�������д��ڸ�fd�����Ƴ�֮ǰ�Ľڵ��ټ����µ�timer
			list.erase(map[timer.fd]);
			list.push_back(timer);
			map[timer.fd] = std::prev(list.end());
		}
	}
	/*
	* @brief ��Ⲣ����ʱ��ʱ��
	*/
	void tick() {
#ifdef DEBUG
		printf("another timer tick begins...\n");
#endif // DEBUG

		if (list.empty()) {
			return;
		}
		// �����¼ʱ����
		std::unique_lock<std::mutex> guard(mutex);
		TimerRecord timer = list.front();
		// ��ȡ��ǰϵͳʱ��
		time_t cur_time = time(NULL);
		while (!list.empty() && timer.expired_time < cur_time) {
			// �ѳ�ʱ
			list.pop_front();
			map[timer.fd] = list.end();

			// ���ó�ʱ������
			timer.expired_handler();

			if (!list.empty())
			{
				timer = list.front();
			}
		}
	}

	static TimerUtils& get_instance() {
		static TimerUtils instance;
		return instance;
	}
};
