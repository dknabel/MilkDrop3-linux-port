#pragma once

#include "milkdrop_compat.h"
#include <string>
#include <memory>
#include <unordered_map>

namespace milkdrop {

/**
 * ExpressionEvaluator - C++ wrapper for EEL2 expression evaluation
 *
 * Manages an EEL2 VM context and provides convenient methods for:
 * - Registering variables (input: audio, time, parameters)
 * - Compiling EEL2 code
 * - Executing compiled expressions
 * - Retrieving variable values
 */
class ExpressionEvaluator {
public:
  ExpressionEvaluator();
  ~ExpressionEvaluator();

  // Non-copyable
  ExpressionEvaluator(const ExpressionEvaluator&) = delete;
  ExpressionEvaluator& operator=(const ExpressionEvaluator&) = delete;

  // Moveable
  ExpressionEvaluator(ExpressionEvaluator&&) = default;
  ExpressionEvaluator& operator=(ExpressionEvaluator&&) = default;

  /**
   * Initialize the evaluator (call once before use)
   */
  bool initialize();

  /**
   * Register a variable for use in expressions
   * Returns pointer to the variable for direct modification
   */
  double *registerVariable(const std::string& name);

  /**
   * Get a variable by name (returns nullptr if not registered)
   */
  double *getVariable(const std::string& name) const;

  /**
   * Compile EEL2 code, return handle for execution
   * Returns nullptr on compilation error
   */
  NSEEL_CODEHANDLE compile(const std::string& code);

  /**
   * Execute previously compiled code, return result
   * Returns 0.0 on error (or actual 0 result)
   */
  double execute(NSEEL_CODEHANDLE handle);

  /**
   * Free compiled code handle
   */
  void freeCode(NSEEL_CODEHANDLE handle);

  /**
   * Get underlying NSEEL_VMCTX (for advanced use)
   */
  NSEEL_VMCTX getVM() { return vm_; }

  /**
   * Check if evaluator is initialized
   */
  bool isInitialized() const { return vm_ != nullptr; }

private:
  NSEEL_VMCTX vm_;
  std::unordered_map<std::string, double*> variables_;
};

}  // namespace milkdrop
