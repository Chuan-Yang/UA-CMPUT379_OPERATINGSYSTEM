#include "findpattern.c"
#include <stdio.h>
#include <stdlib.h>

unsigned int loclength = 5;
struct patmatch locations[20];

int main(int argc, char const *argv[]){

    	int i = 1;
	unsigned int patlength = 9; 


 	char *modify_heap = malloc ((loclength)*(patlength) * sizeof(char));

		modify_heap [i-1] = 'O'; 
		modify_heap [i] = 'v'; 
		modify_heap [i+1] = 'e'; 
		modify_heap [i+2] = 'r'; 
		modify_heap [i+3] = 'W'; 
		modify_heap [i+4] = 'a';
		modify_heap [i+5] = 't';
		modify_heap [i+6] = 'c';
		modify_heap [i+7] = 'h'; 


	unsigned char pattern[patlength+1];
	pattern[0] = 'O'; 
	pattern[1] = 'v'; 
	pattern[2] = 'e'; 
	pattern[3] = 'r'; 
	pattern[4] = 'W'; 
	pattern[5] = 'a';
	pattern[6] = 't';
	pattern[7] = 'c';
	pattern[8] = 'h'; 


	unsigned int match_num=0;
    //printf ("%c %p\n", (*modify_heap), modify_heap);
	match_num = findpattern (pattern, patlength, locations, loclength);



   	printf("Test1\n");
    printf("The memory modification changed on heap by malloc\n");

	printf("\nPass 1\n");
	printf("Total Matches: %d\n", match_num);
	
	for (i = 0; i < match_num; i++){
		if (locations[i].mode == 1)
			printf("0x	%.8X\tMEM_RO\n", locations[i].location);
		else
			printf("0x%.8X\tMEM_RW\n", locations[i].location);
	}
	printf("\n");
		
	free(modify_heap);
}

