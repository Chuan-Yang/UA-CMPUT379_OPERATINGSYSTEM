#include "../findpattern.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// quote from https://gcc.gnu.org/onlinedocs/cpp/Stringification.html
#define xstr(s) str(s)
#define str(s) #s
#ifndef input_pattern
#define input_pattern ""
#endif

int main(int argc, char const *argv[]){

	unsigned int loclength = 5;
	struct patmatch *locations = malloc(sizeof(struct patmatch) * loclength);
    int i = 0;

	// char *pattern = xstr(input_pattern);
	// unsigned int patlength = strlen(pattern); 

	printf("Test1\n");
    printf("Directly check the input pattern\n");

	unsigned int match_num=0;
    //printf ("%c %p\n", (*modify_heap), modify_heap);
	match_num = findpattern (argv[1], strlen(argv[1]), locations, loclength);

	printf("\nPass 1\n");
	printf("Total Matches: %d\n", match_num);
	
	for (i = 0; i < min(match_num,loclength); i++){
		if (locations[i].mode == 1)
			printf("0x%.8X\tMEM_RO\n", locations[i].location);
		else
			printf("0x%.8X\tMEM_RW\n", locations[i].location);
	}
	printf("\n");

    char local_str[strlen(argv[1])];
	strcpy(local_str,argv[1]);

    printf("Test2\n");
    printf("Check again after save the pattern into a local variable\n");

	match_num=0;
    //printf ("%c %p\n", (*modify_heap), modify_heap);
	match_num = findpattern (argv[1], strlen(argv[1]), locations, loclength);

	printf("\nPass 2\n");
	printf("Total Matches: %d\n", match_num);
	
	for (i = 0; i < min(match_num,loclength); i++){
		if (locations[i].mode == 1)
			printf("0x%.8X\tMEM_RO\n", locations[i].location);
		else
			printf("0x%.8X\tMEM_RW\n", locations[i].location);
	}
	printf("\n");
		
	return 0;
}

