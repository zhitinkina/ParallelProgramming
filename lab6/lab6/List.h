#pragma once

class List {

	struct Node {
		int x;
		Node* next;
	};

public:
	List() {
		head = nullptr; 
	}

	~List() {
		ClearNode();
	}

	void AddNode(int value) {
		Node* node = new Node();
		node->x = value;
		node->next = head;
		head = node;
		count++;
	}

	int PopNode() {
		Node* node = head;
		int ret = node->x;
		head = head->next;
		delete node;
		count--;
		return ret;
	}

	int CountNode() {
		return count;
	}

	void ClearNode() {
		Node* next = head;

		while (next) {
			Node* delete_me = next;
			next = next->next;
			delete delete_me; 
		}

		count = 0;
	}
	
private:
	Node* head;
	int count;
};
