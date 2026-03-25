#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "monitor/monitor.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
    wp_pool[i].expr[0] = '\0';
    wp_pool[i].last_val = 0;
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

<<<<<<< HEAD
=======
WP *new_wp(const char *expr_str) {
  if (expr_str == NULL || *expr_str == '\0') {
    printf("Usage: w EXPR\n");
    return NULL;
  }

  if (free_ == NULL) {
    printf("No free watchpoint slots.\n");
    return NULL;
  }

  bool success = false;
  uint32_t val = expr((char *)expr_str, &success);
  if (!success) {
    printf("Invalid expression: %s\n", expr_str);
    return NULL;
  }

  WP *wp = free_;
  free_ = free_->next;

  wp->next = head;
  head = wp;
  wp->last_val = val;
  strncpy(wp->expr, expr_str, sizeof(wp->expr) - 1);
  wp->expr[sizeof(wp->expr) - 1] = '\0';

  printf("Watchpoint %d: %s (current value = 0x%08x)\n", wp->NO, wp->expr, wp->last_val);
  return wp;
}

bool free_wp(int no) {
  WP *prev = NULL;
  WP *cur = head;

  while (cur != NULL) {
    if (cur->NO == no) {
      if (prev == NULL) {
        head = cur->next;
      }
      else {
        prev->next = cur->next;
      }

      cur->next = free_;
      free_ = cur;
      cur->expr[0] = '\0';
      cur->last_val = 0;

      printf("Watchpoint %d deleted.\n", no);
      return true;
    }
    prev = cur;
    cur = cur->next;
  }

  printf("Watchpoint %d not found.\n", no);
  return false;
}

>>>>>>> pa1
void wp_display() {
  WP *cur = head;
  if (cur == NULL) {
    printf("No watchpoints.\n");
    return;
  }

<<<<<<< HEAD
  while (cur != NULL) {
    printf("Watchpoint %d\n", cur->NO);
=======
  printf("Num\tWhat\tValue\n");

  while (cur != NULL) {
    printf("%d\t%s\t0x%08x\n", cur->NO, cur->expr, cur->last_val);
>>>>>>> pa1
    cur = cur->next;
  }
}

<<<<<<< HEAD
/* TODO: Implement the functionality of watchpoint */
=======
bool check_watchpoints() {
  bool changed = false;
  WP *cur = head;

  while (cur != NULL) {
    bool success = false;
    uint32_t new_val = expr(cur->expr, &success);
    if (!success) {
      printf("Failed to evaluate watchpoint %d: %s\n", cur->NO, cur->expr);
      nemu_state = NEMU_STOP;
      return true;
    }

    if (new_val != cur->last_val) {
      printf("Watchpoint %d triggered: %s\n", cur->NO, cur->expr);
      printf("Old value = 0x%08x\n", cur->last_val);
      printf("New value = 0x%08x\n", new_val);
      cur->last_val = new_val;
      changed = true;
    }

    cur = cur->next;
  }

  if (changed) {
    nemu_state = NEMU_STOP;
  }

  return changed;
}
>>>>>>> pa1


