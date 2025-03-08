#include "psimd.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

using namespace llvm;

#define DEBUG_TYPE "psimd"

STATISTIC(mmx_intrinsic_count, "MMX intrinsics");
STATISTIC(simd_intrinsic_count, "Total SIMD intrinsics");

bool psimd::run_on_basic_block(BasicBlock &basic_block) {
  bool changed = false;

  std::vector<Instruction *> to_remove;

  for (auto &instruction : basic_block) {
    auto *call_base = dyn_cast<CallBase>(&instruction);
    if (call_base == nullptr)
      continue;

    Function *function = call_base->getCalledFunction();
    if (function == nullptr)
      continue;

    Intrinsic::ID intrinsic_id = function->getIntrinsicID();
    if (intrinsic_id == Intrinsic::not_intrinsic)
      continue;

    StringRef instrinsic_name = Intrinsic::getBaseName(intrinsic_id);

    if (!instrinsic_name.consume_front("llvm.x86."))
      continue;

    changed = true;
    errs() << "Intrinsic : " << instrinsic_name << '\n';

    if (instrinsic_name.consume_front("mmx.")) {
      if (instrinsic_name == "emms")
        to_remove.push_back(&instruction);

      ++mmx_intrinsic_count;
    }

    ++simd_intrinsic_count;
  }

  for (Instruction *instruction : to_remove)
    instruction->eraseFromParent();

  return changed;
}

PreservedAnalyses psimd::run(llvm::Function &function,
                             llvm::FunctionAnalysisManager &) {
  bool changed = false;

  for (auto &basic_block : function)
    changed |= run_on_basic_block(basic_block);

  return (changed ? llvm::PreservedAnalyses::none()
                  : llvm::PreservedAnalyses::all());
}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "psimd", LLVM_VERSION_STRING,
          [](PassBuilder &pass_builder) {
            pass_builder.registerPipelineParsingCallback(
                [](StringRef name, FunctionPassManager &function_pass_manager,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (name == "psimd") {
                    function_pass_manager.addPass(psimd());
                    return true;
                  }
                  return false;
                });
          }};
}
