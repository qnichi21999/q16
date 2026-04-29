.org 0x0400

    SET R0, 0
    SET R1, 1
    SET R2, 0
    SET R4, 10
    SET R5, 1

loop:
    OUT R0
    MOV R3, R1
    ADD R1, R0
    MOV R0, R3
    ADD R2, R5
    JE  R2, R4, done
    JMP loop

done:
    HALT
