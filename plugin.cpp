#include "llsimd.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"

using namespace llvm;

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "llsimd", LLVM_VERSION_STRING,
          [](PassBuilder &pass_builder) {
            pass_builder.registerPipelineParsingCallback(
                [](StringRef name, FunctionPassManager &function_pass_manager,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (name == "llsimd") {
                    function_pass_manager.addPass(llsimd());
                    return true;
                  }
                  return false;
                });
          }};
}
