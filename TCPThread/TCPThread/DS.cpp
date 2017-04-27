#include "TCPHeader.h"
//HANDLE thread2;
//HANDLE SendEvent;
//HANDLE ReceEvent;

//SOCKET  ListenSocket=INVALID_SOCKET;		///< socket object
//SOCKET  clientsocks=INVALID_SOCKET;
//char    buff[MAX_SIZE];

HANDLE eevent;
struct tSsnNode * First= NULL;

int InsertSession(struct tSsnNode * pSessionNode, int pSession, SOCKET pSocket)
{
	struct tSsnNode* node, *tnode;

	node = (struct tSsnNode*)calloc(1, sizeof(struct tSsnNode));
	node->uSession  = pSession;
	node->uSock     = pSocket;
	node->uLeft     = NULL;
	node->uNick     = NULL;

	if (First == NULL){
		First = node;
	} else{

		tnode = First;

		while(tnode->uLeft != NULL){
			tnode = tnode->uLeft;
		}
		tnode->uLeft = node;
	}

	return node->uSession;
}


int Insert (SOCKET pSocket)
{
	struct tSsnNode * node;
	int      session ;
	node     = First;
	session = rand();

	return InsertSession(node, session, pSocket);
}


void SetNick(struct tSsnNode * pSessionNode, char* pNick)
{
	pSessionNode->uNick = (char*)calloc(10, sizeof(char));
	strncpy(pSessionNode->uNick, pNick, 8);	
	pSessionNode->uNick[strlen(pSessionNode->uNick)]=0x00;
}	

int DeleteSession (SOCKET & pSock, int pSessionid, char *& pNick)
{
	struct tSsnNode* node;
	struct tSsnNode* parent;

	/*if (First == NULL){
	return 1;
	} else if(First->uSession == pSessionid){
	pSock = First->uSock;
	pNick = (First->uNick);

	free(First);
	First = NULL;
	return 1;
	} else{
	node   = First->uLeft;
	parent = First;

	while(node != NULL){

	if (node->uSession == pSessionid){

	pSock = node->uSock;
	pNick = (node->uNick);
	parent->uLeft = node->uLeft;

	free (node);
	node = NULL;
	return 1;
	}
	parent = parent->uLeft;
	node =  node->uLeft;
	}
	}*/

	parent = First;

	if (First == NULL){
		return 1;

	} else if(First->uSession == pSessionid){

		pSock = First->uSock;
		pNick = (First->uNick);

		First = First->uLeft;
		free (parent);
		return 1;
	} else{

		node   = First->uLeft;	

		while(node != NULL){

			if(node->uSession == pSessionid){

				pSock = node->uSock;
				pNick = (node->uNick);

				parent->uLeft = node->uLeft;
				free (node);
				return 1;
			}
			parent = parent->uLeft;
			node =  node->uLeft;
		}
	}

	return 0;
}
