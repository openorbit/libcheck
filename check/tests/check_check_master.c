#include <stdlib.h>
#include <check.h>
#include "check_check.h"


TestResult **tr_fail_array;
TestResult **tr_all_array;

START_TEST(test_check_nfailures)
{
  fail_unless (sub_nfailed == 8, "Unexpected number of failures received");
}
END_TEST

START_TEST(test_check_ntests_run)
{
  fail_unless (sub_ntests == 9, "Unexpected number of tests run");
}
END_TEST

START_TEST(test_check_failure_msgs)
{
  int i;
  char *msgar[] = {
    "Failure expected",
    "Early exit with return value 1",
    /*    "Test passed", */
    "This test should fail",
    "Received signal 11",
    "Received signal 8",
    "Received signal 8",
    "Early exit with return value 1",
    "Completed properly"};

  for (i = 0; i < sub_nfailed; i++) {
    char *msg;   
    msg = tr_msg(tr_fail_array[i]);
    if (strcmp (msg, msgar[i]) != 0) {
      char *emsg = malloc (CMAXMSG);
      snprintf (emsg, CMAXMSG,"Expected %s, got %s", msgar[i], msg);
      fail (emsg);
      free (emsg);
    }
  }
}
END_TEST
  
START_TEST(test_check_failure_lnos)
{
  int i;
  int lnos[] = {
    8,
    14,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1};
  
  for (i = 0; i < sub_nfailed; i++) {

    if (lnos[i] > 0 && tr_lno(tr_fail_array[i]) != lnos[i]) {
      char *emsg = malloc (CMAXMSG);
      snprintf (emsg, CMAXMSG, "Expected lno %d, got %d",
		lnos[i], tr_lno(tr_fail_array[i]));
      fail (emsg);
      free (emsg);
    }    
  }
}
END_TEST

START_TEST(test_check_failure_ftypes)
{
  int i;
  int ftypes[] = {
    CRFAILURE,
    CRERROR,
    /* CRPASS, */
    CRFAILURE,
    CRERROR,
    CRERROR,
    CRERROR,
    CRERROR,
    CRFAILURE};
  
  for (i = 0; i < sub_nfailed; i++) {
    fail_unless (ftypes[i] == tr_rtype(tr_fail_array[i]),
		 "Failure type wrong");
  }
}
END_TEST

START_TEST(test_check_failure_lfiles)
{
  int i;
  for (i = 0; i < sub_nfailed; i++) {

    fail_unless (strcmp(tr_lfile(tr_fail_array[i]), "check_check_sub.c") == 0,
		 "Bad file name");
  }
}
END_TEST

START_TEST(test_check_failure_tcnames)
{
  int i;
  for (i = 0; i < sub_nfailed; i++) {

    fail_unless (strcmp(tr_tcname(tr_fail_array[i]), "Check Servant") == 0,
		 "Bad test case name");
  }
}
END_TEST


START_TEST(test_check_all_msgs)
{
  int i;
  char *msgar[] = {
    "Failure expected",
    "Early exit with return value 1",
    "Test passed",
    "This test should fail",
    "Received signal 11",
    "Received signal 8",
    "Received signal 8",
    "Early exit with return value 1",
    "Completed properly"};

  for (i = 0; i < sub_ntests; i++) {
    char *msg;   
    msg = tr_msg(tr_all_array[i]);
    if (strcmp (msg, msgar[i]) != 0) {
      char *emsg = malloc (CMAXMSG);
      snprintf (emsg, CMAXMSG,"Expected %s, got %s", msgar[i], msg);
      fail (emsg);
      free (emsg);
    }
  }
}
END_TEST  

START_TEST(test_check_all_ftypes)
{
  int i;
  int ftypes[] = {
    CRFAILURE,
    CRERROR,
    CRPASS,
    CRFAILURE,
    CRERROR,
    CRERROR,
    CRERROR,
    CRERROR,
    CRFAILURE};
  
  for (i = 0; i < sub_ntests; i++) {
    fail_unless (ftypes[i] == tr_rtype(tr_all_array[i]),
		 "Failure type wrong");
  }
}
END_TEST

int test_fixture_val = 0;
void test_fixture_setup (void)
{
  test_fixture_val = 1;
}

START_TEST(test_setup)
{
  fail_unless (test_fixture_val == 1,
	       "Value not setup or changed across tests correctly");
  test_fixture_val = 2;
}
END_TEST

void test_fixture_teardown (void)
{
  test_fixture_val = 3;
}

START_TEST(test_teardown)
{
  fail_unless (test_fixture_val == 3, "Value not changed correctly in teardown");
}
END_TEST  


Suite *main_make_suite (void)
{
  Suite *s;
  TCase *tc_core;
  TCase *tc_fixture;
  TCase *tc_post_teardown;
  
  s = suite_create("Master");
  tc_core = tcase_create("Core Tests");
  tc_fixture = tcase_create("Fixture Setup Tests");
  suite_add_tcase (s, tc_core);
  tcase_add_test (tc_core, test_check_nfailures);
  tcase_add_test (tc_core, test_check_ntests_run);
  tcase_add_test (tc_core, test_check_failure_msgs);
  tcase_add_test (tc_core, test_check_failure_ftypes);
  tcase_add_test (tc_core, test_check_failure_lnos);
  tcase_add_test (tc_core, test_check_failure_lfiles);
  tcase_add_test (tc_core, test_check_all_msgs);
  tcase_add_test (tc_core, test_check_all_ftypes);
  tcase_set_fixture(tc_fixture, test_fixture_setup, test_fixture_teardown);
  /* add the test 3 times to make sure we adequately test
     preservation of fixture values across tests, regardless
     of the order in which tests are added to the test case */
  tcase_add_test (tc_fixture, test_setup);
  tcase_add_test (tc_fixture, test_setup);
  tcase_add_test (tc_fixture, test_setup);
  suite_add_tcase (s, tc_fixture);
  tc_post_teardown = tcase_create ("Fixture Teardown Tests");
  tcase_add_test (tc_post_teardown, test_teardown);
  suite_add_tcase (s, tc_post_teardown);
  return s;
}

void setup (void)
{
  char *msg;
  Suite *s = sub_make_suite();
  SRunner *sr = srunner_create(s);
  srunner_run_all(sr, CRSILENT);
  tr_fail_array = srunner_failures(sr);
  tr_all_array = srunner_results(sr);
  sub_nfailed = srunner_ntests_failed(sr);
  sub_ntests = srunner_ntests_run(sr);
}

void cleanup (void)
{
  return;
}
