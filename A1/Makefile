CC = gcc
FLAGS = -m32

findpattern.o: findpattern.c findpattern.h
    $(CC) $(FLAGS) -c findpattern.c

driver1.o: driver1.c
    cd ../tests && $(CC) $(FLAGS) -c driver.c P=$(P)

test1: driver1.o findpattern.o
    $(CC) $(FLAGS) driver1.o findpattern.o -o t1

clean:
    rm -rf output test2 test3 *.o