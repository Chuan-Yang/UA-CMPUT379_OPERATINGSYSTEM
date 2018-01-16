#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

#define	 MY_PORT  2225

void sigIntHandler(int sig){
    exit(0);
}

char *base64encode (const void *b64_encode_this, int encode_this_many_bytes){
    BIO *b64_bio, *mem_bio;      //Declares two OpenSSL BIOs: a base64 filter and a memory BIO.
    BUF_MEM *mem_bio_mem_ptr;    //Pointer to a "memory BIO" structure holding our base64 data.
    b64_bio = BIO_new(BIO_f_base64());                      //Initialize our base64 filter BIO.
    mem_bio = BIO_new(BIO_s_mem());                           //Initialize our memory sink BIO.
    BIO_push(b64_bio, mem_bio);            //Link the BIOs by creating a filter-sink BIO chain.
    BIO_set_flags(b64_bio, BIO_FLAGS_BASE64_NO_NL);  //No newlines every 64 characters or less.
    BIO_write(b64_bio, b64_encode_this, encode_this_many_bytes); //Records base64 encoded data.
    BIO_flush(b64_bio);   //Flush data.  Necessary for b64 encoding, because of pad characters.
    BIO_get_mem_ptr(mem_bio, &mem_bio_mem_ptr);  //Store address of mem_bio's memory structure.
    BIO_set_close(mem_bio, BIO_NOCLOSE);   //Permit access to mem_ptr after BIOs are destroyed.
    BIO_free_all(b64_bio);  //Destroys all BIOs in chain, starting with b64 (i.e. the 1st one).
    BUF_MEM_grow(mem_bio_mem_ptr, (*mem_bio_mem_ptr).length + 1);   //Makes space for end null.
    (*mem_bio_mem_ptr).data[(*mem_bio_mem_ptr).length] = '\0';  //Adds null-terminator to tail.
    return (*mem_bio_mem_ptr).data; //Returns base-64 encoded data. (See: "buf_mem_st" struct).
}

char *base64decode (const void *b64_decode_this, int decode_this_many_bytes){
    BIO *b64_bio, *mem_bio;      //Declares two OpenSSL BIOs: a base64 filter and a memory BIO.
    char *base64_decoded = calloc( (decode_this_many_bytes*3)/4+1, sizeof(char) ); //+1 = null.
    b64_bio = BIO_new(BIO_f_base64());                      //Initialize our base64 filter BIO.
    mem_bio = BIO_new(BIO_s_mem());                         //Initialize our memory source BIO.
    BIO_write(mem_bio, b64_decode_this, decode_this_many_bytes); //Base64 data saved in source.
    BIO_push(b64_bio, mem_bio);          //Link the BIOs by creating a filter-source BIO chain.
    BIO_set_flags(b64_bio, BIO_FLAGS_BASE64_NO_NL);          //Don't require trailing newlines.
    int decoded_byte_index = 0;   //Index where the next base64_decoded byte should be written.
    while ( 0 < BIO_read(b64_bio, base64_decoded+decoded_byte_index, 1) ){ //Read byte-by-byte.
        decoded_byte_index++; //Increment the index until read of BIO decoded data is complete.
    } //Once we're done reading decoded data, BIO_read returns -1 even though there's no error.
    BIO_free_all(b64_bio);  //Destroys all BIOs in chain, starting with b64 (i.e. the 1st one).
    return base64_decoded;        //Returns base-64 decoded data with trailing null terminator.
}


int mencrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext) {

    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))//handleErrors();
    {
        return 0;
    }
    if(!EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    {
        return 0;
    }
    if(!EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
    {
        return 0;
    }
    ciphertext_len = len;
    if(!EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) return 0;
    ciphertext_len += len;

    // Clean up
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}

int mdecrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,  
  unsigned char *iv, unsigned char *plaintext){
    EVP_CIPHER_CTX *ctx;  

    int len;  

    int plaintext_len;  

    /* Create and initialise the context */  
    if(!(ctx = EVP_CIPHER_CTX_new())) return 0;  
 
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))  
        return 0;  
  
    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))  
        return 0;  
    plaintext_len = len;  

     
    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) return 0;  
    plaintext_len += len;  

    /* Clean up */  
    EVP_CIPHER_CTX_free(ctx);  

    return plaintext_len;  
}


int main(int argc, char * argv[])
{
	int	s, number;
	char * welcome;
    welcome = malloc (strlen("CMPUT379 Whiteboard Server v0\n00\n"));

	FILE * keyfile;
    unsigned char iv[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

	unsigned char ciphertext[64];
    unsigned char decryptedtext[100];
	int decryptedtext_len, ciphertext_len;

	char message[1001];		//Use to update the message in an entry

    struct sigaction saSigInt;

    saSigInt.sa_handler = sigIntHandler;
    sigemptyset(&saSigInt.sa_mask);
    sigaction(SIGINT,&saSigInt,0);


	struct	sockaddr_in	server;

	struct	hostent		*host;

	host = gethostbyname (argv[1]);

	if (host == NULL) {
		perror ("Client: cannot get host description");
		exit (1);
	}


	int first = 0;
	while (1){

		// open the socket
		s = socket (AF_INET, SOCK_STREAM, 0);

		if (s < 0) {
			perror ("Client: cannot open socket");
			exit (1);
		}
		bzero (&server, sizeof (server));
		bcopy (host->h_addr, &(server.sin_addr), host->h_length);	
		server.sin_family = host->h_addrtype;
		server.sin_port = htons (strtol(argv[2], (char **)NULL, 10));

		if (connect (s, (struct sockaddr*) & server, sizeof (server))) {
			perror ("Client: cannot connect to server");
			exit (1);
		}
		
		bzero(welcome,strlen(welcome));
		bzero(message, strlen(message));

		//receive and print the welcome message
		recv(s,welcome,strlen("CMPUT379 Whiteboard Server v0\n00\n"),0);

		//sleep(2);
		if (first == 0){
			printf("%s",welcome);
			printf("Check the information: 1\n");
			printf("Update the information without encryption: 2 \n");
			printf("Update the information with encryption: 3 \n");
			printf("Clean the information: 4\n");
			printf("Quit the program: exit or <control-c>\n\n");
			first = 1;
		}
		bzero(welcome,strlen(welcome));
		bzero(message, strlen(message));
		printf("Please enter the commmand: ");
		scanf( "%[^\n]%*c", message );
		//fgets(message, sizeof(message), stdin);
		//scanf("%s", message);
		//printf("%s", message);
		char str[10001], entry_num[11], str_len[11];

		//quit the program
		if (!strcmp(message, "exit")){
			free(welcome);
			exit(EXIT_SUCCESS);
		}
		// check inforamtion
		if (!strcmp(message, "1")){
			printf("Please enter the number of the entry: ");
			scanf( "%[^\n]%*c", entry_num );
			sprintf(message, "?%s\n", entry_num);
		}
		//update information (without encrytion)
		if (!strcmp(message, "2")){
			printf("Please enter the number of the entry: ");
			scanf( "%[^\n]%*c", entry_num );
			printf("Please enter the length of the string: ");
			scanf( "%[^\n]%*c", str_len );
			printf("Please enter the string: ");
			scanf( "%[^\n]%*c", str );
			sprintf(message, "@%sp%s\n%s\n", entry_num, str_len, str);
		}
		//update information (with encrytion)
		if (!strcmp(message, "3")){
			printf("Please enter the number of the entry: ");
			scanf( "%[^\n]%*c", entry_num );
			printf("Please enter the length of the string: ");
			scanf( "%[^\n]%*c", str_len );
			printf("Please enter the string: ");
			scanf( "%[^\n]%*c", str );

			keyfile = fopen(argv[3], "r");

			if (!feof (keyfile)){
				printf("No key file, cannot encrypt a message\n");
				sprintf(message, "@%sp%s\n%s\n", entry_num, str_len, str);
			}
			else{
				printf("The encrypted message is: \n");
				//Encrypte
				char line [10001];
				keyfile = fopen(argv[3], "r");
				char * encode_key;
				while (fgets(line, sizeof (line), keyfile)) {
					int bytes_to_decode = strlen(line);
					encode_key = base64decode(line, bytes_to_decode);
					break;
      			}
				
				char tmp[10001];
				strcpy(tmp, "CMPUT379 Whiteboard Encrypted v0\n");
				strcat(tmp, str);
				strcpy(str, tmp);

				ciphertext_len = mencrypt((str), strlen(str), encode_key, iv, ciphertext);
				
				printf("CMPUT379 Whiteboard Encrypted v0\n%s",str);
				//encode()
				int tmp1 = strlen(ciphertext);
				sprintf(message, "@%sc%d\n%s\n", entry_num, tmp1, ciphertext);
				free(encode_key);
			}	
		}
		// clean an entry
		if (!strcmp(message, "4")){
			printf("Please enter the number of the entry: ");
			scanf( "%[^\n]%*c", entry_num );
			sprintf(message, "@%sp0\n\n", entry_num);
			//printf("%s", message);
		}

		send(s, message, strlen(message), 0);
		bzero(message, strlen(message));
		
		//receive message
		recv(s, message, 1001, 0);
		printf("Server Response:\n%s", message);
		int encrypted = 0, i = 0;
		for (i = 0; i < strlen (message);i++){
			if (message[i] == 'c'){
				encrypted = 1;
				break;
			}
			if (message[i] == 'p' || message[i] == '\n')
				break;
		}

		if (encrypted == 1){
			printf("The message sent back is encrypted\n");
			if (!feof (keyfile))
				printf("No key file, cannot decrypt the encrypted message\n");
			else{
				//decode()
				int success = 0;
				keyfile = fopen(argv[3], "r");
				int first = 0;
				char line[10001];
				while (fgets(line, sizeof (line), keyfile)) {
					if (first == 0){
						first ++;
						continue;
					}
					int bytes_to_decode = strlen(line);
					char * decode_key = base64decode(line, bytes_to_decode);

					decryptedtext_len = mdecrypt(ciphertext, ciphertext_len, decode_key, iv, decryptedtext);
					decryptedtext[decryptedtext_len] = '\0';

					if (strncmp(decryptedtext, "CMPUT379 Whiteboard Encrypted v0", 32) == 0){
						printf("The plain text we encrypted is: %s\n",decryptedtext);
						success = 1;
						break;
					}
					free(decode_key);
				}
				if (success == 0)	printf("Decryption failed, no matched key.\n");
			}
		}

		close (s);
		sleep (2);
	}

	free(welcome);
	exit(EXIT_SUCCESS);
}
