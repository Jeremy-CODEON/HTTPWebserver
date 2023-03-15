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

	// 打开文件，构建缓冲区
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

	// 定义函数指针别名
	using read_fn = ssize_t(*)(buffer_t*, void*, size_t);
	//typedef ssize_t(*read_fn)(buffer_t*, void*, size_t);
	//read_fn fnp = BufferIO::buffer_read;
	//read_fn fnp = BufferIO::buffer_read_n;
	read_fn fnp = BufferIO::buffer_read_line;

	// 从文件中读入到content
	while ((in_bn = fnp(in_bp, content, N)) > 0)
	{
		content[in_bn] = '\0';
		printf("\nanother read, content:\n");
		printf("%s\n", content);
		printf("another read end.\n");

		// 从content写出到文件
		out_bn = BufferIO::buffer_written_n(out_bp, content, in_bn);
		printf("another write %d bytes.\n", out_bn);
	}
	//out_bn = BufferIO::buffer_written_flush(out_bp);
	//printf("another write %d bytes.\n", out_bn);

	// 关闭文件
	if (!BufferIO::buffer_close(in_bp)) {
		printf("close in file successfully.\n");
	}
	if (!BufferIO::buffer_close(out_bp)) {
		printf("close out file successfully.\n");
	}
#endif

#if 0
	// 获取进程池实例
	ProcessPool& process_pool = ProcessPool::get_instance(4);
	//printf("main instance get.\n");


	// 启动进程池
	//process_pool->run();

	ProcessExecutable obj;
	for (int i = 0; i < 10; ++i) {
		process_pool.execute(obj);
	}

	//process_pool->destory();
#endif

#if 0
	/*
	* @brief 进程池执行类，封装子进程执行的函数
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

	// 获取进程池实例
	ProcessPool<ProcessExecutable>& process_pool = ProcessPool<ProcessExecutable>::get_instance(4);
	// 启动进程池
	//process_pool->run();

	ProcessExecutable obj;
	for (int i = 0; i < 10; ++i) {
		process_pool.execute(obj);
	}

	// 销毁子进程
	//process_pool->destory();
#endif

#if 0
	class A{};
	std::shared_ptr<int> shared_null = nullptr;      // 可以将shared_ptr初始化为nullptr
	std::unique_ptr<int> unique_null = nullptr;      // 可以将unique_ptr初始化为nullptr
	//std::weak_ptr<int> weak_null = nullptr;        // 不能将weak_ptr初始化为nullptr

	std::shared_ptr<int> shared_obj = std::make_shared<int>();             // 可以用new基本类型初始化shared_ptr（推荐）
	//std::shared_ptr<int> shared_obj(new int);                            // 可以用new基本类型初始化shared_ptr
	//std::shared_ptr<int> shared_obj = std::shared_ptr<int>(new int);           // 可以用new基本类型初始化shared_ptr（和上一个写法等价）
	std::unique_ptr<int> unique_obj(new int);                              // 可以用new基本类型初始化unique_ptr
	//std::unique_ptr<int> unique_obj = std::unique_ptr<int>(new int);           // 可以用new基本类型初始化unique_ptr（和上一个写法等价）
	//std::weak_ptr<int> weak_obj(new int);                                // 不能用new基本类型初始化weak_ptr

	std::shared_ptr<A> shared_A = std::make_shared<A>();                   // 可以用对象指针初始化shared_ptr（推荐）
	//std::shared_ptr<A> shared_A(new A);                                  // 可以用对象指针初始化shared_ptr
	//std::shared_ptr<A> shared_A = std::shared_ptr<A>(new A);             // 可以用对象指针初始化shared_ptr（和上一个写法等价）
	std::unique_ptr<A> unique_A(new A);                                    // 可以用对象指针初始化unique_ptr
	//std::unique_ptr<A> unique_A = std::unique_ptr<A>(new A);             // 可以用对象指针初始化unique_ptr（和上一个写法等价）
	//std::weak_ptr<A> weak_A(new A);                                      // 不能用对象指针初始化weak_ptr

	std::shared_ptr<int> shared_p1(shared_obj);                            // 可以用shared_ptr初始化shared_ptr
	std::shared_ptr<int> shared_p2(std::move(unique_obj));                 // 可以用unique_ptr初始化shared_ptr，unique_ptr自动置空
	std::unique_ptr<int> unique_p(std::move(unique_obj));                  // 只能用unique_ptr初始化unique_ptr
	std::weak_ptr<int> weak_p1(shared_obj);                                // 可以用shared_ptr初始化weak_ptr
	//std::weak_ptr<int> weak_p2(unique_obj);                              // 不能用unique_ptr初始化weak_ptr
	std::weak_ptr<int> weak_p3(weak_p1);                                   // 可以用weak_ptr初始化weak_ptr
	std::shared_ptr<int> shared_p3(weak_p1);                               // 可以用weak_ptr初始化shared_ptr


	shared_null = shared_obj;                                              // 可以用shared_ptr赋值给shared_ptr
	shared_null = std::move(unique_obj);                                   // 可以用unique_ptr赋值给shared_ptr，unique_ptr自动置空
	//shared_null = weak_p1;                                               // 不能用weak_ptr赋值给shared_ptr
	unique_null = std::move(unique_obj);                                   // 只能用unique_ptr赋值给unique_ptr，前一个unique_ptr自动置空
	weak_p3 = shared_obj;                                                  // 可以用shared_ptr赋值给weak_ptr
	//weak_p3 = std::move(unique_obj);                                     // 不能用unique_ptr赋值给weak_ptr
	weak_p3 = weak_p1;                                                     // 可以用weak_ptr赋值给weak_ptr

	shared_null = nullptr;                            // 可以将nullptr赋值给shared_ptr
	unique_null = nullptr;                            // 可以将nullptr赋值给unique_ptr
    //weak_p1 = nullptr;                              // 不能将nullptr赋值给weak_ptr

	int num = 4;
	std::shared_ptr<int> shared_int(&num);                              // 可以用普通指针初始化shared_ptr
	std::unique_ptr<int> unique_int(&num);                              // 可以用普通指针初始化unique_ptr
	//std::weak_ptr<int> weak_int(&num);                                // 不能用普通指针初始化weak_ptr
	
	//shared_null = &num;                                               // 不能直接将普通指针赋值给shared_ptr
	//unique_null = &num;                                               // 不能直接将普通指针赋值给unique_ptr
	//weak_p1 = &num;                                                   // 不能直接将普通指针赋值给weak_ptr

	shared_null = std::make_shared<int>(num);                           // 可以将封装后的普通指针赋值给shared_ptr
	unique_null = std::unique_ptr<int>(&num);                           // 可以将封装后的普通指针赋值给unique_ptr
	//weak_p1 = std::weak_ptr<int>(&num);                               // 不能将封装后的普通指针赋值给weak_ptr
#endif

#if 0
	/*
	* @brief 进程池执行类，封装子进程执行的函数
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

	// 获取线程池实例
	ThreadPool& thread_pool = ThreadPool::get_instance(4);

	ThreadExecutable obj;
	for (int i = 0; i < 10; ++i) {
		thread_pool.execute(&obj);
	}

	// 由于任务只传指针，资源都在主线程内而不是整个进程生命
	// 因此主线程需要等待任务队列完成再终止
	thread_pool.wait();

	printf("main thread end.\n");
#endif



	return 0;
}