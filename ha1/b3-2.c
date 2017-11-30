#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/sha.h>

/*
************
VALUE MACROS
************
*/

#define VECTOR_START_SIZE	(1)
#define VECTOR_GROW_SPEED	(2)	/* Should be > 1 */

/*
***************
FUNCTION MACROS
***************
*/

#define MIN(a, b)	(a > b ? b : a)
#define ERROR(...)	do 	{															\
							fprintf(stderr, "ERROR (%s:%d): ", __func__, __LINE__); \
							fprintf(stderr, __VA_ARGS__);							\
							fprintf(stderr, "\n");									\
							exit(1);												\
						} while (0)
#define DEBUG(...)	do 	{													\
							printf("DEBUG (%s:%d): ", __func__, __LINE__);	\
							printf(__VA_ARGS__);							\
							printf("\n");									\
						} while (0)
			
/*
*************
TYPEDEFS
*************
*/
typedef struct node_t	node_t;

/*
*******
STRUCTS
*******
*/

struct node_t {
	node_t*			parent;
	node_t*			left;
	node_t*			right;
	unsigned char*	hash;
};

typedef struct {
	node_t**	nodes;
	size_t		size;
	size_t		reserved;
} vector_t;

/*
****************
GLOBAL VARIABLES
****************
*/

static int	g_leaf_depth;	/* The depth for the leaves */

/*
****************
HELPER FUNCTIONS
****************
*/

void* xmalloc(size_t n)
{
	void* ptr = malloc(n);
	
	if (ptr == NULL)
		ERROR("malloc for %zu bytes failed", n);
		
	return ptr;
}

void* xrealloc(void* ptr, size_t n)
{
	void* new_ptr = realloc(ptr, n);
		
	if (new_ptr == NULL)
		ERROR("realloc for %zu bytes failed", n);
		
	return new_ptr;
}

unsigned char* hex_string_to_hex_bytes(char* str)
{
	unsigned char*	bytes;
	unsigned int	tmp;
	int				i;
	
	bytes = xmalloc(SHA_DIGEST_LENGTH);
	
	for (i = 0; i < SHA_DIGEST_LENGTH; i++) {
		sscanf(str + (i * 2), "%02x", &tmp);
		bytes[i] = tmp;
	}
		
	return bytes;
}

void print_hash(unsigned char* hash)
{
	int	i;
	
	for (i = 0; i < SHA_DIGEST_LENGTH; i++)
		printf("%02x", hash[i]);		
}

/*
------
NODE_T
------
*/

node_t* node_copy(node_t* src)
{
	node_t*	dst = xmalloc(sizeof(node_t));
	dst->parent = src->parent;
	dst->left = src->left;
	dst->right = src->right;
	dst->hash = xmalloc(SHA_DIGEST_LENGTH);
	memcpy(dst->hash, src->hash, SHA_DIGEST_LENGTH);
	
	return dst;
}

node_t* node_sibling(node_t* node)
{
	if (node == NULL)
		ERROR("node == NULL");
		
	if (node->parent == NULL)
		ERROR("node->parent == NULL (i.e, node is root)");
		
	if (node->parent->left == node)
		return node->parent->right;
	else
		return node->parent->left;
}

char node_get_direction(node_t* node)
{
	if (node == NULL)
		ERROR("node == NULL");
		
	if (node->parent == NULL)
		return 'L';
		
	if (node->parent->left == node)
		return 'L';
	else
		return 'R';
}

void node_tree_free(node_t* root)
{
	free(root->hash);
	free(root);
}

/*
--------
VECTOR_T
--------
*/

vector_t* vector_new(void)
{
	vector_t*	vector = xmalloc(sizeof(vector_t));
	vector->nodes = xmalloc(sizeof(node_t*) * VECTOR_START_SIZE);
	vector->reserved = VECTOR_START_SIZE;
	vector->size = 0;
	
	return vector;
}

void vector_reallocate(vector_t* vector)
{
	if (vector->size < vector->reserved)
		return;
		
	vector->nodes = xrealloc(vector->nodes, sizeof(node_t*) * vector->reserved * VECTOR_GROW_SPEED);
	vector->reserved *= VECTOR_GROW_SPEED;	
}

void vector_insert_last(vector_t* vector, node_t* node)
{
	vector_reallocate(vector);
	vector->nodes[vector->size++] = node;
}

void vector_insert_last_malloc(vector_t* vector, node_t* src)
{	
	vector_reallocate(vector);
	vector->nodes[vector->size++] = node_copy(src);
}

size_t vector_size(vector_t* vector)
{
	return vector->size;
}

node_t* vector_front(vector_t* vector)
{
	return vector->nodes[0];
}

node_t* vector_back(vector_t* vector)
{
	return vector->nodes[vector->size - 1];
}

node_t* vector_get(vector_t* vector, size_t i)
{
	return vector->nodes[i];
}

void vector_free_shallow(vector_t* vector)
{
	free(vector->nodes);
	free(vector);
}

vector_t* vector_copy(vector_t* vector)
{
	vector_t*	copy = xmalloc(sizeof(vector_t));
	size_t		i;
	
	copy->nodes = xmalloc(sizeof(node_t*) * vector->size);
	
	for (i = 0; i < vector->size; i++)
		copy->nodes[i] = vector->nodes[i];
		
	copy->reserved = vector->size;
	copy->size = vector->size;
	
	return copy;
}

/*
****************
MERKLE FUNCTIONS
****************
*/

node_t* get_merkle_node_at_depth(vector_t* leaves, int i, int j)
{
	node_t*	node;
	node_t* correct;
	
	node = vector_get(leaves, i);
	correct = NULL;
	
	printf("node at index %d: ", i);
	print_hash(node->hash);
	printf("\n");
	
	while (g_leaf_depth > 0) {
		node = node_sibling(node);
		
		printf("path: %c", node_get_direction(node));
		print_hash(node->hash);
		printf("\n");
		
		if (g_leaf_depth == j)
			correct = node;	
		
		node = node->parent;
		g_leaf_depth--;
	}
	
	return correct;
}

node_t* build_merkle_tree(vector_t* leafs)
{
	vector_t*		parents;
	node_t*			parent;
	node_t*			left;
	node_t*			right;
	unsigned char*	concatenation;
	size_t			i;
	
	if (vector_size(leafs) <= 1) {
		parent = vector_front(leafs);
		vector_free_shallow(leafs);
		
		return parent;
	}
		
	if (vector_size(leafs) % 2 != 0)
		vector_insert_last_malloc(leafs, vector_back(leafs));
		
	parents = vector_new();
		
	for (i = 0; i < vector_size(leafs); i += 2) {
		parent = xmalloc(sizeof(node_t));
		left = vector_get(leafs, i);
		right = vector_get(leafs, i + 1);
		
		parent->left = left;
		parent->right = right;
		left->parent = parent;
		right->parent = parent;
		
		concatenation = xmalloc(SHA_DIGEST_LENGTH * 2);
		memcpy(concatenation, left->hash, SHA_DIGEST_LENGTH);
		memcpy(concatenation + SHA_DIGEST_LENGTH, right->hash, SHA_DIGEST_LENGTH);
		parent->hash = xmalloc(SHA_DIGEST_LENGTH);
		SHA1(concatenation, SHA_DIGEST_LENGTH * 2, parent->hash);
		
		vector_insert_last(parents, parent);
		free(concatenation);
	}
	
	vector_free_shallow(leafs);
	
	g_leaf_depth++;
	return build_merkle_tree(parents);
}

int main(void)
{
	vector_t*	leafs;
	node_t*		root;
	node_t*		node;
	char*		line;
	size_t		length;
	int			i;
	int			j;
	
	leafs = vector_new();
	line = NULL;
	length = 0;
	
	getline(&line, &length, stdin);
	i = strtol(line, NULL, 10);
	DEBUG("i: %d", i);
	
	getline(&line, &length, stdin);
	j = strtol(line, NULL, 10);
	DEBUG("j: %d", j);
	
	while (getline(&line, &length, stdin) != -1) {
		node_t* n = xmalloc(sizeof(node_t));
		n->left = NULL;
		n->right = NULL;
		n->parent = NULL;
		n->hash = hex_string_to_hex_bytes(line);
		
		vector_insert_last(leafs, n);
	}
	
	DEBUG("leafs: %zu", vector_size(leafs));	
	
	root = build_merkle_tree(vector_copy(leafs));
	DEBUG("depth: %d", g_leaf_depth);
	
	printf("root: ");
	print_hash(root->hash);
	printf("\n");
	
	node = get_merkle_node_at_depth(leafs, i, j);
	printf("depth node: ");
	print_hash(node->hash);
	printf("\n");
	
	printf("final result: %c", node_get_direction(node));
	print_hash(node->hash);
	print_hash(root->hash);
	printf("\n");
	
	vector_free_shallow(leafs);
	
	if (line != NULL)
		free(line);
		
	node_tree_free(root);
	
	return 0;
}