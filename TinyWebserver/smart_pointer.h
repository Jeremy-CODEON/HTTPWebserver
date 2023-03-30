#pragma once

#include "core.h"

class SharedPointer;
class UniquePointer;
class WeakPointer;

/*
* @brief 测试类
*/
class ObjectTest
{
public:
	int value;
	ObjectTest() :value(0) {
		printf("default constructed.\n");
	}
	ObjectTest(int _value) :value(_value) {
		printf("parameter constructed.\n");
	}
	virtual ~ObjectTest() {
		printf("destroyed.\n");
	}
};

/*
* @brief 智能指针的原子计数块类
*/
class AtomicCount
{
private:
	std::atomic<int> shared_count;  /*共享指针计数 = shared*/
	std::atomic<int> weak_count;  /*弱指针计数 = shared + weak*/
	std::mutex count_mutex;  /*计数互斥量*/

	/*
	* 禁用复制构造函数和赋值运算符重载函数
	*/
	AtomicCount(const AtomicCount&) = delete;
	AtomicCount& operator=(const AtomicCount&) = delete;

public:
	/*
	* @brief 构造函数
	*/
	AtomicCount() :shared_count(1), weak_count(1) {
		printf("automic count block has constructed.\n");
	}
	virtual ~AtomicCount() {
		printf("automic count block has destroyed.\n");
	}

	/*
	* @brief 释放共享指针的计数
	*/
	void release_shared() {
		--shared_count;
		--weak_count;
	}

	/*
	* @brief 增加共享指针的计数
	*/
	void add_shared() {
		++shared_count;
		++weak_count;
	}

	/*
	* @brief 释放弱指针的计数
	*/
	void release_weak() {
		--weak_count;
	}

	/*
	* @brief 增加弱指针的计数
	*/
	void add_weak() {
		++weak_count;
	}

public:
	/*
	* @brief 返回互斥量引用
	*/
	std::mutex& get_mutex() {
		return count_mutex;
	}

	/*
	* @brief 返回共享指针引用计数
	*/
	int get_shared_count() const {
		return shared_count.load();
	}

	/*
	* @brief 返回弱指针引用计数
	*/
	int get_weak_count() const {
		return weak_count.load();
	}
};

/*
* @brief 共享指针类
*/
class SharedPointer
{
private:
	ObjectTest* _pobject;  /*共享对象指针*/
	AtomicCount* _pcount;  /*指向原子代理块*/

	// 声明弱指针友元类
	friend class WeakPointer;  
private:
	/*
	* @brief 释放对资源的引用
	*/
	void release_ref() {
		// 析构需要竞争锁
		std::unique_lock<std::mutex> guard(_pcount->get_mutex());
		_pcount->release_shared();
		if (_pcount->get_shared_count() == 0) {
			if (_pobject != nullptr) {
				// 共享引用计数为0，释放对象
				delete _pobject;
			}				
		}
		if (_pcount->get_weak_count() == 0) {
			// 弱引用计数为0，释放原子计数块
			delete _pcount;
		}
		// 释放锁
		guard.unlock();
		// 指针置空
		_pobject = nullptr;
		_pcount = nullptr;
	}

	/*
	* @brief 增加对资源的引用
	*/
	void add_ref() {
		// 竞争锁
		std::unique_lock<std::mutex> guard(_pcount->get_mutex());
		_pcount->add_shared();
		// 释放锁
		guard.unlock();
	}

public:
	/*
	* @brief 构造函数
	*/
	SharedPointer() :
		_pobject(nullptr), _pcount(new AtomicCount()) {}

	/*
	* @brief 指针构造函数
	* @param ptr => 普通指针，可以直接接受new返回，默认为nullptr
	*/
	explicit SharedPointer(ObjectTest* ptr) :
		_pobject(ptr), _pcount(new AtomicCount()) {}

	/*
	* @brief 复制构造函数
	* @param sptr => 共享指针
	*/
	SharedPointer(const SharedPointer& sptr) :
		_pobject(sptr._pobject), _pcount(sptr._pcount){
		// 增加计数引用
		add_ref();
	}

	/*
	* @brief 独占指针构造函数
	* @param uptr => 独占指针
	*/
	SharedPointer(UniquePointer&& uptr);

	/*
	* @brief 弱指针构造函数
	* @param wptr => 弱指针
	*/
	SharedPointer(const WeakPointer& wptr);

	/*
	* @brief 析构函数
	*/
	virtual ~SharedPointer() {
		// 减少计数引用
		release_ref();
	}

public:
	/*
	* @brief 赋值运算符重载函数
	* @param sptr => 共享指针
	*/
	SharedPointer& operator=(const SharedPointer& sptr) {
		if (_pobject != sptr._pobject) {
			// 减少计数引用
			release_ref();
			// 指向新资源
			_pobject = sptr._pobject;
			_pcount = sptr._pcount;
			// 增加计数引用
			add_ref();
		}
		return *this;
	}

	/*
	* @brief *运算符重载函数
	*/
	ObjectTest& operator*() const {
		return *_pobject;
	}

	/*
	* @brief ->运算符重载函数
	*/
	ObjectTest* operator->() const {
		return _pobject;
	}

public:
	/*
	* @brief 获取共享引用数
	*/
	int use_count() const {
		return _pcount->get_shared_count();
	}

};


/*
* @brief 独占指针类
*/
class UniquePointer
{
private:
	ObjectTest* _pobject;  /*独占对象指针*/
	AtomicCount* _pcount;  /*指向原子代理块*/

	// 声明共享指针友元类
	friend class SharedPointer;

	/*
	* 禁用复制构造函数和赋值运算符重载函数
	*/
	UniquePointer(const UniquePointer&) = delete;
	UniquePointer& operator=(const UniquePointer&) = delete;

	/*
	* @brief 释放旧资源
	*/
	void release_ref() {
		// 竞争锁释放
		if (_pcount != nullptr) {
			std::lock_guard<std::mutex> guard(_pcount->get_mutex());
			if (_pobject != nullptr) {
				delete _pobject;
			}
			delete _pcount;
		}
	}
public:
	/*
	* @brief 无参构造函数
	*/
	UniquePointer() :
		_pobject(nullptr), _pcount(new AtomicCount()) {}

	/*
	* @brief 构造函数
	* @param ptr => 普通指针，可以直接接受new返回，默认为nullptr
	*/
	explicit UniquePointer(ObjectTest* ptr) :
		_pobject(ptr), _pcount(new AtomicCount()) {}

	/*
	* @brief 移动构造函数
	* @param uptr => 独占指针
	*/
	UniquePointer(UniquePointer&& uptr) {
		if (uptr._pobject != nullptr) {
			// 竞争锁转移
			std::lock_guard<std::mutex> guard(uptr._pcount->get_mutex());
			// 指向新资源
			_pobject = uptr._pobject;
			_pcount = uptr._pcount;
			// 将前一个独占指针置空
			uptr._pobject = nullptr;
			uptr._pcount = nullptr;
		}
		else {
			// 指向新资源
			_pobject = uptr._pobject;
			_pcount = uptr._pcount;
			// 将前一个独占指针置空
			uptr._pobject = nullptr;
			uptr._pcount = nullptr;
		}
	}

	/*
	* @brief 析构函数
	*/
	~UniquePointer() {
		// 释放旧资源
		release_ref();
	}

public:
	/*
	* @brief 移动构造函数
	* @param uptr => 独占指针
	*/
	UniquePointer& operator=(UniquePointer&& uptr) {
		if (_pobject != uptr._pobject) {
			// 释放旧资源
			release_ref();
			// 竞争锁转移
			std::lock_guard<std::mutex> guard(uptr._pcount->get_mutex());
			// 指向新资源
			_pobject = uptr._pobject;
			_pcount = uptr._pcount;
			// 将前一个独占指针置空
			uptr._pobject = nullptr;
			uptr._pcount = nullptr;
		}		

		return *this;
	}

	/*
	* @brief *运算符重载函数
	*/
	ObjectTest& operator*() const {
		return *_pobject;
	}

	/*
	* @brief ->运算符重载函数
	*/
	ObjectTest* operator->() const {
		return _pobject;
	}

#ifdef DEBUG
	/*
	* @brief 判断是否为空
	*/
	ObjectTest* get() const {
		return _pobject;
	}
#endif // DEBUG
	
};

/*
* @brief 弱指针类
*/
class WeakPointer
{
private:
	ObjectTest* _pobject;  /*共享对象指针，无法直接返回，只能给共享指针构造*/
	AtomicCount* _pcount;  /*指向原子代理块*/

	// 声明共享指针友元类
	friend class SharedPointer;

private:
	/*
	* 增加计数引用
	*/
	void add_ref() {
		if (_pcount != nullptr) {
			std::unique_lock<std::mutex> guard(_pcount->get_mutex());
			_pcount->add_weak();
		}		
	}

	/*
	* 释放计数引用
	*/
	void release_ref() {
		if (_pcount != nullptr) {
			std::unique_lock<std::mutex> guard(_pcount->get_mutex());
			_pcount->release_weak();
			if (_pcount->get_weak_count() == 0) {
				delete _pcount;
			}
			guard.unlock();
			_pcount = nullptr;
		}		
	}

	/*
	* 禁用无参构造函数
	*/
	WeakPointer() = delete;
public:
	/*
	* @brief 从共享指针构造
	*/
	WeakPointer(const SharedPointer& sptr):
		_pobject(sptr._pobject), _pcount(sptr._pcount) {
		// 增加计数引用
		add_ref();
	}

	/*
	* @brief 从弱指针构造，拷贝构造
	*/
	WeakPointer(const WeakPointer& wptr) :
		_pobject(wptr._pobject), _pcount(wptr._pcount) {
		// 增加计数引用
		add_ref();
	}

	/*
	* @brief 析构函数
	*/
	~WeakPointer() {
		// 释放引用计数
		release_ref();
	}

public:
	/*
	* @brief 从共享指针赋值
	*/
	WeakPointer& operator=(const SharedPointer& sptr) {
		if (_pobject != sptr._pobject) {
			// 释放引用计数
			release_ref();
			_pobject = sptr._pobject;
			_pcount = sptr._pcount;
			// 增加引用计数
			add_ref();
		}
		
		return *this;
	}

	/*
	* @brief 从弱指针赋值
	*/
	WeakPointer& operator=(const WeakPointer& wptr) {
		if (_pobject != wptr._pobject) {
			// 释放引用计数
			release_ref();
			_pobject = wptr._pobject;
			_pcount = wptr._pcount;
			// 增加引用计数
			add_ref();
		}
		return *this;
	}

	SharedPointer lock() {
		if (_pcount == nullptr || _pcount->get_shared_count() == 0) {
			SharedPointer tmp(nullptr);
			return SharedPointer();
		}
		else {
			return SharedPointer(*this);
		}
	}
};

SharedPointer::SharedPointer(UniquePointer&& uptr){
	// 竞争锁转移
	std::lock_guard<std::mutex> guard(uptr._pcount->get_mutex());
	// 指向新资源
	_pobject = uptr._pobject;
	_pcount = uptr._pcount;
	// 将前一个独占指针置空
	uptr._pobject = nullptr;
	uptr._pcount = nullptr;
}

SharedPointer::SharedPointer(const WeakPointer& wptr) :
	_pobject(wptr._pobject), _pcount(wptr._pcount) {
	// 增加计数引用
	add_ref();
}
