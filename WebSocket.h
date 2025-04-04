#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/socket.h>
#include <stdint.h>  // Needed for uint64_t

int is_dataframe(unsigned char *buf){
	int finVal = buf[0] & 0x80;
	int opcode = buf[0] & 0x0F;
	int mask = buf[1] & 0x80;
	int payloadlength = buf[1] & 0x7F;
	if (opcode == 0 && finVal == 0 && mask==0){
		return 0;
	}
	if (opcode == 0 && finVal == 0 && mask == 128){
		return 1;
	}
	 if (opcode >= 1 && mask==128){
	 	return 1;
	 }else{
		 return 0;
	 } 
}

void send_dataframe_to_clients(int payload_length,  char *message) {
	printf("%s\n", message);
	unsigned char frame[2 + payload_length];
	frame[0] = 0x81;
	frame[1] =(unsigned char) payload_length;

	int finVal = frame[0] & 0x80;
	int mask = frame[1] & 0x80;
	int opcode = frame[0] & 0x0F;
	printf("2nd Byte: %d\n", frame[1]);
	printf("mask: %d\n", mask);
	printf("opcode: %d\n", opcode);
	printf("finVal: %d\n", finVal);

	memcpy(frame + 2, message, payload_length);

    for (int i = 0; i < fd_count; i++) {
        if (clients[i].pfd.fd == listener_socket) {
            continue;
        }
	SSL *encrypted_socket = clients[i].ssl;
	 int bytes_sent = SSL_write(encrypted_socket,frame, payload_length +2);
        if (bytes_sent < 0) {
            perror("Error sending data to client");
        }else{
		printf("Data Sent to Client %d! bytes sent: %d\n", clients[i].pfd.fd, bytes_sent);
	}

    }

}
	/*

	int finVal = frame[0] & 0x80;
	int opcode = frame[0] & 0x0F;
	int mask = frame[1] & 0x80;
	payload_length = frame[1] & 0x7F;
	
	printf("SENDING DATAFRAME:\n");
	printf("finVal %d, opcode: %d, mask: %d\n",finVal,opcode,mask);
	printf("payloadLength: %d\n",payload_length);




    for (int i = 0; i < fd_count; i++) {
        if ((*clients)[i].pfd.fd == listener_socket) {
            continue;
        }
        //int bytes_sent = send((*clients)[i].fd, frame, message_length + 2, 0);
	SSL *cSSL = (*clients)[i].ssl;
	 for (int i=0;i<message_length+4;i++){
	 	printf("Frame %d: %d\n", i,frame[i]);
	 }
	 int bytes_sent = SSL_write(cSSL,frame, message_length +4);
        if (bytes_sent < 0) {
            perror("Error sending data to client");
        }
    }
    */

void convert_to_dataframe(char message[],unsigned char frame[], int payload_length){
    printf("PayLoad Length to send: %d\n", payload_length);
    if (payload_length > 125 && payload_length <= 0xFFFF) {  
        frame[0] = 0x82;  
        frame[1] = 0x7E;  
        frame[2] = (payload_length >> 8) & 0xFF;  
	printf("Shifted: %d\n",(payload_length >> 8) & 0xFF); 
        frame[3] = payload_length & 0xFF;         
	printf("next bytes: %d\n",payload_length  & 0xFF); 
        memcpy(&frame[4], message, payload_length);
    }

	/*
	frame[2 + strlen(servermessage)];
	frame[0] = 0x81;
	frame[1] = strlen(servermessage);
	for (int i = 0; i < strlen(servermessage); i++) {
	    frame[2 + i] = servermessage[i];
	}
	*/

}


int  decode_dataframe(unsigned char* buf, char message[]) {
    // Byte 1
    int finVal = buf[0] & 0x80; // Bit 0
    int opcode = buf[0] & 0x0F; // Bits 4-7

    // Bytes 2 - 10 Payload length 
    int mask = buf[1] & 0x80; // Bit 8 Must expect this to be 1

    printf("finVal %d, opcode: %d, mask: %d\n",finVal,opcode,mask);

    if (mask){
	    int payloadlength = buf[1] & 0x7F;
	    printf("Initial Payload Length int: %d\n",buf[1]);
	    printf("Initial Payload Length : %d\n",254 & 0x7F);
	    unsigned char maskingKey[4];
	    if (payloadlength < 126){
		    int offset = 2;
		    for (int i =0; i<4; i++){
		    	maskingKey[i] = buf[2+i];
			offset = 2 +i;
		    }
		    ++offset;
		    unsigned char payload[payloadlength+1];
		    for (int i =0; i<payloadlength; i++){
			    payload[i] = buf[offset+i];
		    }
		    for (int i=0; i<payloadlength; i++){
		    	int message_val = payload[i] ^ maskingKey[i%4];
			message[i] = message_val;
		    }
		    message[payloadlength] = '\0';
		    return payloadlength;
	    }

	    if (payloadlength == 126){
		    unsigned int p1 = buf[2] & 0xFF;
		    unsigned int p2 = buf[3] & 0xFF;
		    payloadlength = (p1 << 8) | p2;
		    printf("Payload Length Extracted: %d\n",payloadlength);
		    int offset = 4;
		    for (int i =0; i<4; i++){
		    	maskingKey[i] = buf[4+i];
			offset = 4 +i;
		    }
		    ++offset;
		    unsigned char payload[payloadlength+1];
		    for (int i =0; i<payloadlength; i++){
			    payload[i] = buf[offset+i];
		    }
		    for (int i=0; i<payloadlength; i++){
		    	int message_val = payload[i] ^ maskingKey[i%4];
			message[i] = message_val;
		    }
		    message[payloadlength] = '\0';
	    }
	    printf("True payload Length : %d\n",payloadlength);
	    return payloadlength;
	}else{
	    printf("No Masking Detected\n\n...");
	}
}

