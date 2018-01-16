#Run the program by entering make test<number> p=<pattern> 

CC = gcc
FLAGS = -m32
p?=

test1: test1.o
	./test1.o $(p) > test_results.txt

test1.o: driver1.c
	$(CC) $(FLAGS) -g -Wall driver1.c -o test1.o

test2: test2.o
	./test2.o $(p) >> test_results.txt

test2.o: driver2.c
	$(CC) $(FLAGS) -g -Wall driver2.c -o test2.o

test3: test3.o
	./test3.o $(p) >> test_results.txt

test3.o: driver3.c
	$(CC) $(FLAGS) -g -Wall driver3.c -o test3.o

clean:
	rm test1.o test2.o test3.o test_results.txt D3test.txt

