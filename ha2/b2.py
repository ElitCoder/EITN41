import sys
import struct
import socket
from pcapfile import savefile

# taken from https://stackoverflow.com/questions/5619685/conversion-from-ip-string-to-integer-and-backward-in-python
def ip2int(addr):                                                               
    return struct.unpack("!I", socket.inet_aton(addr))[0] 

def set_disjoint(all_sets, current_set):
	for x in all_sets:
		#print(x)
		#print(current_set)
		if not set(x).isdisjoint(current_set):
			return False
		
	return True

def exclude(all_sets, current_set):
	change_index = 0
	matches = 0
	i = 0
	
	while i < len(all_sets):
		x = all_sets[i]
		
		#if x == current_set:
		#	i += 1
		#	continue
		
		#if len(set(x).intersection(current_set)) > 0:
		if len(set(current_set).intersection(x)) > 0:
			if matches > 0:
				matches += 1
			else:
				matches = 1
				change_index = i
		'''
		if not set(x).isdisjoint(current_set):
			if matches > 0:
				matches += 1
			else:
				change_index = i
		'''
		i += 1
		
	if matches > 1:
		return (False, change_index)
	elif matches == 0:
		return (False, change_index)
	else:
		return (True, change_index)
	
def check_terminate(all_sets):
	for x in all_sets:
		if len(x) > 1 or len == 0:
			print("size was ", len(x), " instead")
			return False
		
	print("terminate was good!")
	print(all_sets)
	
	return True

def main():
	anip = sys.argv[1]
	mip = sys.argv[2]
	n = int(sys.argv[3])
	all_sets = []
	current_set = []
	found = False
	adding = False
	learning_done = False
	last_ip = ""
	
	capio = open("pcap.pcap", 'rb')
	capfile = savefile.load_savefile(capio, layers=2, verbose=True)
	
	i = 0
	while i < len(capfile.packets):
		pkt = capfile.packets[i]
		
		ip_src = pkt.packet.payload.src.decode('UTF8')
		ip_dst = pkt.packet.payload.dst.decode('UTF8')
		
		if last_ip == mip and not (ip_src == mip):
			if len(all_sets) >= n and found:
				(result, index) = exclude(all_sets, current_set)
				
				if result:
					check_terminate(all_sets)
					print("edit index %d to intersection i = %d" % (index, i))
					print("len current set %d all_sets[index] %d intersection %d" % (len(current_set), len(all_sets[index]), len(set(current_set).intersection(all_sets[index]))))
					
					if len(set(current_set).intersection(all_sets[index])) != 0:
						print()		
						#print("should change index %d to new when len is %d" % (index, len(all_sets)))	
						all_sets[index] = set(current_set).intersection(all_sets[index])
						if check_terminate(all_sets):
							break
					
				found = False
				current_set = []
					
				# learning phase done
			else:
				if found and len(current_set) > 0 and set_disjoint(all_sets, current_set):
					all_sets.append(current_set)
				found = False
				current_set = []		
		
		if ip_src == anip:
			found = True
			
		if ip_src == mip:
			current_set.append(ip_dst)
			
		last_ip = ip_src
		i += 1
		
		if i == len(capfile.packets):
			i = 0
	
	#if len(current_set) > 0:
	#	all_sets.append(current_set)
		
	print(i)
		
	if check_terminate(all_sets):
		print("TERMINATED")
		
	ip_sum = 0
	for x in all_sets:
		#print(x.pop())
		ip_sum += ip2int(x.pop())
		
	print("FINAL:", ip_sum)
	
	# for test ip2int
	#print(ip2int("85.14.156.21") + ip2int("47.56.124.15") + ip2int("210.25.145.218"))
			
if __name__ == "__main__":
	main()