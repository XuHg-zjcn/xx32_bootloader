#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_usart.h"
#include "stm32f1xx_hal.h"

#define USARTx_INSTANCE     (USART1)
#define USARTx_TX_GPIO_PORT (GPIOA)
#define USARTx_TX_PIN       (LL_GPIO_PIN_9)
#define USARTx_RX_GPIO_PORT (GPIOA)
#define USARTx_RX_PIN       (LL_GPIO_PIN_10)

//ref:
//STM32CubeF1/Projects/STM32F103RB-Nucleo/Examples_LL/USART/USART_Communication_Tx

void UART_Init()
{
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
  /* Configure Tx Pin as : Alternate function, High Speed, Push pull, Pull up */
  LL_GPIO_SetPinMode(USARTx_TX_GPIO_PORT, USARTx_TX_PIN, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetPinSpeed(USARTx_TX_GPIO_PORT, USARTx_TX_PIN, LL_GPIO_SPEED_FREQ_HIGH);
  LL_GPIO_SetPinOutputType(USARTx_TX_GPIO_PORT, USARTx_TX_PIN, LL_GPIO_OUTPUT_PUSHPULL);
  LL_GPIO_SetPinPull(USARTx_TX_GPIO_PORT, USARTx_TX_PIN, LL_GPIO_PULL_UP);

  /* Configure Rx Pin as : Input Floating function, High Speed, Pull up */
  LL_GPIO_SetPinMode(USARTx_RX_GPIO_PORT, USARTx_RX_PIN, LL_GPIO_MODE_FLOATING);
  LL_GPIO_SetPinSpeed(USARTx_RX_GPIO_PORT, USARTx_RX_PIN, LL_GPIO_SPEED_FREQ_HIGH);
  LL_GPIO_SetPinPull(USARTx_RX_GPIO_PORT, USARTx_RX_PIN, LL_GPIO_PULL_UP);

  LL_USART_SetTransferDirection(USARTx_INSTANCE, LL_USART_DIRECTION_TX_RX);
  LL_USART_ConfigCharacter(USARTx_INSTANCE, LL_USART_DATAWIDTH_8B, LL_USART_PARITY_NONE, LL_USART_STOPBITS_1);
  LL_USART_SetBaudRate(USARTx_INSTANCE, SystemCoreClock, 115200);
  LL_USART_Enable(USARTx_INSTANCE);
}

void UART_Measure_Baud()
{
  //TODO: implement here
}

uint8_t UART_RxByte()
{
  while(!LL_USART_IsActiveFlag_RXNE(USARTx_INSTANCE));
  return LL_USART_ReceiveData8(USARTx_INSTANCE);
}

int UART_RxByte_Timeout(uint32_t timeout)
{
  uint32_t t_start = HAL_GetTick();
  while(!LL_USART_IsActiveFlag_RXNE(USARTx_INSTANCE) && HAL_GetTick()-t_start<timeout);
  if(LL_USART_IsActiveFlag_RXNE(USARTx_INSTANCE)){
    return LL_USART_ReceiveData8(USARTx_INSTANCE);
  }else{
    return -1;
  }
}

void UART_TxByte(uint8_t byte)
{
  while(!LL_USART_IsActiveFlag_TXE(USARTx_INSTANCE));
  LL_USART_TransmitData8(USARTx_INSTANCE, byte);
}

void UART_TxBytes(uint8_t *p, uint32_t size)
{
  while(size--){
    UART_TxByte(*p++);
  }
}
