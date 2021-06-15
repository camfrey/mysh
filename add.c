#include "user.h"

extern user_t *head;

void insert(char *name)
{
  user_t *temp, *mp3;

  mp3 = (user_t *) malloc(sizeof(user_t));        // malloc space for MP3
  mp3->name = (char *) malloc(strlen(name) + 1);  // malloc space for name
  strcpy(mp3->name, name);                        // "assign" name via copy
  mp3->next = NULL;

  if (head == NULL)
  {
    head = mp3;               // add the first MP3
  }
  else
  {
    temp = head;
    while (temp->next != NULL)
      temp = temp->next;
    temp->next = mp3;         // append to the tail/end
    mp3->prev = temp;
  }
}