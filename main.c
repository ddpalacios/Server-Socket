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
	printf("https://127.0.0.1:%d\n",PORT );
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
				int is_get_request = strncmp(buf, "GET ",4);
				if (is_get_request == 0){
					printf("Processing GET Request\n");
					process_get(buf,cSSL);
				}
				if (is_dataframe(buf)){
					char message[3024];
					int payload_length = decode_dataframe(buf,message); 
					printf("\nMessage Recieved: %s\n", message);
					send_dataframe_to_clients(payload_length, message);

				}
			}else{
				nbytes = recv(readyfd, buf, BUFFER_SIZE, 0);	
				printf("Data Recieved from %d:\nTotal Bytes: %d\n", readyfd, nbytes);
			}

			if(nbytes == 0){
				del_from_pfds(readyfd);
			}
			buf[0] = '\0';
		}
	
	}
}
	
