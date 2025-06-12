// RUN: %clang_cc1 %s -o - "-triple" "riscv32cheriot-unknown-cheriotrtos" "-emit-llvm" "-mframe-pointer=none" "-mcmodel=small" "-target-abi" "cheriot" "-O1" "-Werror" "-cheri-compartment=example" -std=c2x | FileCheck %s

// Test that structs that can fit in two registers are correctly handled, both when used as return values and when passed as an argument. 

#define LENGTH 5
static unsigned int dummies[] = {1, 2, 3, 4, 5};
static unsigned int dummy = 0;

volatile static __attribute__((used)) unsigned int* force_use; 

// CHECK: %struct.TwoIntegers = type { i32, i32 }
struct TwoIntegers {
  unsigned int one;
  unsigned int two;
};

__attribute__((cheri_compartment("example"), noinline)) unsigned int GetValue(void) {
  return ++dummy; 
}

// Here we want to check that the result is returned by value.
// CHECK: define dso_local chericcallcce %struct.TwoIntegers @_Z8InitIntsv() local_unnamed_addr addrspace(200) #1 {
__attribute__((cheri_compartment("example"), noinline)) struct TwoIntegers InitInts(void) {
  // CHECK: entry:
  // CHECK:   %call = tail call chericcallcce i32 @_Z8GetValuev()
  // CHECK:   %call1 = tail call chericcallcce i32 @_Z8GetValuev()
  // CHECK:   %.fca.0.insert = insertvalue %struct.TwoIntegers poison, i32 %call, 0
  // CHECK:   %.fca.1.insert = insertvalue %struct.TwoIntegers %.fca.0.insert, i32 %call1, 1
  struct TwoIntegers Res = {GetValue(), GetValue()};

  // CHECK:   ret %struct.TwoIntegers %.fca.1.insert
  return Res;
}

// Here we want to check that the struct received as parameter is laid out as two different arguments and, again, is returned by value.
// CHECK: define dso_local chericcallcce %struct.TwoIntegers @_Z7ChgInts11TwoIntegers(i32 %x.coerce0, i32 %x.coerce1) local_unnamed_addr addrspace(200) #1 {
__attribute__((cheri_compartment("example"), noinline)) struct TwoIntegers ChgInts(struct TwoIntegers x) {
  // CHECK: entry:
  // CHECK:   %call = tail call chericcallcce i32 @_Z8GetValuev()
  // CHECK:   %sub = sub i32 %x.coerce0, %call
  // CHECK:   %call1 = tail call chericcallcce i32 @_Z8GetValuev()
  // CHECK:   %sub2 = sub i32 %x.coerce1, %call1
  // CHECK:   %.fca.0.insert = insertvalue %struct.TwoIntegers poison, i32 %sub, 0
  // CHECK:   %.fca.1.insert = insertvalue %struct.TwoIntegers %.fca.0.insert, i32 %sub2, 1
  x.one -= GetValue();
  x.two -= GetValue();

  // CHECK:   ret %struct.TwoIntegers %.fca.1.insert
  return x;
}


// Here we want to check that the initial struct received from the callee is handled as a proper struct, i.e. fields are read using `extractvalue` rather than dereferencing pointers.
// CHECK: define dso_local chericcallcce void @_Z9CheckIntsv() local_unnamed_addr addrspace(200) #2 {
__attribute__((cheri_compartment("example"))) void CheckInts() {

  static struct TwoIntegers __attribute__((used)) x = {}; 

  // CHECK: entry:
  // CHECK:   %call = tail call chericcallcce %struct.TwoIntegers @_Z8InitIntsv
  // CHECK:   %0 = extractvalue %struct.TwoIntegers %call, 0
  // CHECK:   %1 = extractvalue %struct.TwoIntegers %call, 1
  // CHECK:   %call1 = tail call chericcallcce %struct.TwoIntegers @_Z7ChgInts11TwoIntegers(i32 %0, i32 %1)


  // CHECK:   %2 = extractvalue %struct.TwoIntegers %call1, 0
  // CHECK:   %3 = extractvalue %struct.TwoIntegers %call1, 1
  // CHECK:   store i32 %2, ptr addrspace(200) @_Z9CheckIntsv.x, align 4, !tbaa !7
  // CHECK:   store i32 %3, ptr addrspace(200) getelementptr inbounds nuw (i8, ptr addrspace(200) @_Z9CheckIntsv.x, i32 4), align 4, !tbaa !7
  x = ChgInts(InitInts());
  
  // CHECK:  ret void
  return;
}


// Do the same but use a struct of two pointers. 

struct TwoPointers {
  unsigned int *one;
  unsigned int *two;
};

// Here we want to check that the result is returned by value.
// CHECK: define dso_local chericcallcce %struct.TwoPointers @_Z8InitPtrsv() local_unnamed_addr addrspace(200) #1
__attribute__((cheri_compartment("example"), noinline)) struct TwoPointers InitPtrs() {
  // CHECK: entry:
  // CHECK:   %call = tail call chericcallcce i32 @_Z8GetValuev()
  // CHECK:   %rem = urem i32 %call, 5
  // CHECK:   %add.ptr = getelementptr inbounds nuw i32, ptr addrspace(200) @dummies, i32 %rem
  // CHECK:   %call1 = tail call chericcallcce i32 @_Z8GetValuev()
  // CHECK:   %rem2 = urem i32 %call1, 5
  // CHECK:   %add.ptr3 = getelementptr inbounds nuw i32, ptr addrspace(200) @dummies, i32 %rem2
  struct TwoPointers x = {dummies + (GetValue() % LENGTH), dummies + (GetValue() % LENGTH)};

  // CHECK:   %.fca.0.insert = insertvalue %struct.TwoPointers poison, ptr addrspace(200) %add.ptr, 0
  // CHECK:   %.fca.1.insert = insertvalue %struct.TwoPointers %.fca.0.insert, ptr addrspace(200) %add.ptr3, 1
  // CHECK:   ret %struct.TwoPointers %.fca.1.insert
  return x;
}

// Here we want to check that the struct received as parameter is laid out as two different arguments and, again, is returned by value.
// CHECK: define dso_local chericcallcce %struct.TwoPointers @_Z7ChgPtrs11TwoPointers(ptr addrspace(200) %x.coerce0, ptr addrspace(200) %x.coerce1) local_unnamed_addr addrspace(200) #1 {
__attribute__((cheri_compartment("example"), noinline)) struct TwoPointers ChgPtrs(struct TwoPointers x) {

  // CHECK: entry:
  // CHECK:   store ptr addrspace(200) %x.coerce0, ptr addrspace(200) @force_use, align 8, !tbaa !11
  force_use = x.one;

  // CHECK:   %call = tail call chericcallcce i32 @_Z8GetValuev()
  // CHECK:   %rem = urem i32 %call, 5
  // CHECK:   %add.ptr = getelementptr inbounds nuw i32, ptr addrspace(200) @dummies, i32 %rem
  x.one = dummies + (GetValue() % LENGTH);

  // CHECK:   store ptr addrspace(200) %x.coerce1, ptr addrspace(200) @force_use, align 8, !tbaa !11
  force_use = x.two;

  // CHECK:   %call2 = tail call chericcallcce i32 @_Z8GetValuev()
  // CHECK:   %rem3 = urem i32 %call2, 5
  // CHECK:   %add.ptr4 = getelementptr inbounds nuw i32, ptr addrspace(200) @dummies, i32 %rem3
  x.two = dummies + (GetValue() % LENGTH);

  // CHECK:   %.fca.0.insert = insertvalue %struct.TwoPointers poison, ptr addrspace(200) %add.ptr, 0
  // CHECK:   %.fca.1.insert = insertvalue %struct.TwoPointers %.fca.0.insert, ptr addrspace(200) %add.ptr4, 1
  // CHECK:   ret %struct.TwoPointers %.fca.1.insert
  return x;
}

// Here we want to check that the struct received from the callee is handled as a proper struct, i.e. fields are read using `extractvalue` rather than dereferencing pointers. Also, we want to check that arguments in composite calls are passed correctly.
// CHECK: define dso_local chericcallcce void @_Z9CheckPtrsv() local_unnamed_addr addrspace(200) #2 {
__attribute__((cheri_compartment("example"))) void CheckPtrs() {

  static struct TwoPointers __attribute__((used)) x = {}; 

  // CHECK: entry:
  // CHECK:   %call = tail call chericcallcce %struct.TwoPointers @_Z8InitPtrsv()
  // CHECK:   %0 = extractvalue %struct.TwoPointers %call, 0
  // CHECK:   %1 = extractvalue %struct.TwoPointers %call, 1
  // CHECK:   %call1 = tail call chericcallcce %struct.TwoPointers @_Z7ChgPtrs11TwoPointers(ptr addrspace(200) %0, ptr addrspace(200) %1)
  // CHECK:   %2 = extractvalue %struct.TwoPointers %call1, 0
  // CHECK:   %3 = extractvalue %struct.TwoPointers %call1, 1
  // CHECK:   store ptr addrspace(200) %2, ptr addrspace(200) @_Z9CheckPtrsv.x, align 8, !tbaa !11
  // CHECK:   store ptr addrspace(200) %3, ptr addrspace(200) getelementptr inbounds nuw (i8, ptr addrspace(200) @_Z9CheckPtrsv.x, i32 8), align 8, !tbaa !11
  x = ChgPtrs(InitPtrs());

  // CHECK:  ret void
  return;
}


// Do the same with a pointer and an integer.

struct PointerAndInt {
  unsigned int *one;
  unsigned int two;
};

// Here we want to check that the result is returned by value.
// CHECK:  define dso_local chericcallcce %struct.PointerAndInt @_Z10InitPtrIntv() local_unnamed_addr addrspace(200) #1 {
__attribute__((cheri_compartment("example"), noinline)) struct PointerAndInt InitPtrInt() {
  // CHECK:  entry:
  // CHECK:    %call = tail call chericcallcce i32 @_Z8GetValuev()
  // CHECK:    %rem = urem i32 %call, 5
  // CHECK:    %add.ptr = getelementptr inbounds nuw i32, ptr addrspace(200) @dummies, i32 %rem
  // CHECK:    %call1 = tail call chericcallcce i32 @_Z8GetValuev()
  struct PointerAndInt x = {dummies + (GetValue() % LENGTH), GetValue()};

  // CHECK:    %.fca.0.insert = insertvalue %struct.PointerAndInt poison, ptr addrspace(200) %add.ptr, 0
  // CHECK:    %.fca.1.insert = insertvalue %struct.PointerAndInt %.fca.0.insert, i32 %call1, 1
  // CHECK:    ret %struct.PointerAndInt %.fca.1.insert
  return x;
}

// Here we want to check that the struct received as parameter is laid out as two different arguments and, again, is returned by value.
// CHECK: define dso_local chericcallcce %struct.PointerAndInt @_Z9ChgPtrInt13PointerAndInt(ptr addrspace(200) %x.coerce0, i32 %x.coerce1) local_unnamed_addr addrspace(200) #1 {
__attribute__((cheri_compartment("example"), noinline)) struct PointerAndInt ChgPtrInt(struct PointerAndInt x) {
  // CHECK: entry:
  // CHECK:   store ptr addrspace(200) %x.coerce0, ptr addrspace(200) @force_use, align 8, !tbaa !11
  force_use = x.one;

  // CHECK:   %call = tail call chericcallcce i32 @_Z8GetValuev()
  // CHECK:   %rem = urem i32 %call, 5
  // CHECK:   %add.ptr = getelementptr inbounds nuw i32, ptr addrspace(200) @dummies, i32 %rem
  x.one = dummies + (GetValue() % LENGTH);

  // CHECK:   %call2 = tail call chericcallcce i32 @_Z8GetValuev()
  // CHECK:   %sub = sub i32 %x.coerce1, %call2
  x.two -= GetValue();

  // CHECK:   %.fca.0.insert = insertvalue %struct.PointerAndInt poison, ptr addrspace(200) %add.ptr, 0
  // CHECK:   %.fca.1.insert = insertvalue %struct.PointerAndInt %.fca.0.insert, i32 %sub, 1
  // CHECK:   ret %struct.PointerAndInt %.fca.1.insert
  return x;
}

// Here we want to check that the struct received from the callee is handled as a proper struct, i.e. fields are read using `extractvalue` rather than dereferencing pointers. Also, we want to check that arguments in composite calls are passed correctly.
// CHECK: define dso_local chericcallcce void @_Z11CheckPtrIntv() local_unnamed_addr addrspace(200) #2 {
__attribute__((cheri_compartment("example"))) void CheckPtrInt() {

  static struct PointerAndInt __attribute__((used)) x = {}; 

  // CHECK: entry:
  // CHECK:   %call = tail call chericcallcce %struct.PointerAndInt @_Z10InitPtrIntv()
  // CHECK:   %0 = extractvalue %struct.PointerAndInt %call, 0
  // CHECK:   %1 = extractvalue %struct.PointerAndInt %call, 1
  // CHECK:   %call1 = tail call chericcallcce %struct.PointerAndInt @_Z9ChgPtrInt13PointerAndInt(ptr addrspace(200) %0, i32 %1)
  // CHECK:   %2 = extractvalue %struct.PointerAndInt %call1, 0
  // CHECK:   %3 = extractvalue %struct.PointerAndInt %call1, 1
  // CHECK:   store ptr addrspace(200) %2, ptr addrspace(200) @_Z11CheckPtrIntv.x, align 8, !tbaa !11
  // CHECK:   store i32 %3, ptr addrspace(200) getelementptr inbounds nuw (i8, ptr addrspace(200) @_Z11CheckPtrIntv.x, i32 8), align 8, !tbaa !7
  x = ChgPtrInt(InitPtrInt());

  // CHECK:   ret void
  return;
}


// Do the same with an integer and a pointer (order does not matter).

struct IntAndPointer {
  unsigned int one;
  unsigned int *two;
};

// Here we want to check that the result is returned by value.
// CHECK: define dso_local chericcallcce %struct.IntAndPointer @_Z10InitIntPtrv() local_unnamed_addr addrspace(200) #1 {
__attribute__((cheri_compartment("example"), noinline)) struct IntAndPointer InitIntPtr() {
  // CHECK: entry:
  // CHECK:   %call = tail call chericcallcce i32 @_Z8GetValuev()
  // CHECK:   %call1 = tail call chericcallcce i32 @_Z8GetValuev()
  // CHECK:   %rem = urem i32 %call1, 5
  // CHECK:   %add.ptr = getelementptr inbounds nuw i32, ptr addrspace(200) @dummies, i32 %rem
  struct IntAndPointer x = {GetValue(), dummies + (GetValue() % LENGTH)};

  // CHECK:   %.fca.0.insert = insertvalue %struct.IntAndPointer poison, i32 %call, 0
  // CHECK:   %.fca.1.insert = insertvalue %struct.IntAndPointer %.fca.0.insert, ptr addrspace(200) %add.ptr, 1
  // CHECK:   ret %struct.IntAndPointer %.fca.1.insert
  return x;
}

// Here we want to check that the struct received as parameter is laid out as two different arguments and, again, is returned by value.
// CHECK: define dso_local chericcallcce %struct.IntAndPointer @_Z9ChgIntPtr13IntAndPointer(i32 %x.coerce0, ptr addrspace(200) %x.coerce1) local_unnamed_addr addrspace(200) #1 {
__attribute__((cheri_compartment("example"), noinline)) struct IntAndPointer ChgIntPtr(struct IntAndPointer x) {

  // CHECK: entry:
  // CHECK:   %call = tail call chericcallcce i32 @_Z8GetValuev()
  // CHECK:   %sub = sub i32 %x.coerce0, %call
  x.one -= GetValue();

  // CHECK:   store ptr addrspace(200) %x.coerce1, ptr addrspace(200) @force_use, align 8, !tbaa !11
  force_use = x.two;

  // CHECK:   %call1 = tail call chericcallcce i32 @_Z8GetValuev()
  // CHECK:   %rem = urem i32 %call1, 5
  // CHECK:   %add.ptr = getelementptr inbounds nuw i32, ptr addrspace(200) @dummies, i32 %rem
  x.two = dummies + (GetValue() % LENGTH);

  // CHECK:   %.fca.0.insert = insertvalue %struct.IntAndPointer poison, i32 %sub, 0
  // CHECK:   %.fca.1.insert = insertvalue %struct.IntAndPointer %.fca.0.insert, ptr addrspace(200) %add.ptr, 1
  // CHECK:   ret %struct.IntAndPointer %.fca.1.insert
  return x;
}

// Here we want to check that the struct received from the callee is handled as a proper struct, i.e. fields are read using `extractvalue` rather than dereferencing pointers. Also, we want to check that arguments in composite calls are passed correctly.
// CHECK: define dso_local chericcallcce void @_Z11CheckIntPtrv() local_unnamed_addr addrspace(200) #2 {
__attribute__((cheri_compartment("example"))) void CheckIntPtr() {

  static struct IntAndPointer __attribute__((used)) x = {}; 

  // CHECK: entry:
  // CHECK:   %call = tail call chericcallcce %struct.IntAndPointer @_Z10InitIntPtrv()
  // CHECK:   %0 = extractvalue %struct.IntAndPointer %call, 0
  // CHECK:   %1 = extractvalue %struct.IntAndPointer %call, 1
  // CHECK:   %call1 = tail call chericcallcce %struct.IntAndPointer @_Z9ChgIntPtr13IntAndPointer(i32 %0, ptr addrspace(200) %1)
  // CHECK:   %2 = extractvalue %struct.IntAndPointer %call1, 0
  // CHECK:   %3 = extractvalue %struct.IntAndPointer %call1, 1
  // CHECK:   store i32 %2, ptr addrspace(200) @_Z11CheckIntPtrv.x, align 8, !tbaa !7
  // CHECK:   store ptr addrspace(200) %3, ptr addrspace(200) getelementptr inbounds nuw (i8, ptr addrspace(200) @_Z11CheckIntPtrv.x, i32 8), align 8, !tbaa !11
  x = ChgIntPtr(InitIntPtr());

  // CHECK:   ret void
  return;
}


// What happens in this case? 

struct Inner {
  unsigned int z;
};

struct Parent {
  unsigned int x; 
  struct Inner y;
};


// Here we want to check that the result is returned by value.
// CHECK: define dso_local chericcallcce %struct.Parent @_Z10InitParentv() local_unnamed_addr addrspace(200) #1 {
__attribute__((cheri_compartment("example"), noinline)) struct Parent InitParent() {
  // CHECK: entry:
  // CHECK:   %call = tail call chericcallcce i32 @_Z8GetValuev()
  // CHECK:   %call1 = tail call chericcallcce i32 @_Z8GetValuev()
  // CHECK:   %.fca.0.insert = insertvalue %struct.Parent poison, i32 %call, 0
  // CHECK:   %.fca.1.0.insert = insertvalue %struct.Parent %.fca.0.insert, i32 %call1, 1, 0
  struct Parent x = {GetValue(), {GetValue()}};

  // CHECK:   ret %struct.Parent %.fca.1.0.insert
  return x;
}

// Here we want to check that the struct received as parameter is laid out as two different arguments and, again, is returned by value.
// CHECK: define dso_local chericcallcce %struct.Parent @_Z9ChgParent6Parent(i32 %x.coerce0, %struct.Inner %x.coerce1) local_unnamed_addr addrspace(200) #1 {
__attribute__((cheri_compartment("example"), noinline)) struct Parent ChgParent(struct Parent x) {

  // CHECK: entry:
  // CHECK:   %x.coerce1.fca.0.extract = extractvalue %struct.Inner %x.coerce1, 0
  // CHECK:   %call = tail call chericcallcce i32 @_Z8GetValuev()
  // CHECK:   %sub = sub i32 %x.coerce0, %call
  x.x -= GetValue();

  // CHECK:   %call2 = tail call chericcallcce i32 @_Z8GetValuev()
  // CHECK:   %sub3 = sub i32 %x.coerce1.fca.0.extract, %call2
  x.y.z -= GetValue();

  // CHECK:   %.fca.0.insert = insertvalue %struct.Parent poison, i32 %sub, 0
  // CHECK:   %.fca.1.0.insert = insertvalue %struct.Parent %.fca.0.insert, i32 %sub3, 1, 0
  // CHECK:   ret %struct.Parent %.fca.1.0.insert
  return x;
}

// Here we want to check that the struct received from the callee is handled as a proper struct, i.e. fields are read using `extractvalue` rather than dereferencing pointers. Also, we want to check that arguments in composite calls are passed correctly.
// CHECK: define dso_local chericcallcce void @_Z11CheckParentv() local_unnamed_addr addrspace(200) #2 {
__attribute__((cheri_compartment("example"))) void CheckParent() {

  static struct Parent __attribute__((used)) x = {}; 

  // CHECK: entry:
  // CHECK:   %call = tail call chericcallcce %struct.Parent @_Z10InitParentv()
  // CHECK:   %0 = extractvalue %struct.Parent %call, 0
  // CHECK:   %1 = extractvalue %struct.Parent %call, 1
  // CHECK:   %call1 = tail call chericcallcce %struct.Parent @_Z9ChgParent6Parent(i32 %0, %struct.Inner %1)
  // CHECK:   %2 = extractvalue %struct.Parent %call1, 0
  // CHECK:   %3 = extractvalue %struct.Parent %call1, 1
  // CHECK:   %.fca.0.extract3 = extractvalue %struct.Inner %3, 0
  // CHECK:   store i32 %2, ptr addrspace(200) @_Z11CheckParentv.x, align 4, !tbaa !7
  // CHECK:   store i32 %.fca.0.extract3, ptr addrspace(200) getelementptr inbounds nuw (i8, ptr addrspace(200) @_Z11CheckParentv.x, i32 4), align 4, !tbaa !7
  x = ChgParent(InitParent());

  // CHECK:   ret void
  return;
}


// For arguments, does it work when the struct is placed in an odd-numbered position? 

// CHECK:  define dso_local chericcallcce %struct.TwoIntegers @_Z8ChgInts2i11TwoIntegers(i32 noundef %new_int, i32 %x.coerce0, i32 %x.coerce1) local_unnamed_addr addrspace(200) #4 {
__attribute__((cheri_compartment("example"), noinline)) struct TwoIntegers ChgInts2(int new_int, struct TwoIntegers x) {

  // CHECK:  %add = add i32 %x.coerce0, %new_int
  // CHECK:  %add1 = add i32 %x.coerce1, %new_int
  // CHECK:  %.fca.0.insert = insertvalue %struct.TwoIntegers poison, i32 %add, 0
  // CHECK:  %.fca.1.insert = insertvalue %struct.TwoIntegers %.fca.0.insert, i32 %add1, 1
  x.one += new_int;
  x.two += new_int;

  // CHECK:  ret %struct.TwoIntegers %.fca.1.insert
  return x;
}

// For arguments, does it work when the function is variadic? 
// CHECK: define dso_local chericcallcce %struct.TwoIntegers @_Z8ChgInts3i11TwoIntegersz(i32 noundef %n, i32 %x.coerce0, i32 %x.coerce1, ...) local_unnamed_addr addrspace(200) #5 {
__attribute__((cheri_compartment("example"), noinline)) struct TwoIntegers ChgInts3(int n, struct TwoIntegers x, ...) {
  __builtin_va_list args;

  // CHECK: call void @llvm.va_start.p200(ptr addrspace(200) nonnull %args)
  __builtin_va_start(args, x);
  
  for (int i = 0; i < n; i++)  {

	// CHECK: for.cond.cleanup:                                 ; preds = %for.body, %entry
	// CHECK:   %x.sroa.0.0.lcssa = phi i32 [ %x.coerce0, %entry ], [ %add, %for.body ]
	// CHECK:   %x.sroa.4.0.lcssa = phi i32 [ %x.coerce1, %entry ], [ %add1, %for.body ]
	// CHECK:   call void @llvm.va_end.p200(ptr addrspace(200) %args)
	// CHECK:   call void @llvm.lifetime.end.p200(i64 8, ptr addrspace(200) nonnull %args) #7
	// CHECK:   %.fca.0.insert = insertvalue %struct.TwoIntegers poison, i32 %x.sroa.0.0.lcssa, 0
	// CHECK:   %.fca.1.insert = insertvalue %struct.TwoIntegers %.fca.0.insert, i32 %x.sroa.4.0.lcssa, 1
	// CHECK:   ret %struct.TwoIntegers %.fca.1.insert

	int v = __builtin_va_arg(args, int);
    x.one += v;
    x.two += v;
  }
  
  __builtin_va_end(args);
  
  return x;
}

// For arguments, does it work correctly when the "optimizable" argument sits across the "put in registers"  and "spill to stack" boundary? 

// CHECK: define dso_local chericcallcce %struct.TwoIntegers @_Z8ChgInts4iiiii11TwoIntegers(i32 noundef %n0, i32 noundef %n1, i32 noundef %n2, i32 noundef %n3, i32 noundef %n4, i32 %x.coerce0, i32 %x.coerce1) local_unnamed_addr addrspace(200) #4 {
__attribute__((cheri_compartment("example"), noinline)) struct TwoIntegers ChgInts4(int n0, int n1, int n2, int n3, int n4, struct TwoIntegers x) {

  // CHECK: entry:
  // CHECK:   %add = add nsw i32 %n1, %n0
  // CHECK:   %add1 = add nsw i32 %add, %n2
  // CHECK:   %add2 = add nsw i32 %add1, %n3
  // CHECK:   %add3 = add nsw i32 %add2, %n4

  // CHECK:   %add4 = add i32 %x.coerce0, %add3
  x.one += n0 + n1 + n2 + n3 + n4;

  // CHECK:   %add9 = add i32 %x.coerce1, %add3
  x.two += n0 + n1 + n2 + n3 + n4;

  // CHECK:   %.fca.0.insert = insertvalue %struct.TwoIntegers poison, i32 %add4, 0
  // CHECK:   %.fca.1.insert = insertvalue %struct.TwoIntegers %.fca.0.insert, i32 %add9, 1
  // CHECK:   ret %struct.TwoIntegers %.fca.1.insert
  return x;
}

// CHECK: attributes #0 = { mustprogress nofree noinline norecurse nosync nounwind willreturn memory(readwrite, argmem: none, inaccessiblemem: none) "cheri-compartment"="example" "interrupt-state"="enabled" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-features"="+32bit,+c,+cap-mode,+e,+m,+xcheri" }
// CHECK: attributes #1 = { mustprogress nofree noinline norecurse nosync nounwind willreturn memory(readwrite, inaccessiblemem: none) "cheri-compartment"="example" "interrupt-state"="enabled" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-features"="+32bit,+c,+cap-mode,+e,+m,+xcheri" }
// CHECK: attributes #2 = { mustprogress nofree norecurse nosync nounwind willreturn memory(readwrite, inaccessiblemem: none) "cheri-compartment"="example" "interrupt-state"="enabled" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-features"="+32bit,+c,+cap-mode,+e,+m,+xcheri" }
