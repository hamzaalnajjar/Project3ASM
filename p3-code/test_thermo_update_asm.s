### test_thermo_update_asm.s: Contains wrapper functions which are
### used to init registers to known "garbage" values before calling a
### student-written function. This helps force out "used a random
### register" problems that will have variable behavior on different
### machines.

.text
.global CALL_set_temp_from_ports
.type	CALL_set_temp_from_ports, @function
CALL_set_temp_from_ports:       
        ## %rdi is a pointer argument
        .cfi_startproc

        ## 8 bytes on stack at the start for retur address
        ## 6 * 8 bytes pushed
        ## need an addtional 8 bytes to align the stack for a call
        subq $56, %rsp

        ## changes ot the rsp require annotation to allow debuggers to
        ## know where the previous stack frame starts. after the above
        ## extension of the stack, the previous stack fraom is 64
        ## bytes above rsp
        .cfi_def_cfa_offset 64

        ## save 6 callee save registers except the stack pointer
        movq %rbp,  0(%rsp)
        movq %rbx,  8(%rsp)
        movq %r12, 16(%rsp)
        movq %r13, 24(%rsp)
        movq %r14, 32(%rsp)
        movq %r15, 40(%rsp)

        ## initialize all registers to ensure they have "garbage"
        ## values in them
        movabsq $0xBADBADBADBADBADD, %rax
        movabsq $0xBADBADBADBADBADD, %rbx
        movabsq $0xBADBADBADBADBADD, %rcx
        movabsq $0xBADBADBADBADBADD, %rdx
        movabsq $0xBADBADBADBADBADD, %rsi
        ## movabsq $0xBADBADBADBADBADD, %rdi
        ## movabsq $0xBADBADBADBADBADD, %rsp
        movabsq $0xBADBADBADBADBADD, %rbp
        movabsq $0xBADBADBADBADBADD, %r8
        movabsq $0xBADBADBADBADBADD, %r9
        movabsq $0xBADBADBADBADBADD, %r10
        movabsq $0xBADBADBADBADBADD, %r11
        movabsq $0xBADBADBADBADBADD, %r12
        movabsq $0xBADBADBADBADBADD, %r13
        movabsq $0xBADBADBADBADBADD, %r14
        movabsq $0xBADBADBADBADBADD, %r15

        ## call target function
        call set_temp_from_ports

        ## store callee regs to check them later
        call get_callee_regs
        
        ## restore 6 callee registers
        movq  0(%rsp), %rbp
        movq  8(%rsp), %rbx
        movq 16(%rsp), %r12
        movq 24(%rsp), %r13
        movq 32(%rsp), %r14
        movq 40(%rsp), %r15

        addq $56, %rsp
	.cfi_def_cfa_offset 8   # annotate another rsp change for debugger
        
        ## rax has the return value from the function
        ret                     
	.cfi_endproc

.global CALL_set_display_from_temp
.type	CALL_set_display_from_temp, @function
CALL_set_display_from_temp:     
        ## %rdi is packed struct arg
        ## %rsi is pointer arg
        .cfi_startproc
        subq $56, %rsp          # see first function for details
        .cfi_def_cfa_offset 64

        ## save 6 callee save registers except the stack pointer
        movq %rbp,  0(%rsp)
        movq %rbx,  8(%rsp)
        movq %r12, 16(%rsp)
        movq %r13, 24(%rsp)
        movq %r14, 32(%rsp)
        movq %r15, 40(%rsp)

        ## initialize all registers to ensure they have "garbage"
        ## values in them
        movabsq $0xBADBADBADBADBADD, %rax
        movabsq $0xBADBADBADBADBADD, %rbx
        movabsq $0xBADBADBADBADBADD, %rcx
        movabsq $0xBADBADBADBADBADD, %rdx
        ## movabsq $0xBADBADBADBADBADD, %rsi
        ## movabsq $0xBADBADBADBADBADD, %rdi
        ## movabsq $0xBADBADBADBADBADD, %rsp
        movabsq $0xBADBADBADBADBADD, %rbp
        movabsq $0xBADBADBADBADBADD, %r8
        movabsq $0xBADBADBADBADBADD, %r9
        movabsq $0xBADBADBADBADBADD, %r10
        movabsq $0xBADBADBADBADBADD, %r11
        movabsq $0xBADBADBADBADBADD, %r12
        movabsq $0xBADBADBADBADBADD, %r13
        movabsq $0xBADBADBADBADBADD, %r14
        movabsq $0xBADBADBADBADBADD, %r15

        ## call the target function
        call set_display_from_temp

        ## store callee regs to check them later
        call get_callee_regs

        ## restore 6 callee registers
        movq  0(%rsp), %rbp
        movq  8(%rsp), %rbx
        movq 16(%rsp), %r12
        movq 24(%rsp), %r13
        movq 32(%rsp), %r14
        movq 40(%rsp), %r15

        addq $56, %rsp
	.cfi_def_cfa_offset 8   # annotate another rsp change for debugger

        ## rax has the return value from the function
        ret                     
	.cfi_endproc

.global CALL_thermo_update
.type	CALL_thermo_update, @function
CALL_thermo_update:     
        ## no arguments
        .cfi_startproc
        subq $56, %rsp          # see first function for details
        .cfi_def_cfa_offset 64

        ## save 6 callee save registers except the stack pointer
        movq %rbp,  0(%rsp)
        movq %rbx,  8(%rsp)
        movq %r12, 16(%rsp)
        movq %r13, 24(%rsp)
        movq %r14, 32(%rsp)
        movq %r15, 40(%rsp)

        ## initialize all registers to ensure they have "garbage"
        ## values in them
        movabsq $0xBADBADBADBADBADD, %rax
        movabsq $0xBADBADBADBADBADD, %rbx
        movabsq $0xBADBADBADBADBADD, %rcx
        movabsq $0xBADBADBADBADBADD, %rdx
        movabsq $0xBADBADBADBADBADD, %rsi
        movabsq $0xBADBADBADBADBADD, %rdi
        ## movabsq $0xBADBADBADBADBADD, %rsp
        movabsq $0xBADBADBADBADBADD, %rbp
        movabsq $0xBADBADBADBADBADD, %r8
        movabsq $0xBADBADBADBADBADD, %r9
        movabsq $0xBADBADBADBADBADD, %r10
        movabsq $0xBADBADBADBADBADD, %r11
        movabsq $0xBADBADBADBADBADD, %r12
        movabsq $0xBADBADBADBADBADD, %r13
        movabsq $0xBADBADBADBADBADD, %r14
        movabsq $0xBADBADBADBADBADD, %r15

        ## call the target function
        call thermo_update

        ## store callee regs to check them later
        call get_callee_regs

        ## restore 6 callee registers
        movq  0(%rsp), %rbp
        movq  8(%rsp), %rbx
        movq 16(%rsp), %r12
        movq 24(%rsp), %r13
        movq 32(%rsp), %r14
        movq 40(%rsp), %r15

        addq $56, %rsp
	.cfi_def_cfa_offset 8   # annotate another rsp change for debugger

        ## rax has the return value from the function
        ret                     
	.cfi_endproc

### store the callee register values in an array to later check that
### they have not changed
get_callee_regs:
        .cfi_startproc
        leaq callee_reg_vals(%rip), %rdi
        movq %rbp,  0(%rdi) 
        movq %rbx,  8(%rdi) 
        movq %r12, 16(%rdi) 
        movq %r13, 24(%rdi) 
        movq %r14, 32(%rdi) 
        movq %r15, 40(%rdi) 
        ret
	.cfi_endproc
        

## ### function which sets all registers to known values before intiating
## ### testing. This ensures somewhat more determinsitic behavior during
## ### testing and will prevent student code from accidentally getting 0
## ### values in registers on some systems (e.g. GRACE login server) but
## ### not other systems (Gradescope grading server)
## .text
## .global init_registers_and_do_tests
## init_registers_and_do_tests:
##         ## save all registers except the stack pointer; not
##         ## techncially necessary but doing so for completeness
##         pushq %rax
##         pushq %rbx
##         pushq %rcx
##         pushq %rdx
##         pushq %rsi
##         pushq %rdi
##         ## pushq %rsp
##         pushq %rbp
##         pushq %r8
##         pushq %r9
##         pushq %r10
##         pushq %r11
##         pushq %r12
##         pushq %r13
##         pushq %r14
##         pushq %r15

##         ## initialize all registers to ensure they have "garbage"
##         ## values in them
##         movq $-1, %rax
##         movq $-1, %rbx
##         movq $-1, %rcx
##         movq $-1, %rdx
##         movq $-1, %rsi
##         movq $-1, %rdi
##         ## movq $-1, %rsp
##         movq $-1, %rbp
##         movq $-1, %r8
##         movq $-1, %r9
##         movq $-1, %r10
##         movq $-1, %r11
##         movq $-1, %r12
##         movq $-1, %r13
##         movq $-1, %r14
##         movq $-1, %r15

##         ## 8 bytes on stack at the start for retur address
##         ## 15 * 8 bytes pushed
##         ## stack should be aligned to a 16-byte boundary for a call
##         call do_tests

##         ## restore all registers
##         popq %r15
##         popq %r14
##         popq %r13
##         popq %r12
##         popq %r11
##         popq %r10
##         popq %r9
##         popq %r8
##         popq %rbp
##         ## popq %rsp
##         popq %rdi
##         popq %rsi
##         popq %rdx
##         popq %rcx
##         popq %rbx
##         popq %rax

##         ret
        
