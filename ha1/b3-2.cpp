#include <iostream>

using namespace std;

vector<string> g_leafs;

class Tree {
public:
	/*
	Tree(string value, bool leaf) {
		m_value = value;
		m_leaf = leaf;
	}
	*/
	
//private:
	Tree m_leaf1, m_leaf2;
	
	string m_value;
	bool m_leaf;
}

void constructTree(Tree &root, int depth, bool left) {
	if(depth != g_leafs.size()) {
		constructTree(root.m_leaf1, depth + 1, true);
		constructTree(root.m_leaf2, depth + 1, false);
	}
	
	if(depth == g_leafs.size()) {
		root.m_leaf = true;
		root.m_value = ""; // get index somehow
	} else {
		// add value from leaf1 and leaf2 and form sha1
		root.m_leaf = false;
	}
}

int main() {
	string temp;
	int i;
	int j;
	
	getline(cin, temp);
	i = stoi(temp);
	
	getline(cin, temp);
	j = stoi(temp);
	
	while(getline(cin, temp)) {
		g_leafs.push_back(temp);
	}
	
	int d = log2(g_leafs.size());
}