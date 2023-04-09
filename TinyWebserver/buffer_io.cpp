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
	// ���뻺����
	buffer_area = new char[core::IO_BUFFERSIZE];
	memset(buffer_area, '\0', core::IO_BUFFERSIZE);  /*��������ʼ��*/
	// ����
	for (int i = 0; i < core::IO_BUFFERSIZE; ++i) {
		*(buffer_area + i) = *(_buff.buffer_area + i);
	}
	// �ƶ���дָ��
	buffer_ptr = buffer_area + (_buff.buffer_ptr - _buff.buffer_area);
}

Buffer::~Buffer()
{
	// д�ļ���ˢ�»�����
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
		// ��λ��������Ƿ�Ϊֻ����ֻд��
		if ((flag & O_WRONLY) == O_WRONLY) {
			// ֻд�ļ�
			bp = new buffer_t(fd, 2);
		}
		else if ((flag & O_RDONLY) == O_RDONLY) {
			// ֻ���ļ���O_RDONLY = 00��������жϣ���Ϊ����һ������
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
	// д�ļ���ˢ�»�����
	if (bp->rw_flag == 2) {
		buffer_written_flush(bp);
	}

	// �ر��ļ�
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
		//memset(bp->buffer_area, '\0', core::IO_BUFFERSIZE);  /*��������ʼ��*/

		// ���ļ��ж�ȡһ������Ϊ��������С���ֽڵ��������У������ǽ���ȡ�ֽ�n
		bp->n_left = read(bp->fd, bp->buffer_area, core::IO_BUFFERSIZE);
		if (bp->n_left == -1) {
			if (errno == EINTR) {
				// ���ж������-1��������ִ��read()
				bp->n_left = 0;
			}
			else {
				// �����������-1��ֱ�ӷ���
				core::unix_error("Buffer read error");
				return -1;
			}
		}
		else if (bp->n_left == 0) {
			// ����EOF
			return 0;
		}
		else {
			// ����дָ�����õ���������ͷ
			bp->buffer_ptr = bp->buffer_area;
		}
	}

#ifdef DEBUG
	//printf("bp_left, n: %d, %d\n", bp->n_left, n);
#endif // DEBUG

	int cnt = n;  /*ʵ��Ҫ��ȡ���ֽ���*/
	if (bp->n_left < n) {
		cnt = bp->n_left;
	}
	// ֱ�Ӵӻ��������������ڴ渴�����ݣ���������read()
	memcpy(usrbuf, bp->buffer_ptr, cnt);
	// �ƶ�������ָ��
	bp->buffer_ptr += cnt;
	bp->n_left -= cnt;

	return cnt;
}

ssize_t BufferIO::buffer_read_n(buffer_t* bp, void* usrbuf, size_t n)
{
	size_t n_left = n;
	ssize_t n_read = 0;
	char* usrbufp = static_cast<char*>(usrbuf);

	// ��ѭ�����Դ����������յ����
	while (n_left > 0) {
#ifdef DEBUG
		printf("n_read, n_left: %d, %d\n", n_read, n_left);
#endif // DEBUG
		if ((n_read = buffer_read(bp, usrbufp, n_left)) == -1) {
			// �����������-1��ֱ�ӷ���
			return -1;
		}
		else if (n_read == 0) {
			// ����EOF
			break;
		}
		else {
			// �ƶ��ڴ�ָ��
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
			// �����������-1��ֱ�ӷ���
			return -1;
		}
		else if (n_read == 0) {
			// ����EOF
			break;
		}
		else {	
			--n_left;
			++usrbufp;
			if (*(usrbufp - 1) == '\n') {
				// �������У����ص��ֽ��������з�
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

	int cnt = n;  /*ʵ��Ҫд����ֽ���*/
	if (cnt > core::IO_BUFFERSIZE - bp->n_left) {
		cnt = core::IO_BUFFERSIZE - bp->n_left;
	}
	// ֱ�Ӵ������ڴ����������и������ݣ���������write()
	memcpy(bp->buffer_ptr, usrbuf, cnt);
	// �ƶ�������ָ��
	bp->buffer_ptr += cnt;
	bp->n_left += cnt;

	while (bp->n_left >= core::IO_BUFFERSIZE) {
		//memset(bp->buffer_area, '\0', core::IO_BUFFERSIZE);  /*��������ʼ��*/

		// ����������С���ֽ�ֱ��д���ļ��У������ǽ�д�ֽ�n
		int n_written = write(bp->fd, bp->buffer_area, core::IO_BUFFERSIZE);
		if (n_written == -1) {
			if (errno == EINTR) {
				// ���ж������-1��������ִ��write()
				n_written = 0;
			}
			else {
				// �����������-1��ֱ�ӷ���
				core::unix_error("Buffer write error");
				return -1;
			}
		}
		else {
			// ����дָ�����õ���������ͷ
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
			// �����������-1��ֱ�ӷ���
			return -1;
		}
		else {
			// �ƶ��ڴ�ָ��
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
				// ���ж������-1��������ִ��write()
				n_written = 0;
			}
			else {
				// �����������-1��ֱ�ӷ���
				core::unix_error("Buffer flush error");
				return -1;
			}
		}
		else {
			// �ƶ��ڴ�ָ��
			bp->n_left = 0;
			bp->buffer_ptr = bp->buffer_area;
		}
	}
	return n_written;
}


