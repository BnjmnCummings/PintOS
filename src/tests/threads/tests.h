#ifndef TESTS_THREADS_TESTS_H
#define TESTS_THREADS_TESTS_H

#include "tests/devices/tests.h"

typedef void test_func (void);

extern void run_test (const char *);
extern void msg (const char *, ...);
extern void fail (const char *, ...);
extern void pass (void);

#endif /* tests/threads/tests.h */
