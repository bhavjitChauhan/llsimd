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
STATISTIC(sse_intrinsic_count, "SSE intrinsics");
STATISTIC(simd_intrinsic_count, "Total SIMD intrinsics");

bool psimd::run_on_basic_block(BasicBlock &basic_block) {
  bool changed = false;

  std::vector<Instruction *> to_remove;

  for (auto &instruction : basic_block) {
    auto *call_inst = dyn_cast<CallInst>(&instruction);
    if (call_inst == nullptr)
      continue;

    Function *function = call_inst->getCalledFunction();
    if (function == nullptr)
      continue;

    Intrinsic::ID intrinsic_id = function->getIntrinsicID();
    if (intrinsic_id == Intrinsic::not_intrinsic)
      continue;

    StringRef intrinsic_name = Intrinsic::getName(intrinsic_id);

    if (!intrinsic_name.consume_front("llvm.x86."))
      continue;

    changed = true;
    errs() << "Intrinsic : " << intrinsic_name << '\n';

    if (intrinsic_name.consume_front("mmx.")) {
      if (intrinsic_name == "emms")
        to_remove.push_back(&instruction);

      ++mmx_intrinsic_count;
    } else if (intrinsic_name.consume_front("sse2.")) {
      if (intrinsic_name.consume_front("psll.")) {
        /*
        ; %result = call <8 x i16> @llvm.x86.sse2.psll.w(<8 x i16> %m1, <8 x i16> %m2)
        %shuffle = shufflevector <8 x i16> %m2, <8 x i16> undef, <8 x i32> <i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0>
        %result = shl <8 x i16> %9, %shuffle
        */

        Value *m1 = call_inst->getArgOperand(0);
        Value *m2 = call_inst->getArgOperand(1);

        IRBuilder<> builder(&instruction);
        Value *undef = UndefValue::get(m2->getType());
        Value *shuffle = builder.CreateShuffleVector(
            m2, undef, ConstantAggregateZero::get(m2->getType()));
        Value *result = builder.CreateShl(m1, shuffle);

        call_inst->replaceAllUsesWith(result);
        to_remove.push_back(&instruction);
      } else if (intrinsic_name.consume_front("pslli.")) {
        /*
        ; %result = call <8 x i16> @llvm.x86.sse2.pslli.w(<8 x i16> %m, i32 %count)
        %trunc = trunc i32 %count to i16
        %insert = insertelement <8 x i16> undef, i16 %trunc, i32 0
        %shuffle = shufflevector <8 x i16> %insert, <8 x i16> undef, <8 x i32> <i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0>
        %result = shl <8 x i16> %m, %shuffle
        */

        Value *m = call_inst->getArgOperand(0);
        Value *count = call_inst->getArgOperand(1);

        IRBuilder<> builder(&instruction);
        Value *convert = nullptr;
        switch (m->getType()->getScalarSizeInBits()) {
        case 16:
          convert = builder.CreateTrunc(count, m->getType()->getScalarType());
          break;
        case 32:
          convert = count;
          break;
        case 64:
          convert = builder.CreateZExt(count, m->getType()->getScalarType());
          break;
        default:
          llvm_unreachable("Unknown intrinsic");
        }

        Value *undef = UndefValue::get(m->getType());
        Value *insert =
            builder.CreateInsertElement(undef, convert, builder.getInt32(0));
        Value *shuffle = builder.CreateShuffleVector(
            insert, undef, ConstantAggregateZero::get(m->getType()));
        Value *result = builder.CreateShl(m, shuffle);

        call_inst->replaceAllUsesWith(result);
        to_remove.push_back(&instruction);
      }

      ++sse_intrinsic_count;
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
