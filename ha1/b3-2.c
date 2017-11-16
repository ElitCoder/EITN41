#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <openssl/sha.h>

#define error(...)	do { fprintf(stderr, "%s\n", __VA_ARGS__); exit(1); } while (0)

typedef struct node_t	node_t;

struct node_t {
	node_t*			left;
	node_t*			right;
	node_t*			parent;
	unsigned char*	value;
};

typedef struct {
	void**	data;
	size_t	count;
	size_t	reserved;
} vector_t;

bool is_leaf(node_t* node)
{
	return node->left == NULL && node->right == NULL;
}

vector_t* new_vector(void)
{
	vector_t*	vector = malloc(sizeof(vector_t));
	
	if (vector == NULL)
		error("malloc failed\n");
		
	vector->count = 0;
	vector->reserved = 1;
	vector->data = malloc(vector->reserved * sizeof(void*));
	
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
	void**	new_data = realloc(vector->data, vector->reserved * 2 * sizeof(void*));
	
	if (new_data == NULL)
		error("realloc failed\n");
		
	vector->data = new_data;
	vector->reserved *= 2;
}

void insert_last(vector_t* vector, void* node)
{
	if (vector->count >= vector->reserved)
		allocate_vector(vector);
		
	vector->data[vector->count++] = node;
}

void** get_data(vector_t* vector)
{
	return vector->data;
}

size_t count(vector_t* vector)
{
	return vector->count;
}

unsigned char* to_bytes(char* str)
{
	unsigned char*	bytes = malloc(SHA_DIGEST_LENGTH);
	unsigned int	tmp;
	int				i;
	
	for (i = 0; i < SHA_DIGEST_LENGTH; i++) {
		sscanf(str + (i * 2), "%02x", &tmp);
		bytes[i] = tmp;
	}
		
	return bytes;
}

node_t* construct_merkle_tree(node_t** nodes, size_t size)
{
	size_t	d;
	node_t*	n = malloc(sizeof(node_t));
	
	n->parent = NULL;
	n->value = NULL;
	
	if (size <= 2) {
		n->left = nodes[0];
		n->right = size == 1 ? nodes[0] : nodes[1];
		
		return n;
	}
	
	d = size % 2 == 0 ? size / 2 : size / 2 + 1;
	
	n->left = construct_merkle_tree(nodes, d);
	n->right = construct_merkle_tree(nodes + d, size - d);
	
	return n;
}

void set_parents(node_t* node, node_t* parent)
{
	if (node == NULL)
		return;
		
	set_parents(node->left, node);
	set_parents(node->right, node);
	
	node->parent = parent;
}

unsigned char* unsafe_strcat(unsigned char* dst, const unsigned char* fst, const unsigned char* snd, size_t len)
{
	size_t	i;
	
	for (i = 0; i < len / 2; i++)
		dst[i] = fst[i];
		
	for (; i < len; i++)
		dst[i] = snd[i - len / 2];
}

unsigned char* get_merkle_root(node_t* root)
{
	unsigned char*	left_val;
	unsigned char*	right_val;
	unsigned char*	combined;
	unsigned char*	hash;
	
	if (is_leaf(root))
		return root->value;
		
	left_val = get_merkle_root(root->left);
	right_val = get_merkle_root(root->right);
	
	combined = malloc(SHA_DIGEST_LENGTH * 2);
	hash = malloc(SHA_DIGEST_LENGTH);
	unsafe_strcat(combined, left_val, right_val, SHA_DIGEST_LENGTH * 2);
	SHA1(combined, SHA_DIGEST_LENGTH * 2, hash);
	
	printf("hash of:");
	for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
		printf("%02x", left_val[i]);
		
	printf("\n");
	printf("and: ");
	for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
		printf("%02x", right_val[i]);
		
	printf("\n");
	printf("gives: ");
	
	for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
		printf("%02x", hash[i]);
		
	printf("\n");
	
	return hash;
}

int main(void)
{
	int				i;
	int				j;
	char*			tmp = NULL;
	size_t			size;
	vector_t*		lines = new_vector();
	vector_t*		nodes = new_vector();
	node_t*			n;
	node_t*			root;
	unsigned char*	merkle_root;
	int				k;
	
	getline(&tmp, &size, stdin);
	i = strtol(tmp, NULL, 10);
	
	getline(&tmp, &size, stdin);
	j = strtol(tmp, NULL, 10);
	
	while (getline(&tmp, &size, stdin) != -1) {
		n = malloc(sizeof(node_t));
		n->left = NULL;
		n->right = NULL;
		n->parent = NULL;
		n->value = to_bytes(tmp);
				
		insert_last(nodes, n);
	}
	
	root = construct_merkle_tree((node_t**)get_data(nodes), count(nodes));
	set_parents(root, NULL);
	
	merkle_root = get_merkle_root(root);
	
	printf("merkle root: ");
	
	for (k = 0; k < SHA_DIGEST_LENGTH; k++)
		printf("%02x ", merkle_root[k]);
		
	printf("\n");
	
	free(tmp);
	free_vector(lines);
	free_vector(nodes);
	
	return 0;
}