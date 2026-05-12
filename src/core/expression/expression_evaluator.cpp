#include "expression_evaluator.h"
#include <iostream>
#include <cstring>

namespace milkdrop {

ExpressionEvaluator::ExpressionEvaluator() : vm_(nullptr) {}

ExpressionEvaluator::~ExpressionEvaluator() {
  // Free all cached code handles first
  for (auto& pair : codeCache_) {
    NSEEL_code_free(pair.second);
  }
  codeCache_.clear();

  // Then free the VM
  if (vm_) {
    NSEEL_VM_free(vm_);
    vm_ = nullptr;
  }
}

bool ExpressionEvaluator::initialize() {
  if (vm_) {
    std::cerr << "ExpressionEvaluator already initialized" << std::endl;
    return false;
  }

  // Initialize EEL2 library (one-time setup)
  NSEEL_init();

  // Create a new VM context
  vm_ = NSEEL_VM_alloc();
  if (!vm_) {
    std::cerr << "Failed to allocate EEL2 VM context" << std::endl;
    return false;
  }

  // Reserve capacity for typical preset variables
  variables_.reserve(VARIABLE_MAP_INITIAL_SIZE);

  return true;
}

double *ExpressionEvaluator::registerVariable(const std::string& name) {
  if (!vm_) {
    std::cerr << "ExpressionEvaluator not initialized" << std::endl;
    return nullptr;
  }

  // Register variable in EEL2 VM
  double *var = NSEEL_VM_regvar(vm_, name.c_str());
  if (!var) {
    std::cerr << "Failed to register variable: " << name << std::endl;
    return nullptr;
  }

  // Cache in our map
  variables_[name] = var;
  return var;
}

double *ExpressionEvaluator::getVariable(const std::string& name) const {
  auto it = variables_.find(name);
  if (it == variables_.end()) {
    return nullptr;
  }
  return it->second;
}

NSEEL_CODEHANDLE ExpressionEvaluator::compile(const std::string& code) {
  if (!vm_) {
    std::cerr << "ExpressionEvaluator not initialized" << std::endl;
    return nullptr;
  }

  if (code.empty()) {
    std::cerr << "Cannot compile empty code" << std::endl;
    return nullptr;
  }

  // Check if already compiled
  auto it = codeCache_.find(code);
  if (it != codeCache_.end()) {
    return it->second;
  }

  // Compile new code
  NSEEL_CODEHANDLE handle = NSEEL_code_compile(vm_, code.c_str());
  if (!handle) {
    std::cerr << "Failed to compile EEL2 code" << std::endl;
    return nullptr;
  }

  // Cache the compiled handle
  codeCache_[code] = handle;
  return handle;
}

double ExpressionEvaluator::execute(NSEEL_CODEHANDLE handle) {
  if (!handle) {
    std::cerr << "Invalid code handle for execution" << std::endl;
    return 0.0;
  }

  double result = NSEEL_code_execute(handle);
  return result;
}

void ExpressionEvaluator::freeCode(NSEEL_CODEHANDLE handle) {
  if (handle) {
    NSEEL_code_free(handle);
  }
}

}  // namespace milkdrop
