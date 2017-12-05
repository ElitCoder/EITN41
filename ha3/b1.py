import random
import hashlib
import struct
import binascii
import sys
from joblib import Parallel, delayed
import time


right_answer = ''

def int_to_bytes(num):
	return struct.pack(">I", num)

def main2():
	X =	int(sys.argv[1])
	
	k = bin(random.getrandbits(16))[2:].zfill(16)
	v = bin(0)
	vplus = bin(1)
	
	#print(v, k)
	#print(int((v + k[2:])[2:], 2))
	#print(int((v + k[2:]), 2))
	#print(type(k))
	right_answer = hashlib.sha1(int_to_bytes(int(v + k[2:], 2))).hexdigest()
	#print(hashlib.sha1(int_to_bytes(int(vplus + k[2:], 2))).hexdigest())
	
	right_answer = v[2:] + k
	right_answer = right_answer.zfill(24)
	right_answer = int(right_answer, 2).to_bytes(3, byteorder='big')
	
	right_hash = hashlib.sha1(right_answer).hexdigest()
	'''
	print(right_answer)
	print(len(right_answer))
	print(right_hash)
	'''
	i = 0
	collisions = 0
	
	for i in range(2**16 - 1):
		a = vplus[2:] + bin(i)[2:].zfill(16)
		a = a.zfill(24)
		a = int(a, 2).to_bytes(3, byteorder='big')
		
		#print(a)
		#print(len(a))
		
		test_answer = hashlib.sha1(a).hexdigest()
		
		#print(test_answer)
		
		#if i == 2:
		#	break
		
		test_binary = bin(int(test_answer, 16))[2:].zfill(160)
		right_binary = bin(int(right_hash, 16))[2:].zfill(160)
	
		'''
		print(test_answer)
		print(test_binary)
		print(right_hash)
		print(right_binary)	
		'''
		
		if test_binary[:X] == right_binary[:X]:
			print(bin(i)[2:].zfill(16))
			print("found collision")
			collisions += 1
	
	#print("found", collisions, "collisions")
	
	return collisions
	
if __name__ == "__main__":
	nbr_collisions = 0
	timestamp = time.time()
	res = Parallel(n_jobs=4)(delayed(main2)() for i in range(10000))
	print(time.time() - timestamp, "s")
	print(res)
	print("collisions", sum(res) / 10000)
	print(len(res))
	
	#for i in range(5000):
	#	print(i)
	#	nbr_collisions += main2()
	
	#nbr_collisions /= 5000	