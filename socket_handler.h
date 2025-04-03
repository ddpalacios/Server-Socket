
#include "Clients.h"
#include "RequestHandler.h"

socklen_t addrlen;
struct sockaddr_storage remoteaddr;
int listener_socket;

void del_from_pfds(int fd){
	for(int i=0;i<fd_count;i++){
		if (pfds[i].fd == fd){
			printf("Removing FD %d\n", fd);
			pfds[i] = pfds[fd_count-1];
			clients[i] = clients[fd_count-1];
			break;
		}
	
	}
	fd_count--;
	close(fd);
}

void add_pfd(int newfd){
	if (fd_count == fd_size){
		fd_size *=2;
		clients  = realloc(clients, sizeof(*clients) * (fd_size));
		pfds  = realloc(pfds, sizeof(*pfds) * (fd_size));
		clients  = realloc(clients, sizeof(*clients) * (fd_size));
	}
	pfds[fd_count].fd = newfd;
	pfds[fd_count].events = POLLIN;
	clients[fd_count].pfd = pfds[fd_count];
	fd_count++;
	printf("File Descriptor Added!\nFD: %d\nTotal FDs Added: %d\n",clients[fd_count-1].pfd.fd ,fd_count);

	/*
	if (*fd_count == *fd_size){
		*fd_size *=2;
		*pfds  = realloc(*pfds, sizeof(**pfds) * (*fd_size));
		*clients  = realloc(*clients, sizeof(**clients) * (*fd_size));
	}
	(*pfds)[*fd_count].fd = newfd;
	(*pfds)[*fd_count].events = POLLIN;
	(*clients)[*fd_count].pfd = (*pfds)[*fd_count];
	if (cSSL != NULL){
	(*clients)[*fd_count].ssl = cSSL;
		printf("SSL Added\n");
		printf("cSSL pointer: %p\n", (void *)cSSL);
	}
	*/
}

void create_listener_socket(int PORT){
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
	server_address.sin_addr.s_addr = INADDR_ANY;
	listener_socket = socket(AF_INET, SOCK_STREAM, 0);
	int yes = 1;
	setsockopt(listener_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	bind(listener_socket, (struct sockaddr*)&server_address, sizeof(server_address));
	listen(listener_socket,5);
	add_pfd(listener_socket);
}

int accept_client(){
	addrlen = sizeof(remoteaddr);
	int newfd = accept(listener_socket, (struct sockaddr *)&remoteaddr, &addrlen);
	if (newfd == -1){
		perror("accept");
		return newfd;
	}
	add_pfd(newfd);
	return newfd;
}

void encrypt_client(int fd, SSL *cSSL){
	for(int i=0; i<fd_count; i++){
		if (clients[i].pfd.fd == fd){
			printf("Found Client\n");
			clients[i].ssl = cSSL;
			break;
		}
	
	}


}
int  encrypt_socket(int fd){
	printf("Starting to encryot...\n");
		SSL *cSSL = SSL_new(sslctx);
		int ssl_set = SSL_set_fd(cSSL, fd);
		int ssl_err = SSL_accept(cSSL);
		if (ssl_err <= 0) {
		    printf("ERROR\n");
		    printf("SSL ERROR: %d\n", ssl_err);
		    int err = SSL_get_error(cSSL, ssl_err);
		    printf("SSL ERROR: %d\n", err);
		    ERR_print_errors_fp(stderr);  
		    ShutdownSSL(cSSL);
		    return ssl_err;
		}else{
			encrypt_client(fd, cSSL);
			printf("Client %d Has been encrypted ssl_set: %d\n", fd,ssl_set);
			return ssl_err; 
		
		}
}

