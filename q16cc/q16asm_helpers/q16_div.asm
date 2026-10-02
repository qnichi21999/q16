__q16_div:
    SET R5, 0
    SET R3, 0
    SET R4, 16

__q16_div_loop:
    JE R4, R0, __q16_end

    LSH R3, 1
    LSH R5, 1

    PUSH R4
    SET R4, 0x8000
    AND R4, R1
    JE R4, R0, __q16_no_bit

    SET R4, 1
    OR R3, R4

__q16_no_bit:
    POP R4
    LSH R1, 1

    JE R3, R2, __q16_subtract
    JG R3, R2, __q16_subtract
    JMP __q16_next_iter

__q16_subtract:
    SUB R3, R2

    PUSH R4
    SET R4, 1
    OR R5, R4
    POP R4

__q16_next_iter:
    PUSH R1
    SET R1, 1
    SUB R4, R1
    POP R1

    JMP __q16_div_loop

__q16_end:
    RET

