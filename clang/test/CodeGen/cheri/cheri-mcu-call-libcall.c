// RUN: %clang_cc1 %s -o - "-triple" "riscv32cheriot-unknown-cheriotrtos" "-emit-llvm" "-mframe-pointer=none" "-mcmodel=small" "-target-abi" "cheriot" "-Oz" "-Werror" "-cheri-compartment=example" | FileCheck %s
#define LIBCALL __attribute__((cheri_libcall))

LIBCALL
int add(int a, int b);

LIBCALL
int foo(void);

// CHECK: define dso_local i32 @callFromNotLibcall() local_unnamed_addr addrspace(200) #0 {
int callFromNotLibcall(void) {
  // CHECK: call cherilibcallcc i32 @_Z3addii(i32 noundef 1, i32 noundef 2) #2
  // CHECK: call cherilibcallcc i32 @_Z3foov() #2
  return add(1, 2) + foo();
}

// CHECK: declare cherilibcallcc i32 @_Z3addii(i32 noundef, i32 noundef) local_unnamed_addr addrspace(200) #1
// CHECK: declare cherilibcallcc i32 @_Z3foov() local_unnamed_addr addrspace(200) #1

// CHECK: attributes #0
// CHECK-SAME: "cheri-compartment"="example"
