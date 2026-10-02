__q16_mod:
    PUSH R7
    CALL __q16_div
    MOV R5, R3
    POP R7
    RET

