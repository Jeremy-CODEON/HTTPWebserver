#include "base_io.h"

ssize_t BaseIO::read_n(int fd, void* usrbuf, size_t n)
{
	size_t n_left = n;
	ssize_t n_read = 0;
	char *usrbufp = static_cast<char *>(usrbuf);

	while (n_left > 0) {
		n_read = read(fd, usrbufp, n_left);
		if (n_read < 0) {
			if (errno == EINTR) {
				// 由中断引起的-1，则重新执行read()
				n_read = 0;
			}
			else {
				// 由其他引起的-1，直接返回
				core::unix_error("Read error");
				return -1;
			}
		}
		else {
			if (n_read == 0) {
				// 读到EOF
				break;
			}
		}

		n_left -= n_read;
		usrbufp += n_read;
	}

	// 读则未必从fd中读n字节到usrbuf中
	return (n - n_left);
}

ssize_t BaseIO::written_n(int fd, void* usrbuf, size_t n)
{
	size_t n_left = n;
	ssize_t n_written = 0;
	char* usrbufp = static_cast<char*>(usrbuf);

	while (n_left > 0) {
	n_written = write(fd, usrbufp, n_left);
		if (n_written == -1) {
			if (errno == EINTR) {
				// 由中断引起的-1，则重新执行read()
				n_written = 0;
			}
			else {
				// 由其他引起的-1，直接返回
				core::unix_error("Write error");
				return -1;
			}
		}

		n_left -= n_written;
		usrbufp += n_written;
	}

	// 写则usrbuf必定要写n字节到fd中
	return n;
}
