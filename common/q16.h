#ifndef Q16_COMMON_H
#define Q16_COMMON_H

#define TEXT_START 0x0400
#define TEXT_END 0x7FF
#define HEAP_START 0x8000
#define HEAP_END 0xDFFF
#define push(sp, value) (*(sp)++ = (value))
#define pop(sp)         (*--(sp))
#define peek(sp) (*((sp) - 1))

#include <stdint.h>

typedef enum {
    OP_HALT  = 0x00,
    OP_SET   = 0x01,
    OP_MOV   = 0x02,
    OP_LOAD  = 0x03,
    OP_STORE = 0x04,
    OP_ADD   = 0x05,
    OP_SUB   = 0x06,
    OP_AND   = 0x07,
    OP_OR    = 0x08,
    OP_LSH   = 0x09,
    OP_JMP   = 0x0A,
    OP_JE   = 0x0B,
    OP_JG   = 0x0C,
    OP_PUSH  = 0x0D,
    OP_POP   = 0x0E,
    OP_CALL  = 0x0F,
    OP_RET   = 0x10,
    OP_OUT   = 0x11
} OpCode;


#endif