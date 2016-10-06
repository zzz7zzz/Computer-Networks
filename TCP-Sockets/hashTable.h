#ifndef _hashtable_h   /* Include guard */
#define _hashtable_h

struct node {
	char zipcode[6];
	int zipcodeInt; 
	char city[20];
	char state[5];
	struct node *next;
};

struct hash {
	struct node *head;
	int count;
};

void createHash();
void insertInHash(char *zipcode, char *city, char *state);
void deleteFromHash(char *zipcode);
struct node * searchInHash(char *zipcode);
void printNode(struct node *n);
void printList(struct hash *h);
void parseDataFromCSV(char buffer[]);


#endif