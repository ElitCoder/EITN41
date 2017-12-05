#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#include <openssl/sha.h>
#include <omp.h>

typedef struct {
	unsigned char	v;
	union {
		unsigned short	s;
		unsigned char	b[2];
	};
} value_t;

unsigned char* bytes_to_string(unsigned char* b, size_t len)
{
	unsigned char*	string = malloc(len * 2 + 1);
	size_t			i;
	
	for (i = 0; i < len; i++)
		sprintf((char*)(string + i * 2), "%02x", b[i]);
	
	string[len * 2] = '\0';
	
	return string;
}

void hex_to_binary(char* dst, unsigned char* src)
{
	char tmp[9];
	itoa(src[0], tmp, 2);
	
	printf("%s\n", tmp);
}

void collisions(unsigned char** vzero, unsigned char** vone, unsigned int len, int X)
{
	unsigned int	i;
	unsigned int	j;
	size_t			nbr_collisions = 0;
	unsigned char*	tmp1;
	unsigned char*	tmp2;
	
	hex_to_binary(NULL, vzero[0]);
	
	printf("checking collision\n");
	
	#pragma omp parallel for private(i, j) reduction(+:nbr_collisions)
	for (i = 0; i < len; i++) {		
		for (j = 0; j < len; j++)
			if (memcmp(vzero[i], vone[j], 2) == 0)
				nbr_collisions++;		
	}
	
	printf("collisions: %zu\n", nbr_collisions);
	printf("proc %f\n", (double)nbr_collisions / len);
}

unsigned char** create(value_t* val)
{
	unsigned char**	results;
	unsigned int	n;
	unsigned int	i;
	unsigned char	tmp[3];
	
	printf("creating hashes\n");
	
	assert(sizeof(unsigned short) == 2);
	assert(sizeof(unsigned char) == 1);
		
	n = (unsigned int)pow(2, 16);
	results = malloc(sizeof(unsigned char*) * n);
	
	if (results == NULL)
		printf("malloc failed\n");
	
	for (i = 0; i < n; i++) {
		results[i] = malloc(SHA_DIGEST_LENGTH);
		
		if (results[i] == NULL)
			printf("malloc failed\n");
	}
			
	for (i = 0; i < n; i++) {
		val->s = i;
		tmp[0] = val->v;
		tmp[1] = val->b[0];
		tmp[2] = val->b[1];
		
		SHA1(tmp, 3, results[i]);				
	}
	
	return results;
}

void free_results(unsigned char** p)
{
	unsigned int	i;
	
	for (i = 0; i < (int)pow(2, 16); i++)
		free(p[i]);
		
	free(p);
}

int main(void)
{
	unsigned char** zero;
	unsigned char**	one;
	value_t	b = { .v = 0, .s = 0 };
	
	zero = create(&b);
	b.v = 1;
	one = create(&b);
	
	collisions(zero, one, (int)pow(2, 16), 1);
	
	free_results(zero);
	free_results(one);
	
	return 0;
}