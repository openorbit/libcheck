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

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "error.h"
#include "list.h"
#include "check.h"
#include "check_impl.h"
#include "check_msg.h"
#include "check_log.h"

enum {
  CK_FORK_UNSPECIFIED = -1
};

static void srunner_run_init (SRunner *sr, enum print_output print_mode);
static void srunner_run_end (SRunner *sr, enum print_output print_mode);
static void srunner_iterate_suites (SRunner *sr,
				    enum print_output print_mode);
static void srunner_run_tcase (SRunner *sr, TCase *tc);
static void srunner_iterate_tcase_tfuns (SRunner *sr, TCase *tc);
static void srunner_add_failure (SRunner *sr, TestResult *tf);
static TestResult *tfun_run (char *tcname, TF *tf);
static TestResult *tfun_run_nofork (char *tcname, TF *tf);
static TestResult *receive_result_info (MsgSys *msgsys, int status, char *tcname);
static TestResult *receive_result_info_nofork (MsgSys *msgsys, char *tcname);
static void receive_last_loc_info (MsgSys *msgsys, TestResult *tr);
static void receive_failure_info (MsgSys *msgsys, int status, TestResult *tr);
static List *srunner_resultlst (SRunner *sr);

static char *signal_msg (int sig);
static char *exit_msg (int exitstatus);
static int non_pass (int val);


SRunner *srunner_create (Suite *s)
{
  SRunner *sr = emalloc (sizeof(SRunner)); /* freed in srunner_free */
  sr->slst = list_create();
  list_add_end(sr->slst, s);
  sr->stats = emalloc (sizeof(TestStats)); /* freed in srunner_free */
  sr->stats->n_checked = sr->stats->n_failed = sr->stats->n_errors = 0;
  sr->resultlst = list_create();
  sr->log_fname = NULL;
  sr->loglst = NULL;
  sr->fstat = CK_FORK_UNSPECIFIED;
  return sr;
}

void srunner_add_suite (SRunner *sr, Suite *s)
{
  list_add_end(sr->slst, s);
}

void srunner_free (SRunner *sr)
{
  List *l;
  TestResult *tr;
  if (sr == NULL)
    return;
  
  free (sr->stats);
  list_free(sr->slst);

  l = sr->resultlst;
  for (list_front(l); !list_at_end(l); list_advance(l)) {
    tr = list_val(l);
    free(tr->file);
    if (tr->rtype == CRFAILURE || tr->rtype == CRERROR)
      free(tr->msg);
    free(tr);
  }
  list_free (sr->resultlst);

  free (sr);
}

static void srunner_run_init (SRunner *sr, enum print_output print_mode)
{
  set_fork_status(srunner_fork_status(sr));
  srunner_init_logging (sr, print_mode);
  log_srunner_start (sr);
}

static void srunner_run_end (SRunner *sr, enum print_output print_mode)
{
  log_srunner_end (sr);
  srunner_end_logging (sr);
  set_fork_status(CK_FORK);  
}

static void srunner_iterate_suites (SRunner *sr,
				    enum print_output print_mode)
  
{
  List *slst;
  List *tcl;
  TCase *tc;

  slst = sr->slst;
  
  for (list_front(slst); !list_at_end(slst); list_advance(slst)) {
    Suite *s = list_val(slst);
    
    log_suite_start (sr, s);

    tcl = s->tclst;
  
    for (list_front(tcl);!list_at_end (tcl); list_advance (tcl)) {
      tc = list_val (tcl);
      srunner_run_tcase (sr, tc);
    }
  }
}

void srunner_run_all (SRunner *sr, enum print_output print_mode)
{
  if (sr == NULL)
    return;
  if (print_mode < 0 || print_mode >= CRLAST)
    eprintf("Bad print_mode argument to srunner_run_all: %d", print_mode);
      
  srunner_run_init (sr, print_mode);
  srunner_iterate_suites (sr, print_mode);
  srunner_run_end (sr, print_mode);
}

static void srunner_add_failure (SRunner *sr, TestResult *tr)
{
  sr->stats->n_checked++;
  list_add_end (sr->resultlst, tr);
  switch (tr->rtype) {
    
  case CRPASS:
    return;
  case CRFAILURE:
    sr->stats->n_failed++;
    return;
  case CRERROR:
    sr->stats->n_errors++;
    return;
  }
}

static void srunner_iterate_tcase_tfuns (SRunner *sr, TCase *tc)
{
  List *tfl;
  TF *tfun;
  TestResult *tr = NULL;

  tfl = tc->tflst;
  
  for (list_front(tfl); !list_at_end (tfl); list_advance (tfl)) {
    tfun = list_val (tfl);
    switch (srunner_fork_status(sr)) {
    case CK_FORK:
      tr = tfun_run (tc->name, tfun);
      break;
    case CK_NOFORK:
      tr = tfun_run_nofork (tc->name, tfun);
      break;
    default:
      eprintf("Bad fork status in SRunner");
    }
    srunner_add_failure (sr, tr);
    log_test_end(sr, tr);
  }
}
  
static void srunner_run_tcase (SRunner *sr, TCase *tc)
{
  MsgSys *msgsys;

  msgsys = init_msgsys();

  if (tc->setup)
    tc->setup();
  
  srunner_iterate_tcase_tfuns(sr,tc);
  
  if (tc->teardown)
    tc->teardown();
  delete_msgsys();
}

static void receive_last_loc_info (MsgSys *msgsys, TestResult *tr)
{
  Loc *loc;
  loc = receive_last_loc_msg (msgsys);
  if (loc == NULL) {
    char *s = emalloc (strlen ("unknown") + 1);
    strcpy(s,"unknown");
    tr->file = s;
    tr->line = -1;
  } else {
    tr->file = loc->file;
    tr->line = loc->line;
    free (loc);
  }
}  

static void receive_failure_info (MsgSys *msgsys, int status, TestResult *tr)
{
  char *fmsg;

  if (WIFSIGNALED(status)) {
    tr->rtype = CRERROR;
    tr->msg = signal_msg (WTERMSIG(status));
    return;
  }
  
  if (WIFEXITED(status)) {
    
    if (WEXITSTATUS(status) == 0) {
      tr->rtype = CRPASS;
      /* TODO: It would be cleaner to strdup this &
	 not special case the free...*/
      tr->msg = "Test passed";
    }
    else {
      
      fmsg = receive_failure_msg (msgsys);
      if (fmsg == NULL) { /* implies early exit */
	tr->rtype = CRERROR;
	tr->msg =  exit_msg (WEXITSTATUS(status));
      }
      else {
	tr->rtype = CRFAILURE;
	tr->msg = emalloc(strlen(fmsg) + 1);
	strcpy (tr->msg, fmsg);
	free (fmsg);
      }
    }
  } else {
    eprintf ("Bad status from wait() call\n");
  }
}

static TestResult *receive_result_info (MsgSys *msgsys, int status, char *tcname)
{
  TestResult *tr = emalloc (sizeof(TestResult));

  tr->tcname = tcname;
  receive_last_loc_info (msgsys, tr);
  receive_failure_info (msgsys, status, tr);
  return tr;
}

static TestResult *receive_result_info_nofork (MsgSys *msgsys, char *tcname)
{
  /*TODO: lots of overlap with receive_failure_info
    Find a way to generalize */

  char *fmsg;
  TestResult *tr = emalloc (sizeof(TestResult));

  tr->tcname = tcname;
  
  receive_last_loc_info (msgsys, tr);
  fmsg = receive_failure_msg (msgsys);
  
  if (fmsg == NULL) { /* we got through the procedure */
    tr->rtype = CRPASS;
    tr->msg = "Test passed";
  }
  else {
    tr->rtype = CRFAILURE;
    tr->msg = emalloc(strlen(fmsg) + 1);
    strcpy (tr->msg, fmsg);
    free (fmsg);
  }
  return tr;
}

static TestResult *tfun_run_nofork (char *tcname, TF *tfun)
{
  MsgSys  *msgsys;

  msgsys = get_recv_msgsys();
  tfun->fn();
  return receive_result_info_nofork (msgsys, tcname);
}

  
static TestResult *tfun_run (char *tcname, TF *tfun)
{
  pid_t pid;
  int status = 0;
  MsgSys  *msgsys;

  msgsys = get_recv_msgsys();

  pid = fork();
  if (pid == -1)
     eprintf ("Unable to fork:");
  if (pid == 0) {
    tfun->fn();
    _exit(EXIT_SUCCESS);
  }
  (void) wait(&status);
  return receive_result_info(msgsys, status, tcname);
}

int srunner_ntests_failed (SRunner *sr)
{
  return sr->stats->n_failed + sr->stats->n_errors;
}

int srunner_ntests_run (SRunner *sr)
{
  return sr->stats->n_checked;
}

TestResult **srunner_failures (SRunner *sr)
{
  int i = 0;
  TestResult **trarray;
  List *rlst;
  trarray = malloc (sizeof(trarray[0]) * srunner_ntests_failed (sr));

  rlst = srunner_resultlst (sr);
  for (list_front(rlst); !list_at_end(rlst); list_advance(rlst)) {
    TestResult *tr = list_val(rlst);
    if (non_pass(tr->rtype))
      trarray[i++] = tr;
    
  }
  return trarray;
}

TestResult **srunner_results (SRunner *sr)
{
  int i = 0;
  TestResult **trarray;
  List *rlst;

  trarray = malloc (sizeof(trarray[0]) * srunner_ntests_run (sr));

  rlst = srunner_resultlst (sr);
  for (list_front(rlst); !list_at_end(rlst); list_advance(rlst)) {
    trarray[i++] = list_val(rlst);
  }
  return trarray;
}

static List *srunner_resultlst (SRunner *sr)
{
  return sr->resultlst;
}

char *tr_msg (TestResult *tr)
{
  return tr->msg;
}

int tr_lno (TestResult *tr)
{
  return tr->line;
}

char *tr_lfile (TestResult *tr)
{
  return tr->file;
}

int tr_rtype (TestResult *tr)
{
  return tr->rtype;
}

char *tr_tcname (TestResult *tr)
{
  return tr->tcname;
}


static char *signal_msg (int signal)
{
  char *msg = emalloc (CMAXMSG); /* free'd by caller */
  snprintf(msg, CMAXMSG, "Received signal %d", signal);
  return msg;
}

static char *exit_msg (int exitval)
{
  char *msg = emalloc(CMAXMSG); /* free'd by caller */
  snprintf(msg, CMAXMSG,
	   "Early exit with return value %d", exitval);
  return msg;
}

static int non_pass (int val)
{
  return val == CRFAILURE || val == CRERROR;
}

enum fork_status srunner_fork_status (SRunner *sr)
{
  if (sr->fstat == CK_FORK_UNSPECIFIED) {
    char *env = getenv("CK_FORK");
    if (env == NULL)
      return CK_FORK;
    if (strcmp(env,"no") == 0)
      return CK_NOFORK;
    else
      return CK_FORK;
  } else
    return sr->fstat;
}

void srunner_set_fork_status (SRunner *sr, enum fork_status fstat)
{
  sr->fstat = fstat;
}

