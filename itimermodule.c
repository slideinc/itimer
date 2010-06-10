/*
 * Copyright (c) 2005-2010 Slide, Inc
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of the author nor the names of other
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Python.h"
#include <sys/time.h>

static char module_doc[] = 
"This module wraps setitimer(2) and getitimer(2) unix syscalls. Those\n\
functions can be used as to implement subsecond alarm signal delivery.\n\
\n\
Functions:\n\n\
alarm(seconds) -- deliver alarm with subsecond precission\n\
setitimer(which,secs,interval) -- setitimer to fire\n\
getitimer(which) -- get current value of timer\n\
\n\
Constants:\n\n\
ITIMER_REAL -- real time, delivers SIGALRM\n\
ITIMER_VIRTUAL -- process virtual time, SIGVTALRM\n\
ITIMER_PROF -- process virtual time + system time, SIGPROF\n\
";

static PyObject *ErrorObject;

#define USPS    1000000.0       /* # of microseconds in a second */

static void SetTimevalFromDouble(double d,struct timeval *tv)
{
  tv->tv_sec = floor(d);
  tv->tv_usec = fmod(d,1.0)*USPS;
}

static inline double DoubleFromTimeval(struct timeval *tv)
{
  return tv->tv_sec + (double)(tv->tv_usec / USPS);
}

static PyObject *
itimer_alarm(PyObject *self, PyObject *args)
{
  double secs;
  struct itimerval new,old;
  if (!PyArg_ParseTuple(args, "d:alarm", &secs ))
    return NULL;
  /* Itimer version */
  SetTimevalFromDouble(secs,&new.it_value);
  new.it_interval.tv_sec = new.it_interval.tv_usec = 0;
  if(setitimer(ITIMER_REAL, &new, &old) != 0) {
    PyErr_SetFromErrno(ErrorObject);
    return NULL;
  }
  return PyFloat_FromDouble(DoubleFromTimeval(&old.it_value));
}

static PyObject *itimer_retval(struct itimerval *iv)
{
  PyObject *r,*v;
  if(!(r=PyTuple_New(2)))
    return NULL;
  if(!(v=PyFloat_FromDouble(DoubleFromTimeval(&iv->it_value))))
    return NULL;
  PyTuple_SET_ITEM(r,0,v);
  if(!(v=PyFloat_FromDouble(DoubleFromTimeval(&iv->it_interval))))
    return NULL;
  PyTuple_SET_ITEM(r,1,v);
  return r;
}

static PyObject *
itimer_setitimer(PyObject *self, PyObject *args)
{
  double first;
  double interval=0;
  int which;
  struct itimerval new,old;
  if(!PyArg_ParseTuple(args, "id|d:setitimer", 
		       &which, &first, &interval) )
    return NULL;
  /* Let OS do checking on which */
  SetTimevalFromDouble(first,&new.it_value);
  SetTimevalFromDouble(interval,&new.it_interval);
  if(setitimer(which, &new, &old) != 0) {
    PyErr_SetFromErrno(ErrorObject);
    return NULL;
  }
  return itimer_retval(&old);
}

static PyObject *
itimer_getitimer(PyObject *self, PyObject *args)
{
  int which;
  struct itimerval old;
  if(!PyArg_ParseTuple(args, "i:getitimer", &which))
    return NULL;
  if(getitimer(which,&old) != 0) {
    PyErr_SetFromErrno(ErrorObject);
    return NULL;
  }
  return itimer_retval(&old);
}

/* List of functions defined in the module */

static PyMethodDef itimer_methods[] = {
	{"alarm",	itimer_alarm,		METH_VARARGS,
	 "alarm(seconds)\n\n"
	 "Arrange for SIGALRM to arrive after the given number of seconds.\n"
	 "The argument may be floating point number for subsecond precision." },
	{"setitimer",	itimer_setitimer,	METH_VARARGS,
	 "setitimer(which, secs, interval)\n\n"
	 "sets given itimer to fire after value secs and after that every interval\n"
	 "seconds. Interval can be omited. Clear by setting seconds to zero.\n"
         "Returns old values as a tuple. " },
	{"getitimer",	itimer_getitimer,	METH_VARARGS, 
	 "getitimer(which)\n\n"
	 "returns current value of given itimer" },
	{NULL,		NULL}		/* sentinel */
};


/* Initialization function for the module (*must* be called initxx) */

DL_EXPORT(void)
inititimer(void)
{
	PyObject *m, *d;
	PyObject *v;

	/* Create the module and add the functions */
	m = Py_InitModule3("itimer", itimer_methods, module_doc);

	/* Add some symbolic constants to the module */
	d = PyModule_GetDict(m);
	ErrorObject = PyErr_NewException("itimer.error", NULL, NULL);
#ifdef ITIMER_REAL
	v=PyInt_FromLong(ITIMER_REAL);
	PyDict_SetItemString(d, "ITIMER_REAL", v);
	Py_DECREF(v);
#endif
#ifdef ITIMER_VIRTUAL
	v=PyInt_FromLong(ITIMER_VIRTUAL);
	PyDict_SetItemString(d, "ITIMER_VIRTUAL",v);
	Py_DECREF(v);
#endif
#ifdef ITIMER_PROF
	v=PyInt_FromLong(ITIMER_PROF);
	PyDict_SetItemString(d, "ITIMER_PROF",v);
	Py_DECREF(v);
#endif
	PyDict_SetItemString(d, "error", ErrorObject);
}

