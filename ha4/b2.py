import requests

def test(url):
	response = requests.get(url, verify=False)
	time = response.elapsed.total_seconds()
	
	if response.text.strip() == "1":
		print("CORRECT")
	
	return time

def main():
	base_url = "https://eitn41.eit.lth.se:3119/ha4/addgrade.php?name=Martin&grade=4&signature="
	possibilites = ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "a", "b", "c", "d", "e", "f"]
	current_time = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]	
	
	for i in range(20):
		print("iteration", i + 1)
		for j in range(len(possibilites)):
			current_time[j] = test(base_url + possibilites[j])
			print("time for", possibilites[j], ":", current_time[j])
		right = current_time.index(max(current_time))
		base_url += possibilites[right]
		
	test(base_url)
	print(base_url)
		
if __name__ == "__main__":
	main()