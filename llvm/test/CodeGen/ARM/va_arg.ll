; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=armv7-none-linux-gnueabi -pre-RA-sched=source | FileCheck %s
; Test that we correctly align elements when using va_arg

define i64 @test1(i32 %i, ...) nounwind optsize {
; CHECK-LABEL: test1:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    .pad #16
; CHECK-NEXT:    sub sp, sp, #16
; CHECK-NEXT:    add r0, sp, #4
; CHECK-NEXT:    stmib sp, {r1, r2, r3}
; CHECK-NEXT:    add r0, r0, #7
; CHECK-NEXT:    bic r1, r0, #7
; CHECK-NEXT:    orr r0, r1, #4
; CHECK-NEXT:    str r0, [sp]
; CHECK-NEXT:    ldr r0, [r1]
; CHECK-NEXT:    add r2, r1, #8
; CHECK-NEXT:    str r2, [sp]
; CHECK-NEXT:    ldr r1, [r1, #4]
; CHECK-NEXT:    add sp, sp, #16
; CHECK-NEXT:    bx lr
entry:
  %g = alloca ptr, align 4
  call void @llvm.va_start.p0(ptr %g)
  %0 = va_arg ptr %g, i64
  call void @llvm.va_end.p0(ptr %g)
  ret i64 %0
}

define double @test2(i32 %a, ptr %b, ...) nounwind optsize {
; CHECK-LABEL: test2:
; CHECK:       @ %bb.0: @ %entry
; CHECK-NEXT:    .pad #12
; CHECK-NEXT:    sub sp, sp, #12
; CHECK-NEXT:    add r0, sp, #4
; CHECK-NEXT:    stmib sp, {r2, r3}
; CHECK-NEXT:    add r0, r0, #11
; CHECK-NEXT:    bic r0, r0, #3
; CHECK-NEXT:    str r2, [r1]
; CHECK-NEXT:    add r1, r0, #8
; CHECK-NEXT:    str r1, [sp]
; CHECK-NEXT:    vldr d16, [r0]
; CHECK-NEXT:    vmov r0, r1, d16
; CHECK-NEXT:    add sp, sp, #12
; CHECK-NEXT:    bx lr
entry:
  %ap = alloca ptr, align 4                       ; <ptr> [#uses=3]
  call void @llvm.va_start.p0(ptr %ap)
  %0 = va_arg ptr %ap, i32                       ; <i32> [#uses=0]
  store i32 %0, ptr %b
  %1 = va_arg ptr %ap, double                    ; <double> [#uses=1]
  call void @llvm.va_end.p0(ptr %ap)
  ret double %1
}


declare void @llvm.va_start.p0(ptr) nounwind

declare void @llvm.va_end.p0(ptr) nounwind
