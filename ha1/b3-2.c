#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <openssl/sha.h>

#define MIN(a, b)	(a > b ? b : a)
#define error(...)	do { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); exit(1); } while (0)

typedef struct node_t	node_t;

struct node_t {
	node_t*			left;		/*	Leaf child */
	node_t*			right;		/*	Right child */
	node_t*			parent;		/*	Parent node */
	unsigned char*	value;		/*	Hash value */
	int				d;			/*	Node depth */
	int				li;			/*	Leaf index */
	bool			copy_value;	/*	If the node is inserted to complete the tree */
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

/*
	true	- right sibling
	false	- left sibling
*/
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

bool insert_intermediate_nodes(node_t* root)
{
	int d_left;
	int d_right;
	
	if (root->left == NULL && root->right == NULL) {
		// it's a leaf
		return false;
	} else if (root->left == NULL) {
		// missing left sibling
		error("insert_intermediate_nodes: root->left == NULL not implemented");
	} else if(root->right == NULL) {
		// missing right sibling
		error("insert_intermediate_nodes: root->right == NULL not implemented");
	}
	
	insert_intermediate_nodes(root->left);
	insert_intermediate_nodes(root->right);
	
	d_left = root->left->d;
	d_right = root->right->d;
		
	if (d_right > d_left) {
		node_t*	new_parent = malloc(sizeof(node_t));
		new_parent->left = root->right;
		new_parent->right = NULL /* NEW SIBLING */;
		new_parent->parent = root;
		new_parent->value = NULL;//malloc(SHA_DIGEST_LENGTH); // hash of left & new right
		new_parent->d = d_right - 1;
		new_parent->li = -1;
		new_parent->copy_value = false;
		
		node_t*	new_sibling = malloc(sizeof(node_t));
		new_sibling->left = NULL;
		new_sibling->right = NULL;
		new_sibling->parent = new_parent;
		new_sibling->value = NULL;//malloc(SHA_DIGEST_LENGTH); // value of root->right
		new_sibling->d = d_right;
		new_sibling->li = -1;
		new_sibling->copy_value = true;
		
		new_parent->right = new_sibling;
		root->right->parent = new_parent;
		root->right = new_parent;
		
		printf("ran correctly with %d %d\n", d_left, d_right);
		
		// needs to insert intermediate node with copy of sibling and parent
		// with sha1 of both nodes
		return true;
	} else if (d_left > d_right) {
		// should not happen
		error("insert_intermediate_nodes: d_right < d_left %d %d", d_right, d_left);
	}
	
	return false;
}

int set_parents(node_t* node, node_t* parent, int d)
{
	int d_ret_left;
	int	d_ret_right;
	
	if (node == NULL)
		return g_max_d + 1;
		
	node->copy_value = false;

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

void unsafe_strcat(unsigned char* dst, const unsigned char* fst, const unsigned char* snd, size_t len)
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
		
	if (root->left->copy_value)
		error("get_merkle_root: root->left->copy_value should not be true");
		
	left_val = get_merkle_root(root->left);
	
	if (root->right->copy_value) {
		right_val = malloc(SHA_DIGEST_LENGTH);
		memcpy(right_val, left_val, SHA_DIGEST_LENGTH);
		root->right->value = right_val;
	} else {
		right_val = get_merkle_root(root->right);
	}
	
	combined = malloc(SHA_DIGEST_LENGTH * 2);
	hash = malloc(SHA_DIGEST_LENGTH);
	unsafe_strcat(combined, left_val, right_val, SHA_DIGEST_LENGTH * 2);
	SHA1(combined, SHA_DIGEST_LENGTH * 2, hash);
	
	root->value = hash;
	
	printf("hash: ");
	for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
		printf("%02x", hash[i]);
	printf("\n");
	
	free(combined);
	return root->value;
}

unsigned char* get_merkle_depth_node(node_t** nodes, int i, int j)
{
	unsigned char*	resulting_hash;
	node_t*			n;
	node_t*			p;
	
	n = nodes[i];
	p = NULL;
	
	while (n && n->d > j) {
		printf("%d %d\n", n->d, j);
		n = get_sibling(n);
		p = n;
		
		for (int k = 0; k < SHA_DIGEST_LENGTH; k++)
			printf("%02x", n->value[k]);
		printf("\n");
		
		n = n->parent;
	}
	
	n = p;
	
	resulting_hash = malloc(SHA_DIGEST_LENGTH + 1);
	resulting_hash[0] = sibling_direction(n) ? 'R' : 'L';
	memcpy(resulting_hash + 1, n->value, SHA_DIGEST_LENGTH);
	
	return resulting_hash;
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
	
	while (insert_intermediate_nodes(root))
		;
	
	merkle_root = get_merkle_root(root);
	merkle_path = get_merkle_depth_node((node_t**)get_data(nodes), i, j);
	
	printf("merkle root: ");
	for (k = 0; k < SHA_DIGEST_LENGTH; k++)
		printf("%02x", merkle_root[k]);
	printf("\n");
	printf("merkle depth node for (i = %d, j = %d): ", i, j);
	printf("%c", merkle_path[0]);
	for (k = 1; k < SHA_DIGEST_LENGTH + 1; k++)
		printf("%02x", merkle_path[k]);
	for (k = 0; k < SHA_DIGEST_LENGTH; k++)
		printf("%02x", merkle_root[k]);
	printf("\n");
	
	free(tmp);
	free_vector(lines);
	free_vector(nodes);
	
	return 0;
}