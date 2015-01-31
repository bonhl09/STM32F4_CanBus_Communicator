/*
AUTHOR: Christopher Braga
PURPOSE: This program uses an external transciever and an on board CANBUS controller to communicate with other STM32F4 microcontrollers
CREDIT: Credit goes to STMElectronics for board design, peripheral examples/library, and STM Startup configuration file provided
*/
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4_discovery.h"

/** @addtogroup STM32F4xx_StdPeriph_Examples
  * @{
  */

/** @addtogroup CAN_Networking
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define KEY_PRESSED     0x00
#define KEY_NOT_PRESSED 0x01
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
CAN_InitTypeDef        CAN_InitStructure;
CAN_FilterInitTypeDef  CAN_FilterInitStructure;
CanTxMsg TxMessage;
extern CanRxMsg RxMessage;

/* Private function prototypes -----------------------------------------------*/
void NVIC_Config(void);
void CAN_Config(void);
void Init_RxMes(CanRxMsg *RxMessage);
void Delay(void);
void confirm_message(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
	NVIC_Config();

	/* Initalizing LEDs 1-4, User Button, and CAN BUS */
	STM_EVAL_LEDInit(LED3);
	STM_EVAL_LEDInit(LED4);
	STM_EVAL_LEDInit(LED5);
	STM_EVAL_LEDInit(LED6);
  
	STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_GPIO); 
	
	/* CAN configuration */
	CAN_Config();
  
	
	/* Infinite loop. Pulling used instead of interrupts due to project requirements */
	while(1)
	{
		// Send message over CAN BUS when user button is pressed.
		if (STM_EVAL_PBGetState(BUTTON_USER) == Bit_SET){
			CAN_Transmit(CANx,&TxMessage);	
			Delay();
		}

		if (CAN_GetFlagStatus(CANx , CAN_FLAG_FMP0) == SET) {
			CAN_Receive(CANx, CAN_FIFO0, &RxMessage);
			//If message contains correct group # display the message using the LED Display
			if (RxMessage.Data[0] == 26%16){ 
				LED_Display(RxMessage.Data[1]);
			}
				CAN_ClearFlag(CANx,  CAN_FLAG_FMP0);
		}

	}
}

/**
  * @brief  Configures the CAN.
  * @param  None
  * @retval None
  */
void CAN_Config(void)
{
	 GPIO_InitTypeDef  GPIO_InitStructure;
  
	/* CAN GPIOs configuration **************************************************/

	/* Enable GPIO clock */
	RCC_AHB1PeriphClockCmd(CAN_GPIO_CLK, ENABLE);

	/* Connect CAN pins to AF9 (Alternate Function 9?)*/
	GPIO_PinAFConfig(CAN_GPIO_PORT, CAN_RX_SOURCE, CAN_AF_PORT); //D , PINSOURCE 0, CAN1
	GPIO_PinAFConfig(CAN_GPIO_PORT, CAN_TX_SOURCE, CAN_AF_PORT); //D , PINSOURCE 1, CAN1
  
	/* Configure CAN RX and TX pins */

	GPIO_InitStructure.GPIO_Pin = CAN_RX_PIN | CAN_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; //set to alternate function
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP; 
	GPIO_Init(CAN_GPIO_PORT, &GPIO_InitStructure);

	/* CAN configuration ********************************************************/  
	/* Enable CAN clock */
	RCC_APB1PeriphClockCmd(CAN_CLK, ENABLE);
  
	/* CAN register init */
	CAN_DeInit(CANx);

	/* CAN cell init */
	CAN_InitStructure.CAN_TTCM = DISABLE;
	CAN_InitStructure.CAN_ABOM = DISABLE;
	CAN_InitStructure.CAN_AWUM = DISABLE;
	CAN_InitStructure.CAN_NART = DISABLE;
	CAN_InitStructure.CAN_RFLM = DISABLE;
	CAN_InitStructure.CAN_TXFP = DISABLE;
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
    
	/* CAN Baudrate = 1 MBps (CAN clocked at 30 MHz) */
	CAN_InitStructure.CAN_BS1 = CAN_BS1_6tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_8tq;
	CAN_InitStructure.CAN_Prescaler = 4;
	CAN_Init(CANx, &CAN_InitStructure);
	
	

	/* CAN filter init */
	#ifdef  USE_CAN1
		CAN_FilterInitStructure.CAN_FilterNumber = 6;
	#else /* USE_CAN2 */
		CAN_FilterInitStructure.CAN_FilterNumber = 14;
	#endif  /* USE_CAN1 */


	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
	
	
	//Set software filter to sort for CAN messages from control station
	CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0000;
	CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0xFFE0;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);
  
	
  /* Transmit Structure preparation */

	TxMessage.StdId = 26; //group id
	TxMessage.RTR = CAN_RTR_Data;
	TxMessage.IDE = CAN_ID_STD;
	TxMessage.DLC = 1; //1 because only 1 data index sent
	TxMessage.Data[0] = 26%16;//group id. Set to meet design requirements
}

/**
  * @brief  Configures the NVIC for CAN.
  * @param  None
  * @retval None
  */
void NVIC_Config(void)
{
  NVIC_InitTypeDef  NVIC_InitStructure;
	
//THIS IS SETTING UP INTERUPT REQUESTS FOR CAN1 and CAN2
#ifdef  USE_CAN1 
  NVIC_InitStructure.NVIC_IRQChannel = CAN1_RX0_IRQn; 
#else  /* USE_CAN2 */
  NVIC_InitStructure.NVIC_IRQChannel = CAN2_RX0_IRQn;
#endif /* USE_CAN1 */

  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  Initializes the Rx Message.
  * @param  RxMessage: pointer to the message to initialize
  * @retval None
  */
void Init_RxMes(CanRxMsg *RxMessage)
{
  uint8_t i = 0;

  RxMessage->StdId = 0x00;
  RxMessage->ExtId = 0x00;
  RxMessage->IDE = CAN_ID_STD;
  RxMessage->DLC = 0;
  RxMessage->FMI = 0;
  for (i = 0;i < 1;i++)
  {
    RxMessage->Data[i] = 0x00;
  }
}

/**
  * @brief  Turn ON/OFF the dedicated led according to the message
  * @param  msg: the message received from the TA board
  * @retval None
  */
void LED_Display(uint8_t msg)
{
	uint8_t bit1 = 0x01 & msg;//(0x02 & msg);
	uint8_t bit2 = 0x02 & msg;
	uint8_t bit3 = 0x04 & msg;
	uint8_t bit4 = 0x08 & msg;
	/* Turn off all leds */
	STM_EVAL_LEDOff(LED3);
	STM_EVAL_LEDOff(LED4);
	 STM_EVAL_LEDOff(LED5);
	STM_EVAL_LEDOff(LED6);

	if (bit1) {STM_EVAL_LEDOn(LED4);}
	if (bit2) {STM_EVAL_LEDOn(LED3);}
	if (bit3) {STM_EVAL_LEDOn(LED5);}
	if (bit4) {STM_EVAL_LEDOn(LED6);}	

	/* you may choose to use the leds to show received/sent messages */
}

/**
  * @brief  Delay
  * @param  None
  * @retval None
  */
	
void Delay(void)
{
  uint16_t nTime = 0x0000;

  for(nTime = 0; nTime <0xFFF; nTime++) {}
}

/**
* @brief  Configure board LEDs based on message content
* @param  None
* @retval None
*/
void confirm_message(void)
{
	if (RxMessage.StdId != 0x26)
	{
		STM_EVAL_LEDOn(LED5);
		return;
	}
	
	if (RxMessage.DLC != 1)
	{
		STM_EVAL_LEDOn(LED3);
		return;
	}
	
	if (RxMessage.Data[0] != 0x14%16)
	{
		STM_EVAL_LEDOn(LED5);
		return;
	}
	
	STM_EVAL_LEDOn(LED4);
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}


	
#endif

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/