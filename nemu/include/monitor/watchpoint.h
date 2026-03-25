#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  char expr[128];
  uint32_t last_val;


} WP;

<<<<<<< HEAD
void wp_display();
=======
WP *new_wp(const char *expr);
bool free_wp(int no);
void wp_display();
bool check_watchpoints();
>>>>>>> pa1

#endif
