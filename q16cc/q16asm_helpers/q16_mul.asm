__q16_mul:
    SET R5, 0
    PUSH R4
    SET R4, 16

__q16_mul_loop:
    JE R4, R0, __q16_mul_end

    LSH R5, 1

    PUSH R3
    SET R3, 0x8000
    AND R3, R1
    JE R3, R0, __q16_mul_no_add

    ADD R5, R2

__q16_mul_no_add:
    POP R3
    LSH R1, 1

    PUSH R3
    SET R3, 1
    SUB R4, R3
    POP R3

    JMP __q16_mul_loop

__q16_mul_end:
    POP R4
    RET

