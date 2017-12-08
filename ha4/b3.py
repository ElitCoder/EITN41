import sys
import hashlib
import binascii
import math

def XOR(a, b):
	result = int(a, 16) ^ int(b, 16)
	return '{:x}'.format(result).zfill(min(len(a), len(b)))
	#return binascii.hexlify(''.join(chr(ord(c1) ^ ord(c2)) for c1, c2 in zip(binascii.unhexlify(a[-len(b):]), binascii.unhexlify(b))))
	'''
	b_a = binascii.unhexlify(a)
	b_b = binascii.unhexlify(b)
	
	print(b_a)
	print(b_b)
	
	print(len(a))
	print(len(b))
	print(len(b_a))
	print(len(b_b))
	'''
	#return str(binascii.hexlify(bytes(x ^ y for x, y in zip(binascii.unhexlify(a), binascii.unhexlify(b)))))[2:-1]

def OAEP_encode(M, seed):
	print("OAEP_ENCODE")
	
	correct_em = "0000255975c743f5f11ab5e450825d93b52a160aeef9d3778a18b7aa067f90b2178406fa1e1bf77f03f86629dd5607d11b9961707736c2d16e7c668b367890bc6ef1745396404ba7832b1cdfb0388ef601947fc0aff1fd2dcd279dabde9b10bfc51f40e13fb29ed5101dbcb044e6232e6371935c8347286db25c9ee20351ee82"
	l_hash = hashlib.sha1(binascii.unhexlify("")).hexdigest()
	h_len = 20
	k = 128
	ps_len = k - int(len(M) / 2) - 2 * h_len - 2
	ps = ""
	
	if ps_len > 0:
		ps = "00" * ps_len

	DB = l_hash + ps + "01" + M
	
	db_mask = MGF1(seed, k - h_len - 1)
	masked_db = XOR(db_mask, DB)
	seed_mask = MGF1(masked_db, h_len)
	masked_seed = XOR(seed, seed_mask)
	EM = "00" + masked_seed + masked_db
		
	print("Y :", "00")
	print("masked_seed :", masked_seed)
	print("masked_db :", masked_db)
		
	print("EM:", EM)

def OAEP_decode(EM):
	print("OAEP_DECODE")
	l_hash = hashlib.sha1(binascii.unhexlify("")).hexdigest()
	h_len = 20
	k = 128
	
	print(len(EM))
	
	Y = EM[2:]
	EM = EM[2:]
	masked_seed = EM[: h_len * 2]
	EM = EM[h_len * 2 :]
	masked_db = EM[: k * 2 - h_len * 2 - 1]
	
	print("Y :", Y)
	print("masked_seed :", masked_seed)
	print("masked_db :", masked_db)
	
	seed_mask = MGF1(masked_db, h_len)
	seed = XOR(masked_seed, seed_mask)
	db_mask = MGF1(seed, k - h_len - 1)
	DB = XOR(masked_db, db_mask)
	M = DB[h_len * 2 + DB[h_len * 2 :].find("01") + 2 :]
	
	print(M)

def i2osp(x, x_len):
	if x >= 256 ** x_len:
		print("integer too large")

	X = []
	ret = ""
	
	while x > 0:
		X.append(int(x % 256))
		x = x // 256
		
	left = x_len - len(X)
	
	for i in range(left):
		X.append(0)
	
	for d in X[::-1]:
		ret += "%0.2x" % d
		
	return ret

def MGF1(mgf_seed, mask_len):
	print("MGF1")
	
	h_len = 20
	mgf_seed_hash = hashlib.sha1(binascii.unhexlify(mgf_seed)).hexdigest()
	T = ""
	
#	print(mgf_seed)
	#print(len(mgf_seed))
	#print(mgf_seed_hash)
	#print(len(mgf_seed_hash))
	
	if mask_len > 2**32 * h_len:
		print("mask too long")
	
	for counter in range(math.ceil(mask_len / h_len)):
		#print(counter)
		C = i2osp(counter, 4)
		T += hashlib.sha1(binascii.unhexlify(mgf_seed + C)).hexdigest()
		
	return T[:mask_len * 2]
	#print(T[:mask_len * 2])
	
if __name__ == "__main__":
	#MGF1("12", 1)
	print(MGF1("9b4bdfb2c796f1c16d0c0772a5848b67457e87891dbc8214", 21))
	#OAEP_encode("fd5507e917ecbe833878", "1e652ec152d0bfcd65190ffc604c0933d0423381")
	OAEP_encode("c107782954829b34dc531c14b40e9ea482578f988b719497aa0687", "1e652ec152d0bfcd65190ffc604c0933d0423381")
	OAEP_decode("0063b462be5e84d382c86eb6725f70e59cd12c0060f9d3778a18b7aa067f90b2178406fa1e1bf77f03f86629dd5607d11b9961707736c2d16e7c668b367890bc6ef1745396404ba7832b1cdfb0388ef601947fc0aff1fd2dcd279dabde9b10bfc51efc06d40d25f96bd0f4c5d88f32c7d33dbc20f8a528b77f0c16a7b4dcdd8f")