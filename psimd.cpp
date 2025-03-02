#include "psimd.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

#define DEBUG_TYPE "psimd"

STATISTIC(basic_block_count, "basic_block_count");

bool psimd::runOnBasicBlock(BasicBlock &BB) {
  bool changed = false;

  ++basic_block_count;

  return changed;
}

PreservedAnalyses psimd::run(llvm::Function &F,
                             llvm::FunctionAnalysisManager &) {
  bool changed = false;

  for (auto &BB : F)
    changed |= runOnBasicBlock(BB);

  return (changed ? llvm::PreservedAnalyses::none()
                  : llvm::PreservedAnalyses::all());
}

llvm::PassPluginLibraryInfo getPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "psimd", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "psimd") {
                    FPM.addPass(psimd());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getPassPluginInfo();
}
