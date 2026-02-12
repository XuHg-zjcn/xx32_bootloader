/************************************************************************
 * UART baud measure
 * Copyright (C) 2025-2026  Xu Ruijun
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 ************************************************************************/
#include <stdlib.h>
#include "py32f0xx_ll_bus.h"
#include "py32f0xx_ll_tim.h"
#include "py32f0xx_ll_gpio.h"

#define MB_GPIO_PORT  (GPIOA)
#define MB_PIN        (LL_GPIO_PIN_3)
#define MB_AF         (LL_GPIO_AF_13)

void MB_TIMInit()
{
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_TIM1);

  LL_GPIO_SetPinMode(MB_GPIO_PORT, MB_PIN, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetAFPin_0_7(MB_GPIO_PORT, MB_PIN, MB_AF);
  LL_GPIO_SetPinPull(MB_GPIO_PORT, MB_PIN, LL_GPIO_PULL_UP);

  LL_TIM_DisableCounter(TIM1);
  LL_TIM_SetClockDivision(TIM1, LL_TIM_CLOCKDIVISION_DIV1);
  LL_TIM_SetPrescaler(TIM1, 0);         //No Prescaler
  LL_TIM_SetAutoReload(TIM1, 65535);
  LL_TIM_SetCounter(TIM1, 1);
  LL_TIM_SetCounterMode(TIM1, LL_TIM_COUNTERMODE_UP);
  LL_TIM_CC_DisableChannel(TIM1, LL_TIM_CHANNEL_CH1);
  LL_TIM_CC_DisableChannel(TIM1, LL_TIM_CHANNEL_CH2);
  LL_TIM_IC_SetActiveInput(TIM1, LL_TIM_CHANNEL_CH1, LL_TIM_ACTIVEINPUT_DIRECTTI);   //CH1捕获TI1
  LL_TIM_IC_SetActiveInput(TIM1, LL_TIM_CHANNEL_CH2, LL_TIM_ACTIVEINPUT_INDIRECTTI); //CH2捕获TI1
  LL_TIM_IC_SetFilter(TIM1, LL_TIM_CHANNEL_CH1, LL_TIM_IC_FILTER_FDIV1_N8);
  LL_TIM_IC_SetPrescaler(TIM1, LL_TIM_CHANNEL_CH1, LL_TIM_ICPSC_DIV1);
  LL_TIM_IC_SetPolarity(TIM1, LL_TIM_CHANNEL_CH1, LL_TIM_IC_POLARITY_FALLING);
  LL_TIM_IC_SetFilter(TIM1, LL_TIM_CHANNEL_CH2, LL_TIM_IC_FILTER_FDIV1_N8);
  LL_TIM_IC_SetPrescaler(TIM1, LL_TIM_CHANNEL_CH2, LL_TIM_ICPSC_DIV1);
  LL_TIM_IC_SetPolarity(TIM1, LL_TIM_CHANNEL_CH2, LL_TIM_IC_POLARITY_RISING);
  LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH1);
  LL_TIM_CC_EnableChannel(TIM1, LL_TIM_CHANNEL_CH2);
}

void MB_Wait_Idle()
{
  LL_GPIO_SetPinMode(MB_GPIO_PORT, MB_PIN, LL_GPIO_MODE_INPUT);
  LL_GPIO_SetPinPull(MB_GPIO_PORT, MB_PIN, LL_GPIO_PULL_UP);
  uint32_t tick_end = HAL_GetTick() + 20;
  //wait to high level enough time
  while (HAL_GetTick() < tick_end) {
    if(!LL_GPIO_IsInputPinSet(MB_GPIO_PORT, MB_PIN)){
      tick_end = HAL_GetTick() + 20;
      continue;
    }
  }
}

int MB_Measure(){
  uint32_t update_count = 0;
  uint32_t t_edges[8];
  uint32_t t_edge_i = 0;
  LL_TIM_DisableCounter(TIM1);
  LL_TIM_SetCounter(TIM1, 1);
  LL_TIM_ClearFlag_UPDATE(TIM1);
  LL_TIM_ClearFlag_CC1(TIM1);
  LL_TIM_ClearFlag_CC2(TIM1);
  LL_TIM_EnableCounter(TIM1);
  while(1){
    _Bool isUpdate = 0;
    if(LL_TIM_IsActiveFlag_UPDATE(TIM1)){
      LL_TIM_ClearFlag_UPDATE(TIM1);
      update_count++;
      isUpdate = 1;
    }
    if(LL_TIM_IsActiveFlag_CC1(TIM1)){
      uint32_t cap1 = LL_TIM_IC_GetCaptureCH1(TIM1);
      if(t_edge_i % 2 != 0){
	return -2;
      }
      uint32_t cap1e;
      if(isUpdate && cap1 > 0xefff){
	cap1e = ((update_count-1)<<16)|(cap1&0xffff);
      }else{
	cap1e = (update_count<<16)|(cap1&0xffff);
      }
      t_edges[t_edge_i++] = cap1e;
    }
    if(LL_TIM_IsActiveFlag_CC2(TIM1)){
      uint32_t cap2 = LL_TIM_IC_GetCaptureCH2(TIM1);
      if(t_edge_i % 2 != 1){
	return -2;
      }
      uint32_t cap2e;
      if(isUpdate && cap2 > 0xefff){
	cap2e = ((update_count-1)<<16)|(cap2&0xffff);
      }else{
	cap2e = (update_count<<16)|(cap2&0xffff);
      }
      t_edges[t_edge_i++] = cap2e;
    }
    if(update_count > 5000){
      return -1;  //any timeout
    }
    if(t_edge_i >= 4){
      break;
    }
  }
  if(t_edge_i == 4){
    uint32_t td1 = t_edges[1]-t_edges[0];
    uint32_t td2 = t_edges[2]-t_edges[1];
    uint32_t td3 = t_edges[3]-t_edges[2];
    uint32_t tdm = (td1+td3)/2;
    if(abs(td1-td3) > tdm/4){
      return -2;
    }
    int e = td2 - tdm*7;
    if(abs(e) > (td1+td3)/4){
      return -2;
    }
    return (t_edges[2]-t_edges[0])/8;
  }
  return -1;
}

int MB_Measure_Baud()
{
  while(1){
    MB_Wait_Idle();
    MB_TIMInit();
    int m = MB_Measure();
    if(120 < m && m < 15000){
      LL_TIM_DisableCounter(TIM1);
      LL_APB1_GRP2_DisableClock(LL_APB1_GRP2_PERIPH_TIM1);
      LL_GPIO_SetPinMode(MB_GPIO_PORT, MB_PIN, LL_GPIO_MODE_INPUT);
      return m;
    }
    LL_TIM_DisableCounter(TIM1);
  }
}
