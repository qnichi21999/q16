#ifndef H_GLOBALS
#define H_GLOBALS


typedef struct {
    uint16_t r[8];
    uint16_t pc;
    uint16_t sp;
    uint8_t memory[65536];
    int halted;
} CPU;


#endif // globals