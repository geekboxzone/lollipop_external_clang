// RUN: clang -std=gnu99 -checker-simple -verify %s &&
// RUN: clang -std=gnu99 -checker-simple -analyzer-store-region -verify %s

#include<stdint.h>
#include <assert.h>

void f1(int *p) {  
  if (p) *p = 1;
  else *p = 0; // expected-warning{{ereference}}
}

struct foo_struct {
  int x;
};

int f2(struct foo_struct* p) {
  
  if (p)
    p->x = 1;
    
  return p->x++; // expected-warning{{Dereference of null pointer.}}
}

int f3(char* x) {
  
  int i = 2;
  
  if (x)
    return x[i - 1];
  
  return x[i+1]; // expected-warning{{Dereference of null pointer.}}
}

int f3_b(char* x) {
  
  int i = 2;
  
  if (x)
    return x[i - 1];
  
  return x[i+1]++; // expected-warning{{Dereference of null pointer.}}
}

int f4(int *p) {
  
  uintptr_t x = (uintptr_t) p;
  
  if (x)
    return 1;
    
  int *q = (int*) x;
  return *q; // expected-warning{{Dereference of null pointer.}}
}

int f5() {
  
  char *s = "hello world";
  return s[0]; // no-warning
}

int bar(int* p, int q) __attribute__((nonnull));

int f6(int *p) { 
  return !p ? bar(p, 1) // expected-warning {{Null pointer passed as an argument to a 'nonnull' parameter}}
         : bar(p, 0);   // no-warning
}

int* qux();

int f7(int x) {
  
  int* p = 0;
  
  if (0 == x)
    p = qux();
  
  if (0 == x)
    *p = 1; // no-warning
    
  return x;
}

int f8(int *p, int *q) {
  if (!p)
    if (p)
      *p = 1; // no-warning
  
  if (q)
    if (!q)
      *q = 1; // no-warning
}

int* qux();

int f9(unsigned len) {
  assert (len != 0);
  int *p = 0;
  unsigned i;

  for (i = 0; i < len; ++i)
   p = qux(i);

  return *p++; // no-warning
}

int f9b(unsigned len) {
  assert (len > 0);  // note use of '>'
  int *p = 0;
  unsigned i;

  for (i = 0; i < len; ++i)
   p = qux(i);

  return *p++; // no-warning
}

int* f10(int* p, signed char x, int y) {
  // This line tests symbolication with compound assignments where the
  // LHS and RHS have different bitwidths.  The new symbolic value
  // for 'x' should have a bitwidth of 8.
  x &= y;
  
  // This tests that our symbolication worked, and that we correctly test
  // x against 0 (with the same bitwidth).
  if (!x) {
    if (!p) return;
    *p = 10;
  }
  else p = 0;

  if (!x)
    *p = 5; // no-warning

  return p;
}

// Test case from <rdar://problem/6407949>
void f11(unsigned i) {
  int *x = 0;
  if (i >= 0) {
    // always true
  } else {
    *x = 42; // no-warning
  }
}

void f11b(unsigned i) {
  int *x = 0;
  if (i <= ~(unsigned)0) {
    // always true
  } else {
    *x = 42; // no-warning
  }
}

