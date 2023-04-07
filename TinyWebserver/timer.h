#pragma once

#include "core.h"

/*
* @brief 定时器记录类
*/
class TimerRecord {
public:
	time_t expired_time;  /*超时时间*/
	int fd;  /*socket文件描述符*/
	int epollfd;  /*epoll文件描述符*/
public:
	/*
	* @brief 构造函数
	* @param _time => 超时时间
	* @param _fd => socket fd
	* @param _epollfd => epoll fd
	*/
	TimerRecord(time_t _time, int _fd, int _epollfd) :
		expired_time(_time + core::EXPIRED_TIME), fd(_fd), epollfd(_epollfd) {}
	/*
	* @brief 超时处理函数
	*/
	void expired_handler() {
		printf("close sockfd %d by timer expired handler\n", fd);
		core::EpollUtils::epoll_remove(epollfd, fd);  // 移除并关闭fd
	}
};

#if 0
/*
* @定时器比较仿函数
*/
class TimerRecordGreater {
public:
	bool operator()(const TimerRecord& _a, const TimerRecord& _b) {
		return _a.expired_time > _b.expired_time;
	}
};

/*
* @brief 定时器管理类（小顶堆+哈希表）
*/
class TimerUtils {
private:
	std::priority_queue<TimerRecord, std::vector<TimerRecord>, TimerRecordGreater> heap;  /*小顶堆*/
	std::unordered_map<int, int> map;  /*计数哈希表{fd, count}*/
	std::mutex mutex;  /*读写互斥量*/

	TimerUtils() = default;
	TimerUtils(const TimerUtils&) = delete;
	TimerUtils& operator=(const TimerUtils&) = delete;
	virtual ~TimerUtils() = default;
public:
	/*
	* @brief 增加定时器
	*/
	void add_timer(TimerRecord& timer) {
		// 增加记录时加锁
		std::unique_lock<std::mutex> guard(mutex);
		heap.push(timer);
		map[timer.fd]++;
	}
	/* 
	* @brief 检测并处理超时定时器
	*/
	void tick() {
#ifdef DEBUG
		printf("another timer tick begins...\n");
#endif // DEBUG

		if (heap.empty()) {
			return;
		}
		// 处理记录时加锁
		std::unique_lock<std::mutex> guard(mutex); 
		TimerRecord timer = heap.top();
		// 获取当前系统时间
		time_t cur_time = time(NULL); 	
		while (!heap.empty() && timer.expired_time < cur_time) {
			// 已超时
			heap.pop();
			map[timer.fd]--;
			
			if (map[timer.fd] == 0) {
				// 调用超时处理函数
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
* @brief 定时器管理类（双向链表+指向哈希表）
*/
class TimerUtils {
private:
	using iterator_type = std::list<TimerRecord>::iterator;  /*迭代器别名*/
private:
	std::list<TimerRecord> list;  /*双向链表*/	
	std::unordered_map<int, iterator_type> map;  /*定位哈希表{fd, list::iterator}*/
	std::mutex mutex;  /*读写互斥量*/

	TimerUtils() = default;
	TimerUtils(const TimerUtils&) = delete;
	TimerUtils& operator=(const TimerUtils&) = delete;
	virtual ~TimerUtils() = default;
public:
	/*
	* @brief 增加定时器
	*/
	void add_timer(TimerRecord& timer) {
		// 增加记录时加锁
		std::unique_lock<std::mutex> guard(mutex);
		if (map.find(timer.fd) == map.end() || map[timer.fd] == list.end()) {
			// 双向链表中不存在该fd，直接在末尾加入
			list.push_back(timer);
			map[timer.fd] = std::prev(list.end());
		}
		else {
			// 双向链表中存在该fd，先移除之前的节点再加入新的timer
			list.erase(map[timer.fd]);
			list.push_back(timer);
			map[timer.fd] = std::prev(list.end());
		}
	}
	/*
	* @brief 检测并处理超时定时器
	*/
	void tick() {
#ifdef DEBUG
		printf("another timer tick begins...\n");
#endif // DEBUG

		if (list.empty()) {
			return;
		}
		// 处理记录时加锁
		std::unique_lock<std::mutex> guard(mutex);
		TimerRecord timer = list.front();
		// 获取当前系统时间
		time_t cur_time = time(NULL);
		while (!list.empty() && timer.expired_time < cur_time) {
			// 已超时
			list.pop_front();
			map[timer.fd] = list.end();

			// 调用超时处理函数
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
