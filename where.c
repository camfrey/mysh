#include "get_path.h"

void where(char *command, struct pathelement *p)
{
  char cmd[64], *ch;

  while (p != NULL) {       
    sprintf(cmd, "%s/%s", p->element, command);
    if (access(cmd, X_OK) == 0) {
      printf("%s\n",cmd);
    }
    p = p->next;
  }
}
