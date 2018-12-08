#include "XMC1100.h"
#include "xmc_uart.h"
#include "xmc_gpio.h"
#include "xmc_ccu4.h"
#include "xmc_scu.h"

const XMC_UART_CH_CONFIG_t uart_config = 
{	
  .data_bits = 8U,
  .stop_bits = 1U,
  .baudrate = 921600
};

const XMC_GPIO_CONFIG_t uart_rx_config = 
{
   .mode = XMC_GPIO_MODE_INPUT_TRISTATE
};

const XMC_GPIO_CONFIG_t uart_tx_config = 
{
   .mode = XMC_GPIO_MODE_OUTPUT_PUSH_PULL_ALT6
};

const XMC_CCU4_SLICE_COMPARE_CONFIG_t period_config =
{
  .timer_mode 		   = XMC_CCU4_SLICE_TIMER_COUNT_MODE_EA,
  .monoshot   		   = XMC_CCU4_SLICE_TIMER_REPEAT_MODE_REPEAT,
  .shadow_xfer_clear   = 0U,
  .dither_timer_period = 0U,
  .dither_duty_cycle   = 0U,
  .prescaler_mode	   = XMC_CCU4_SLICE_PRESCALER_MODE_NORMAL,
  .mcm_enable		   = 0U,
  .prescaler_initval   = 0U,
  .float_limit		   = 0U,
  .dither_limit		   = 0U,
  .passive_level 	   = XMC_CCU4_SLICE_OUTPUT_PASSIVE_LEVEL_LOW,
  .timer_concatenation = 0U
};

void CCU40_0_IRQHandler(void)
{
   XMC_CCU4_SLICE_ClearEvent(CCU40_CC40, XMC_CCU4_SLICE_IRQ_ID_PERIOD_MATCH);
   // XMC_UART_CH_Transmit(XMC_UART0_CH0, 0);
   PORT1->OUT ^= (PORT1_OUT_P1_Msk);
}

int main()
{
   volatile uint32_t delay = 0;
   // Enable LED PORT
   PORT1->IOCR0 =  (0x10 << PORT1_IOCR0_PC1_Pos) | (0x10 << PORT1_IOCR0_PC0_Pos);
   PORT1->OUT = PORT1_OUT_P1_Msk | PORT1_OUT_P0_Msk;

   // UART init
   XMC_UART_CH_Init(XMC_UART0_CH0, &uart_config);
   XMC_USIC_CH_SetInputSource(XMC_UART0_CH0, XMC_USIC_CH_INPUT_DX0, USIC0_C0_DX0_DX3INS);
   XMC_UART_CH_SetInputSource(XMC_UART0_CH0, XMC_USIC_CH_INPUT_DX3, USIC0_C0_DX3_P2_2);
   XMC_UART_CH_Start(XMC_UART0_CH0);

   // Enable UART port
   XMC_GPIO_Init(P2_1, &uart_tx_config);
   XMC_GPIO_Init(P2_2, &uart_rx_config);

   // Init timer for period
   // Unit init
   XMC_CCU4_SetModuleClock(CCU40, XMC_CCU4_CLOCK_SCU);
   XMC_CCU4_Init(CCU40, XMC_CCU4_SLICE_MCMS_ACTION_TRANSFER_PR_CR);
   XMC_CCU4_EnableClock(CCU40, 0);
   XMC_CCU4_StartPrescaler(CCU40);
   
   // Period timer init
   XMC_CCU4_SLICE_CompareInit(CCU40_CC40, &period_config);
   XMC_CCU4_SLICE_EnableEvent(CCU40_CC40, XMC_CCU4_SLICE_IRQ_ID_PERIOD_MATCH);
   XMC_CCU4_SLICE_SetInterruptNode(CCU40_CC40, XMC_CCU4_SLICE_IRQ_ID_PERIOD_MATCH, XMC_CCU4_SLICE_SR_ID_0);
   NVIC_SetPriority(CCU40_0_IRQn, 10);
   NVIC_EnableIRQ(CCU40_0_IRQn);
   XMC_CCU4_SLICE_SetTimerPeriodMatch(CCU40_CC40, 1450);
   XMC_CCU4_EnableShadowTransfer(CCU40, XMC_CCU4_SHADOW_TRANSFER_SLICE_0);
   XMC_CCU4_SLICE_StartTimer(CCU40_CC40);
   // XMC_SCU_SetCcuTriggerHigh(XMC_SCU_CCU_TRIGGER_CCU40);
   // PWM init


   for (;;) {
      // for (delay = 0; delay < 0x100000; delay++);
      if (XMC_UART_CH_GetStatusFlag(XMC_UART0_CH0) & XMC_UART_CH_STATUS_FLAG_RECEIVE_INDICATION) {
         XMC_UART_CH_ClearStatusFlag(XMC_UART0_CH0, XMC_UART_CH_STATUS_FLAG_RECEIVE_INDICATION);
         //XMC_UART_CH_Transmit(XMC_UART0_CH0, XMC_UART_CH_GetReceivedData(XMC_UART0_CH0));
      } else if (delay++ == 0x100000) {
         PORT1->OUT ^= (PORT1_OUT_P0_Msk);
        // XMC_UART_CH_Transmit(XMC_UART0_CH0, 0);
         delay = 0;
      }
   }
   return 0;
}