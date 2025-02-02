// NOTE: Assertions have been autogenerated by utils/update_cc_test_checks.py
/// The subobject bounds logic used to trigger assertions when using opaque
/// pointers with "remaining size" bounds. Check that these work for typed and untyped pointers
// REQUIRES: asserts
// RUN: %riscv64_cheri_purecap_cc1 -cheri-bounds=subobject-safe -o - -emit-llvm %s | FileCheck %s --check-prefix=OPAQUE

union union_two_chararrays {
  char chararray16[16];
  char chararray32[32];
};
// OPAQUE-LABEL: @bounds_subobject_union_two_chararrays(
// OPAQUE-NEXT:  entry:
// OPAQUE-NEXT:    [[TWOARRAYS_ADDR:%.*]] = alloca ptr addrspace(200), align 16, addrspace(200)
// OPAQUE-NEXT:    store ptr addrspace(200) [[TWOARRAYS:%.*]], ptr addrspace(200) [[TWOARRAYS_ADDR]], align 16
// OPAQUE-NEXT:    [[TMP0:%.*]] = load ptr addrspace(200), ptr addrspace(200) [[TWOARRAYS_ADDR]], align 16
// OPAQUE-NEXT:    [[TMP1:%.*]] = call ptr addrspace(200) @llvm.cheri.cap.bounds.set.i64(ptr addrspace(200) [[TMP0]], i64 32)
// OPAQUE-NEXT:    ret ptr addrspace(200) [[TMP1]]
//
void *bounds_subobject_union_two_chararrays(union union_two_chararrays *twoarrays) {
  return &twoarrays->chararray16;
}
