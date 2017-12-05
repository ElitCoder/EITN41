#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#include <openssl/sha.h>
#include <omp.h>

#define MAX_X	(160)

typedef struct {
	unsigned char	v;
	union {
		unsigned short	s;
		unsigned char	b[2];
	};
} value_t;

static unsigned long long	g_results[MAX_X + 1];

unsigned char* bytes_to_string(unsigned char* b, size_t len)
{
	unsigned char*	string = malloc(len * 2 + 1);
	size_t			i;
	
	for (i = 0; i < len; i++)
		sprintf((char*)(string + i * 2), "%02x", b[i]);
	
	string[len * 2] = '\0';
	
	return string;
}

void char_to_binary(unsigned char* hash, char* dst)
{
	int	i;
	int	j;

	for (i = 0; i < SHA_DIGEST_LENGTH; i++)
		for (j = 0; j < 8; j++)
			dst[i * 8 + j] = ((hash[i] >> j) & 1) + '0';
}

bool collisions(char** vzero, char** vone, unsigned int len, int X)
{
	unsigned int		i;
	unsigned int		j;
	unsigned long long	nbr_collisions = 0;
	
	#pragma omp parallel for private(i, j) reduction(+: nbr_collisions)
	for (i = 0; i < len; i++) {
		for (j = 0; j < len; j++) {
			if (memcmp(vzero[i], vone[j], X) == 0)
				nbr_collisions++;		
		}
	}
	
	g_results[X] = nbr_collisions;
	
	return nbr_collisions > 0;
}

char** create(value_t* val)
{
	char**	results;
	unsigned int	n;
	unsigned int	i;
	unsigned char	tmp[3];
	unsigned char*	hash;
	
	printf("creating hashes\n");
		
	n = (unsigned int)pow(2, 16);
	results = malloc(sizeof(char*) * n);
	
	if (results == NULL)
		printf("malloc failed\n");
		
	hash = malloc(SHA_DIGEST_LENGTH);
	
	if (hash == NULL)
		printf("malloc failed\n");
	
	for (i = 0; i < n; i++) {
		results[i] = malloc(SHA_DIGEST_LENGTH * 8);
		
		if (results[i] == NULL)
			printf("malloc failed\n");
	}
			
	for (i = 0; i < n; i++) {
		val->s = i;
		tmp[0] = val->v;
		tmp[1] = val->b[0];
		tmp[2] = val->b[1];
		
		SHA1(tmp, 3, hash);
		char_to_binary(hash, results[i]);
	}
	
	free(hash);
	
	return results;
}

void free_results(char** p)
{
	unsigned int	i;
	
	for (i = 0; i < (int)pow(2, 16); i++)
		free(p[i]);
		
	free(p);
}

int main(void)
{
	char** zero;
	char**	one;
	int		i;
	int		j;
	int		n;
	value_t	b = { .v = 0, .s = 0 };
	
	n = (int)pow(2, 16);
	
	zero = create(&b);
	b.v = 1;
	one = create(&b);
	
	for (i = 1; i <= MAX_X; i++) {
		printf("running for x = %d\n", i);
		
		if (!collisions(zero, one, n, i))
			break;
	}
			
	for (j = 1; j < i; j++)
		printf("X = %d bits, %llu collisions, %f%c\n", j, g_results[j], (double)g_results[j] / ((double)n * (double)n), '%');
	
	free_results(zero);
	free_results(one);
	
	return 0;
}