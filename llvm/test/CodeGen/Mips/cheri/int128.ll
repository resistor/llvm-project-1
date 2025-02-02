; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; See tools/clang/test/CodeGen/cheri/libcxx-filesystem-crash.cpp
; RUN: %cheri_purecap_llc -disable-cheri-addressing-mode-folder=true < %s | FileCheck %s -check-prefixes CHECK
; RUN: %cheri_purecap_llc -disable-cheri-addressing-mode-folder=false < %s | FileCheck %s -check-prefixes CHECK

declare void @llvm.lifetime.start.p200(i64 immarg, ptr addrspace(200) nocapture) addrspace(200)
declare void @llvm.lifetime.end.p200(i64 immarg, ptr addrspace(200) nocapture) addrspace(200)

define i128 @test2(ptr addrspace(200) nocapture readonly dereferenceable(16) %e) local_unnamed_addr addrspace(200) {
; CHECK-LABEL: test2:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    cincoffset $c11, $c11, -16
; CHECK-NEXT:    .cfi_def_cfa_offset 16
; CHECK-NEXT:    clc $c1, $zero, 0($c3)
; CHECK-NEXT:    csc $c1, $zero, 0($c11)
; CHECK-NEXT:    cld $2, $zero, 0($c11)
; CHECK-NEXT:    cld $3, $zero, 8($c11)
; CHECK-NEXT:    cjr $c17
; CHECK-NEXT:    cincoffset $c11, $c11, 16
entry:
  %ref.tmp.sroa.0 = alloca ptr addrspace(200), align 16, addrspace(200)
  %tmpcast = bitcast ptr addrspace(200) %ref.tmp.sroa.0 to ptr addrspace(200)
  %ref.tmp.sroa.0.0..sroa_cast4 = bitcast ptr addrspace(200) %ref.tmp.sroa.0 to ptr addrspace(200)
  call void @llvm.lifetime.start.p200(i64 16, ptr addrspace(200) nonnull %ref.tmp.sroa.0.0..sroa_cast4)
  %0 = bitcast ptr addrspace(200) %e to ptr addrspace(200)
  %1 = load ptr addrspace(200), ptr addrspace(200) %0, align 16
  store ptr addrspace(200) %1, ptr addrspace(200) %ref.tmp.sroa.0, align 16
  %ref.tmp.sroa.0.0.ref.tmp.sroa.0.0. = load i128, ptr addrspace(200) %tmpcast, align 16
  call void @llvm.lifetime.end.p200(i64 16, ptr addrspace(200) nonnull %ref.tmp.sroa.0.0..sroa_cast4)
  ret i128 %ref.tmp.sroa.0.0.ref.tmp.sroa.0.0.
}
