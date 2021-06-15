//
// adapted from http://www.cprogramming.com/snippets/source-code/singly-linked-list-insert-remove-add-count
//
// Written by: Cameron Frey and Liam Jordan

#include "mp3.h"
#define  BUFFERSIZE 128

user_t *head;

void insert(char *title,char *name, int data);
void print(bool front2back);
void freeList();
void delete(char *nameBuffer);

int main()
{
  int i, num, len;
  struct node *n;
  char titleBuffer[BUFFERSIZE],nameBuffer[BUFFERSIZE], c;

  head = NULL;

  while (1) {
    printf("\nList Operations\n");
    printf("===============\n");
    printf("(1) Insert\n");
    printf("(2) Delete\n");
    printf("(3) Display from beginning to end\n");
    printf("(4) Display from end to beginning\n");
    printf("(5) Exit\n");
    printf("Enter your choice : ");
    if (scanf("%d%c", &i, &c) <= 0) {          // use c to capture \n
        printf("Enter only an integer...\n");
        exit(0);
    } else {
        switch(i)
        {
        case 1: printf("Enter the title to insert : ");
		              if (fgets(titleBuffer, BUFFERSIZE , stdin) != NULL) {
                    len = strlen(titleBuffer);
                    titleBuffer[len - 1] = '\0';   // override \n to become \0
                  } else {
                    printf("wrong name...");
                    exit(-1);
                  }
                  printf("Enter the artist to insert : ");
                  if (fgets(nameBuffer, BUFFERSIZE , stdin) != NULL) {
                    len = strlen(nameBuffer);
                    nameBuffer[len - 1] = '\0';   // override \n to become \0
                  } else {
                    printf("wrong name...");
                    exit(-1);
                  }
                printf("Enter the number to insert : ");
                scanf("%d%c", &num, &c);  // use c to capture \n
                printf("[%s] [%s] [%d]\n",titleBuffer, nameBuffer, num);
                insert(titleBuffer,nameBuffer, num);
                break;
        case 2: printf("Enter the artist to insert : ");
                if (fgets(nameBuffer, BUFFERSIZE , stdin) != NULL) {
                  len = strlen(nameBuffer);
                  nameBuffer[len - 1] = '\0';   // override \n to become \0
                } else {
                  printf("wrong name...");
                  exit(-1);
                }
                if (head == NULL)
                  printf("List is Empty\n");
                else
                  delete(nameBuffer);
                break;
        case 3: if (head == NULL)
                  printf("List is Empty\n");
                else
                  print(true);
                break;
        case 4: if (head == NULL)
                  printf("List is Empty\n");
                else
                  print(false);
                break;
        case 5: freeList();
                return 0;
        default: printf("Invalid option\n");
        }
    }
  }
  freeList();
  return 0;
}
