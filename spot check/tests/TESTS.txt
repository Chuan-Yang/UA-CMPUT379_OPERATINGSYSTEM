Driver Strategy:

Every time before store the pattern, we call findpattern() directly to check the space where 
storing pattern.

Driver1: use malloc to store the pattern into HEAP space

Driver2: use local variable to store the pattern into STACK space.
	 First call of findpattern(): directly check without saving it in a local variable
	 Second call of findpattern(): saving pattern into a local variable and check again

Driver3: use fprintf() to save pattern in the "D3test.txt" and use mmap to save it to the 		 address
