#include "../findpattern.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// quote from https://gcc.gnu.org/onlinedocs/cpp/Stringification.html
#define xstr(s) str(s)
#define str(s) #s
#ifndef input_pattern
#define input_pattern ""
#endif
#define min(a,b)            (((a) < (b)) ? (a) : (b))

int main(int argc, char const *argv[]){
	unsigned int loclength = 20;
	struct patmatch *locations = malloc(sizeof(struct patmatch) * loclength);
	FILE *fp;
	fp = fopen("D3test.txt", "w");
	int size = ftell(fp);
    	int i = 0;
	fclose(fp);


	printf("Test1\n");
   	printf("Directly check the input pattern\n");

	unsigned int match_num=0;
    	//printf ("%c %p\n", (*modify_heap), modify_heap);
	match_num = findpattern ((unsigned char *)argv[1], strlen(argv[1]), locations, loclength);

	printf("\nPass 1\n");
	printf("Total Matches: %d\n", match_num);
	
	for (i = 0; i < min(match_num, loclength)
; i++){
		if (locations[i].mode == 1)
			printf("0x%.8X\tMEM_RO\n", locations[i].location);
		else
			printf("0x%.8X\tMEM_RW\n", locations[i].location);
	}
	printf("\n");

	int file = open("D3test.txt", O_RDWR); 
	fprintf(fp, "%s", argv[1]);
	char *c = (char *) mmap(0,size,PROT_WRITE,MAP_SHARED,file,0);
	


	printf("Test2\n");
   	printf("The memory modification changed on file by mmap\n");

	match_num=0;
    	//printf ("%c %p\n", (*modify_heap), modify_heap);
	match_num = findpattern ((unsigned char *)argv[1], strlen(argv[1]), locations, loclength);

	printf("\nPass 2\n");
	printf("Total Matches: %d\n", match_num);
	
	for (i = 0; i < min(match_num, loclength)
; i++){
		if (locations[i].mode == 1)
			printf("0x%.8X\tMEM_RO\n", locations[i].location);
		else
			printf("0x%.8X\tMEM_RW\n", locations[i].location);
	}
	printf("\n");

	return 0;
}
