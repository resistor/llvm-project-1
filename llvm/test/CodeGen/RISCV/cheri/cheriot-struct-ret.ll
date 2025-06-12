; RUN: llc --filetype=asm --mcpu=cheriot --mtriple=riscv32cheriot-unknown-cheriotrtos -target-abi cheriot  %s -mattr=+xcheri,+cap-mode,+xcheriot -o - | FileCheck %s

target datalayout = "e-m:e-p:32:32-i64:64-n32-S128-pf200:64:64:64:32-A200-P200-G200"
target triple = "riscv32cheriot-unknown-cheriotrtos"

%struct.TwoIntegers = type { i32, i32 }

@dummy = internal unnamed_addr addrspace(200) global i32 0, align 4

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind willreturn memory(readwrite, argmem: none, inaccessiblemem: none)
define dso_local chericcallcce i32 @_Z8GetValuev() local_unnamed_addr addrspace(200) #0 {
entry:
  %0 = load i32, ptr addrspace(200) @dummy, align 4, !tbaa !7
  %inc = add i32 %0, 1
  store i32 %inc, ptr addrspace(200) @dummy, align 4, !tbaa !7
  ret i32 %inc
}

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind willreturn memory(readwrite, inaccessiblemem: none)
;; CHECK: _Z8InitIntsv:                           # @_Z8InitIntsv
define dso_local chericcallcce %struct.TwoIntegers @_Z8InitIntsv() local_unnamed_addr addrspace(200) #1 {
entry:
  %call = tail call chericcallcce i32 @_Z8GetValuev()
  %call1 = tail call chericcallcce i32 @_Z8GetValuev()
  %.fca.0.insert = insertvalue %struct.TwoIntegers poison, i32 %call, 0
  %.fca.1.insert = insertvalue %struct.TwoIntegers %.fca.0.insert, i32 %call1, 1

  ;; CHECK: # %bb.0:                                # %entry
  ;; CHECK-NEXT: 	ct.cincoffset	csp, csp, -16
  ;; CHECK-NEXT: 	ct.csc	cra, 8(csp)                     # 8-byte Folded Spill
  ;; CHECK-NEXT: 	ct.csc	cs0, 0(csp)                     # 8-byte Folded Spill
  ;; CHECK-NEXT: 	ct.ccall	_Z8GetValuev

  ;; Save the first value in `s0`
  ;; CHECK-NEXT: 	mv	s0, a0
  ;; CHECK-NEXT: 	ct.ccall	_Z8GetValuev

  ;; Store the results back in `a0` (from [s0]) and `a1`
  ;; CHECK-NEXT: 	mv	a1, a0
  ;; CHECK-NEXT: 	mv	a0, s0

  ;; Restore cra and previous sp
  ;; CHECK-NEXT: 	ct.clc	cra, 8(csp)                     # 8-byte Folded Reload
  ;; CHECK-NEXT: 	ct.clc	cs0, 0(csp)                     # 8-byte Folded Reload
  ;; CHECK-NEXT: 	ct.cincoffset	csp, csp, 16

  ;; CHECK-NEXT: 	ct.cret

  ret %struct.TwoIntegers %.fca.1.insert
}

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind willreturn memory(readwrite, inaccessiblemem: none)
;; CHECK: _Z7ChgInts11TwoIntegers:                # @_Z7ChgInts11TwoIntegers
define dso_local chericcallcce %struct.TwoIntegers @_Z7ChgInts11TwoIntegers(i32 %x.coerce0, i32 %x.coerce1) local_unnamed_addr addrspace(200) #1 {
entry:

  ;; Read the values from the registers
  ;; CHECK:	        mv	s0, a1
  ;; CHECK-NEXT:	mv	s1, a0


  ;; Call the first `GetValue` (shall be the first word of the result value).
  ;; CHECK-NEXT:  ct.ccall	_Z8GetValuev
  %call = tail call chericcallcce i32 @_Z8GetValuev()

  ;; Save the result of the subtraction in `s1`
  ;; CHECK-NEXT:	sub	s1, s1, a0
  %sub = sub i32 %x.coerce0, %call

  ;; CHECK-NEXT:	ct.ccall	_Z8GetValuev
  %call1 = tail call chericcallcce i32 @_Z8GetValuev()

  ;; Save the result of the subtraction directly in `a1` (second ret value).
  ;; CHECK-NEXT:	sub	a1, s0, a0
  %sub2 = sub i32 %x.coerce1, %call1

  %.fca.0.insert = insertvalue %struct.TwoIntegers poison, i32 %sub, 0
  %.fca.1.insert = insertvalue %struct.TwoIntegers %.fca.0.insert, i32 %sub2, 1

  ;; Put `s1` back in `a0`.
  ;; CHECK-NEXT:        mv	a0, s1

  ;; Usual callee epilogue and return.
  ;; CHECK-NEXT:        ct.clc	cra, 24(csp)
  ;; CHECK-NEXT: 	ct.clc	cs0, 16(csp)
  ;; CHECK-NEXT: 	ct.clc	cs1, 8(csp)
  ;; CHECK-NEXT: 	ct.cincoffset	csp, csp, 32
  ;; CHECK-NEXT: 	ct.cret

  ret %struct.TwoIntegers %.fca.1.insert
}

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind willreturn memory(none)
;; CHECK: _Z8ChgInts2i11TwoIntegers:              # @_Z8ChgInts2i11TwoIntegers
define dso_local chericcallcce %struct.TwoIntegers @_Z8ChgInts2i11TwoIntegers(i32 noundef %new_int, i32 %x.coerce0, i32 %x.coerce1) local_unnamed_addr addrspace(200) #2 {
entry:

  ;; CHECK:       add	a1, a1, a0
  %add = add i32 %x.coerce0, %new_int
  ;; CHECK-NEXT:  add	a2, a2, a0
  %add1 = add i32 %x.coerce1, %new_int

  %.fca.0.insert = insertvalue %struct.TwoIntegers poison, i32 %add, 0
  %.fca.1.insert = insertvalue %struct.TwoIntegers %.fca.0.insert, i32 %add1, 1

  ;; CHECK-NEXT:  mv	a0, a1
  ;; CHECK-NEXT:  mv	a1, a2
  ;; CHECK-NEXT:  cret
  ret %struct.TwoIntegers %.fca.1.insert
}

; Function Attrs: nofree noinline norecurse nosync nounwind
define dso_local chericcallcce %struct.TwoIntegers @_Z8ChgInts3i11TwoIntegersz(i32 noundef %n, i32 %x.coerce0, i32 %x.coerce1, ...) local_unnamed_addr addrspace(200) #3 {
entry:
  %args = alloca ptr addrspace(200), align 8, addrspace(200)
  call void @llvm.lifetime.start.p200(i64 8, ptr addrspace(200) nonnull %args) #6
  call void @llvm.va_start.p200(ptr addrspace(200) nonnull %args)
  %cmp5 = icmp sgt i32 %n, 0
  br i1 %cmp5, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %args.promoted = load ptr addrspace(200), ptr addrspace(200) %args, align 8
  br label %for.body



for.body:                                         ; preds = %for.body.preheader, %for.body
  %x.sroa.4.09 = phi i32 [ %add1, %for.body ], [ %x.coerce1, %for.body.preheader ]
  %i.08 = phi i32 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %x.sroa.0.07 = phi i32 [ %add, %for.body ], [ %x.coerce0, %for.body.preheader ]
  %argp.next46 = phi ptr addrspace(200) [ %argp.next, %for.body ], [ %args.promoted, %for.body.preheader ]
  %argp.next = getelementptr inbounds nuw i8, ptr addrspace(200) %argp.next46, i32 4
  store ptr addrspace(200) %argp.next, ptr addrspace(200) %args, align 8
  %0 = load i32, ptr addrspace(200) %argp.next46, align 4, !tbaa !7
  %add = add i32 %0, %x.sroa.0.07
  %add1 = add i32 %0, %x.sroa.4.09
  %inc = add nuw nsw i32 %i.08, 1
  %exitcond.not = icmp eq i32 %inc, %n
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body, !llvm.loop !11

  ;; CHECK:       .LBB4_2:        # %for.body
  ;; CHECK-NEXT:                  # =>This Inner Loop Header: Depth=1
  ;; CHECK-NEXT: 	cincoffset	ca4, ca3, 4
  ;; CHECK-NEXT: 	csc	ca4, 8(csp)
  ;; CHECK-NEXT: 	clw	a3, 0(ca3)
  ;; CHECK-NEXT: 	addi	a0, a0, -1
  ;; CHECK-NEXT: 	add	a1, a1, a3
  ;; CHECK-NEXT: 	add	a2, a2, a3
  ;; CHECK-NEXT: 	cmove	ca3, ca4
  ;; CHECK-NEXT: 	bnez	a0, .LBB4_2

for.cond.cleanup:                                 ; preds = %for.body, %entry
  %x.sroa.0.0.lcssa = phi i32 [ %x.coerce0, %entry ], [ %add, %for.body ]
  %x.sroa.4.0.lcssa = phi i32 [ %x.coerce1, %entry ], [ %add1, %for.body ]
  call void @llvm.va_end.p200(ptr addrspace(200) %args)
  call void @llvm.lifetime.end.p200(i64 8, ptr addrspace(200) nonnull %args) #6
  %.fca.0.insert = insertvalue %struct.TwoIntegers poison, i32 %x.sroa.0.0.lcssa, 0
  %.fca.1.insert = insertvalue %struct.TwoIntegers %.fca.0.insert, i32 %x.sroa.4.0.lcssa, 1

  ;; CHECK-NEXT: .LBB4_3:                                # %for.cond.cleanup
  ;; CHECK-NEXT: 	mv	a0, a1
  ;; CHECK-NEXT: 	mv	a1, a2
  ;; CHECK-NEXT: 	cincoffset	csp, csp, 16
  ;; CHECK-NEXT: 	cret

  ret %struct.TwoIntegers %.fca.1.insert
}

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind willreturn memory(none)
;; CHECK: _Z8ChgInts4iiiii11TwoIntegers:          # @_Z8ChgInts4iiiii11TwoIntegers
define dso_local chericcallcce %struct.TwoIntegers @_Z8ChgInts4iiiii11TwoIntegers(i32 noundef %n0, i32 noundef %n1, i32 noundef %n2, i32 noundef %n3, i32 noundef %n4, i32 %x.coerce0, i32 %x.coerce1) local_unnamed_addr addrspace(200) #4 {
entry:
  %add = add nsw i32 %n1, %n0
  %add1 = add nsw i32 %add, %n2
  %add2 = add nsw i32 %add1, %n3
  %add3 = add nsw i32 %add2, %n4
  %add4 = add i32 %x.coerce0, %add3
  %add9 = add i32 %x.coerce1, %add3
  %.fca.0.insert = insertvalue %struct.TwoIntegers poison, i32 %add4, 0
  %.fca.1.insert = insertvalue %struct.TwoIntegers %.fca.0.insert, i32 %add9, 1
  ;; CHECK:         cgetbase        t2, ct0
  ;; CHECK:         bne     t0, t2, .LBB5_1
  ;; CHECK:         blt     t0, sp, .LBB5_1
  ;; CHECK:         cgetlen t1, ct0
  ;; CHECK:         li      t2, 4
  ;; CHECK:         blt     t1, t2, .LBB5_1
  ;; CHECK:         cgetperm        t1, ct0
  ;; CHECK:         li      t2, 126
  ;; CHECK:         bne     t1, t2, .LBB5_1
  ;; CHECK:         clw     t0, 0(ct0)
  ;; CHECK:         add     a0, a0, a1
  ;; CHECK:         add     a2, a2, a3
  ;; CHECK:         add     a0, a0, a2
  ;; CHECK:         add     a1, a0, a4
  ;; CHECK:         add     a0, a5, a1
  ;; CHECK:         add     a1, a1, t0
  ;; CHECK:         cret
  ret %struct.TwoIntegers %.fca.1.insert
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p200(i64 immarg, ptr addrspace(200) nocapture) addrspace(200) #4

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.va_start.p200(ptr addrspace(200)) addrspace(200) #5

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p200(i64 immarg, ptr addrspace(200) nocapture) addrspace(200) #4

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.va_end.p200(ptr addrspace(200)) addrspace(200) #5

attributes #0 = { mustprogress nofree noinline norecurse nosync nounwind willreturn memory(readwrite, argmem: none, inaccessiblemem: none) "cheri-compartment"="example" "interrupt-state"="enabled" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-features"="+32bit,+c,+cap-mode,+e,+m,+xcheri" }
attributes #1 = { mustprogress nofree noinline norecurse nosync nounwind willreturn memory(readwrite, inaccessiblemem: none) "cheri-compartment"="example" "interrupt-state"="enabled" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-features"="+32bit,+c,+cap-mode,+e,+m,+xcheri" }
attributes #2 = { mustprogress nofree noinline norecurse nosync nounwind willreturn memory(none) "cheri-compartment"="example" "interrupt-state"="enabled" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-features"="+32bit,+c,+cap-mode,+e,+m,+xcheri" }
attributes #3 = { nofree noinline norecurse nosync nounwind "cheri-compartment"="example" "interrupt-state"="enabled" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-features"="+32bit,+c,+cap-mode,+e,+m,+xcheri" }
attributes #4 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #5 = { mustprogress nocallback nofree nosync nounwind willreturn }
attributes #6 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !4, !5}
!llvm.ident = !{!6}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"target-abi", !"cheriot"}
!2 = !{i32 6, !"riscv-isa", !3}
!3 = !{!"rv32e2p0_m2p0_c2p0_zmmul1p0_xcheri0p0"}
!4 = !{i32 1, !"Code Model", i32 1}
!5 = !{i32 8, !"SmallDataLimit", i32 0}
!6 = !{!"clang version 20.1.3"}
!7 = !{!8, !8, i64 0}
!8 = !{!"int", !9, i64 0}
!9 = !{!"omnipotent char", !10, i64 0}
!10 = !{!"Simple C/C++ TBAA"}
!11 = distinct !{!11, !12, !13}
!12 = !{!"llvm.loop.mustprogress"}
!13 = !{!"llvm.loop.unroll.disable"}
