
#include "nty_coroutine.h"

#include <arpa/inet.h>

#define MAX_CLIENT_NUM			1000000

void server_reader(void *arg) {
	int fd = *(int *)arg;
	int ret = 0;

	struct pollfd fds;
	fds.fd = fd;
	fds.events = POLLIN;

	//nty_coroutine_detach();

	while (1) {
		
		char buf[100] = {0};
		ret = nty_recv(fd, buf, 100);
		if (ret > 0) {
			if(fd > MAX_CLIENT_NUM) 
			printf("read from server: %.*s\n", ret, buf);

			//proc
			
			ret = nty_send(fd, buf, strlen(buf));
			if (ret == -1) {
				nty_close(fd);
				break;
			}
		} else if (ret == 0) {	
			nty_close(fd);
			break;
		}

		
	}
}


void server(void *arg) {

	unsigned short port = *(unsigned short *)arg;
	free(arg);

	int fd = nty_socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) return ;

	struct sockaddr_in local, remote;
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = INADDR_ANY;
	bind(fd, (struct sockaddr*)&local, sizeof(struct sockaddr_in));

	listen(fd, 5);
	printf("listen port : %d\n", port);

	while (1) {
		socklen_t len = sizeof(struct sockaddr_in);
		int cli_fd = nty_accept(fd, (struct sockaddr*)&remote, &len);
		if (cli_fd % 1000 == 999)
			printf("client fd : %d\n", cli_fd);

		nty_coroutine *read_co;
		nty_coroutine_create(&read_co, server_reader, &cli_fd);

	}
}

int main(int argc, char *argv[]) {
	nty_coroutine *co = NULL;

	int i = 0;
	unsigned short base_port = 8080;
	for (i = 0;i < 100;i ++) {
		unsigned short *port = calloc(1, sizeof(unsigned short));
		*port = base_port + i;
		nty_coroutine_create(&co, server, port);
	}

	nty_schedule_run();

	return 0;
}


