// Written by: Cameron Frey and Liam Jordan

#include "user.h"

extern user_t *head;

void delete(char *nameBuffer){
    user_t *tempCount;
    user_t *iter;
    int  length = 0;

    tempCount = head;
    iter = head;

    while (tempCount != NULL) {
        length++;
        tempCount = tempCount->next;
    }

    while(iter!=NULL){
        if(strcmp(iter->name,nameBuffer) == 0){
            if(iter == head && iter->next==NULL){
                user_t *temp = iter;
                iter = NULL;
                head = NULL;
                free(temp->name);
                free(temp);
            }
            else if(iter == head){
                head = head->next;
                user_t *temp = iter;
                iter = iter->next;
                temp->next->prev = temp->prev;
                free(temp->name);
                free(temp);
            }
            else if(iter->next==NULL){
                user_t *temp = iter;
                iter = iter->next;
                temp->prev->next = temp->next;
                free(temp->name);
                free(temp);
            }
            else{
                user_t *temp = iter;
                iter = iter->next;
                temp->next->prev = temp->prev;
                temp->prev->next = temp->next;
                free(temp->name);
                free(temp);
            }
        }
        else{
            iter = iter->next;
        }
    }
    return;
}