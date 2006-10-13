#ifndef CHECK_H
#define CHECK_H

/*
  Check: a unit test framework for C
  Copyright (C) 2001, Arien Malec

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/*! Magic values */
enum {
  CMAXMSG = 100, /*! maximum length of a message, including terminating nul */
};

/* Core suite/test case types and functions */

/*! opaque type for a test suite */
typedef struct Suite Suite;

/*! opaque type for a test case
  For the moment, test cases can only be run through a suite */
typedef struct TCase TCase; 

/* type for a test function */
typedef void (*TFun) (int);

/*! type for a setup/teardown function */
typedef void (*SFun) (void);

/*! Create a test suite */
Suite *suite_create (char *name);

/*! Free a test suite
  (For the moment, this also frees all contained test cases) */
void suite_free (Suite *s);

/*! Create a test case */
TCase *tcase_create (char *name);

/*! Free a test case
  (Note that as it stands, one will normally free the contaning suite) */
void tcase_free (TCase *tc);

/*! Add a test case to a suite */
void suite_add_tcase (Suite *s, TCase *tc);

/*! Add a test function to a test case
  (macro version) */
#define tcase_add_test(tc,tf) _tcase_add_test(tc,tf,"" # tf "")
/*! Add a test function to a test case
  (function version -- use this when the macro won't work */
void _tcase_add_test (TCase *tc, TFun tf, char *fname);

/*! Add fixture setup/teardown functions to a test case
   Note that setup/teardown functions are not run in a separate
   address space, like test functions, and so must not
   exit or signal (e.g., segfault) */
void tc_set_fixture(TCase *tc, SFun setup, SFun teardown);

/* Internal functions to mark the start and end of a test function */
void tcase_fn_start (int msqid, char *fname, char *file, int line);

/*! Start a unit test with START_TEST(unit_name), end with END_TEST
  One must use braces within a START_/END_ pair to declare new variables */
#define START_TEST(__testname)\
void __testname (int __msqid)\
{\
  tcase_fn_start (__msqid,""# __testname, __FILE__, __LINE__);\

#define END_TEST }


/*! Fail the test case unless result is true */
#define fail_unless(result,msg) _fail_unless(__msqid,result,__FILE__,__LINE__,msg)
void _fail_unless (int msqid, int result, char *file, int line, char *msg);

/*! Always fail */
#define fail(msg) _fail_unless(__msqid,0,__FILE__,__LINE__,msg)

/*! Mark the last point reached in a unit test
   (useful for tracking down where a segfault, etc. occurs */
#define mark_point() _mark_point(__msqid,__FILE__,__LINE__)
void _mark_point (int msqid, char *file, int line);

/* Suite running functions */


/*! Result of a test */
enum test_result {
  CRPASS, /*!< Test passed*/
  CRFAILURE, /*!< Test completed but failed */
  CRERROR /*!< Test failed to complete (signal or non-zero early exit) */
};

/*! Specifies the verbosity of srunner printing */
enum print_verbosity {
  CRSILENT, /*!< No output */
  CRMINIMAL, /*!< Only summary output */
  CRNORMAL, /*!< All failed tests */
  CRVERBOSE, /*!< All tests */
  CRLAST
};


/*! Holds state for a running of a test suite */
typedef struct SRunner SRunner;

/*! Opaque type for a test failure */
typedef struct TestResult TestResult;

/* accessors for tr fields */

/*! Type of result */
int tr_rtype (TestResult *tr);
/*! Failure message */
char *tr_msg (TestResult *tr);
/*! Line number at which failure occured */
int tr_lno (TestResult *tr);
/*! File name at which failure occured */
char *tr_lfile (TestResult *tr);
/*! Test case in which unit test was run */
char *tr_tcname (TestResult *tr);

/*! Creates an SRunner for the given suite */
SRunner *srunner_create (Suite *s);
/*! Frees an SRunner */
void srunner_free (SRunner *sr);

/* Test running */

/*! Runs an SRunner, printing results as specified
    (see enum print_verbosity)*/
void srunner_run_all (SRunner *sr, int print_mode);

/* Next functions are valid only after the suite has been
   completely run, of course */

/*! Number of failed tests in a run suite
  Includes failures + errors */
int srunner_ntests_failed (SRunner *sr);

/*! Total number of tests run in a run suite */
int srunner_ntests_run (SRunner *sr);

/*! Return an array of results for all failures
   Number of failures is equal to srunner_nfailed_tests
   Memory is alloc'ed and must be freed, but individual
   TestResults must not */
TestResult **srunner_failures (SRunner *sr);

/*! Return an array of results for all run tests
  Number of failrues is equal to srunner_ntests_run
  Memory is alloc'ed and must be freed, but individual
  TestResults must not */
TestResult **srunner_results (SRunner *sr);
/* Printing */

/*! Print a summary report of %passed, #checks, failures */
void srunner_print_summary (SRunner *sr);

/*! Print a detailed report of test results
   See enum print_verbosity for explaination of print_mode*/
void srunner_print_results (SRunner *sr, int print_mode);


#endif /* CHECK_H */
