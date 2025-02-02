; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc -debug-only=isel < %s 2>&1 | FileCheck %s

; REQUIRES: asserts

target triple = "aarch64-unknown-linux-gnu"

; Ensure that only no offset frame indexes are folded into SVE load/stores when
; accessing fixed width objects.
define void @foo(ptr %a) #0 {
; CHECK-LABEL: foo:
; CHECK:       SelectionDAG has 15 nodes:
; CHECK-NEXT:    t0: ch,glue = EntryToken
; CHECK-NEXT:    t11: nxv2i1 = PTRUE_D TargetConstant:i32<31>
; CHECK-NEXT:    t2: i64,ch = CopyFromReg t0, Register:i64 %0
; CHECK-NEXT:    t17: nxv2i64,ch = LD1D_IMM<Mem:(volatile load (s512) from %ir.a)> t11, t2, TargetConstant:i64<0>, t0
; CHECK-NEXT:    t7: i64 = ADDXri TargetFrameIndex:i64<1>, TargetConstant:i32<0>, TargetConstant:i32<0>
; CHECK-NEXT:    t5: i64 = ADDXri TargetFrameIndex:i64<0>, TargetConstant:i32<0>, TargetConstant:i32<0>
; CHECK-NEXT:    t16: ch = ST1D_IMM<Mem:(volatile store (s512) into %ir.r0)> t17, t11, t5, TargetConstant:i64<0>, t17:1
; CHECK-NEXT:    t15: ch = ST1D_IMM<Mem:(volatile store (s512) into %ir.r1)> t17, t11, t7, TargetConstant:i64<0>, t16
; CHECK-NEXT:    t9: ch = RET_ReallyLR t15
; CHECK-EMPTY:
entry:
  %r0 = alloca <8 x i64>
  %r1 = alloca <8 x i64>
  %r = load volatile <8 x i64>, ptr %a
  store volatile <8 x i64> %r, ptr %r0
  store volatile <8 x i64> %r, ptr %r1
  ret void
}

attributes #0 = { nounwind "target-features"="+sve" vscale_range(4,4) }
