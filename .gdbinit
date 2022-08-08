add-symbol-file kernel/kernel
target remote localhost:1234
tui new-layout debug_kernel {-horizontal src 1 asm 1} 2 status 0 cmd 1
tui layout debug_kernel
break main
