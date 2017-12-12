from subprocess import check_output

def main():
	rsa_ok_str = ["openssl", "rsa", "-check", "-in", "censored_key.pem"]
	rsa_ok = check_output(rsa_ok_str).decode('utf-8')
	
	if rsa_ok.find("n does not equal p q") == -1: # key ok
		return None
	else:
		print("n != p * q <- need to recalculate key")
	
	asn1_str = ["openssl", "asn1parse", "-in", "censored_key.pem"]
	asn1_ok = check_output(asn1_str).decode('utf-8')
	
	asn1_values = [s for s in asn1_ok.strip().split("\n") if s.find("INTEGER") > 0]
	asn1_hex_values = [s.split(":")[-1] for s in asn1_values]
	
	#print(asn1_hex_values)
	
	# list is
	# 0: version number
	# 1: mod
	# 2: public e
	# 3: private d
	# 4: prime p
	# 5: prime q
	# 6: exp d mod (p - 1)
	# 7: exp d mod (q - 1)
	# 8: co-eff
	
	p = int(asn1_hex_values[4], 16)
	q = int(asn1_hex_values[5], 16)
	n_old = int(asn1_hex_values[1], 16)
	e = 65537
	
	if int(asn1_hex_values[2], 16) != e:
		print("e mismatch, using 65537 instead")
	
	print("p: ")
	print(p)
	print("\nq:")
	print(q)
	print("\nn_old:")
	print(n_old)
	print("\nn_new:")
	print(p * q)
	print()
	
	if n_old != p * q:
		print("mod mismatch!")
		
	ver = int(asn1_hex_values[0], 16)
	n_new = p * q
	d = int(asn1_hex_values[3], 16)
	e1 = int(asn1_hex_values[6], 16)
	e2 = int(asn1_hex_values[7], 16)
	coeff = int(asn1_hex_values[8], 16)
		
	new_asn_file = open("new_asn", "w")
	new_asn_file.write("asn1=SEQUENCE:RSAKEY\n\n")
	new_asn_file.write("[RSAKEY]\n")
	#new_asn_file.write("RSAKEY ::= SEQUENCE {\n")
	new_asn_file.write("version=INTEGER:" + str(ver) + "\n")
	new_asn_file.write("modulus=INTEGER:" + str(n_new) + "\n")
	new_asn_file.write("pubExp=INTEGER:" + str(e) + "\n")
	new_asn_file.write("privExp=INTEGER:" + str(d) + "\n")
	new_asn_file.write("p=INTEGER:" + str(p) + "\n")
	new_asn_file.write("q=INTEGER:" + str(q) + "\n")
	new_asn_file.write("e1=INTEGER:" + str(e1) + "\n")
	new_asn_file.write("e2=INTEGER:" + str(e2) + "\n")
	new_asn_file.write("coeff=INTEGER:" + str(coeff))# + "\n}")
	
	new_asn_file.close()
	
	der_str = ["openssl", "asn1parse", "-genconf", "new_asn", "-out", "new_der"]
	der_ok = check_output(der_str)
	
	rsa_new_str = ["openssl", "rsa", "-in", "new_der", "-inform", "DER", "-text", "-check"]
	rsa_new_ok = check_output(rsa_new_str).decode('utf-8')
	
	print(der_ok)
	print(rsa_new_ok)
	
	rsa_split = rsa_new_ok.split("-----BEGIN RSA PRIVATE KEY-----")
	rsa_key = rsa_split[-1].split("-----END RSA PRIVATE KEY-----")[0]
	
	print(rsa_key)
	
	rsa_key_file = open("fixed_rsa.pem", "w")
	rsa_key_file.write("-----BEGIN RSA PRIVATE KEY-----")
	rsa_key_file.write(rsa_key)
	rsa_key_file.write("-----END RSA PRIVATE KEY-----")
	rsa_key_file.close()
	
	#message = "Qe7+h9OPQ7PN9CmF0ZOmD32fwpJotrUL67zxdRvhBn2U3fDtoz4iUGRXNOxwUXdJ2Cmz7zjS0DE8ST5dozBysByz/u1H//iAN+QeGlFVaS1Ee5a/TZilrTCbGPWxfNY4vRXHP6CB82QxhMjQ7/x90/+JLrhdAO99lvmdNetGZjY="
	
	message_dump_str = "openssl base64 -d -in message > message.bin"#["openssl", "base64", "-d", "-in", "message", ">", "message.bin"]
	message_dump_ok = check_output(message_dump_str, shell=True)
	
	message_decrypt_str = "openssl rsautl -decrypt -in message.bin -inkey fixed_rsa.pem"
	message_decrypt_ok = check_output(message_decrypt_str, shell=True).decode('utf-8')
	
	print("raw:", message_decrypt_ok)
	print("stripped:", message_decrypt_ok.strip())
	
if __name__ == "__main__":
	main()