#pragma once

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/PassManager.h"

struct llsimd : public llvm::PassInfoMixin<llsimd> {
  llvm::PreservedAnalyses run(llvm::Function &function,
                              llvm::FunctionAnalysisManager &);
  static bool run_on_basic_block(llvm::BasicBlock &basic_block);

  // Without isRequired returning true, this pass will be skipped for functions
  // decorated with the optnone LLVM attribute. Note that clang -O0 decorates
  // all functions with optnone.
  static bool isRequired() { return true; }
};
