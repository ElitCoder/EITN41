#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/sha.h>

unsigned char* sha1(unsigned char* d, size_t len)
{
	unsigned char*	hash = malloc(SHA_DIGEST_LENGTH);

	SHA1(d, len, hash);
	
	return hash;
}

unsigned char* int_to_bytes(int n)
{
	unsigned char*	bytes = malloc(sizeof(int));
	
	assert(sizeof(int) == 4);
	
	bytes[0] = (n >> 24) & 0xFF;
	bytes[1] = (n >> 16) & 0xFF;
	bytes[2] = (n >> 8) & 0xFF;
	bytes[3] = n & 0xFF;
	
	return bytes;
}

unsigned char* bytes_to_string(unsigned char* b, size_t len)
{
	unsigned char*	string = malloc(len * 2 + 1);
	size_t			i;
	
	for (i = 0; i < len; i++)
		sprintf((char*)(string + i * 2), "%02x", b[i]);
	
	string[len * 2] = '\0';
	
	return string;
}

unsigned char* string_to_bytes(unsigned char* s, size_t len)
{
	unsigned char*	b;
	size_t			i;
	
	assert(len % 2 == 0);
	
	b = malloc(len / 2);
	
	for (i = 0; i < len / 2; i++)
		sscanf((const char*)(s + (i * 2)), "%02x", &b[i]);
		
	return b;
}

int main(void)
{
	char*			hex_chars = "fedcba9876543210";
	unsigned char*	bytes;
	unsigned char*	byte_string;
	unsigned char*	from_string;
	unsigned char*	hash;
	unsigned char*	hash_string;
	unsigned char*	eight_to_bytes;
	size_t			i;
	
	printf("int_to_bytes\n");
	bytes = int_to_bytes(200);
	printf("%02x %02x %02x %02x\n", bytes[0], bytes[1], bytes[2], bytes[3]);
	
	printf("bytes_to_string\n");
	byte_string = bytes_to_string(bytes, sizeof(int));
	printf("%s\n", byte_string);
	
	printf("from string\n");
	from_string = string_to_bytes(byte_string, strlen((const char*)byte_string));
	
	for (i = 0; i < strlen((const char*)byte_string) / 2; i++)
		printf("%02x ", from_string[i]);
		
	printf("\n");
	
	printf("to hash\n");
	hash = sha1(bytes, 4);
	
	for (i = 0; i < SHA_DIGEST_LENGTH; i++)
		printf("%02x ", hash[i]);
		
	printf("\n");
	
	printf("hash to string\n");
	hash_string = bytes_to_string(hash, 20);
	printf("%s\n", hash_string);
	
	assert(strlen(hash_string) == 40);
	
	printf("8 bytes\n\n");
	eight_to_bytes = string_to_bytes(hex_chars, strlen(hex_chars));
	
	for (i = 0; i < strlen((const char*)hex_chars) / 2; i++)
		printf("%02x ", eight_to_bytes[i]);
		
	printf("\n");
		
	free(bytes);
	free(byte_string);
	free(from_string);
	
	return 0;
}