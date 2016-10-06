/* A hashtable to contain all the information about zipcodes and US cities and states */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hashTable.h"

struct hash *hashTable = NULL;
/* arbitrary prime number one-tenth of the 40,000-ish data
 * for the size of hashtable and to compute hash key */
const int HASHTABLECOUNT = 4813; 

struct node * createNode(char *zipcode, char *city, char *state) {
	struct node *newnode;
	int zipcodeInt;
	
	newnode = (struct node *) malloc(sizeof(struct node));
	strcpy(newnode->zipcode, zipcode);
	zipcodeInt = atoi(zipcode);
	newnode->zipcodeInt = zipcodeInt;
	strcpy(newnode->city, city);
	strcpy(newnode->state, state);
	newnode->next = NULL;
	return newnode;
}


void createHash() 
{
	hashTable = (struct hash *) calloc(HASHTABLECOUNT, sizeof(struct hash)); 
	return;
}

void insertInHash(char *zipcode, char *city, char *state) 
{		

	/* Create the hashKey using the zipcode*/
	int zipcodeInt = atoi(zipcode);
	int hashIndex = zipcodeInt % HASHTABLECOUNT;

	/* Create a new node for insertion */
	struct node *newnode =  createNode(zipcode, city, state);

	/* If hash bucket is empty, newnode becomes head of the bucket */
	if (!hashTable[hashIndex].head) {
			hashTable[hashIndex].head = newnode;
			hashTable[hashIndex].count = 1;
// 			printf("Inserted ------\n");
// 			printNode(hashTable[hashIndex].head);
			return;
	} 
	
	/* If hash bucket is not empty, new node becomes head, and head is pushed back */
	newnode->next = hashTable[hashIndex].head;
	hashTable[hashIndex].head = newnode;
	hashTable[hashIndex].count++;
	
	printf("Inserted ------\n");
	printNode(hashTable[hashIndex].head);
	return;
}


void deleteFromHash(char *zipcode) {
	/* find the bucket in the hash table using zipcode-hashkey */
	int zipcodeInt = atoi(zipcode); 
	int hashIndex = zipcodeInt % HASHTABLECOUNT;
	
	struct node *currNode, *prevNode, *nodeForDeletion;
	
	/* get the linked list head from current bucket */
	nodeForDeletion = hashTable[hashIndex].head;
	
	/* error checking: is linked list empty */
	if (!nodeForDeletion) {
			printf("Nothing to delete. Given data not present in hash Table sad.\n");
			return;
	}
	
	/* if head is the node, delete head and update the head and count */
	if (nodeForDeletion->zipcodeInt == zipcodeInt) {
		hashTable[hashIndex].head = nodeForDeletion->next;
		hashTable[hashIndex].count--;
		return;
	}
	
	/* when node for deletion is not the head, traverse through list to delete node */
	prevNode = nodeForDeletion;
	currNode = nodeForDeletion->next;
	while (currNode != NULL) {
			if (currNode->zipcodeInt == zipcodeInt) {
				prevNode->next = currNode->next;
				hashTable[hashIndex].count--;
				return;
			}
			prevNode = currNode;
			currNode = currNode->next;
	}
}

struct node * searchInHash(char *zipcode) 
{
	int zipcodeInt = atoi(zipcode);
	int hashKey = zipcodeInt % HASHTABLECOUNT;

	/* Select the right bucket */
	struct node *currNode = hashTable[hashKey].head;

	/* Search in bucket */
	while (currNode != NULL) {
		if (currNode->zipcodeInt == zipcodeInt) {
			printf("Found in hash --------\n");
			printNode(currNode);

			return currNode;
		}
		currNode = currNode->next;
	}
	
	printf("Not found in hash:(\n");
	return NULL;
}


void printList(struct hash *h)
{
	struct node *currNode;
	currNode = h->head;
	while (currNode != NULL) {
		printf("Zipcode: %s; City: %s; State: %s\n", 
			currNode->zipcode,
			currNode->city,
			currNode->state);
		currNode = currNode->next;	
	}
	return;
}

void printNode(struct node *n) 
{
	printf("Zipcode: %s; City: %s; State: %s\n", 
			n->zipcode,
			n->city,
			n->state);
}


void parseDataFromCSV(char buffer[])
{
	char *zipcode, *city, *state;

	state = strtok(buffer, ",");
	city = strtok(NULL, ",");
	zipcode = strtok(NULL, ",");

	insertInHash(zipcode,city,state);
	
	return;
}

// int main(int argc, char *argv[])
// {
// 	FILE *input_file = fopen("cityzip.csv","r");
// 	char buffer[1024];
// 	
// 	if (input_file == 0) 
// 	{
// 		fprintf(stderr,"Cannot open input file.\n");
// 	}
// 	
// 	createHash();
// 
//     /* skip the first line in csv file */
// 	fgets(buffer, 1024, input_file);
// 	
// 	while (1)
// 	{
// 
// 		if (fgets(buffer, 1024, input_file) != NULL)
// 		{
// 			parseDataFromCSV(buffer);
// 			bzero(buffer,1024);
// 
// 		} else {
// 			break;
// 		}
// 
// 
// 	} 
// 	
// 	printNode(searchInHash("14120"));
// 
// 
//     fclose(input_file);
// 
//     return 0;
// }
//  
	
