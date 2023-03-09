#include "base_io.h"
#include "buffer_io.h"
#include "process_pool.h"

#if 1
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
	virtual int process() {
		print_content();
	}
};
#endif

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

	// ��ȡ���̳�ʵ��
	ProcessPool<ProcessExecutable>* process_pool = ProcessPool<ProcessExecutable>::get_instance(4);
	// �������̳�
	//process_pool->run();

	ProcessExecutable obj;
	for (int i = 0; i < 10; ++i) {
		process_pool->execute(obj);
	}

	process_pool->destory();
	
	return 0;
}