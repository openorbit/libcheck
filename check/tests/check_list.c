#include <string.h>
#include <stdlib.h>
#include "list.h"
#include "check.h"


START_TEST(test_create)
{
  List *lp;

  lp = list_create();

  fail_unless (list_val(lp) == NULL,
	       "Current list value should be NULL for newly created list");

  fail_unless (list_at_end(lp),
	       "Newly created list should be at end");
  list_advance(lp);
  fail_unless (list_at_end(lp),
	       "Advancing a list at end should produce a list at end");
  list_free (lp);
}
END_TEST

START_TEST(test_free)
{
  List *lp = list_create();
  list_add_end (lp, "abc");
  list_add_end (lp, "123");
  list_add_end (lp, NULL);
  list_free (lp);
}
END_TEST

START_TEST(test_add_end)
{
  List * lp = list_create();
  char * tval = "abc";
  
  list_add_end (lp, tval);
  
  fail_unless (list_val (lp) != NULL,
	       "List current val should not be null after new insertion");
  fail_unless (!list_at_end (lp),
	       "List should be at end after new insertion");
  fail_unless (strcmp(tval, (char *) list_val (lp)) == 0,
	       "List current val should equal newly inserted val");
}
END_TEST

START_TEST(test_add_a_bunch)
{
  List *lp;
  int i, j;
  for (i = 0; i < 3; i++) {
    lp = list_create();
    for (j = 0; j < 1000; j++)
      list_add_end (lp, "abc");
    list_free(lp);
  }
}
END_TEST

START_TEST(test_add_end_and_next)
{
  List *lp = list_create();
  char *tval1 = "abc";
  char *tval2 = "123";
  
  list_add_end (lp, tval1);
  list_add_end (lp, tval2);
  list_front(lp);
  fail_unless (strcmp (tval1, list_val (lp)) == 0,
	       "List head val should equal first inserted val");
  list_advance (lp);
  fail_unless (!list_at_end (lp),
	       "List should not be at end after two adds and one next");
  fail_unless (strcmp (tval2, list_val (lp)) == 0,
	       "List val should equal second inserted val");
  list_advance(lp);
  fail_unless (list_at_end (lp),
	       "List should be at and after two adds and two nexts");
  list_free (lp);
}
END_TEST



int main (void)
{
  Suite *s = suite_create("Lists");
  TCase * tc = tcase_create("Lists");
  SRunner *sr = srunner_create (s);

  suite_add_tcase (s, tc);
  tcase_add_test (tc, test_create);
  tcase_add_test (tc, test_add_end);
  tcase_add_test (tc, test_add_end_and_next);
  tcase_add_test (tc, test_add_a_bunch);
  tcase_add_test (tc, test_free);

  srunner_run_all (sr, CRNORMAL);
  return (srunner_ntests_failed(sr) == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
