#include <iostream>

using namespace std;

void increaseMultiplier(int &multiplier) {
	multiplier++;
	
	if(multiplier > 2)
		multiplier = 1;
}

int digitSum(int value) {
	int sum = 0;
	
	while(value > 0) {
		sum += value % 10;
		value /= 10;
	}
	
	return sum;
}

int main() {
	string answer = "";
	string luhnString;
	
	while(getline(cin, luhnString)) {
		int multiplier = 1;
		int sum = 0;
		int xMultiplier = -1;
				
		for(int i = luhnString.size() - 1; i >= 0; i--) {
			if(luhnString.at(i) == 'X') {
				xMultiplier = multiplier;
				increaseMultiplier(multiplier);
								
				continue;
			}
			
			int current = (luhnString.at(i) - '0') * multiplier;
			
			if(current > 9)
				current -= 9;
			
			sum += current;
			increaseMultiplier(multiplier);
		}
		
		int tenMult = 0;
		
		while(tenMult < sum)
			tenMult += 10;
			
		int left = (tenMult - sum);
		int x = -1;
		
		if(xMultiplier > 1)
			x = left % 2 == 0 ? left / 2 : (left + 9) / 2;
		else
			x = left;
			
		answer += (x + '0');
	}
	
	cout << answer << endl;
	
	return 0;
}