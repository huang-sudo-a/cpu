/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char exp[32];
  int last;
  /* TODO: Add more members if necessary */
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;
//static int tail=0;
void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

WP* new_wp(char *args,bool *success){
  WP* newwp=free_;
  word_t outcome=expr(args,success);
  if(!*success){
	printf("wrong expri\n");
	return newwp;
  }
  if(free_==NULL){
	printf("no free wp\n");
	*success=false;
  }
  strncpy(newwp->exp, args, sizeof(newwp->exp) - 1);
  newwp->last=outcome;
  newwp->next=NULL;
  free_=free_->next;
  WP *h=head;
  if(h==NULL){head=newwp;h=newwp;}
  while(h->next!=NULL){h=h->next;}
  h->next=newwp;
  return newwp;
}

void free_wp(int n){
  WP* h=head;
  WP* pre=NULL;
  while(h!=NULL){
    if(h->NO==n){
      if(pre==NULL)head=h->next;
      else pre->next=h->next;
      h->next=free_;
      free_=h;
      printf("free NO.%d wp successfully\n",n);
      return ;
    }
    pre=h;
    h=h->next;
  }
  printf("no NO.%d wp\n",n);
  return ;
}

void show_wp(){
	WP* h=head;
	int val;
	bool success=true;
	if(h==NULL){
		printf("no wp yet\n");
		return ;
	}
	while(h!=NULL){
		val=expr(h->exp,&success);
		if(!success){
			printf("NO.%d wp are invild\n",h->NO);
			continue;
		}
		printf("NO.%d expr:%s val= %d\n",h->NO,h->exp,val);
		h=h->next;	

  }
}
void wp_check(){
  WP* h=head;
        int val;
        bool success=true;
        if(h==NULL){
          return ;
        }
        while(h!=NULL){
                val=expr(h->exp,&success);
                if(!success){
                        printf("NO.%d wp are invild\n",h->NO);
                        continue;
                }
		if(h->last!=val){
			printf("NO.%d expr:%s has changed now val= %d,last=%d\n",h->NO,h->exp,val,h->last);
			getchar();
		}
    h=h->next;
  }
           return ;
}





