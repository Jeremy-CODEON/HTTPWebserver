#include "buffer_io.h"

Buffer::Buffer(int _fd, int _flag) : fd(_fd), rw_flag(_flag), buffer_ptr(nullptr), buffer_area(nullptr) 
{
	assert((rw_flag == 1) || (rw_flag == 2));

	n_left = 0;
	buffer_area = new char[core::IO_BUFFERSIZE];
	buffer_ptr = buffer_area;

#ifdef DEBUG
	printf("Buffer create.\n");
#endif // DEBUG
}

Buffer::Buffer(const Buffer& _buff)
{
	fd = _buff.fd;
	rw_flag = _buff.rw_flag;
	n_left = _buff.n_left;
	// 申请缓冲区
	buffer_area = new char[core::IO_BUFFERSIZE];
	memset(buffer_area, '\0', core::IO_BUFFERSIZE);  /*缓冲区初始化*/
	// 拷贝
	for (int i = 0; i < core::IO_BUFFERSIZE; ++i) {
		*(buffer_area + i) = *(_buff.buffer_area + i);
	}
	// 移动读写指针
	buffer_ptr = buffer_area + (_buff.buffer_ptr - _buff.buffer_area);
}

Buffer::~Buffer()
{
	// 写文件先刷新缓冲区
	if (rw_flag == 2) {
		BufferIO::buffer_written_flush(this);
	}

	if (buffer_area != nullptr) {
		delete[]buffer_area;
	}

#ifdef DEBUG
	printf("Buffer delete.\n");
#endif // DEBUG
}

buffer_t* BufferIO::buffer_open(const char* filename, int flag, mode_t mode)
{
	buffer_t* bp = nullptr;
	int fd = open(filename, flag, mode);	
	if (fd == -1) {
		core::unix_error("Open error");
	}
	else {		
		// 用位运算检验是否为只读或只写打开
		if ((flag & O_WRONLY) == O_WRONLY) {
			// 只写文件
			bp = new buffer_t(fd, 2);
		}
		else if ((flag & O_RDONLY) == O_RDONLY) {
			// 只读文件，O_RDONLY = 00必须最后判断，因为条件一定成立
			bp = new buffer_t(fd, 1);
		}
		else {
			core::unix_error("Buffer open error");
		}
	}
	return bp;
}

int BufferIO::buffer_close(buffer_t* bp)
{
	// 写文件先刷新缓冲区
	if (bp->rw_flag == 2) {
		buffer_written_flush(bp);
	}

	// 关闭文件
	if (close(bp->fd) == 0) {
		delete bp;
		bp = nullptr;
	}
	else {
		core::unix_error("Buffer close error");
		return -1;
	}
	return 0;
}

ssize_t BufferIO::buffer_read(buffer_t* bp, void* usrbuf, size_t n)
{
	assert(bp != nullptr);
	assert(bp->rw_flag == 1);
	
	while (bp->n_left <= 0) {
		//memset(bp->buffer_area, '\0', core::IO_BUFFERSIZE);  /*缓冲区初始化*/

		// 从文件中读取一块至多为缓冲区大小的字节到缓冲区中，而不是仅读取字节n
		bp->n_left = read(bp->fd, bp->buffer_area, core::IO_BUFFERSIZE);
		if (bp->n_left == -1) {
			if (errno == EINTR) {
				// 由中断引起的-1，则重新执行read()
				bp->n_left = 0;
			}
			else {
				// 由其他引起的-1，直接返回
				core::unix_error("Buffer read error");
				return -1;
			}
		}
		else if (bp->n_left == 0) {
			// 读到EOF
			return 0;
		}
		else {
			// 将读写指针重置到缓冲区开头
			bp->buffer_ptr = bp->buffer_area;
		}
	}

#ifdef DEBUG
	//printf("bp_left, n: %d, %d\n", bp->n_left, n);
#endif // DEBUG

	int cnt = n;  /*实际要读取的字节数*/
	if (bp->n_left < n) {
		cnt = bp->n_left;
	}
	// 直接从缓冲区中往虚拟内存复制内容，而不调用read()
	memcpy(usrbuf, bp->buffer_ptr, cnt);
	// 移动缓冲区指针
	bp->buffer_ptr += cnt;
	bp->n_left -= cnt;

	return cnt;
}

ssize_t BufferIO::buffer_read_n(buffer_t* bp, void* usrbuf, size_t n)
{
	size_t n_left = n;
	ssize_t n_read = 0;
	char* usrbufp = static_cast<char*>(usrbuf);

	// 用循环可以处理缓冲区读空的情况
	while (n_left > 0) {
#ifdef DEBUG
		printf("n_read, n_left: %d, %d\n", n_read, n_left);
#endif // DEBUG
		if ((n_read = buffer_read(bp, usrbufp, n_left)) == -1) {
			// 由其他引起的-1，直接返回
			return -1;
		}
		else if (n_read == 0) {
			// 读到EOF
			break;
		}
		else {
			// 移动内存指针
			n_left -= n_read;
			usrbufp += n_read;
		}
	}

	return (n - n_left);
}

ssize_t BufferIO::buffer_read_line(buffer_t* bp, void* usrbuf, size_t n)
{
	int n_left = n;
	int n_read = 0;
	char* usrbufp = static_cast<char*>(usrbuf);

	while (n_left > 0) {
		if ((n_read = buffer_read(bp, usrbufp, 1)) == -1) {
			// 由其他引起的-1，直接返回
			return -1;
		}
		else if (n_read == 0) {
			// 读到EOF
			break;
		}
		else {	
			--n_left;
			++usrbufp;
			if (*(usrbufp - 1) == '\n') {
				// 读到换行，返回的字节数含换行符
				break;
			}
		}
	}

	return (n - n_left);
}

ssize_t BufferIO::buffer_written(buffer_t* bp, void* usrbuf, size_t n)
{
	assert(bp != nullptr);
	assert(bp->rw_flag == 2);		

	int cnt = n;  /*实际要写入的字节数*/
	if (cnt > core::IO_BUFFERSIZE - bp->n_left) {
		cnt = core::IO_BUFFERSIZE - bp->n_left;
	}
	// 直接从虚拟内存往缓冲区中复制内容，而不调用write()
	memcpy(bp->buffer_ptr, usrbuf, cnt);
	// 移动缓冲区指针
	bp->buffer_ptr += cnt;
	bp->n_left += cnt;

	while (bp->n_left >= core::IO_BUFFERSIZE) {
		//memset(bp->buffer_area, '\0', core::IO_BUFFERSIZE);  /*缓冲区初始化*/

		// 将缓冲区大小的字节直接写到文件中，而不是仅写字节n
		int n_written = write(bp->fd, bp->buffer_area, core::IO_BUFFERSIZE);
		if (n_written == -1) {
			if (errno == EINTR) {
				// 由中断引起的-1，则重新执行write()
				n_written = 0;
			}
			else {
				// 由其他引起的-1，直接返回
				core::unix_error("Buffer write error");
				return -1;
			}
		}
		else {
			// 将读写指针重置到缓冲区开头
			bp->n_left = 0;
			bp->buffer_ptr = bp->buffer_area;
		}
	}

	return cnt;
}

ssize_t BufferIO::buffer_written_n(buffer_t* bp, void* usrbuf, size_t n)
{
	size_t n_left = n;
	ssize_t n_written = 0;
	char* usrbufp = static_cast<char*>(usrbuf);

	while (n_left > 0) {
		if ((n_written = buffer_written(bp, usrbufp, n_left)) == -1) {
			// 由其他引起的-1，直接返回
			return -1;
		}
		else {
			// 移动内存指针
			n_left -= n_written;
			usrbufp += n_written;
		}
	}

	return (n - n_left);
}

ssize_t BufferIO::buffer_written_flush(buffer_t* bp)
{
	int n_written = 0;

	while (bp->n_left > 0) {
		if ((n_written = write(bp->fd, bp->buffer_area, bp->n_left)) == -1) {
			if (errno == EINTR) {
				// 由中断引起的-1，则重新执行write()
				n_written = 0;
			}
			else {
				// 由其他引起的-1，直接返回
				core::unix_error("Buffer flush error");
				return -1;
			}
		}
		else {
			// 移动内存指针
			bp->n_left = 0;
			bp->buffer_ptr = bp->buffer_area;
		}
	}
	return n_written;
}


