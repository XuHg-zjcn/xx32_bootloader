void Op_GoProgram()
{
  __disable_irq();
  ((void (*)())(addr_j+4))();
}

void Op_ReadMem(uint32_t addr, uint8_t *pbuff, uint32_t size)
{
  const uint8_t *pmem = (const uint8_t *)addr;
  while(size--){
    *pbuff++ = *pmem++;
  }
}

void Op_WriteMem(uint32_t addr, const uint8_t *pbuff, uint32_t size)
{
}

void Op_WriteFlash(uint32_t addr, const uint8_t *pbuff, uint32_t size)
{
  
}

void Op_WriteMemFlash(uint32_t addr, const uint8_t *pbuff, uint32_t size)
{
  if(addr & 0xff000000 == 0x08000000){
    Op_WriteFlash(addr, pbuff, size);
  }else if(addr & 0xff000000 == 0x20000000){
    Op_WriteMem(addr, pbuff, size);
  }
}
