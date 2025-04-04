#include "socket_handler.h"
#include "WebSocket.h"
#define PORT 9034
#define BUFFER_SIZE 2056


int main(){
	InitializeSSL();
	int fd_size = 50;
	int fd_count = 0;
	unsigned char *buf =  malloc(BUFFER_SIZE);
	struct pollfd *pfds = malloc(sizeof(struct pollfd) * fd_size);
	struct Client *clients = malloc(sizeof(struct Client) * fd_size);
	listener_socket = create_listener_socket(&pfds, PORT);
	add_pfd(&clients, &pfds, listener_socket,&fd_count, &fd_size);
	fd_count++;
	printf("SSL Options: %lx\n", SSL_CTX_get_options(sslctx));
	printf("https://127.0.0.1:%d\n",PORT );

	while (1){
		printf("\nlistening...\n");
		int activity = poll(pfds, fd_count, -1);
		if (activity < 0){
			perror("poll");
		}
		if (pfds[0].fd == listener_socket){
			if (pfds[0].revents & POLLIN){
				int newfd = accept_client();
				encrypt_socket(newfd, &clients, &pfds, &fd_count);
				add_pfd(&clients, &pfds, newfd, &fd_count, &fd_size);
				fd_count++;
			}
		}

		for (int i=0; i<fd_count; i++){
			//printf("Checking fd %d, revents: %d\n", pfds[i].fd, pfds[i].revents);
			if (pfds[i].revents & POLLIN){
				if (pfds[i].fd == listener_socket){
					continue;	
				}
				SSL *cSSL = clients[i].ssl;
				int nbytes = SSL_read(cSSL, buf, BUFFER_SIZE);
				if (nbytes <=0){
					close(pfds[i].fd);
					del_from_pfds(&clients,&pfds, i, &fd_count);
				}
				buf[nbytes] = '\0';
				printf("\n\nMessage Recieved From CLIENT %d...\n\n", clients[i].pfd.fd);
				int is_get_request = strncmp(buf, "GET ",4);
				if (is_get_request==0){
					process_get(buf, clients[i].pfd.fd, cSSL);
				
					}
				
				}
			}
		
	}
		return 0;
}
		/*
		if (pfds[0].fd == listener_socket){
			if (pfds[0].revents & POLLIN){
				int newfd = accept_client();
				add_pfd(&clients, &pfds, newfd, &fd_count, &fd_size);
				encrypt_socket(newfd, &clients, &pfds, &fd_count);
				fd_count++;
			}
		}
		
		for (int i=0; i<fd_count; i++){
			//printf("Checking fd %d, revents: %d\n", pfds[i].fd, pfds[i].revents);
			if (pfds[i].revents & POLLIN){
				if (pfds[i].fd == listener_socket){
					continue;	
				}
				SSL *cSSL = clients[i].ssl;
				int nbytes = SSL_read(cSSL, buf, BUFFER_SIZE);
				if (nbytes <=0){
					close(pfds[i].fd);
					del_from_pfds(&clients,&pfds, i, &fd_count);
				}
				buf[nbytes] = '\0';
				printf("\n\nMessage Recieved From CLIENT %d...\n\n", clients[i].pfd.fd);
				int is_get_request = strncmp(buf, "GET ",4);
				if (is_get_request==0){
					process_get(buf, clients[i].pfd.fd, cSSL);
				
					}
				
				}
			}
		
		if (is_dataframe(buf)){
	//		printf("\nData Frame Message Recieved\n");
			char message[3024];
			int payload_length = decode_dataframe(buf,message); 
			printf("\nMessage Recieved: %s\n", message);
			unsigned char frame[4+payload_length];
			convert_to_dataframe(message, frame, payload_length);
			send_dataframe_to_clients(frame,payload_length, &pfds,fd_count,listener_socket, &clients);

			send_dataframe_to_clients(frame,payload_length, &pfds,fd_count,listener_socket, &clients);

			decode_dataframe(buf,message); 
			unsigned char frame[1024];
			convert_to_dataframe(message, frame);
			send_dataframe_to_clients(frame,strlen(message), &pfds,fd_count,listener_socket, &clients);
			//printf("Message Recieved: %s\n", message);
		}
			
	}
	*/
	
