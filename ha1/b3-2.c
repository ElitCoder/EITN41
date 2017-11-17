#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <openssl/sha.h>

#define MIN(a, b)	(a > b ? b : a)
#define error(...)	do { fprintf(stderr, "%s\n", __VA_ARGS__); exit(1); } while (0)

typedef struct node_t	node_t;

struct node_t {
	node_t*			left;	/*	Leaf child */
	node_t*			right;	/*	Right child */
	node_t*			parent;	/*	Parent node */
	unsigned char*	value;	/*	Hash value */
	int				d;		/*	Node depth */
	int				li;		/*	Leaf index */
};

typedef struct {
	void**	data;
	size_t	count;
	size_t	reserved;
} vector_t;

static int	g_max_d;

bool is_leaf(node_t* node)
{
	return node->left == NULL && node->right == NULL;
}

bool sibling_direction(node_t* node)
{
	if (node->parent == NULL)
		return false;
		
	if (node->parent->left == node)
		return false;
		
	return true;
}

node_t* get_sibling(node_t* node)
{
	if (node->parent == NULL)
		return NULL;
		
	if (node->parent->left == node)
		return node->parent->right;
		
	return node->parent->left;
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
	size_t	pot;
	
	n->parent = NULL;
	n->value = NULL;
	n->d = -1;
	n->li = -1;
	
	if (size <= 2) {
		n->left = nodes[0];
		n->right = size == 1 ? NULL : nodes[1];
		
		return n;
	}
	
	d = size / 2;
	pot = 1;
	
	while (pot < d)
		pot *= 2;
		
	if (pot > size)
		error("pot > size");
		
	d = pot;
		
	printf("dividing into %zu and %zu\n", d, size - d);
	
	n->left = construct_merkle_tree(nodes, d);
	n->right = construct_merkle_tree(nodes + d, size - d);
	
	return n;
}

int set_parents(node_t* node, node_t* parent, int d)
{
	int d_ret_left;
	int	d_ret_right;
	
	if (node == NULL)
		return g_max_d + 1;

	d_ret_left = set_parents(node->left, node, d + 1);
	d_ret_right = set_parents(node->right, node, d + 1);
	
	node->parent = parent;
	
	if (d > g_max_d)
		g_max_d = d;
	
	if (node->left == NULL && node->right == NULL) {
		node->d = g_max_d;
		printf("set leaf d to %d\n", node->d);
	} else {
		node->d = MIN(d_ret_right, d_ret_left);
		printf("set node to %d\n", node->d);
	}
	
	return node->d - 1;
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
	int				diff;
	
	if (root == NULL)
		return NULL;
	
	if (is_leaf(root))
		return root->value;
		
	left_val = get_merkle_root(root->left);
	right_val = get_merkle_root(root->right);
		
	if ((left_val == NULL || right_val == NULL) || (root->left->d != root->right->d)) {
		combined = malloc(SHA_DIGEST_LENGTH * 2);
		hash = malloc(SHA_DIGEST_LENGTH);
		
		if (left_val == NULL) {
			unsafe_strcat(combined, right_val, right_val, SHA_DIGEST_LENGTH * 2);
			SHA1(combined, SHA_DIGEST_LENGTH * 2, hash);
			
			left_val = malloc(SHA_DIGEST_LENGTH);
			memcpy(left_val, hash, SHA_DIGEST_LENGTH);
		} else if (right_val == NULL) {
			unsafe_strcat(combined, left_val, left_val, SHA_DIGEST_LENGTH * 2);
			SHA1(combined, SHA_DIGEST_LENGTH * 2, hash);
			
			right_val = malloc(SHA_DIGEST_LENGTH);
			memcpy(right_val, hash, SHA_DIGEST_LENGTH);
		} else {			
			if (root->left->d < root->right->d) {
				//error("should not happen");
				node_t*	croot = root;
				diff = root->right->d - root->left->d;
				
				assert(diff == 1);
				
				while (diff > 0) {
					printf("loops\n");
					unsafe_strcat(combined, right_val, right_val, SHA_DIGEST_LENGTH * 2);
					SHA1(combined, SHA_DIGEST_LENGTH * 2, hash);
					
					unsigned char* old_right_val = right_val;
					right_val = malloc(SHA_DIGEST_LENGTH);
					//right_val = malloc(SHA_DIGEST_LENGTH);
					memcpy(right_val, hash, SHA_DIGEST_LENGTH);
					diff--;
					
					assert(croot->li == -1);
					
					/*
					root->right = parent;
					parent->parent = root;
					*/
					
					node_t* parent = malloc(sizeof(node_t));
					parent->left = croot->right;
					parent->right = /*sibling*/NULL;
					parent->parent = root;
					parent->value = right_val;
					parent->d = croot->right->d - 1;
					parent->li = -1;
					
					node_t*	sibling = malloc(sizeof(node_t));
					sibling->left = NULL;
					sibling->right = NULL;
					sibling->parent = parent;
					sibling->value = old_right_val;
					sibling->d = croot->right->d;
					sibling->li = -1;
					
					//assert(croot->parent != NULL);
					
					/*
					if (croot->parent->left == croot)
						croot->parent->left = parent;
					else
						croot->parent->right = parent;
					
					croot->parent = parent;
					*/
					parent->right = sibling;
					croot->right = parent;
					//croot = croot->parent;
				}
			} else {
				error("should not happen");
				node_t*	croot = root;
				diff = root->left->d - root->right->d;
				
				while (diff > 0) {
					unsafe_strcat(combined, left_val, left_val, SHA_DIGEST_LENGTH * 2);
					SHA1(combined, SHA_DIGEST_LENGTH * 2, hash);
					
					left_val = malloc(SHA_DIGEST_LENGTH);
					memcpy(left_val, hash, SHA_DIGEST_LENGTH);
					diff--;
					
					assert(croot->li == -1);
					
					node_t* parent = malloc(sizeof(node_t));
					parent->left = root;
					parent->right = /*sibling*/NULL;
					parent->parent = croot->parent;
					parent->value = left_val;
					parent->d = croot->d - 1;
					parent->li = -1;
					
					node_t*	sibling = malloc(sizeof(node_t));
					sibling->left = NULL;
					sibling->right = NULL;
					sibling->parent = parent;
					sibling->value = left_val;
					sibling->d = croot->d;
					sibling->li = -1;
					
					if (croot->parent->left == croot)
						croot->parent->left = parent;
					else
						croot->parent->right = parent;
					
					croot->parent = parent;
					parent->right = sibling;
					
					croot = croot->parent;
				}
			}
		}
		
		free(combined);
		free(hash);
	}
		
	combined = malloc(SHA_DIGEST_LENGTH * 2);
	hash = malloc(SHA_DIGEST_LENGTH);
	unsafe_strcat(combined, left_val, right_val, SHA_DIGEST_LENGTH * 2);
	SHA1(combined, SHA_DIGEST_LENGTH * 2, hash);
	
	printf("gives: ");
	for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
		printf("%02x", hash[i]);
	printf("\n");

/*	
	root->value = hash;
	
	return root->value;
	*/
	
	return hash;
}

unsigned char* get_merkle_depth_node(node_t** nodes, int i, int j)
{
	node_t*			n = nodes[i];
	node_t*			p;
	unsigned char	app = 'L';
	unsigned char*	ret;
	
	printf("i %d, j %d %d %02x\n", i, j, n->d, n->value[0]);
	
	while (n->d > j) {
		p = n;
		printf("%d\n", n->d);
		
		n = get_sibling(n);
		
		if (n == NULL)
			break;
		
		for (int k = 0; k < SHA_DIGEST_LENGTH; k++)
			printf("%02x", n->value[k]);
		printf("\n");
		
		n = n->parent;
		
		if (n == NULL)
			error("is null");
			
		if (n == nodes[i]->parent)
			continue;
			
		for (int k = 0; k < SHA_DIGEST_LENGTH; k++)
			printf("%02x", n->value[k]);
		printf("\n");
	}
		
	printf("after loop\n");
		
	if (sibling_direction(p))
		app = 'R';
		
	ret = malloc(SHA_DIGEST_LENGTH + 1);
	ret[0] = app;
	memcpy(ret + 1, p->value, SHA_DIGEST_LENGTH);
	
	return ret;
}

int main(void)
{
	int				i;
	int				j;
	int				li;
	char*			tmp = NULL;
	size_t			size;
	vector_t*		lines = new_vector();
	vector_t*		nodes = new_vector();
	node_t*			n;
	node_t*			root;
	unsigned char*	merkle_root;
	unsigned char*	merkle_path;
	int				k;
	
	getline(&tmp, &size, stdin);
	i = strtol(tmp, NULL, 10);
	
	getline(&tmp, &size, stdin);
	j = strtol(tmp, NULL, 10);
	
	li = 0;
	while (getline(&tmp, &size, stdin) != -1) {
		n = malloc(sizeof(node_t));
		n->left = NULL;
		n->right = NULL;
		n->parent = NULL;
		n->value = to_bytes(tmp);
		n->li = li;
				
		insert_last(nodes, n);
		li++;
	}
	
	printf("count(nodes) = %zu\n", count(nodes));
	
	root = construct_merkle_tree((node_t**)get_data(nodes), count(nodes));
	set_parents(root, NULL, 1);
	
	merkle_root = get_merkle_root(root);
	merkle_path = get_merkle_depth_node((node_t**)get_data(nodes), i, j);
	
	printf("merkle root: ");
	for (k = 0; k < SHA_DIGEST_LENGTH; k++)
		printf("%02x", merkle_root[k]);
	printf("\n");
	printf("merkle depth node for (i = %d, j = %d): ", i, j);
	printf("%c", merkle_path[0]);
	for (k = 1; k < SHA_DIGEST_LENGTH; k++)
		printf("%02x", merkle_path[k]);
	for (k = 0; k < SHA_DIGEST_LENGTH; k++)
		printf("%02x", merkle_root[k]);
	printf("\n");
	
	free(tmp);
	free_vector(lines);
	free_vector(nodes);
	
	return 0;
}