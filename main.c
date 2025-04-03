#include "socket_handler.h"
#include "WebSocket.h"
#define PORT 9034
#define BUFFER_SIZE 2056



SSL *get_encrypted_socket(int fd){

	for(int i=0; i<fd_count; i++){
		if (clients[i].pfd.fd == fd){
			return clients[i].ssl;  
		
		}
	}

}


int get_ready_file_descriptor(){
	for(int i=0; i<fd_count; i++){
		if (pfds[i].revents & POLLIN){
			return pfds[i].fd;
		}
	}
}


int main(){
	int isEncrypted = 1;
	if (isEncrypted){
		InitializeSSL();
	
	}

	unsigned char *buf =  malloc(BUFFER_SIZE);
	clients = malloc(sizeof(struct Client) * fd_size);
	pfds = malloc(sizeof(struct pollfd) * fd_size);
	create_listener_socket(PORT);

	while(1){
		printf("Current File Descriptors:\n");
		for(int i=0; i<fd_count; i++){
			printf("FD %d Action BIT: %d\n", pfds[i].fd, pfds[i].revents);
		}
		
		printf("listening for an action...\n");

		if (poll(pfds, fd_count, -1) < 0){
			perror("poll");
		}
		int readyfd = get_ready_file_descriptor();
		
		if (readyfd == listener_socket){
			int newfd = accept_client();
			if (isEncrypted){
				int success = encrypt_socket(newfd);
				if (success <=0 ){
					del_from_pfds(newfd);
				}
			}
		}else{
			int nbytes;
			if (isEncrypted){
				SSL *cSSL = get_encrypted_socket(readyfd);
				nbytes = SSL_read(cSSL, buf, BUFFER_SIZE);
			}else{
				nbytes = recv(readyfd, buf, BUFFER_SIZE, 0);	
			}
			printf("Data Recieved from %d:\nTotal Bytes: %d\n", readyfd, nbytes);

			if(nbytes == 0){
				del_from_pfds(readyfd);
			}else if (nbytes > 0){
				printf("\nFD %d > %s\n",readyfd, buf);
			}
		}
	
	}

}

	/*
	int fd_size = 50;
	int fd_count = 0;
	unsigned char *buf =  malloc(BUFFER_SIZE);
	//struct pollfd *pfds = malloc(sizeof(struct pollfd) * fd_size);
	struct Client *clients = malloc(sizeof(struct Client) * fd_size);
	listener_socket = create_listener_socket(&pfds, PORT);
	add_pfd(&clients, &pfds, listener_socket,NULL,&fd_count, &fd_size);
	fd_count++;
	printf("https://127.0.0.1:%d\n",PORT );

	while (1){
		printf("Waiting for an Action...\n");
		int activity = poll(pfds, fd_count, -1);
		if (activity < 0){
			perror("poll");
		}

		for (int i=0; i<fd_count; i++){
			printf("FD: %d POLLIN %d | Revents %d\n",pfds[i].fd, POLLIN ,pfds[i].revents);
			if (pfds[i].revents & POLLIN){
				if (pfds[i].fd == listener_socket){
					int newfd = accept_client();
					add_pfd(&clients, &pfds, newfd,NULL, &fd_count, &fd_size);
					fd_count++;
				}					
			
			}
		
		}
		

		
	}
		return 0;
}
		printf("POLL PASSED. Total FDs = %d\n", fd_count);
		for (int i=0; i<fd_count; i++){
			if (pfds[i].revents & POLLIN){
				printf("POLLIN Request from FD: %d\n", pfds[i].fd);
				if (pfds[i].fd == listener_socket){
					int newfd = accept_client();
					printf("new FD: %d\n", newfd);
					SSL *cSSL = encrypt_socket(newfd, &clients, &pfds, &fd_count);
					if (cSSL != NULL){
						add_pfd(&clients, &pfds, newfd,cSSL, &fd_count, &fd_size);
						fd_count++;
					}
				}
				else{
					SSL *cSSL = clients[i].ssl;
					int nbytes = SSL_read(cSSL, buf, BUFFER_SIZE);
					printf("NBYTES Recieved: %d\n", nbytes);
					buf[nbytes] = '\0';
					printf("Message Recieved From CLIENT %d\n\nMessage:\n %s", clients[i].pfd.fd,buf);
					if (nbytes <0){
						close(pfds[i].fd);
						del_from_pfds(&clients,&pfds, i, &fd_count);
					}
					int is_get_request = strncmp(buf, "GET ",4);
					if (is_get_request==0){
						process_get(buf, clients[i].pfd.fd, cSSL);
						}
				}
			}
		}
		*/
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
	
