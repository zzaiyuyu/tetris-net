#ifndef LLIST_H
#define LLIST_H
#include <stdio.h>
#include <iostream>

typedef struct node_{
	int sock;
	struct node_* next;
	node_(int data){
		next = NULL;
		sock = data;
	}
}Node;

//带头结点的单链表
class List{
public:
	List(){
		pHead = new Node(0);
	}
	void push(int sock){
		Node* pNew = new Node(sock);
		pNew->next = pHead->next; 		
		pHead->next = pNew;
	}	
	void del(int sock){
		Node* pCur = pHead->next;
		Node* pPre = pHead;
		while(pCur && pCur->sock != sock){
			pPre = pCur;
			pCur = pCur->next;
		}	
		if(pCur != NULL){
			pPre->next = pCur->next;
			printf("删除sock%d\n", sock);
			delete pCur;
		}
		else{
			printf("未找到sock%d", sock);
		}
	}
	void print(){
		Node* pCur = pHead;
		while(pCur){
			printf("sock%d->", pCur->sock);
			pCur = pCur->next;
		}
		printf("\n");
	}
	~List()
	{
		Node* pCur = pHead;
		while(pCur){
			Node* pDel = pCur;
			pCur = pCur->next;
			delete pDel;
		}
	}

	Node* head(){
		return pHead;
	}
private:
	Node* pHead;
};

#endif

#if 0
int main()
{
	List sockList;
	sockList.init();
	sockList.push(5);
	sockList.push(3);
	sockList.push(4);
	sockList.print();
	sockList.del(3);
	sockList.print();
}
#endif
