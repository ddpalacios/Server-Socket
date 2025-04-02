
#include "Clients.h"
#include "RequestHandler.h"

socklen_t addrlen;
struct sockaddr_storage remoteaddr;
int listener_socket;

void del_from_pfds(struct Client **clients, struct pollfd **pfds, int i, int *fd_count){
	printf("Removing FD %d\n", (*pfds)[i].fd);
	(*pfds)[i] = (*pfds)[*fd_count-1];
	(*clients)[i] = (*clients)[*fd_count-1];
	(*fd_count)--;

}

void add_pfd(struct Client **clients, struct pollfd **pfds, int newfd, int *fd_count, int *fd_size){
	printf("Adding New Client: %d\n", newfd);
	if (*fd_count == *fd_size){
		*fd_size *=2;
		*pfds  = realloc(*pfds, sizeof(**pfds) * (*fd_size));
		*clients  = realloc(*clients, sizeof(**clients) * (*fd_size));
	}
	(*pfds)[*fd_count].fd = newfd;
	(*pfds)[*fd_count].events = POLLIN;
	(*clients)[*fd_count].pfd = (*pfds)[*fd_count];
}

int create_listener_socket(struct pollfd **pfds,int PORT){
	int server_socket;
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	int yes = 1;
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));
	listen(server_socket,5);
	return server_socket;
}

int accept_client(){
		addrlen = sizeof(remoteaddr);
		int newfd = accept(listener_socket, (struct sockaddr *)&remoteaddr, &addrlen);
		if (newfd == -1){
			perror("accept");
			return newfd;
			
		}

		return newfd;
}

void encrypt_socket(int newfd, struct Client **clients, struct pollfd **pfds, int *fd_count){
		SSL *cSSL = SSL_new(sslctx);
		int ssl_set = SSL_set_fd(cSSL, newfd );
		int ssl_err = SSL_accept(cSSL);
		if (ssl_err <= 0) {
		    printf("ERROR\n");
		    printf("SSL ERROR: %d\n", ssl_err);
		    int err = SSL_get_error(cSSL, ssl_err);
		    printf("SSL ERROR: %d\n", err);
		    ERR_print_errors_fp(stderr);  
		    ShutdownSSL(cSSL);
		    close(newfd);
		}
		(*clients)[*fd_count].pfd = (*pfds)[*fd_count];
		(*clients)[*fd_count].ssl = cSSL;
}

