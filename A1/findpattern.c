#include "findpattern.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <setjmp.h>
#include <math.h>
#include <sys/types.h> 

jmp_buf env;
char * current_point = (char *)startAddr;
char * check_loop;

/**
 * sigsegv_handler: handle the Segmatention Fault: when trying to access the kernel space  
 * Jump the whole page which cannot be accessed
 */

void sigsegv_handler(int sig){

    current_point += getpagesize();
    siglongjmp(env, 1);
    return;

}

/**
 * checkPattern: this function is used to help the findpattern
 * When it finds a character which is the same as the first character of pattern,
 * it will jump to checkPattern and begin to scan the next characters and compare with pattern.
 * If we find it matches, return 1. Otherwise, return 0.
 */

int checkPattern(int patlength, char *pattern){

    int find_match = 1; // Check if pattern matched
    int match_point = 0; // Record the index of checking for pattern
    char * i;
    char * endAddr = (char *) 0xffffffff;  // The End Address

    i = current_point;

    if ((current_point + patlength) >= endAddr)
        return find_match;
    for (; i < current_point + patlength; i++){
        unsigned char current_byte = *i;
        if (current_byte == pattern[match_point]){ match_point += 1; }
        else { find_match = 0; break; }
    }
    return find_match;
}

unsigned int findpattern (unsigned char *pattern, unsigned int patlength, struct patmatch *locations, unsigned int loclength){

    struct sigaction sigsegv_act;
    sigsegv_act.sa_handler = sigsegv_handler;
    sigemptyset(&sigsegv_act.sa_mask);
    sigsegv_act.sa_flags = 0;
    sigaction(SIGSEGV, &sigsegv_act, NULL); 


    unsigned int match_num  = 0;

    int j = 0;

    //initialize
    for (j = 0; j< loclength; j++){
        locations[j].location = 0;
        locations[j].mode = 1;
    }

    
    char * starAddr = (char *) 0x00000000;
    char * endAddr = (char *) 0xffffffff;
    // unsigned int xx=0, test = 0; //for test
    check_loop = starAddr;
    
    for (current_point = 0; current_point <= endAddr; current_point++){

        int jmp_tmp = sigsetjmp (env, 1);
        if (current_point >= check_loop) {check_loop = current_point;}
        else    {break;}

        char current_byte = *current_point;
        // Catch the Segmentaion Fault to check if this page is accessable

        //Test
        //xx+=1;
        //printf("%p %d %c\n",current_point, xx, current_byte);
        
        if (current_byte == pattern [0]){
            if ( checkPattern(patlength, pattern) == 1){
                // Check the max len for the array
                match_num += 1;
                if (loclength >= match_num) {
                    //printf("%p\n", current_point); for test
                    locations[match_num - 1].location = (unsigned int) current_point;
                    current_point += (patlength - 1); 
                    //Check the mode

                    *current_point = 'a';
                    locations[match_num - 1].mode = MEM_RW;
                }
            }
        }
    }
    return match_num;
}
