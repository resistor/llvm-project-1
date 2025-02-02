; RUN: %cheri_llc %s -o - | FileCheck %s
; ModuleID = 'brooks.c'
%struct.tvbuff = type { i32, ptr addrspace(200) }

; Check that we are correctly selecting clwu for 32-bit loads.
; Function Attrs: nounwind
define ptr addrspace(200) @tvb_new_with_subset(ptr addrspace(200) nocapture readonly %backing, ptr addrspace(200) %tvb) #0 {
; CHECK-LABEL: tvb_new_with_subset:
; CHECK:       clwu $1, $zero, 0($c4)
entry:
  %real_data = getelementptr inbounds %struct.tvbuff, ptr addrspace(200) %backing, i64 0, i32 1
  %0 = load ptr addrspace(200), ptr addrspace(200) %real_data, align 32
  %foo = getelementptr inbounds %struct.tvbuff, ptr addrspace(200) %tvb, i64 0, i32 0
  %1 = load i32, ptr addrspace(200) %foo, align 4
  %2 = ptrtoint ptr addrspace(200) %0 to i64
  %3 = zext i32 %1 to i64
  %add = add i64 %3, %2
  %4 = getelementptr i8, ptr addrspace(200) null, i64 %add
  %real_data1 = getelementptr inbounds %struct.tvbuff, ptr addrspace(200) %tvb, i64 0, i32 1
  store ptr addrspace(200) %4, ptr addrspace(200) %real_data1, align 32
  ret ptr addrspace(200) %tvb
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.4 "}
