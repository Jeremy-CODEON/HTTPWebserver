#pragma once

#include "core.h"

class SharedPointer;
class UniquePointer;
class WeakPointer;

/*
* @brief ������
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
* @brief ����ָ���ԭ�Ӽ�������
*/
class AtomicCount
{
private:
	std::atomic<int> shared_count;  /*����ָ����� = shared*/
	std::atomic<int> weak_count;  /*��ָ����� = shared + weak*/
	std::mutex count_mutex;  /*����������*/

	/*
	* ���ø��ƹ��캯���͸�ֵ��������غ���
	*/
	AtomicCount(const AtomicCount&) = delete;
	AtomicCount& operator=(const AtomicCount&) = delete;

public:
	/*
	* @brief ���캯��
	*/
	AtomicCount() :shared_count(1), weak_count(1) {
		printf("automic count block has constructed.\n");
	}
	virtual ~AtomicCount() {
		printf("automic count block has destroyed.\n");
	}

	/*
	* @brief �ͷŹ���ָ��ļ���
	*/
	void release_shared() {
		--shared_count;
		--weak_count;
	}

	/*
	* @brief ���ӹ���ָ��ļ���
	*/
	void add_shared() {
		++shared_count;
		++weak_count;
	}

	/*
	* @brief �ͷ���ָ��ļ���
	*/
	void release_weak() {
		--weak_count;
	}

	/*
	* @brief ������ָ��ļ���
	*/
	void add_weak() {
		++weak_count;
	}

public:
	/*
	* @brief ���ػ���������
	*/
	std::mutex& get_mutex() {
		return count_mutex;
	}

	/*
	* @brief ���ع���ָ�����ü���
	*/
	int get_shared_count() const {
		return shared_count.load();
	}

	/*
	* @brief ������ָ�����ü���
	*/
	int get_weak_count() const {
		return weak_count.load();
	}
};

/*
* @brief ����ָ����
*/
class SharedPointer
{
private:
	ObjectTest* _pobject;  /*�������ָ��*/
	AtomicCount* _pcount;  /*ָ��ԭ�Ӵ����*/

	// ������ָ����Ԫ��
	friend class WeakPointer;  
private:
	/*
	* @brief �ͷŶ���Դ������
	*/
	void release_ref() {
		// ������Ҫ������
		std::unique_lock<std::mutex> guard(_pcount->get_mutex());
		_pcount->release_shared();
		if (_pcount->get_shared_count() == 0) {
			if (_pobject != nullptr) {
				// �������ü���Ϊ0���ͷŶ���
				delete _pobject;
			}				
		}
		if (_pcount->get_weak_count() == 0) {
			// �����ü���Ϊ0���ͷ�ԭ�Ӽ�����
			delete _pcount;
		}
		// �ͷ���
		guard.unlock();
		// ָ���ÿ�
		_pobject = nullptr;
		_pcount = nullptr;
	}

	/*
	* @brief ���Ӷ���Դ������
	*/
	void add_ref() {
		// ������
		std::unique_lock<std::mutex> guard(_pcount->get_mutex());
		_pcount->add_shared();
		// �ͷ���
		guard.unlock();
	}

public:
	/*
	* @brief ���캯��
	*/
	SharedPointer() :
		_pobject(nullptr), _pcount(new AtomicCount()) {}

	/*
	* @brief ָ�빹�캯��
	* @param ptr => ��ָͨ�룬����ֱ�ӽ���new���أ�Ĭ��Ϊnullptr
	*/
	explicit SharedPointer(ObjectTest* ptr) :
		_pobject(ptr), _pcount(new AtomicCount()) {}

	/*
	* @brief ���ƹ��캯��
	* @param sptr => ����ָ��
	*/
	SharedPointer(const SharedPointer& sptr) :
		_pobject(sptr._pobject), _pcount(sptr._pcount){
		// ���Ӽ�������
		add_ref();
	}

	/*
	* @brief ��ռָ�빹�캯��
	* @param uptr => ��ռָ��
	*/
	SharedPointer(UniquePointer&& uptr);

	/*
	* @brief ��ָ�빹�캯��
	* @param wptr => ��ָ��
	*/
	SharedPointer(const WeakPointer& wptr);

	/*
	* @brief ��������
	*/
	virtual ~SharedPointer() {
		// ���ټ�������
		release_ref();
	}

public:
	/*
	* @brief ��ֵ��������غ���
	* @param sptr => ����ָ��
	*/
	SharedPointer& operator=(const SharedPointer& sptr) {
		if (_pobject != sptr._pobject) {
			// ���ټ�������
			release_ref();
			// ָ������Դ
			_pobject = sptr._pobject;
			_pcount = sptr._pcount;
			// ���Ӽ�������
			add_ref();
		}
		return *this;
	}

	/*
	* @brief *��������غ���
	*/
	ObjectTest& operator*() const {
		return *_pobject;
	}

	/*
	* @brief ->��������غ���
	*/
	ObjectTest* operator->() const {
		return _pobject;
	}

public:
	/*
	* @brief ��ȡ����������
	*/
	int use_count() const {
		return _pcount->get_shared_count();
	}

};


/*
* @brief ��ռָ����
*/
class UniquePointer
{
private:
	ObjectTest* _pobject;  /*��ռ����ָ��*/
	AtomicCount* _pcount;  /*ָ��ԭ�Ӵ����*/

	// ��������ָ����Ԫ��
	friend class SharedPointer;

	/*
	* ���ø��ƹ��캯���͸�ֵ��������غ���
	*/
	UniquePointer(const UniquePointer&) = delete;
	UniquePointer& operator=(const UniquePointer&) = delete;

	/*
	* @brief �ͷž���Դ
	*/
	void release_ref() {
		// �������ͷ�
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
	* @brief �޲ι��캯��
	*/
	UniquePointer() :
		_pobject(nullptr), _pcount(new AtomicCount()) {}

	/*
	* @brief ���캯��
	* @param ptr => ��ָͨ�룬����ֱ�ӽ���new���أ�Ĭ��Ϊnullptr
	*/
	explicit UniquePointer(ObjectTest* ptr) :
		_pobject(ptr), _pcount(new AtomicCount()) {}

	/*
	* @brief �ƶ����캯��
	* @param uptr => ��ռָ��
	*/
	UniquePointer(UniquePointer&& uptr) {
		if (uptr._pobject != nullptr) {
			// ������ת��
			std::lock_guard<std::mutex> guard(uptr._pcount->get_mutex());
			// ָ������Դ
			_pobject = uptr._pobject;
			_pcount = uptr._pcount;
			// ��ǰһ����ռָ���ÿ�
			uptr._pobject = nullptr;
			uptr._pcount = nullptr;
		}
		else {
			// ָ������Դ
			_pobject = uptr._pobject;
			_pcount = uptr._pcount;
			// ��ǰһ����ռָ���ÿ�
			uptr._pobject = nullptr;
			uptr._pcount = nullptr;
		}
	}

	/*
	* @brief ��������
	*/
	~UniquePointer() {
		// �ͷž���Դ
		release_ref();
	}

public:
	/*
	* @brief �ƶ����캯��
	* @param uptr => ��ռָ��
	*/
	UniquePointer& operator=(UniquePointer&& uptr) {
		if (_pobject != uptr._pobject) {
			// �ͷž���Դ
			release_ref();
			// ������ת��
			std::lock_guard<std::mutex> guard(uptr._pcount->get_mutex());
			// ָ������Դ
			_pobject = uptr._pobject;
			_pcount = uptr._pcount;
			// ��ǰһ����ռָ���ÿ�
			uptr._pobject = nullptr;
			uptr._pcount = nullptr;
		}		

		return *this;
	}

	/*
	* @brief *��������غ���
	*/
	ObjectTest& operator*() const {
		return *_pobject;
	}

	/*
	* @brief ->��������غ���
	*/
	ObjectTest* operator->() const {
		return _pobject;
	}

#ifdef DEBUG
	/*
	* @brief �ж��Ƿ�Ϊ��
	*/
	ObjectTest* get() const {
		return _pobject;
	}
#endif // DEBUG
	
};

/*
* @brief ��ָ����
*/
class WeakPointer
{
private:
	ObjectTest* _pobject;  /*�������ָ�룬�޷�ֱ�ӷ��أ�ֻ�ܸ�����ָ�빹��*/
	AtomicCount* _pcount;  /*ָ��ԭ�Ӵ����*/

	// ��������ָ����Ԫ��
	friend class SharedPointer;

private:
	/*
	* ���Ӽ�������
	*/
	void add_ref() {
		if (_pcount != nullptr) {
			std::unique_lock<std::mutex> guard(_pcount->get_mutex());
			_pcount->add_weak();
		}		
	}

	/*
	* �ͷż�������
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
	* �����޲ι��캯��
	*/
	WeakPointer() = delete;
public:
	/*
	* @brief �ӹ���ָ�빹��
	*/
	WeakPointer(const SharedPointer& sptr):
		_pobject(sptr._pobject), _pcount(sptr._pcount) {
		// ���Ӽ�������
		add_ref();
	}

	/*
	* @brief ����ָ�빹�죬��������
	*/
	WeakPointer(const WeakPointer& wptr) :
		_pobject(wptr._pobject), _pcount(wptr._pcount) {
		// ���Ӽ�������
		add_ref();
	}

	/*
	* @brief ��������
	*/
	~WeakPointer() {
		// �ͷ����ü���
		release_ref();
	}

public:
	/*
	* @brief �ӹ���ָ�븳ֵ
	*/
	WeakPointer& operator=(const SharedPointer& sptr) {
		if (_pobject != sptr._pobject) {
			// �ͷ����ü���
			release_ref();
			_pobject = sptr._pobject;
			_pcount = sptr._pcount;
			// �������ü���
			add_ref();
		}
		
		return *this;
	}

	/*
	* @brief ����ָ�븳ֵ
	*/
	WeakPointer& operator=(const WeakPointer& wptr) {
		if (_pobject != wptr._pobject) {
			// �ͷ����ü���
			release_ref();
			_pobject = wptr._pobject;
			_pcount = wptr._pcount;
			// �������ü���
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
	// ������ת��
	std::lock_guard<std::mutex> guard(uptr._pcount->get_mutex());
	// ָ������Դ
	_pobject = uptr._pobject;
	_pcount = uptr._pcount;
	// ��ǰһ����ռָ���ÿ�
	uptr._pobject = nullptr;
	uptr._pcount = nullptr;
}

SharedPointer::SharedPointer(const WeakPointer& wptr) :
	_pobject(wptr._pobject), _pcount(wptr._pcount) {
	// ���Ӽ�������
	add_ref();
}
