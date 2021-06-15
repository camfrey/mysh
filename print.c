// Written by: Cameron Frey and Liam Jordan
#include "user.h"

extern user_t *head;

void print(bool front2back)
{
  user_t *temp;
  int  i = 0;

  temp = head;

  while (temp->next != NULL) {
    i++;
    if (front2back)
      printf("(%d)--%s\n", i,temp->name);
    temp = temp->next;
  }
  if (front2back) printf("(%d)--%s\n", ++i, temp->name);
  if(!front2back){
    i++;
    // temp = temp->prev;
    while(temp != NULL){
      printf("(%d)--%s\n", i--, temp->name);
      temp = temp->prev;
    }
  }
}