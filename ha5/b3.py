import struct
import binascii
import math
import base64

def int_to_bytes(num):
	return num.to_bytes((num.bit_length() + 7) // 8, byteorder='big')

def bytes_to_str(bytes):
	return str(binascii.hexlify(bytes))[2:-1]

def long_def_form(num_hex):
	len_bytes = int((len(num_hex) / 2) // 256)
	first = int(128 + 1 + len_bytes)
	fill_to = (len_bytes + 1) * 2
	len_hex = bytes_to_str(int_to_bytes(len(num_hex) // 2)).lstrip("0").zfill(fill_to)
	first_hex = bytes_to_str(int_to_bytes(first))
		
	return "02" + first_hex + len_hex + num_hex

def der(num):
	if num < 0:
		return "num < 0 not supported"

	num_hex = bytes_to_str(int_to_bytes(num)).zfill(2)
	
	if len(bin(num)[2:]) % 8 == 0:
		num_hex = "00" + num_hex
		
	if len(num_hex) > 127:
		return long_def_form(num_hex)
	
	#print(len(num_hex) // 2)
	
	len_hex = bytes_to_str(int_to_bytes(len(num_hex) // 2)).lstrip("0").zfill(2)
		
	return "02" + len_hex + num_hex

# from stack overflow
def egcd(a, b):
    if a == 0:
        return (b, 0, 1)
    g, y, x = egcd(b%a,a)
    return (g, x - (b//a) * y, y)

def calc_d(e, p, q):
	g, x, y = egcd(e, (p - 1) * (q - 1))
	
	return x % ((p - 1) * (q - 1))
	
def calc_coeff(q, p):
	g, x, y = egcd(q, p)
	
	return x % p

def base64_der_rsa(p, q):
	e = 65537
	n = p * q
	d = calc_d(e, p, q)
	e1 = d % (p - 1)
	e2 = d % (q - 1)
	coeff = calc_coeff(q, p)
	
	total_key = der(0) + der(n) + der(e) + der(d) + der(p) + der(q) + der(e1) + der(e2) + der(coeff)
	length = bytes_to_str(int_to_bytes(len(total_key) // 2))

	print(length)

	if len(length) > 2:
		fill_to = (len(length) // 2)
		long_def = 128 + (len(length) // 2)
		length = bytes_to_str(int_to_bytes(long_def)).zfill(2) + length.zfill(fill_to)
		
	print(length)
	return "30" + length + total_key

def main():
	e = 2530368937
	l = 161863091426469985001358176493540241719547661391527305133576978132107887717901972545655469921112454527920502763568908799229786534949082469136818503316047702610019730504769581772016806386178260077157969035841180863069299401978140025225333279044855057641079117234814239380100022886557142183337228046784055073741
	
	#print(short_def_form(e))
	#print(short_def_form(l))
	
	#der(123)
	#print(der(l))
	
	p = 139721121696950524826588106850589277149201407609721772094240512732263435522747938311240453050931930261483801083660740974606647762343797901776568952627044034430252415109426271529273025919247232149498325412099418785867055970264559033471714066901728022294156913563009971882292507967574638004022912842160046962763
	q = 141482624370070397331659016840167171669762175617573550670131965177212458081250216130985545188965601581445995499595853199665045326236858265192627970970480636850683227427420000655754305398076045013588894161738893242561531526805416653594689480170103763171879023351810966896841177322118521251310975456956247827719
	
	key = base64_der_rsa(p, q)
	key_64 = base64.b64encode(binascii.unhexlify(key))
	print("rsa:", key)
	print()
	print("base64:", str(key_64)[2:-1])
	
if __name__ == "__main__":
	main()