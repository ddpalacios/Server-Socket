#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include "HTMLReader.h"

void trim(char *str) {
    // Trim leading spaces and newlines
    while (*str && (isspace((unsigned char)*str) || *str == '\n')) {
        str++;
    }

    // Trim trailing spaces and newlines
    char *end = str + strlen(str) - 1;
    while (end > str && (isspace((unsigned char)*end) || *end == '\n')) {
        end--;
    }

    // Null-terminate the string after trimming
    *(end + 1) = '\0';
}



void get_header_value(const char *buf, const char *key, char *value, size_t value_size){
	char *key_start = strstr(buf, key);
	if (key_start){
		key_start += strlen(key);
		while(*key_start == ' ' || *key_start == ':')key_start++;
		char *line_end = strstr(key_start, "\r\n");

		if (line_end){
			size_t len= line_end - key_start;
			if (len >= value_size) len = value_size -1;
			strncpy(value, key_start, len);
			value[len] = '\0';
		}
	}else{
		value[0] = '\0';
	}
}

void process_get(char *buf,  int client_fd,SSL *cSSL){
	char *start = buf + 4;
	char *end = strchr(start, ' ');

	if (strstr(buf, "/websocket")){
		printf("\nClient %d Requested WebSocket Connection...\n\n", client_fd);
		char websocketKey[32];
		get_header_value(buf, "Sec-WebSocket-Key", websocketKey, sizeof(websocketKey));
		char *magicKey = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
		char combinedKey[200] = {0};
		snprintf(combinedKey, sizeof(combinedKey), "%s%s", websocketKey, magicKey);
		unsigned char hash[SHA_DIGEST_LENGTH];
		SHA1((const unsigned char *)combinedKey, strlen(combinedKey), hash);
		char base64Hash[SHA_DIGEST_LENGTH * 2] = {0};
		int base64Length = EVP_EncodeBlock((unsigned char *)base64Hash, hash, SHA_DIGEST_LENGTH);
		char http_header[4096];
		snprintf(http_header, sizeof(http_header),
			 "HTTP/1.1 101 Switching Protocols\r\n"
			 "Upgrade: websocket\r\n"
			 "Connection: Upgrade\r\n"
			 "Sec-WebSocket-Accept: %s\r\n"
			 "Access-Control-Allow-Origin: *\r\n"
			 "\r\n", base64Hash);
		printf("Sending handshake response to client %d...\n\n", client_fd);
		printf("%s\n", http_header);
		SSL_write(cSSL,http_header,strlen(http_header));
	}

	if (end){
		*end='\0';
		if (strcmp(start, "/") ==0){
			char *html_buffer = get_file_buffer("index.html");
			int html_length = strlen(html_buffer);
			char http_header[2048];
			snprintf(http_header,sizeof(http_header),
					"HTTP/1.1 200 OK\r\n"
					"Content-Type: text/html\r\n"
					"Connection: close\r\n"
					"Content-Length: %d\r\n"
					"\r\n",html_length);
			SSL_write(cSSL,http_header,strlen(http_header));
			SSL_write(cSSL,html_buffer,html_length);
			free(html_buffer);
		}

		if (strstr(buf, "/test_send")){
			char *html_buffer = "{\"value\": \"Hello, Mr. Palacios\"}";
			int html_length = strlen(html_buffer);
			char http_header[2048];
			snprintf(http_header,sizeof(http_header),
					"HTTP/1.1 200 OK\r\n"
					"Content-Type: text/json\r\n"
					"Content-Length: %d\r\n"
					"\r\n",html_length);
			SSL_write(cSSL,http_header,strlen(http_header));
			SSL_write(cSSL,html_buffer,html_length);
		}
	}
}
