import sys
from binascii import hexlify, unhexlify

'''
depending on b, we run

0: (sa ^ sb) + (sa ^ sb ^ da ^ db)
1: (sa ^ sb) ^ m
'''

# calculates our broadcast message by going bit by bit
def get_broadcast(sa, da):
	ma = ''
	
	for i in range(0, 16):
		ma += str(int(sa[i]) ^ int(da[i]))
		
	return ma
	
# run the parallel dc to get the anon message
def run(sa, da, sb, db, m, b):
	if b:
		return (sa ^ sb) ^ m
	else:
		return da ^ db ^ (sa ^ sb)

def main():
	sa = bin(int(sys.argv[1], 16))[2:].zfill(16)
	sb = bin(int(sys.argv[2], 16))[2:].zfill(16)
	da = bin(int(sys.argv[3], 16))[2:].zfill(16)
	db = bin(int(sys.argv[4], 16))[2:].zfill(16)
	m = bin(int(sys.argv[5], 16))[2:].zfill(16)
	b = int(sys.argv[6])
	
	final = ''
	broadcast = get_broadcast(sa, sb)
	
	if b:
		for i in range(0, 16):
			final += str(run(int(sa[i]), int(da[i]), int(sb[i]), int(db[i]), int(m[i]), b))
	
		print(hex(int(final, 2))[2:].zfill(4))
	else:
		final += broadcast
		
		for i in range(0, 16):
			final += str(run(int(sa[i]), int(da[i]), int(sb[i]), int(db[i]), int(m[i]), b))
	
		print(hex(int(final, 2))[2:].zfill(8))
	
if __name__ == "__main__":
	main()