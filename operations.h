#ifndef OPERATIONS_H
#define OPERATIONS_H

void Op_GoProgram();
void Op_ReadMem(uint32_t addr, uint8_t *pbuff, uint32_t size);
void Op_WriteMem(uint32_t addr, const uint8_t *pbuff, uint32_t size);
void Op_WriteFlash(uint32_t addr, const uint8_t *pbuff, uint32_t size);
void Op_WriteMemFlash(uint32_t addr, const uint8_t *pbuff, uint32_t size);

#endif
