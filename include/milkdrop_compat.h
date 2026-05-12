#pragma once

/*
 * milkdrop_compat.h
 *
 * Compatibility layer for EEL2 integration.
 * Provides C++ wrappers and type definitions for Milkdrop3 interoperability.
 */

#include <cstdint>
#include <cstddef>

// Re-export EEL2 public API
#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations (from ns-eel.h)
typedef void *NSEEL_VMCTX;
typedef void *NSEEL_CODEHANDLE;

// Stub declarations (actual definitions in eel2/ns-eel.h)
void NSEEL_init(void);
NSEEL_VMCTX NSEEL_VM_alloc(void);
double *NSEEL_VM_regvar(NSEEL_VMCTX ctx, const char *name);
NSEEL_CODEHANDLE NSEEL_code_compile(NSEEL_VMCTX ctx, const char *code);
double NSEEL_code_execute(NSEEL_CODEHANDLE handle);
void NSEEL_code_free(NSEEL_CODEHANDLE handle);
void NSEEL_VM_free(NSEEL_VMCTX ctx);

#ifdef __cplusplus
}
#endif

namespace milkdrop {

// C++ wrapper to safely manage EEL2 VM lifetime
class EEL2VM {
public:
  EEL2VM();
  ~EEL2VM();

  // Deleted copy/move (VMs are non-transferable)
  EEL2VM(const EEL2VM&) = delete;
  EEL2VM& operator=(const EEL2VM&) = delete;

  NSEEL_VMCTX get() { return vm_; }
  bool is_valid() const { return vm_ != nullptr; }

private:
  NSEEL_VMCTX vm_;
};

}  // namespace milkdrop
