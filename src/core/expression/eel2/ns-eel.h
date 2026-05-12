/*
 * ns-eel.h - Nullsoft Expression Evaluator v2 (public API)
 * Extracted from Milkdrop3 /code/ns-eel2/
 *
 * This file will be replaced with actual source from Milkdrop3.
 * For now: stub to allow CMake integration to proceed.
 */

#ifndef _NS_EEL_H_
#define _NS_EEL_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void *NSEEL_VMCTX;
typedef void *NSEEL_CODEHANDLE;

/* Initialize EEL2 library */
void NSEEL_init(void);

/* Create a new VM context */
NSEEL_VMCTX NSEEL_VM_alloc(void);

/* Register a variable in the VM */
double *NSEEL_VM_regvar(NSEEL_VMCTX ctx, const char *name);

/* Compile an expression */
NSEEL_CODEHANDLE NSEEL_code_compile(NSEEL_VMCTX ctx, const char *code);

/* Execute compiled code, return result */
double NSEEL_code_execute(NSEEL_CODEHANDLE handle);

/* Free compiled code */
void NSEEL_code_free(NSEEL_CODEHANDLE handle);

/* Free VM context */
void NSEEL_VM_free(NSEEL_VMCTX ctx);

#ifdef __cplusplus
}
#endif

#endif
