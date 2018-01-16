#define startAddr 0x00000000
#define MEM_RW 0
#define MEM_RO 1

struct patmatch {
    unsigned int location;
    unsigned char mode; /* MEM_RW, or MEM_RO */
};

void sigsegv_handler(int sig);
int checkPattern(int patlength, char *pattern);
unsigned int findpattern (unsigned char *pattern, unsigned int patlength, struct patmatch *locations, unsigned int loclength);