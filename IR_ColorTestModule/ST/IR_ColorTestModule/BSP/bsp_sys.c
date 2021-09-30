
//#include <bsp.h>
//#include <bsp_sys.h>

//#define VECT_TAB_OFFSET  0x00 /*!< Vector Table base offset field. 
//                                   This value must be a multiple of 0x200. */
//								   
///* PLL_VCO = (HSE_VALUE or HSI_VALUE / PLL_M) * PLL_N */
//#define PLL_M      25
//#define PLL_N      240

///* SYSCLK = PLL_VCO / PLL_P */
//#define PLL_P      2

///* USB OTG FS, SDIO and RNG Clock =  PLL_VCO / PLLQ */
//#define PLL_Q      5


///** @addtogroup STM32F2xx_System_Private_Variables
//  * @{
//  */

//  uint32_t SystemCoreClock = 120000000;

//  __I uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
//  
//static void SetSysClock(void);

//void SystemInit(void)
//{
//  /* Reset the RCC clock configuration to the default reset state ------------*/
//  /* Set HSION bit */
//  RCC->CR |= (uint32_t)0x00000001;

//  /* Reset CFGR register */
//  RCC->CFGR = 0x00000000;

//  /* Reset HSEON, CSSON and PLLON bits */
//  RCC->CR &= (uint32_t)0xFEF6FFFF;

//  /* Reset PLLCFGR register */
//  RCC->PLLCFGR = 0x24003010;

//  /* Reset HSEBYP bit */
//  RCC->CR &= (uint32_t)0xFFFBFFFF;

//  /* Disable all interrupts */
//  RCC->CIR = 0x00000000;

////#ifdef DATA_IN_ExtSRAM
////  SystemInit_ExtMemCtl(); 
////#endif /* DATA_IN_ExtSRAM */
//         
//  /* Configure the System clock source, PLL Multiplier and Divider factors, 
//     AHB/APBx prescalers and Flash settings ----------------------------------*/
//  SetSysClock();

//  /* Configure the Vector Table location add offset address ------------------*/
////#ifdef VECT_TAB_SRAM
////  SCB->VTOR = SRAM_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal SRAM */
////#else
//  SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH */
////#endif
//}



///**
//  * @brief  Configures the System clock source, PLL Multiplier and Divider factors, 
//  *         AHB/APBx prescalers and Flash settings
//  * @Note   This function should be called only once the RCC clock configuration  
//  *         is reset to the default reset state (done in SystemInit() function).   
//  * @param  None
//  * @retval None
//  */
//static void SetSysClock(void)
//{
///******************************************************************************/
///*            PLL (clocked by HSE) used as System clock source                */
///******************************************************************************/
//  __IO uint32_t StartUpCounter = 0, HSEStatus = 0;
//  
//  /* Enable HSE */
//  RCC->CR |= ((uint32_t)RCC_CR_HSEON);
// 
//  /* Wait till HSE is ready and if Time out is reached exit */
//  do
//  {
//    HSEStatus = RCC->CR & RCC_CR_HSERDY;
//    StartUpCounter++;
//  } while((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

//  if ((RCC->CR & RCC_CR_HSERDY) != RESET)
//  {
//    HSEStatus = (uint32_t)0x01;
//  }
//  else
//  {
//    HSEStatus = (uint32_t)0x00;
//  }

//  if (HSEStatus == (uint32_t)0x01)
//  {
//    /* HCLK = SYSCLK / 1*/
//    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;
//      
//    /* PCLK2 = HCLK / 2*/
//    RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;
//    
//    /* PCLK1 = HCLK / 4*/
//    RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;

//    /* Configure the main PLL */
//    RCC->PLLCFGR = PLL_M | (PLL_N << 6) | (((PLL_P >> 1) -1) << 16) |
//                   (RCC_PLLCFGR_PLLSRC_HSE) | (PLL_Q << 24);

//    /* Enable the main PLL */
//    RCC->CR |= RCC_CR_PLLON;

//    /* Wait till the main PLL is ready */
//    while((RCC->CR & RCC_CR_PLLRDY) == 0)
//    {
//    }
//   
//    /* Configure Flash prefetch, Instruction cache, Data cache and wait state */
//    FLASH->ACR = FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_LATENCY_3WS;

//    /* Select the main PLL as system clock source */
//    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
//    RCC->CFGR |= RCC_CFGR_SW_PLL;

//    /* Wait till the main PLL is used as system clock source */
//    while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS ) != RCC_CFGR_SWS_PLL);
//    {
//    }
//  }
//  else
//  { /* If HSE fails to start-up, the application will have wrong clock
//         configuration. User can add here some code to deal with this error */
//  }

//}
