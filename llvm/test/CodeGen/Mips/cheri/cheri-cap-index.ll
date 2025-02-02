; RUN: %cheri_llc -verify-machineinstrs %s -o - | FileCheck %s

; Function Attrs: nounwind
define i32 @loop(ptr addrspace(200) nocapture %x, i32 %len) #0 {
entry:
  %cmp9 = icmp sgt i32 %len, 0
  br i1 %cmp9, label %for.body, label %for.end

for.body:                                         ; preds = %entry, %for.body
; Check that we are emitting loads and stores relative to the argument
; capability, not relative to some computed result.  
; CHECK: clw	$5, $1, 0($c3)
; CHECK: csw	$2, $1, 0($c3)

  %i.011 = phi i32 [ %inc, %for.body ], [ 0, %entry ]
  %sum.010 = phi i32 [ %add, %for.body ], [ 0, %entry ]
  %0 = sext i32 %i.011 to i256
  %arrayidx = getelementptr inbounds i32, ptr addrspace(200) %x, i256 %0
  %1 = load i32, ptr addrspace(200) %arrayidx, align 4
  %add = add nsw i32 %1, %sum.010
  store i32 %add, ptr addrspace(200) %arrayidx, align 4
  %inc = add nsw i32 %i.011, 1
  %exitcond = icmp eq i32 %inc, %len
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  %sum.0.lcssa = phi i32 [ 0, %entry ], [ %add, %for.body ]
  ret i32 %sum.0.lcssa
}

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf"="true" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
