All inputs are taken from stdin, i.e using "./a.out < input.txt"

B3-2 needs the openssl package to compile.
Install with "sudo apt-get install libssl-dev" if running a Ubuntu-based distro.

Compilation:
B1:
	g++ b1.cc
	
B2:
	gcc -std=c11 -Wextra -Wall -pedantic -O3 b2.c -lm
	
B3-1:
	Run python file as "python3 b3-1.py < input.txt"
	
B3-2:
	gcc -std=c11 -Wextra -Wall -pedantic -O3 b3-2.c -lcrypto
	
PADDINGPADDINGPADDINGPADDINGPADDINGPADDINGPADDINGPADDINGPADDINGPADDINGPADDINGPADDINGPADDINGPADDINGPADDINGPADDINGPADDINGPADDINGPADDINGPADDINGPADDINGPADDINGPADDINGPADDINGPADDINGPADDINGPADDINGPADDINGPADDINGPADDING