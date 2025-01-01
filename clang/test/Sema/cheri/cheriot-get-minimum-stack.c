// RUN: %clang_cc1 "-triple" "riscv32cheriot-unknown-cheriotrtos" "-mframe-pointer=none" "-mcmodel=small" "-target-abi" "cheriot" "-Oz" "-Werror" "-cheri-compartment=example" -std=c2x -o - %s -fsyntax-only -verify

__attribute__((cheri_compartment("example")))
int readback(void)
{
	return __builtin_cheriot_get_specified_minimum_stack(); // expected-error{{cannot evaluate __builtin_get_specificed_minimum_stack in function without specified minimum stack size}}
}
