#include <iostream>
#include <vector>

#include <openssl/sha.h>

using namespace std;

typedef basic_string<unsigned char> ustring;

class Node {
public:
	Node(Node *left, Node *right, ustring value, bool direction) :
		m_left(left), m_right(right), m_value(value), m_direction(direction) {}
	
	bool isLeft() {
		return !m_direction;
	}
	
	bool isRight() {
		return m_direction;
	}
	
	void setParent(Node *parent) {
		m_parent = parent;
	}
	
	Node *getLeft() {
		return m_left;
	}
	
	Node *getRight() {
		return m_right;
	}
	
	bool isLeaf() {
		return m_left == nullptr && m_right == nullptr;
	}
	
	ustring getValue() {
		return m_value;
	}
	
private:
	Node *m_left;
	Node *m_right;
	Node *m_parent;
	
	ustring m_value;
	bool m_direction;
};

Node *constructTree(Node *transactions[], int size, bool direction) {
	if (size <= 2)
		return new Node(transactions[0],
			 			size == 1 ? transactions[0] : transactions[1],
						ustring(), direction);
		
	int divider = size % 2 == 0 ? size / 2 : size / 2 + 1;
	
	return new Node(
		constructTree(transactions, divider, false),
		constructTree(transactions + divider, size - divider, true),
		ustring(),
		direction);
}

void setParents(Node *node, Node *parent) {
	if (!node)
		return;
		
	setParents(node->getLeft(), node);
	setParents(node->getRight(), node);
	
	node->setParent(parent);
}

string toString(ustring str);

ustring toByteString(ustring str) {
	ustring result = ustring();
	unsigned char temp;
	string tempString;
	
	string intermediate = toString(str);
	
	printf("Enter function\n");
	
	for (unsigned int i = 0; i < intermediate.size(); i += 2) {
		printf("here %lu %u\n", intermediate.size(), i);
		tempString = intermediate[i] + intermediate[i + 1];
		temp = strtol(tempString.c_str(), NULL, 16);
		result += temp;
		//sscanf((intermediate.c_str())[i], "%02x", &temp);
		//result += temp;
		
		
		//i += 2;
	}
	
	return result;
}

ustring unsignedCharToUString(unsigned char a[], int len) {
	ustring result = ustring();
	
	for (int i = 0; i != len; ++i)
		result += a[i];
		
	return result;
}

ustring getMerkleRoot(Node *root) {
	if (root->isLeaf())
		return root->getValue();
		
	ustring leftValue = getMerkleRoot(root->getLeft());
	ustring rightValue = getMerkleRoot(root->getRight());
	
	printf("size %d %d\n", leftValue.size(), rightValue.size());
	
	ustring combined = leftValue + rightValue;
	unsigned char hashValue[SHA_DIGEST_LENGTH];
	SHA1(combined.c_str(), combined.size(), hashValue);
	
	//make hashValue into 40 byte string
	
	return unsignedCharToUString(hashValue, SHA_DIGEST_LENGTH);
}

string makeMerklePath(Node *node) {
	return "";
}

ustring toUString(string str) {
	ustring result;
	
	for (const auto& c : str)
		result += c;
		
	return result;
}

string toString(ustring str) {
	string result;
	
	for (const auto& c : str)
		result += c;
		
	return result;
}

int main() {
	string temp;
	
	getline(cin, temp);
	int i = stoi(temp);
	
	getline(cin, temp);
	int j = stoi(temp);
	
	vector<string> transactions;
	while (getline(cin, temp)) {
		transactions.push_back(temp);
	}
	
	Node *transactionNodes[transactions.size()];
	for (int k = 0; k != transactions.size(); ++k)
		transactionNodes[k] = new Node(nullptr, nullptr, toByteString(toUString(transactions.at(k))), (k & 1) != 0);
		
	Node *rootNode = constructTree(transactionNodes, transactions.size(), false);
	setParents(rootNode, nullptr);
	
	string merklePath = makeMerklePath(transactionNodes[i]);
	ustring merkleRoot = getMerkleRoot(rootNode);
	
	cout << "Merkle root: ";
	
	for (int k = 0; k < merkleRoot.size(); k++)
		printf("%02x ", merkleRoot.c_str()[k]);
		
	cout << endl;
	
	return 0;
}