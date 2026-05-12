/*
 * nseel-eval.c - EEL2 Evaluator implementation stub
 * This is a minimal implementation to allow compilation and linking.
 * A full implementation would be integrated from Milkdrop3 source.
 */

#include "ns-eel.h"
#include "ns-eel-int.h"
#include <stdlib.h>
#include <string.h>

/* Stub implementations for EEL2 functions */

static int eel2_initialized = 0;

void NSEEL_init(void) {
  eel2_initialized = 1;
}

NSEEL_VMCTX NSEEL_VM_alloc(void) {
  if (!eel2_initialized) {
    NSEEL_init();
  }
  /* Allocate a simple VM context (for now, just a dummy pointer) */
  return malloc(sizeof(struct NSEEL_VMCTX_));
}

double *NSEEL_VM_regvar(NSEEL_VMCTX ctx, const char *name) {
  /* Stub: return a simple double value */
  if (!ctx) return NULL;
  double *var = (double *)malloc(sizeof(double));
  *var = 0.0;
  return var;
}

NSEEL_CODEHANDLE NSEEL_code_compile(NSEEL_VMCTX ctx, const char *code) {
  /* Stub: return a dummy handle */
  if (!ctx || !code) return NULL;
  return malloc(sizeof(int));
}

double NSEEL_code_execute(NSEEL_CODEHANDLE handle) {
  /* Stub: return 0.0 */
  if (!handle) return 0.0;
  return 0.0;
}

void NSEEL_code_free(NSEEL_CODEHANDLE handle) {
  if (handle) {
    free(handle);
  }
}

void NSEEL_VM_free(NSEEL_VMCTX ctx) {
  if (ctx) {
    free(ctx);
  }
}
