; RUN: %cheri_purecap_llc -cheri-cap-table-abi=plt %s -o - -mxcaptable=false | %cheri_FileCheck %s -check-prefixes=CHECK,SMALLTABLE
; RUN: %cheri_purecap_llc -cheri-cap-table-abi=plt %s -o - -mxcaptable=true | %cheri_FileCheck %s -check-prefixes=CHECK,BIGTABLE
source_filename = "/Users/alex/cheri/llvm/tools/clang/test/CodeGen/CHERI/cap-table-call-extern.c"


declare void @external_func() addrspace(200)

@fnptr_uninit = common local_unnamed_addr addrspace(200) global ptr addrspace(200) null, align 32

@fnptr_init = local_unnamed_addr addrspace(200) global ptr addrspace(200) @external_func, align 32


; Function Attrs: nounwindm
define void @test_call() {
entry:
  call void @external_func()
  ; CHECK-LABEL: test_call:
  ; SMALLTABLE: clcbi	$c12, %capcall20(external_func)($c18)
  ; BIGTABLE:      lui	$1, %capcall_hi(external_func)
  ; BIGTABLE-NEXT: daddiu	$1, $1, %capcall_lo(external_func)
  ; BIGTABLE-NEXT: clc	$c12, $1, 0($c18)
  ret void
}

define ptr addrspace(200) @test_load_fnptr() {
entry:
  ret ptr addrspace(200) @external_func
  ; CHECK-LABEL: test_load_fnptr:
  ; SMALLTABLE: clcbi	$c3, %captab20(external_func)($c26)
  ; BIGTABLE:      lui	$1, %captab_hi(external_func)
  ; BIGTABLE-NEXT: daddiu	$1, $1, %captab_lo(external_func)
  ; BIGTABLE-NEXT: clc	$c3, $1, 0($c26)
}
