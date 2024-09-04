.text 
.global set_temp_from_ports
.global thermo_update

# Adjust thermometer display to show the current temperature
thermo_update:
        pushq   %rdx            # push 64-bit reg to the stack
        pushq   $0              # put a 0 temp_t struct on the stack, %rsp is the pointer
        movq    %rsp, %rdi      # set arg1 as &temp

        pushq   $0              # int display = 0
        movq    %rsp, %rsi      # set arg2 as &display

        call    set_temp_from_ports
        cmpl    $1, %eax        # if(set_temp_from_ports(&temp) == 1)
        je      .ERROR2         # go to error

        movl    (%rdi), %edi    # set arg1 as temp 
        call    set_display_from_temp
        cmpl    $1, %eax        # if(set_display_from_temp(temp, &display) == 1){
        je      .ERROR2         # go to error
        movl    (%rsi), %edx    # edx = *display
        movl    %edx, THERMO_DISPLAY_PORT(%rip)   # THERMO_DISPLAY_PORT = display

        popq    %rdi            # restore memory of temp on the stack
        popq    %rsi            # restore memory of display on the stack
        popq    %rdx            # restore the memory of call on the stack

        movl    $0, %eax
        ret                     # return 0

    .ERROR2:
        popq    %rdi            # restore memory of temp on the stack
        popq    %rsi            # restore memory of display on the stack
        popq    %rdx            # restore the memory of call on the stack

        movl    $1, %eax
        ret                     # return 1

# read temperature from ports to temp
# return 0 if temp is above max value, else 1
set_temp_from_ports:
        movb    THERMO_STATUS_PORT(%rip), %sil      # copy char STATUS_PORT to sil
        movw    THERMO_SENSOR_PORT(%rip), %dx       # copy short SENSOR_PORT to dx

        cmpw    $64000, %dx         # compa/////////////re 64000 with SENSOR
        ja      .INVALID            # jump if SENSOR > 64000 (unsigned)

    .CONVERT:
        movw    %dx, %cx            # move SENSOR to cx
        shrw    $6, %cx             # tenths_deg = SENSOR / 64        %rcx: tenths_deg
        # since cx is unsigned short, and the first bit might be 1. Use logical shift to zero out
        # bits before the actual value

        movw    %dx, %r10w          # move SENSOR to r10w             %rbx: reminder
        andw    $63, %r10w          # mask to get the value of reminder

        cmpw    $32, %r10w          # if (reminder >= 32)
        jl     .NO_ROUNDUP          # jp if reminder < 32
        incw    %cx                 # tenths_deg++
    .NO_ROUNDUP:
        addw    $-500, %cx          # tenths_deg -= 500

        testb   %sil, %sil          # check if STATUS is 0
        jnz     .CONVERT_FAHRENHEIT # covert to F if it is not 0

        movb    $0, 2(%rdi)         # temp->is_fahrenheit = 0
        movw    %cx, 0(%rdi)        # temp->tehnths_degree = tenths_deg
        jmp      .END               # go to return


    .CONVERT_FAHRENHEIT:
        movb    $1, 2(%rdi)         # temp->is_fahrenheit = 1
        imulw   $9, %cx             # tenth_deg = tenth_deg * 9
        movw    %cx, %ax            # prepare for division
        cwtl                        # sign extend ax to long word
        cltq                        # sign extend eax to quad word
        cqto                        # sign extend ax to dx
        movw    $5, %r8w            # set esi to 5
        idivw   %r8w                # do division
        movw    %ax, %cx            # tenth_deg = tenth_deg / 5
        addw    $320, %cx           # tenth_deg += 320
        movw    %cx, 0(%rdi)        # temp->tenths_degrees = tenths_deg
        jmp      .END

    .INVALID:                       # if temp is above max value
        movq    $1, %rax            # return 1
        ret

    .END:
        movq    $0, %rax            # return 0
        ret



# data area associated with set_display_from_temp
.data
array:                  # an array of bit masks for each digit
        .int 0b0111111  # array[0] = 0b0111111
        .int 0b0000110
        .int 0b1011011
        .int 0b1001111
        .int 0b1100110
        .int 0b1101101
        .int 0b1111101
        .int 0b0000111
        .int 0b1111111
        .int 0b1101111  # array[9] = 0b1101111


.text
.global set_display_from_temp

# alter the bits of integer pointed to by display to reflect the temperature in struct arg temp
# return 1 if temperature is out of bound or improper indication of C/F is given
set_display_from_temp:
        movl    %edi, %edx      # dl = temp
        shrl    $16, %edx       # dl = temp.is_fahrenheit
        cmpb    $0, %dl         # check if dl == 1
        je      .IS_CELCIUS     # if dl == 1, no need to check if dl == 0
        cmpb    $1, %dl         # check if dl == 0
        je      .IS_FAHRENHEIT
        jmp     .ERROR          # if dl == 1 or 0, it is invalid

    .IS_CELCIUS:
        movl    %edi, %ecx      # cx = temp.tenths_deg
        andl    $0xFFFF, %ecx
        cmpw   $-500, %cx       # compare -500 with degree
        jl      .ERROR          # error if degree < -500
        cmpw   $500, %cx        # compare 500 with degree
        jg      .ERROR          # error if degree > 500

        # set the indication of deg C
        movl    $0b01,(%rsi)    # *display = 0b01
        sall    $28, (%rsi)     # *display << 28

        jmp     .VALID          # temp is valid

    .IS_FAHRENHEIT:
        movl    %edi, %ecx      # cx = temp.tenths_degrees
        andl    $0xFFFF, %ecx
        cmpw   $-580, %cx       # compare -580 with degree
        jl      .ERROR          # error if degree < -580
        cmpw   $1220, %cx       # compare 500 with degree
        jg      .ERROR          # error if degree > 500

        # set the indication of deg F
        movl    $0b10,(%rsi)    # *display = 0b10
        sall    $28, (%rsi)     # *display << 28

        jmp     .VALID          # temp is valid

    .VALID:
        testw   %cx, %cx        # check if cx is positive
        jns      .SET_DISPLAY   # jump if cx is nonnegative
        negw    %cx             # negate if cx is negative

    .SET_DISPLAY:
        movw    %cx, %ax            # prepare for division
        cwtl                        # sign extend ax to long word
        cltq                        # sign extend eax to quad word
        cqto                        # sign extend ax to cx
        movw    $10, %r8w           # move 10 to cx
        idivw   %r8w                # degree / 10
        movq    %rdx, %r9           # %r9: tenth (reminder)
        cltq                        # sign extend eax to quad word
        cqto                        # sign extend ax to cx
        idivw   %r8w                # ax / 10
        movq    %rdx, %r10          # %r10: ones
        cltq                        # sign extend eax to quad word
        cqto                        # sign extend ax to cx
        idivw   %r8w                # ax / 10
        movq    %rdx, %r11          # %r11: tens
        cltq                        # sign extend eax to quad word
        cqto                        # sign extend ax to cx
        idivw   %r8w                # ax / 10
        movq    %rdx, %r12          # %r12: hundreds

        leaq    array(%rip), %r13       # r13 points to an array, rip to enable relocation
        movl    (%r13, %r9, 4), %r14d   # r14d = array[tenth]
        xorl    %r14d, (%rsi)           # *display = *display | mask

        movl    (%r13, %r10, 4), %r14d  # mask = mask[temp_ones]
        sall    $7, %r14d               # mask = mask << 7
        xorl    %r14d, (%rsi)           # *display = *display | mask

        testw    %r12w, %r12w           # compare hudrend with 0
        jz      .TEN_DIG                # if hundreds == 0, it is not a hundred digit number

        movl    (%r13, %r11, 4), %r14d  # mask = mask[temp_tenths]
        sall    $14, %r14d              # mask = mask << 14
        xorl    %r14d, (%rsi)           # *display = *display | mask

        movl    (%r13, %r12, 4), %r14d  # mask = mask[temp_hundreds]
        sall    $21, %r14d              # mask = mask << 21
        xorl    %r14d, (%rsi)           # *display = *display | mask

        movl    %edi, %ecx              # reset cx to be the temp.tenths_deg
        testw   %cx, %cx
        jns     .END2                   # jump to the end if deg >= 0
        movl    $0b100000, %r14d        # mask = 0b1000000
        sall    $28, %r14d              # mask = mask << 28
        xorl    %r14d, (%rsi)           # *display = *display | mask
        jmp      .END2

    .TEN_DIG:
        testw   %r11w, %r11w            # check if tenth == 0
        jz      .ONE_DIGIT              # if tenth == 0, it is a one digit number

        movl    (%r13, %r11, 4), %r14d  # mask = mask[temp_tenths]
        sall    $14, %r14d              # mask = mask << 14
        xorl    %r14d, (%rsi)           # *display = *display | mask

        movl    %edi, %ecx              # reset cx to temp.tenths_deg
        testw   %cx, %cx
        jns     .END2                   # jump to the end if deg >= 0
        movl    $0b1000000, %r14d       # mask = 0b1000000
        sall    $21, %r14d              # mask = mask << 21
        xorl    %r14d, (%rsi)           # *display = *display | mask
        jmp      .END2

    .ONE_DIGIT:
        movl    %edi, %ecx              # reset cx to temp.tenths_deg
        testw   %cx, %cx
        jns     .END2                   # jump to the end if deg >= 0
        movl    $0b1000000, %r14d       # mask = 0b1000000
        sall    $14, %r14d              # mask = mask << 21
        xorl    %r14d, (%rsi)           # *display = *display | mask

    .END2:
        movl $0, %eax           # return 0
        ret

    .ERROR:
        movl    $1, %eax        # return 1
        ret
