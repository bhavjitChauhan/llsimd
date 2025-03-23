#include "llsimd.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;

#define DEBUG_TYPE "llsimd"

STATISTIC(mmx_intrinsic_count, "MMX intrinsics");
STATISTIC(sse_intrinsic_count, "SSE intrinsics");
STATISTIC(simd_intrinsic_count, "Total SIMD intrinsics");

bool llsimd::run_on_basic_block(BasicBlock &basic_block) {
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
      if (intrinsic_name == "pmadd.wd") {
        /*
        ; %result = call <4 x i32> @llvm.x86.sse2.pmadd.wd(<8 x i16> %m1, <8 x i16> %m2)
        %sext1 = sext <8 x i16> %m1 to <8 x i32>
        %sext2 = sext <8 x i16> %m2 to <8 x i32>
        %mul = mul <8 x i32> %sext1, %sext2
        %even = shufflevector <8 x i32> %mul, <8 x i32> undef, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
        %odd = shufflevector <8 x i32> %mul, <8 x i32> undef, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
        %result = add <4 x i32> %even, %odd
        */

        Value *m1 = call_inst->getArgOperand(0);
        Value *m2 = call_inst->getArgOperand(1);

        IRBuilder<> builder(&instruction);
        Value *sext1 = builder.CreateSExt(
            m1, VectorType::get(builder.getInt32Ty(), 8, false), "sext1");
        Value *sext2 = builder.CreateSExt(
            m2, VectorType::get(builder.getInt32Ty(), 8, false), "sext2");
        Value *mul = builder.CreateMul(sext1, sext2, "mul");
        Value *even = builder.CreateShuffleVector(
            mul, UndefValue::get(mul->getType()),
            ConstantDataVector::get(builder.getContext(),
                                    ArrayRef<uint8_t>({0, 2, 4, 6})), "even");
        Value *odd = builder.CreateShuffleVector(
            mul, UndefValue::get(mul->getType()),
            ConstantDataVector::get(builder.getContext(),
                                    ArrayRef<uint8_t>({1, 3, 5, 7})), "odd");
        Value *result = builder.CreateAdd(even, odd, "result");

        call_inst->replaceAllUsesWith(result);
        to_remove.push_back(&instruction);
      } else if (intrinsic_name == "pmulh.w") {
        /*
        ; %result = call <8 x i16> @llvm.x86.sse2.pmulh.w(<8 x i16> %m1, <8 x i16> %m2)
        %sext1 = sext <8 x i16> %m1 to <8 x i32>
        %sext2 = sext <8 x i16> %m2 to <8 x i32>
        %mul = mul <8 x i32> %sext1, %sext2
        %high = ashr <8 x i32> %mul, <i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16, i32 16>
        %result = trunc <8 x i32> %high to <8 x i16>
        */

        Value *m1 = call_inst->getArgOperand(0);
        Value *m2 = call_inst->getArgOperand(1);

        IRBuilder<> builder(&instruction);
        Value *sext1 = builder.CreateSExt(
            m1, VectorType::get(builder.getInt32Ty(), 8, false), "sext1");
        Value *sext2 = builder.CreateSExt(
            m2, VectorType::get(builder.getInt32Ty(), 8, false), "sext2");
        Value *mul = builder.CreateMul(sext1, sext2, "mul");
        Value *high = builder.CreateAShr(
            mul, ConstantDataVector::get(
                     builder.getContext(),
                     ArrayRef<uint32_t>({16, 16, 16, 16, 16, 16, 16, 16})), "high");
        Value *result = builder.CreateTrunc(
            high, VectorType::get(builder.getInt16Ty(), 8, false), "result");

        call_inst->replaceAllUsesWith(result);
        to_remove.push_back(&instruction);
      } else if (intrinsic_name == "packsswb.128") {
        /*
        ; %result = call <16 x i8> @llvm.x86.sse2.packsswb.128(<8 x i16> %m1, <8 x i16> %m2)
        %min1 = call <8 x i16> @llvm.smax.v8i16(<8 x i16> %m1, <8 x i16> splat(i16 -128))
        %max1 = call <8 x i16> @llvm.smin.v8i16(<8 x i16> %min1, <8 x i16> splat(i16 127))
        %min2 = call <8 x i16> @llvm.smax.v8i16(<8 x i16> %m2, <8 x i16> splat(i16 -128))
        %max2 = call <8 x i16> @llvm.smin.v8i16(<8 x i16> %min2, <8 x i16> splat(i16 127))
        %trunc1 = trunc <8 x i16> %max1 to <8 x i8>
        %trunc2 = trunc <8 x i16> %max2 to <8 x i8>
        %result = shufflevector <8 x i8> %trunc1, <8 x i8> %trunc2, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
        */

        Value *m1 = call_inst->getArgOperand(0);
        Value *m2 = call_inst->getArgOperand(1);

        IRBuilder<> builder(&instruction);
        Value *splat_min =
            builder.CreateVectorSplat(8, builder.getInt16(INT8_MIN), "splat_min");
        Value *splat_max =
            builder.CreateVectorSplat(8, builder.getInt16(INT8_MAX), "splat_max");
        Value *min1 = builder.CreateCall(
            Intrinsic::getOrInsertDeclaration(
                function->getParent(), Intrinsic::smax,
                {VectorType::get(builder.getInt16Ty(), 8, false)}),
            {m1, splat_min}, "min1");
        Value *max1 = builder.CreateCall(
            Intrinsic::getOrInsertDeclaration(
                function->getParent(), Intrinsic::smin,
                {VectorType::get(builder.getInt16Ty(), 8, false)}),
            {min1, splat_max}, "max1");
        Value *min2 = builder.CreateCall(
            Intrinsic::getOrInsertDeclaration(
                function->getParent(), Intrinsic::smax,
                {VectorType::get(builder.getInt16Ty(), 8, false)}),
            {m2, splat_min}, "min2");
        Value *max2 = builder.CreateCall(
            Intrinsic::getOrInsertDeclaration(
                function->getParent(), Intrinsic::smin,
                {VectorType::get(builder.getInt16Ty(), 8, false)}),
            {min2, splat_max}, "max2");
        Value *trunc1 = builder.CreateTrunc(
            max1, VectorType::get(builder.getInt8Ty(), 8, false), "trunc1");
        Value *trunc2 = builder.CreateTrunc(
            max2, VectorType::get(builder.getInt8Ty(), 8, false), "trunc2");
        Value *result = builder.CreateShuffleVector(
            trunc1, trunc2,
            ConstantDataVector::get(
                builder.getContext(),
                ArrayRef<u_int8_t>(
                    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15})), "result");

        call_inst->replaceAllUsesWith(result);
        to_remove.push_back(&instruction);
      } else if (intrinsic_name == "packssdw.128") {
        /*
        ; %result = call <8 x i16> @llvm.x86.sse2.packssdw.128(<4 x i32> %m1, <4 x i32> %m2)
        %min1 = call <4 x i32> @llvm.smax.v4i32(<4 x i32> %m1, <4 x i32> splat(i32 -32768))
        %max1 = call <4 x i32> @llvm.smin.v4i32(<4 x i32> %min1, <4 x i32> splat(i32 32767))
        %min2 = call <4 x i32> @llvm.smax.v4i32(<4 x i32> %m2, <4 x i32> splat(i32 -32768))
        %max2 = call <4 x i32> @llvm.smin.v4i32(<4 x i32> %min2, <4 x i32> splat(i32 32767))
        %trunc1 = trunc <4 x i32> %max1 to <4 x i16>
        %trunc2 = trunc <4 x i32> %max2 to <4 x i16>
        %result = shufflevector <4 x i16> %trunc1, <4 x i16> %trunc2, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
        */

        Value *m1 = call_inst->getArgOperand(0);
        Value *m2 = call_inst->getArgOperand(1);

        IRBuilder<> builder(&instruction);
        Value *splat_min =
            builder.CreateVectorSplat(4, builder.getInt32(INT16_MIN), "splat_min");
        Value *splat_max =
            builder.CreateVectorSplat(4, builder.getInt32(INT16_MAX), "splat_max");
        Value *min1 = builder.CreateCall(
            Intrinsic::getOrInsertDeclaration(
                function->getParent(), Intrinsic::smax,
                {VectorType::get(builder.getInt32Ty(), 4, false)}),
            {m1, splat_min}, "min1");
        Value *max1 = builder.CreateCall(
            Intrinsic::getOrInsertDeclaration(
                function->getParent(), Intrinsic::smin,
                {VectorType::get(builder.getInt32Ty(), 4, false)}),
            {min1, splat_max}, "max1");
        Value *min2 = builder.CreateCall(
            Intrinsic::getOrInsertDeclaration(
                function->getParent(), Intrinsic::smax,
                {VectorType::get(builder.getInt32Ty(), 4, false)}),
            {m2, splat_min}, "min2");
        Value *max2 = builder.CreateCall(
            Intrinsic::getOrInsertDeclaration(
                function->getParent(), Intrinsic::smin,
                {VectorType::get(builder.getInt32Ty(), 4, false)}),
            {min2, splat_max}, "max2");
        Value *trunc1 = builder.CreateTrunc(
            max1, VectorType::get(builder.getInt16Ty(), 4, false), "trunc1");
        Value *trunc2 = builder.CreateTrunc(
            max2, VectorType::get(builder.getInt16Ty(), 4, false), "trunc2");
        Value *result = builder.CreateShuffleVector(
            trunc1, trunc2,
            ConstantDataVector::get(
                builder.getContext(),
                ArrayRef<u_int8_t>({0, 1, 2, 3, 4, 5, 6, 7})), "result");

        call_inst->replaceAllUsesWith(result);
        to_remove.push_back(&instruction);
      } else if (intrinsic_name == "packuswb.128") {
        /*
        ; %result = call <16 x i8> @llvm.x86.sse2.packuswb.128(<8 x i16> %m1, <8 x i16> %m2)
        %min1 = call <8 x i16> @llvm.smax.v8i16(<8 x i16> %m1, <8 x i16> zeroinitializer)
        %max1 = call <8 x i16> @llvm.smin.v8i16(<8 x i16> %min1, <8 x i16> splat(i16 255))
        %min2 = call <8 x i16> @llvm.smax.v8i16(<8 x i16> %m2, <8 x i16> zeroinitializer)
        %max2 = call <8 x i16> @llvm.smin.v8i16(<8 x i16> %min2, <8 x i16> splat(i16 255))
        %trunc1 = trunc <8 x i16> %max1 to <8 x i8>
        %trunc2 = trunc <8 x i16> %max2 to <8 x i8>
        %result = shufflevector <8 x i8> %trunc1, <8 x i8> %trunc2, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
        */

        Value *m1 = call_inst->getArgOperand(0);
        Value *m2 = call_inst->getArgOperand(1);

        IRBuilder<> builder(&instruction);
        Value *splat_max =
            builder.CreateVectorSplat(8, builder.getInt16(UINT8_MAX), "splat_max");
        Value *min1 = builder.CreateCall(
            Intrinsic::getOrInsertDeclaration(
                function->getParent(), Intrinsic::smax,
                {VectorType::get(builder.getInt16Ty(), 8, false)}),
            {m1, ConstantAggregateZero::get(m1->getType())}, "min1");
        Value *max1 = builder.CreateCall(
            Intrinsic::getOrInsertDeclaration(
                function->getParent(), Intrinsic::smin,
                {VectorType::get(builder.getInt16Ty(), 8, false)}),
            {min1, splat_max}, "max1");
        Value *min2 = builder.CreateCall(
            Intrinsic::getOrInsertDeclaration(
                function->getParent(), Intrinsic::smax,
                {VectorType::get(builder.getInt16Ty(), 8, false)}),
            {m2, ConstantAggregateZero::get(m2->getType())}, "min2");
        Value *max2 = builder.CreateCall(
            Intrinsic::getOrInsertDeclaration(
                function->getParent(), Intrinsic::smin,
                {VectorType::get(builder.getInt16Ty(), 8, false)}),
            {min2, splat_max}, "max2");
        Value *trunc1 = builder.CreateTrunc(
            max1, VectorType::get(builder.getInt8Ty(), 8, false), "trunc1");
        Value *trunc2 = builder.CreateTrunc(
            max2, VectorType::get(builder.getInt8Ty(), 8, false), "trunc2");
        Value *result = builder.CreateShuffleVector(
            trunc1, trunc2,
            ConstantDataVector::get(
                builder.getContext(),
                ArrayRef<u_int8_t>(
                    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15})), "result");

        call_inst->replaceAllUsesWith(result);
        to_remove.push_back(&instruction);
      } else if (intrinsic_name.consume_front("psll.")) {
        /*
        ; %result = call <8 x i16> @llvm.x86.sse2.psll.w(<8 x i16> %m1, <8 x i16> %m2)
        %shuffle = shufflevector <8 x i16> %m2, <8 x i16> undef, <8 x i32> zeroinitializer
        %result = shl <8 x i16> %m1, %shuffle
        */

        Value *m1 = call_inst->getArgOperand(0);
        Value *m2 = call_inst->getArgOperand(1);

        IRBuilder<> builder(&instruction);
        Value *undef = UndefValue::get(m2->getType());
        Value *shuffle = builder.CreateShuffleVector(
            m2, undef, ConstantAggregateZero::get(m2->getType()), "shuffle");
        Value *result = builder.CreateShl(m1, shuffle, "result");

        call_inst->replaceAllUsesWith(result);
        to_remove.push_back(&instruction);
      } else if (intrinsic_name.consume_front("pslli.")) {
        /*
        ; %result = call <8 x i16> @llvm.x86.sse2.pslli.w(<8 x i16> %m, i32 %count)
        %trunc = trunc i32 %count to i16
        %insert = insertelement <8 x i16> undef, i16 %trunc, i32 0
        %shuffle = shufflevector <8 x i16> %insert, <8 x i16> undef, <8 x i32> zeroinitializer
        %result = shl <8 x i16> %m, %shuffle
        */

        Value *m = call_inst->getArgOperand(0);
        Value *count = call_inst->getArgOperand(1);

        IRBuilder<> builder(&instruction);
        Value *convert = nullptr;
        switch (m->getType()->getScalarSizeInBits()) {
        case 16:
          convert = builder.CreateTrunc(count, m->getType()->getScalarType(), "convert");
          break;
        case 32:
          convert = count;
          break;
        case 64:
          convert = builder.CreateZExt(count, m->getType()->getScalarType(), "convert");
          break;
        default:
          llvm_unreachable("Unknown intrinsic");
        }
        Value *undef = UndefValue::get(m->getType());
        Value *insert =
            builder.CreateInsertElement(undef, convert, builder.getInt32(0), "insert");
        Value *shuffle = builder.CreateShuffleVector(
            insert, undef, ConstantAggregateZero::get(m->getType()), "shuffle");
        Value *result = builder.CreateShl(m, shuffle, "result");

        call_inst->replaceAllUsesWith(result);
        to_remove.push_back(&instruction);
      } else if (intrinsic_name.consume_front("psra.")) {
        /*
        ; %result = call <8 x i16> @llvm.x86.sse2.psra.w(<8 x i16> %m1, <8 x i16> %m2)
        %shuffle = shufflevector <8 x i16> %m2, <8 x i16> undef, <8 x i32> zeroinitializer
        %result = ashr <8 x i16> %m1, %shuffle
        */

        Value *m1 = call_inst->getArgOperand(0);
        Value *m2 = call_inst->getArgOperand(1);

        IRBuilder<> builder(&instruction);
        Value *undef = UndefValue::get(m2->getType());
        Value *shuffle = builder.CreateShuffleVector(
            m2, undef, ConstantAggregateZero::get(m2->getType()), "shuffle");
        Value *result = builder.CreateAShr(m1, shuffle, "result");

        call_inst->replaceAllUsesWith(result);
        to_remove.push_back(&instruction);
      } else if (intrinsic_name.consume_front("psrai.")) {
        /*
        ; %result = call <8 x i16> @llvm.x86.sse2.psrai.w(<8 x i16> %m1, i32 %count)
        %trunc = trunc i32 %count to i16
        %insert = insertelement <8 x i16> undef, i16 %trunc, i32 0
        %shuffle = shufflevector <8 x i16> %insert, <8 x i16> undef, <8 x i32> zeroinitializer
        %result = ashr <8 x i16> %m1, %shuffle
        */

        Value *m1 = call_inst->getArgOperand(0);
        Value *m2 = call_inst->getArgOperand(1);

        IRBuilder<> builder(&instruction);
        Value *convert = nullptr;
        switch (m1->getType()->getScalarSizeInBits()) {
        case 16:
          convert = builder.CreateTrunc(m2, m1->getType()->getScalarType(), "convert");
          break;
        case 32:
          convert = m2;
          break;
        default:
          llvm_unreachable("Unknown intrinsic");
        }
        Value *undef = UndefValue::get(m1->getType());
        Value *insert =
            builder.CreateInsertElement(undef, convert, builder.getInt32(0), "insert");
        Value *shuffle = builder.CreateShuffleVector(
            insert, undef, ConstantAggregateZero::get(m1->getType()), "shuffle");
        Value *result = builder.CreateAShr(m1, shuffle, "result");

        call_inst->replaceAllUsesWith(result);
        to_remove.push_back(&instruction);
      } else if (intrinsic_name.consume_front("psrl.")) {
        /*
        ; %result = call <8 x i16> @llvm.x86.sse2.psrl.w(<8 x i16> %m1, <8 x i16> %m2)
        %shuffle = shufflevector <8 x i16> %m2, <8 x i16> undef, <8 x i32> zeroinitializer
        %result = lshr <8 x i16> %m1, %shuffle
        */

        Value *m1 = call_inst->getArgOperand(0);
        Value *m2 = call_inst->getArgOperand(1);

        IRBuilder<> builder(&instruction);
        Value *undef = UndefValue::get(m2->getType());
        Value *shuffle = builder.CreateShuffleVector(
            m2, undef, ConstantAggregateZero::get(m2->getType()), "shuffle");
        Value *result = builder.CreateLShr(m1, shuffle, "result");

        call_inst->replaceAllUsesWith(result);
        to_remove.push_back(&instruction);
      } else if (intrinsic_name.consume_front("psrli.")) {
        /*
        ; %result = call <8 x i16> @llvm.x86.sse2.psrli.w(<8 x i16> %m, i32 %count)
        %trunc = trunc i32 %count to i16
        %insert = insertelement <8 x i16> undef, i16 %trunc, i32 0
        %shuffle = shufflevector <8 x i16> %insert, <8 x i16> undef, <8 x i32> zeroinitializer
        %result = lshr <8 x i16> %m, %shuffle
        */

        Value *m = call_inst->getArgOperand(0);
        Value *count = call_inst->getArgOperand(1);

        IRBuilder<> builder(&instruction);
        Value *convert = nullptr;
        switch (m->getType()->getScalarSizeInBits()) {
        case 16:
          convert = builder.CreateTrunc(count, m->getType()->getScalarType(), "convert");
          break;
        case 32:
          convert = count;
          break;
        case 64:
          convert = builder.CreateZExt(count, m->getType()->getScalarType(), "convert");
          break;
        default:
          llvm_unreachable("Unknown intrinsic");
        }
        Value *undef = UndefValue::get(m->getType());
        Value *insert =
            builder.CreateInsertElement(undef, convert, builder.getInt32(0), "insert");
        Value *shuffle = builder.CreateShuffleVector(
            insert, undef, ConstantAggregateZero::get(m->getType()), "shuffle");
        Value *result = builder.CreateLShr(m, shuffle, "result");

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

PreservedAnalyses llsimd::run(llvm::Function &function,
                             llvm::FunctionAnalysisManager &) {
  bool changed = false;

  for (auto &basic_block : function)
    changed |= run_on_basic_block(basic_block);

  return (changed ? llvm::PreservedAnalyses::none()
                  : llvm::PreservedAnalyses::all());
}

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
