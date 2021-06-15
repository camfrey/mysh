// Written by: Cameron Frey and Liam Jordan
#ifndef USER_H
#define USER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
 
typedef struct user {
  char       *name;
  struct user *next;
  struct user *prev;
} user_t;

#endif