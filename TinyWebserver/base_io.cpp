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
				// ���ж������-1��������ִ��read()
				n_read = 0;
			}
			else {
				// �����������-1��ֱ�ӷ���
				core::unix_error("Read error");
				return -1;
			}
		}
		else {
			if (n_read == 0) {
				// ����EOF
				break;
			}
		}

		n_left -= n_read;
		usrbufp += n_read;
	}

	// ����δ�ش�fd�ж�n�ֽڵ�usrbuf��
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
				// ���ж������-1��������ִ��read()
				n_written = 0;
			}
			else {
				// �����������-1��ֱ�ӷ���
				core::unix_error("Write error");
				return -1;
			}
		}

		n_left -= n_written;
		usrbufp += n_written;
	}

	// д��usrbuf�ض�Ҫдn�ֽڵ�fd��
	return n;
}
