import struct
import binascii
import hashlib

def int_to_bytes(num):
	return struct.pack(">I", num)

def bytes_to_string(b):
	return binascii.hexlify(b)

def sha1(b):
	return hashlib.sha1(b).hexdigest()

def main():	
	print bytes_to_string(int_to_bytes(201))
	print sha1(int_to_bytes(201))

if __name__ == "__main__":
	main()