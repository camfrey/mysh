// Written by: Cameron Frey and Liam Jordan
#include "user.h"

extern user_t *head;

void freeList()
{
  user_t *temp;
  int  i = 0;

  while (head != NULL) {
    temp = head;
    head = head->next; // point to next MP3 record
    free(temp->name);  // first free name inside MP3 record
    free(temp);        // then free MP3 record
    i++;
  }
  printf("free %d user(s)...\n", i);
}
