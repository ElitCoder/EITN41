from scipy import interpolate
from numpy import asarray, poly1d

def main():
	# construct f1
	f1 = poly1d([18, 20, 12, 2])
	
	# calculate f1(1)
	fown = f1(1)
	
	# calculate f1(1) + f2(1) + f3(1) + f4(1) + ... fn(1)
	first_f = fown + 44 + 23 + 34 + 41 + 42
	
	# the x points on f(x)
	x = [1, 3, 5, 6]
	
	# results of f(x)
	y = [first_f, 2930, 11816, 19751]
	
	# calculate the lagrange interpolation of x and y, transform into the same form as the array 
	# sent into f1 above to get the constant at the last array index
	print(int(asarray(interpolate.lagrange(x, y))[-1]))
	
	# print the whole poly
	print(interpolate.lagrange(x, y))
	
	# print poly as array
	print(asarray(interpolate.lagrange(x, y)))

if __name__ == "__main__":
	main()