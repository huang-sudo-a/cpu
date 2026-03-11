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

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

enum {
  TK_NOTYPE = 256, TK_EQ,
  TK_NUM,TK_TIME,TK_DIV,TK_LP,TK_RP
  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"[0-9]+",TK_NUM},    //number
  {"\\-",'-'},
  {"\\*", TK_TIME},
  {"\\/", TK_DIV},
  {"\\(", TK_LP},
  {"\\)", TK_RP},
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}//TK_NOTYPE

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /*: TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        switch (rules[i].token_type) {
        case TK_NOTYPE:
        // 空格，直接跳过，不存 token
        break;
        case TK_NUM:
        case TK_TIME:
        case TK_DIV:
        case TK_LP:
        case TK_RP:
        case '+':
        case '-':
        // 这些都是要存的普通 token
        if (nr_token >= 32) {printf("the array is full\n");return false; }
        
        tokens[nr_token].type =rules[i].token_type;
        strncpy(tokens[nr_token].str, substr_start, substr_len);
        tokens[nr_token].str[substr_len] = '\0';
        nr_token++;
        break;

    default:
        // 未知的 token 类型，说明 rules 数组加了新规则但 switch 没跟上
        printf("1111\n");
	TODO();
        break;
        }
/*
static struct rule {
 31   const char *regex;
 32   int token_type;
 33 } 
*/
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}
int eval(int p,int q);

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  word_t outcome=eval(0,nr_token-1);
  /* TODO: Insert codes to evaluate the expression. */
  //TODO();:

  return outcome;//0
}



/*const int get_nrtoken(){
  return nr_token;
}*/



//expr
static bool check_parentheses(int p,int q){ 
        if(p=='('&&q==')')return true;
        return false;
}

static int searchmain(int p,int q){ 
        int first2=p;
        while(p<=q){
                if(tokens[q].type==')')return first2;//wrong
                if (tokens[q].type=='+'||tokens[q].type=='-')return q;
                if(first2==p&&(tokens[q].type==TK_TIME||tokens[q].type==TK_DIV))first2=q;
                q--;
        }
        return first2;
}// search main operator

int eval(int p,int q){ 
  printf("p:%d  q:%d\n",p,q);  
  if (p > q) {
    printf("error\n");
  }
  else if (p == q) {
    return strtol(tokens[q].str,NULL,0);
  }
  else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */    
    return eval(p + 1, q - 1); 
  }
  else {
    int op = searchmain(p,q);
    uint32_t val1 = eval(p, op - 1); 
    uint32_t val2 = eval(op + 1, q); 
    int op_type=tokens[op].type;
    switch (op_type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case TK_TIME: return val1 * val2;
      case TK_DIV: return val1 / val2;
      default: printf("op_type:%c\n",op_type);//assert(0);
      }    
  }
  return -1;//risk
}
