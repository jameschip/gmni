
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#define GEMINI_PORT		1965
#define BUFFER_SIZE		1024

int open_connection(const char *hostname, int port);

SSL_CTX* InitCTX( void )
{
	SSL_METHOD *method;
	SSL_CTX *ctx;
	OpenSSL_add_all_algorithms();  		/* Load cryptos, et.al. */
	SSL_load_error_strings();   		/* Bring in and register error messages */
	method = TLSv1_2_client_method();  	/* Create new client-method instance */
	ctx = SSL_CTX_new(method);   		/* Create new context */
	if ( ctx == NULL ) {
		ERR_print_errors_fp(stderr);
		abort();
	}
	return ctx;
}


int main(int argc, char *argv[]){
	int fd;
	char buffer[1024];
	SSL_CTX *ctx;
	SSL *ssl;
	
	SSL_library_init();
	ctx = InitCTX();
	

	fd = open_connection( "gemini.conman.org", GEMINI_PORT  ); 

   	ssl = SSL_new(ctx);      /* create new SSL connection state */
    SSL_set_fd(ssl, fd);    /* attach the socket descriptor */
    if ( SSL_connect(ssl) < 0 ) {
		ERR_print_errors_fp(stderr);
		exit(0);
    }   /* perform the connection */
	else {
		SSL_write( ssl ,"gemini://gemini.conman.org\r\n", strlen( "gemini://gemini.conman.org\r\n" ) );
		SSL_read(ssl, buffer, sizeof(buffer));
	}
	
	// int sn = send( fd, "gemini://gemini.conman.org\r\n", strlen( "gemini://gemini.conman.org\r\n" ), 0 );

	// if ( sn < 0  ) {
		// perror("socket");
        // exit(1);
	// } else {
		// printf( "Send %d bytes.\n", sn );
	// }
	// 
	// bzero(buffer, BUFFER_SIZE);
// 
	// int rn = read( fd, buffer, BUFFER_SIZE );
// 
	// if ( rn < 0 ) {
		// perror("socket");
        // exit(1);
	// } else {
		// printf( "Read %d bytes:\n", rn );
		printf("%s\n", buffer ); 
	// }

	shutdown(fd, SHUT_RDWR); 
	close(fd); 

	return 0;
}


int open_connection(const char *hostname, int port) {
	int sd, err;
	struct addrinfo hints = {}, *addrs;
	char port_str[16] = {};

	hints.ai_family = AF_INET; 
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	sprintf(port_str, "%d", port);

	err = getaddrinfo(hostname, port_str, &hints, &addrs);
	if (err != 0) {
		perror("socket");
        exit(1);
	}

	for(struct addrinfo *addr = addrs; addr != NULL; addr = addr->ai_next) {
		sd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
		if (sd == -1) {
			perror("socket");
	        exit(1); 
		}

		if (connect(sd, addr->ai_addr, addr->ai_addrlen) == 0)
			break;

	}

	freeaddrinfo(addrs);

	if (sd == -1) {
		perror("socket");
        exit(1);
	}

	return sd;
}