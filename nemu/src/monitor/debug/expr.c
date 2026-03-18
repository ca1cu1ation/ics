#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

enum {
  TK_NOTYPE = 256,
  TK_DEC,
  TK_HEX,
  TK_REG,
  TK_EQ,
  TK_NEQ,
  TK_LE,
  TK_GE,
  TK_LT,
  TK_GT,
  TK_AND,
  TK_NEG,
  TK_DEREF

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},                // spaces
  {"0[xX][0-9a-fA-F]+", TK_HEX},    // hex number
  {"[0-9]+", TK_DEC},               // decimal number
  {"\\$[a-zA-Z][a-zA-Z0-9]*", TK_REG}, // register
  {"==", TK_EQ},                    // equal
  {"!=", TK_NEQ},                   // not equal
  {"<=", TK_LE},                    // less than or equal
  {">=", TK_GE},                    // greater than or equal
  {"<", TK_LT},                     // less than
  {">", TK_GT},                     // greater than
  {"&&", TK_AND},                   // logical and
  {"\\+", '+'},                    // plus
  {"-", '-'},                       // minus
  {"\\*", '*'},                    // multiply/dereference
  {"/", '/'},                       // divide
  {"\\(", '('},                    // left parenthesis
  {"\\)", ')'}                     // right parenthesis
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

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
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[64];
int nr_token;

static bool is_value_token(int type) {
  return type == TK_DEC || type == TK_HEX || type == TK_REG || type == ')';
}

static bool read_register_value(const char *name, uint32_t *value) {
  if (strcmp(name, "eip") == 0) {
    *value = cpu.eip;
    return true;
  }

  int i;
  for (i = 0; i < 8; i ++) {
    if (strcmp(name, regsl[i]) == 0) {
      *value = reg_l(i);
      return true;
    }
  }

  for (i = 0; i < 8; i ++) {
    if (strcmp(name, regsw[i]) == 0) {
      *value = reg_w(i);
      return true;
    }
  }

  for (i = 0; i < 8; i ++) {
    if (strcmp(name, regsb[i]) == 0) {
      *value = reg_b(i);
      return true;
    }
  }

  return false;
}

static bool check_parentheses(int p, int q) {
  if (tokens[p].type != '(' || tokens[q].type != ')') {
    return false;
  }

  int i;
  int balance = 0;
  for (i = p; i <= q; i ++) {
    if (tokens[i].type == '(') {
      balance ++;
    }
    else if (tokens[i].type == ')') {
      balance --;
      if (balance == 0 && i < q) {
        return false;
      }
      if (balance < 0) {
        return false;
      }
    }
  }

  return balance == 0;
}

static int precedence(int type) {
  switch (type) {
    case TK_AND: return 1;
    case TK_EQ:
    case TK_NEQ:
    case TK_LE:
    case TK_GE:
    case TK_LT:
    case TK_GT: return 2;
    case '+':
    case '-': return 3;
    case '*':
    case '/': return 4;
    case TK_NEG:
    case TK_DEREF: return 5;
    default: return -1;
  }
}

static bool is_unary(int type) {
  return type == TK_NEG || type == TK_DEREF;
}

static int dominant_op(int p, int q) {
  int i;
  int balance = 0;
  int op = -1;
  int min_pri = 100;

  for (i = p; i <= q; i ++) {
    int type = tokens[i].type;
    if (type == '(') {
      balance ++;
      continue;
    }
    if (type == ')') {
      balance --;
      continue;
    }
    if (balance != 0) {
      continue;
    }

    int pri = precedence(type);
    if (pri < 0) {
      continue;
    }

    if (pri < min_pri) {
      min_pri = pri;
      op = i;
      continue;
    }

    if (pri == min_pri && !is_unary(type)) {
      op = i;
    }
  }

  return op;
}

static uint32_t eval(int p, int q, bool *success) {
  if (p > q) {
    *success = false;
    return 0;
  }

  if (p == q) {
    char *endptr = NULL;
    uint32_t val = 0;

    if (tokens[p].type == TK_DEC) {
      val = (uint32_t)strtoul(tokens[p].str, &endptr, 10);
      if (*endptr != '\0') {
        *success = false;
      }
      return val;
    }

    if (tokens[p].type == TK_HEX) {
      val = (uint32_t)strtoul(tokens[p].str, &endptr, 16);
      if (*endptr != '\0') {
        *success = false;
      }
      return val;
    }

    if (tokens[p].type == TK_REG) {
      if (read_register_value(tokens[p].str + 1, &val)) {
        return val;
      }
      *success = false;
      return 0;
    }

    *success = false;
    return 0;
  }

  if (check_parentheses(p, q)) {
    return eval(p + 1, q - 1, success);
  }

  int op = dominant_op(p, q);
  if (op < 0) {
    *success = false;
    return 0;
  }

  if (tokens[op].type == TK_NEG) {
    uint32_t val = eval(op + 1, q, success);
    return (uint32_t)(-(int32_t)val);
  }

  if (tokens[op].type == TK_DEREF) {
    uint32_t addr = eval(op + 1, q, success);
    return vaddr_read(addr, 4);
  }

  uint32_t val1 = eval(p, op - 1, success);
  if (!*success) {
    return 0;
  }
  uint32_t val2 = eval(op + 1, q, success);
  if (!*success) {
    return 0;
  }

  switch (tokens[op].type) {
    case '+': return val1 + val2;
    case '-': return val1 - val2;
    case '*': return val1 * val2;
    case '/':
      if (val2 == 0) {
        *success = false;
        return 0;
      }
      return val1 / val2;
    case TK_EQ: return val1 == val2;
    case TK_NEQ: return val1 != val2;
    case TK_AND: return val1 && val2;
    case TK_LE: return val1 <= val2;
    case TK_GE: return val1 >= val2;
    case TK_LT: return val1 < val2;
    case TK_GT: return val1 > val2;
    default:
      *success = false;
      return 0;
  }
}

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

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE:
            break;

          case TK_DEC:
          case TK_HEX:
          case TK_REG:
            if (nr_token >= (int)(sizeof(tokens) / sizeof(tokens[0]))) {
              printf("Too many tokens in expression\n");
              return false;
            }
            tokens[nr_token].type = rules[i].token_type;
            if (substr_len >= (int)sizeof(tokens[nr_token].str)) {
              printf("Token is too long\n");
              return false;
            }
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            nr_token ++;
            break;

          default:
            if (nr_token >= (int)(sizeof(tokens) / sizeof(tokens[0]))) {
              printf("Too many tokens in expression\n");
              return false;
            }
            tokens[nr_token].type = rules[i].token_type;
            tokens[nr_token].str[0] = '\0';
            nr_token ++;
            break;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  for (i = 0; i < nr_token; i ++) {
    if (tokens[i].type == '-') {
      if (i == 0 || !is_value_token(tokens[i - 1].type)) {
        tokens[i].type = TK_NEG;
      }
    }
    else if (tokens[i].type == '*') {
      if (i == 0 || !is_value_token(tokens[i - 1].type)) {
        tokens[i].type = TK_DEREF;
      }
    }
  }

  return true;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  if (nr_token == 0) {
    *success = false;
    return 0;
  }

  *success = true;
  return eval(0, nr_token - 1, success);
}
