#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <netdb.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>



SSL_CTX *sslctx;


struct Client{
	struct pollfd pfd;
	SSL *ssl;

};

void InitializeSSL()
{
    SSL_load_error_strings();
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    sslctx = SSL_CTX_new( SSLv23_server_method());
    SSL_CTX_set_options(sslctx, SSL_OP_SINGLE_DH_USE);
    if (SSL_CTX_use_certificate_file(sslctx, "my_cert.pem" , SSL_FILETYPE_PEM) < 0){
    	printf("ERROR LOADING CERTIFICATE\n");
    
    
    }
    if (SSL_CTX_use_PrivateKey_file(sslctx, "my_key.pem", SSL_FILETYPE_PEM) < 0){
    	printf("ERROR LOADING KEY\n");
    
    
    }
}

void DestroySSL()
{
    ERR_free_strings();
    EVP_cleanup();
}

void ShutdownSSL(SSL *cSSL)
{
    SSL_shutdown(cSSL);
    SSL_free(cSSL);
}

