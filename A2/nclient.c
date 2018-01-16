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

#define	 MY_PORT  2225

void sigIntHandler(int sig){
    exit(0);
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

	FILE * keyfile;
	unsigned char key[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    unsigned char iv[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

	unsigned char ciphertext[64];
    unsigned char decryptedtext[100];
	int decryptedtext_len, ciphertext_len;

    welcome = malloc (strlen("CMPUT379 Whiteboard Server v0\n00\n"));
	

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
		server.sin_family = AF_INET;
		server.sin_addr.s_addr = inet_addr("127.0.0.1");
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
		scanf("%s", message);
		
		int entry_num = 0, str_len = 0;
		char str[10001];

		//quit the program
		if (!strcmp(message, "exit")){
			free(welcome);
			exit(EXIT_SUCCESS);
		}
		// check inforamtion
		if (!strcmp(message, "1")){
			printf("Please enter the number of the entry: ");
			scanf("%d", &entry_num);
			sprintf(message, "?%d\n", entry_num);
		}
		//update information (without encrytion)
		if (!strcmp(message, "2")){
			printf("Please enter the number of the entry: ");
			scanf("%d", &entry_num);
			printf("Please enter the length of the string: ");
			scanf("%d", &str_len);
			printf("Please enter the string: ");
			scanf("%s", str);
			sprintf(message, "@%dp%d\n%s\n", entry_num, str_len, str);
		}
		//update information (with encrytion)
		if (!strcmp(message, "3")){
			printf("Please enter the number of the entry: ");
			scanf("%d", &entry_num);
			printf("Please enter the length of the string: ");
			scanf("%d", &str_len);
			printf("Please enter the string: ");
			scanf("%s", str);
			
			if (argc < 4){
				printf("No key file, cannot encrypt a message\n");
				sprintf(message, "@%dp%d\n%s\n", entry_num, str_len, str);
			}
			else{
				
				keyfile = fopen(argv[3], "r+");
				while (!feof (fp)) {
        			if (fgets(line, sizeof (line), fp) {
						
       				 }
      			}

				ciphertext_len = mencrypt((str), strlen(str), key, iv, ciphertext);
				
				printf("CMPUT379 Whiteboard Encrypted v0\n%s",str);
				//encode()
				
				sprintf(message, "@%dc%d\n%s\n", entry_num, ciphertext_len,ciphertext);
			}	
		}
		// clean an entry
		if (!strcmp(message, "4")){
			printf("Please enter the number of the entry: ");
			scanf("%d", &entry_num);
			sprintf(message, "@%dp0\n\n", entry_num);
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
			if (argc < 4)
				printf("No key file, cannot decrypt the encrypted message\n");
			else{
				keyfile = fopen(argv[3], "r+");
				while (fgets(line, sizeof (line), fp) {}

				decryptedtext_len = mdecrypt(ciphertext, ciphertext_len, key, iv, decryptedtext);
				decryptedtext[decryptedtext_len] = '\0';
				//decode()
				printf("The plain text we encrypted is: %s",decryptedtext);
			}
		}

		close (s);
		sleep (2);
	}

	free(welcome);
	exit(EXIT_SUCCESS);
}
