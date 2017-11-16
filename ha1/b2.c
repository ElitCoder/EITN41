#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <math.h>
#include <time.h>
#include <string.h>

#define DEBUG			(0)
#define MIN_ITERATIONS	(20)
#define LAMBDA			(3.66)

#define error(...)	do { fprintf(stderr, "%s\n", __VA_ARGS__); exit(1); } while (0)

typedef struct {
	char	b;	/* Number of balls in bin */
	bool	f;	/* Has the bin already been filled? */
} bin_t;

typedef struct {
	int*	data;
	size_t	count;
	size_t	reserved;
} vector_t;

vector_t* new_vector(void)
{
	vector_t*	vector = malloc(sizeof(vector_t));
	
	if (vector == NULL)
		error("malloc failed\n");
		
	vector->count = 0;
	vector->reserved = 1;
	vector->data = malloc(vector->reserved * sizeof(int));
	
	if (vector->data == NULL)
		error("malloc failed\n");
	
	return vector;
}

void free_vector(vector_t* vector)
{
	free(vector->data);
	free(vector);
}

void allocate_vector(vector_t* vector)
{
	int*	new_data = realloc(vector->data, vector->reserved * 2 * sizeof(int));
	
	if (new_data == NULL)
		error("realloc failed\n");
		
	vector->data = new_data;
	vector->reserved *= 2;
}

void insert_last(vector_t* vector, int nbr)
{
	if (vector->count >= vector->reserved)
		allocate_vector(vector);
		
	vector->data[vector->count++] = nbr;
}

int* get_data(vector_t* vector)
{
	return vector->data;
}

size_t count(vector_t* vector)
{
	return vector->count;
}

double mean(int* r, int n)
{
	double	m;
	int		i;
	
	m = 0.0;
	
	/*	Calculate mean */
	for (i = 0; i < n; i++)
		m += r[i];
		
	return m / n;
}

double std_dev(int* r, int ci)
{
	double	m;
	double	v;
	int		i;
	
	v = 0.0;
	m = mean(r, ci);
	
	/*	Calculate variance */
	for (i = 0; i < ci; i++)
		v += (r[i] - m) * (r[i] - m);
		
	v /= (ci - 1);
	
	/* Calculate standard deviation */
	return sqrt(v);
}

double avg(int* r, int ci)
{
	double	s = 0.0;
	int		i;
	
	for (i = 0; i < ci; i++)
		s += r[i];
		
	return s / ci;
}

int run(int k, int c, bin_t* a, int b)
{
	
	int		m;	/*	Currently made coins */
	int		r;	/*	Random number */
	int		i;	/* 	Loop variable */
		
	m = 0;

	for (i = 0; m < c; i++) {
		r = rand() % b;
		
		if (a[r].f)
			continue;
			
		a[r].b++;
		
		if (a[r].b >= k) {
			a[r].f = true;
			m++;
		}
	}

	return i;
}

int main(int argc, char** argv)
{
	bin_t*		a;		/*	Array for holding the current balls */
	vector_t*	r;		/* 	Array for holding the results */
	double		m;		/*	Current mean value */
	double		bound;	/*	Current bound */
	double		up;		/* 	Current upper bound */
	double		low;	/*	Current lower bound */
	int			u;		/*	Number of bits to identify the bin, i.e there are b = 2^u bins */
	int			k;		/*	Number of collisions needed to make a coin */
	int			c;		/*	Number of coins to be created */
	int			ci;		/*	Confidence interval width */
	int 		b;		/*	Number of bins */
	int			i;		/*	Loop variable */
	
	srand(time(NULL));
	
	if (argc != 5) {
		/*	Test parameters */
		u = 20;
		k = 7;
		c = 10000;
		ci = 10000;
		
		printf("warning: running in test\n");
	} else {
		u = strtol(argv[1], NULL, 10);
		k = strtol(argv[2], NULL, 10);
		c = strtol(argv[3], NULL, 10);
		ci = strtol(argv[4], NULL, 10);
	}

	b = pow(2, u);

#if DEBUG
	printf("u: %d\nk: %d\nc: %d\nb: %d\n", u, k, c, b);
#endif

	if (b > RAND_MAX)
		error("b is bigger than RAND_MAX");

	a = calloc(b, sizeof(bin_t));
	r = new_vector();

	if (a == NULL)
		error("out of memory\n");
	
	/* Run simulation */
	for (i = 0;; i++) {
		printf("iteration %d\n", i);
		
		insert_last(r, run(k, c, a, b));
		
		memset(a, 0, b * sizeof(bin_t));
		
		if (i < MIN_ITERATIONS)
			continue;
			
		/* check up and low bound */
		m = mean(get_data(r), count(r));
		bound = LAMBDA * (std_dev(get_data(r), count(r)) / sqrt(count(r)));
		up = m + bound;
		low = m - bound;
		
		if (up - low < ci)
			break;	/* We're done */
	}
	
	printf("answer: %f\n", m);
	
	free(a);
	free_vector(r);
	
	return 0;
}