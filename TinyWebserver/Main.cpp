#include "base_io.h"
#include "buffer_io.h"
#include "process_pool.h"
#include "thread_pool.h"

#include <chrono>

int main() {
#if 0
	printf("Hello world!\n");
#endif

#if 0
	char c;
	while (BaseIO::read_n(STDIN_FILENO, &c, 1) != 0) {
		BaseIO::written_n(STDOUT_FILENO, &c, 1);
	}
#endif

#if 0
	std::string in_filename("/home/jeremy_wsl/projects/TinyWebserver/test/in.txt");
	std::string out_filename("/home/jeremy_wsl/projects/TinyWebserver/test/out.txt");

	// ���ļ�������������
	buffer_t *in_bp = BufferIO::buffer_open(in_filename.c_str(), O_RDONLY);
	if (in_bp != nullptr) {
		printf("open in file successfully.\n");
	}
	buffer_t* out_bp = BufferIO::buffer_open(out_filename.c_str(), O_WRONLY);
	if (out_bp != nullptr) {
		printf("open out file successfully.\n");
	}

	constexpr int N = 100;
	char content[N + 1];
	int in_bn = 0, out_bn = 0;

	// ���庯��ָ�����
	using read_fn = ssize_t(*)(buffer_t*, void*, size_t);
	//typedef ssize_t(*read_fn)(buffer_t*, void*, size_t);
	//read_fn fnp = BufferIO::buffer_read;
	//read_fn fnp = BufferIO::buffer_read_n;
	read_fn fnp = BufferIO::buffer_read_line;

	// ���ļ��ж��뵽content
	while ((in_bn = fnp(in_bp, content, N)) > 0)
	{
		content[in_bn] = '\0';
		printf("\nanother read, content:\n");
		printf("%s\n", content);
		printf("another read end.\n");

		// ��contentд�����ļ�
		out_bn = BufferIO::buffer_written_n(out_bp, content, in_bn);
		printf("another write %d bytes.\n", out_bn);
	}
	//out_bn = BufferIO::buffer_written_flush(out_bp);
	//printf("another write %d bytes.\n", out_bn);

	// �ر��ļ�
	if (!BufferIO::buffer_close(in_bp)) {
		printf("close in file successfully.\n");
	}
	if (!BufferIO::buffer_close(out_bp)) {
		printf("close out file successfully.\n");
	}
#endif

#if 0
	// ��ȡ���̳�ʵ��
	ProcessPool& process_pool = ProcessPool::get_instance(4);
	//printf("main instance get.\n");


	// �������̳�
	//process_pool->run();

	ProcessExecutable obj;
	for (int i = 0; i < 10; ++i) {
		process_pool.execute(obj);
	}

	//process_pool->destory();
#endif

#if 0
	/*
	* @brief ���̳�ִ���࣬��װ�ӽ���ִ�еĺ���
	*/
	class ProcessExecutable :public ProcessExecutableBase {
	private:
		char* content;

		void print_content() {
			content = "Hello world!";
			printf("process task: %s\n", content);
		}
	public:
		virtual int process() override {
			print_content();
		}
	};	

	// ��ȡ���̳�ʵ��
	ProcessPool<ProcessExecutable>& process_pool = ProcessPool<ProcessExecutable>::get_instance(4);
	// �������̳�
	//process_pool->run();

	ProcessExecutable obj;
	for (int i = 0; i < 10; ++i) {
		process_pool.execute(obj);
	}

	// �����ӽ���
	//process_pool->destory();
#endif

#if 0
	class A{};
	std::shared_ptr<int> shared_null = nullptr;      // ���Խ�shared_ptr��ʼ��Ϊnullptr
	std::unique_ptr<int> unique_null = nullptr;      // ���Խ�unique_ptr��ʼ��Ϊnullptr
	//std::weak_ptr<int> weak_null = nullptr;        // ���ܽ�weak_ptr��ʼ��Ϊnullptr

	std::shared_ptr<int> shared_obj = std::make_shared<int>();             // ������new�������ͳ�ʼ��shared_ptr���Ƽ���
	//std::shared_ptr<int> shared_obj(new int);                            // ������new�������ͳ�ʼ��shared_ptr
	//std::shared_ptr<int> shared_obj = std::shared_ptr<int>(new int);           // ������new�������ͳ�ʼ��shared_ptr������һ��д���ȼۣ�
	std::unique_ptr<int> unique_obj(new int);                              // ������new�������ͳ�ʼ��unique_ptr
	//std::unique_ptr<int> unique_obj = std::unique_ptr<int>(new int);           // ������new�������ͳ�ʼ��unique_ptr������һ��д���ȼۣ�
	//std::weak_ptr<int> weak_obj(new int);                                // ������new�������ͳ�ʼ��weak_ptr

	std::shared_ptr<A> shared_A = std::make_shared<A>();                   // �����ö���ָ���ʼ��shared_ptr���Ƽ���
	//std::shared_ptr<A> shared_A(new A);                                  // �����ö���ָ���ʼ��shared_ptr
	//std::shared_ptr<A> shared_A = std::shared_ptr<A>(new A);             // �����ö���ָ���ʼ��shared_ptr������һ��д���ȼۣ�
	std::unique_ptr<A> unique_A(new A);                                    // �����ö���ָ���ʼ��unique_ptr
	//std::unique_ptr<A> unique_A = std::unique_ptr<A>(new A);             // �����ö���ָ���ʼ��unique_ptr������һ��д���ȼۣ�
	//std::weak_ptr<A> weak_A(new A);                                      // �����ö���ָ���ʼ��weak_ptr

	std::shared_ptr<int> shared_p1(shared_obj);                            // ������shared_ptr��ʼ��shared_ptr
	std::shared_ptr<int> shared_p2(std::move(unique_obj));                 // ������unique_ptr��ʼ��shared_ptr��unique_ptr�Զ��ÿ�
	std::unique_ptr<int> unique_p(std::move(unique_obj));                  // ֻ����unique_ptr��ʼ��unique_ptr
	std::weak_ptr<int> weak_p1(shared_obj);                                // ������shared_ptr��ʼ��weak_ptr
	//std::weak_ptr<int> weak_p2(unique_obj);                              // ������unique_ptr��ʼ��weak_ptr
	std::weak_ptr<int> weak_p3(weak_p1);                                   // ������weak_ptr��ʼ��weak_ptr
	std::shared_ptr<int> shared_p3(weak_p1);                               // ������weak_ptr��ʼ��shared_ptr


	shared_null = shared_obj;                                              // ������shared_ptr��ֵ��shared_ptr
	shared_null = std::move(unique_obj);                                   // ������unique_ptr��ֵ��shared_ptr��unique_ptr�Զ��ÿ�
	//shared_null = weak_p1;                                               // ������weak_ptr��ֵ��shared_ptr
	unique_null = std::move(unique_obj);                                   // ֻ����unique_ptr��ֵ��unique_ptr��ǰһ��unique_ptr�Զ��ÿ�
	weak_p3 = shared_obj;                                                  // ������shared_ptr��ֵ��weak_ptr
	//weak_p3 = std::move(unique_obj);                                     // ������unique_ptr��ֵ��weak_ptr
	weak_p3 = weak_p1;                                                     // ������weak_ptr��ֵ��weak_ptr

	shared_null = nullptr;                            // ���Խ�nullptr��ֵ��shared_ptr
	unique_null = nullptr;                            // ���Խ�nullptr��ֵ��unique_ptr
    //weak_p1 = nullptr;                              // ���ܽ�nullptr��ֵ��weak_ptr

	int num = 4;
	std::shared_ptr<int> shared_int(&num);                              // ��������ָͨ���ʼ��shared_ptr
	std::unique_ptr<int> unique_int(&num);                              // ��������ָͨ���ʼ��unique_ptr
	//std::weak_ptr<int> weak_int(&num);                                // ��������ָͨ���ʼ��weak_ptr
	
	//shared_null = &num;                                               // ����ֱ�ӽ���ָͨ�븳ֵ��shared_ptr
	//unique_null = &num;                                               // ����ֱ�ӽ���ָͨ�븳ֵ��unique_ptr
	//weak_p1 = &num;                                                   // ����ֱ�ӽ���ָͨ�븳ֵ��weak_ptr

	shared_null = std::make_shared<int>(num);                           // ���Խ���װ�����ָͨ�븳ֵ��shared_ptr
	unique_null = std::unique_ptr<int>(&num);                           // ���Խ���װ�����ָͨ�븳ֵ��unique_ptr
	//weak_p1 = std::weak_ptr<int>(&num);                               // ���ܽ���װ�����ָͨ�븳ֵ��weak_ptr
#endif

#if 0
	/*
	* @brief ���̳�ִ���࣬��װ�ӽ���ִ�еĺ���
	*/
	class ThreadExecutable :public ThreadExecutableBase {
	private:
		char* content;

		void print_content() {
			content = "Hello world!";
			printf("thread task: %s\n", content);
		}
	public:
		virtual int process() override {
			print_content();
		}
	};

	// ��ȡ�̳߳�ʵ��
	ThreadPool& thread_pool = ThreadPool::get_instance(4);

	ThreadExecutable obj;
	for (int i = 0; i < 10; ++i) {
		thread_pool.execute(&obj);
	}

	// ��������ֻ��ָ�룬��Դ�������߳��ڶ�����������������
	// ������߳���Ҫ�ȴ���������������ֹ
	thread_pool.wait();

	printf("main thread end.\n");
#endif



	return 0;
}