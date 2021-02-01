/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "printf.h"
#include "tcp_client.h"
#include "mqttclient.h"

#include "semphr.h"
#include "queue.h"

#include "tftlcd.h"
#include "lcd_print.h"
#include "pic.h"

#include "iot_interface.h"
#include "protocol.h"
#include "mcu_if.h"
#include "spec_opcode.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

#include "queue.h"
QueueHandle_t MQTT_Data_Queue = NULL;

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 256 * 4
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
//task_led
osThreadId_t task_led_handle;
const osThreadAttr_t task_led_attributes = {
    .name = "task_led",
    .priority = (osPriority_t) osPriorityHigh,
    .stack_size = 256 * 4
};
void task_led_entry(void *argument);

//task_test_c
osThreadId_t task_test_c_handle;
const osThreadAttr_t task_test_c_attributes = {
    .name = "task_test_c",
    .priority = (osPriority_t) osPriorityNormal,
    .stack_size = 256 * 4
};
void task_test_c_entry(void *argument);


//task_test_c
osThreadId_t lcd_dis_handle;
const osThreadAttr_t lcd_dis_attributes = {
    .name = "lcd_dis",
    .priority = (osPriority_t) osPriorityRealtime,
    .stack_size = 256 * 4
};
void lcd_dis_entry(void *argument);


osThreadId_t task_wifi_handle;
const osThreadAttr_t task_wifi_attributes = {
    .name = "task_wifi",
    .priority = (osPriority_t) osPriorityNormal,
    .stack_size = 256 * 10
};
void task_wifi_entry(void *argument);


/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

extern void MX_LWIP_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
    /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
    /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
    /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
    /* add queues, ... */
    /* 创建Test_Queue */
    MQTT_Data_Queue = xQueueCreate(10, sizeof(DHT11_Data_TypeDef));    /*消息的大?? */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
    /* add threads, ... */
    task_led_handle = osThreadNew(task_led_entry, NULL, &task_led_attributes);
//	lcd_dis_handle = osThreadNew(lcd_dis_entry, NULL, &lcd_dis_attributes);
//	task_test_c_handle = osThreadNew(task_test_c_entry, NULL, &task_test_c_attributes);
//	task_wifi_handle = osThreadNew(task_wifi_entry, NULL, &task_wifi_attributes);

  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for LWIP */
  MX_LWIP_Init();
  /* USER CODE BEGIN StartDefaultTask */

    mqtt_thread_init();

    /* Infinite loop */
    for(;;)
    {
        task_tcp_client();
        osDelay(10);
    }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
uint32_t task_led_cnt = 0;
char buff[200] = {0};
DHT11_Data_TypeDef test = {0};

void task_led_entry(void *argument)
{
    uint8_t led_flag = 1;
    
    //TFTLCD_Init_ILI9225();

    while(1)
    {
        HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, (GPIO_PinState)(led_flag & (1 << 0)));
        HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, (GPIO_PinState)(led_flag & (1 << 1)));
        HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, (GPIO_PinState)(led_flag & (1 << 2)));


        led_flag <<= 1;
        if(led_flag >= 0x10)
        {
            led_flag = 0x01;
			
			if(mqtt_connect_flag == 1)
			{
				test.humidity += 0.1;
				test.temperature += 0.1;
				int res = xQueueSend(MQTT_Data_Queue, &test, 1000);
				printf("send_res = %d\r\n", res);
			}
        }
        
        //打印空闲堆栈
        //lcd_print("xPortGetFreeHeapSize = %d", xPortGetFreeHeapSize());
        //lcd_print("xPortGetMinimumEverFreeHeapSize = %d",xPortGetMinimumEverFreeHeapSize());


        osDelay(500);
    }
}

void task_test_c_entry(void *argument)
{
    uint32_t task_cnt = 0;
    
    while(1)
    {

        task_cnt++;
        //lcd_print("task_cnt = %04d", task_cnt);
        int * temp = malloc(10000);
        *temp = 1000;
        (*temp)++;
        lcd_print("*temp = %d", *temp);
        
        if(task_cnt > 20)
        {
//            task_cnt = 0;
//            __HAL_RCC_PWR_CLK_ENABLE();
//            HAL_GPIO_DeInit(GPIOA, GPIO_PIN_All);
//            HAL_GPIO_DeInit(GPIOB, GPIO_PIN_All);
//            HAL_GPIO_DeInit(GPIOC, GPIO_PIN_All);
//            HAL_GPIO_DeInit(GPIOD, GPIO_PIN_All);
//            HAL_GPIO_DeInit(GPIOE, GPIO_PIN_All);
//            HAL_GPIO_DeInit(GPIOF, GPIO_PIN_All);
//            HAL_GPIO_DeInit(GPIOG, GPIO_PIN_All);
//            __disable_irq();
//            HAL_PWREx_EnableFlashPowerDown();
//            __HAL_RCC_CLEAR_RESET_FLAGS();
//            HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
//            __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
//            HAL_PWR_EnterSTANDBYMode();
        }
        
        osDelay(200);
    }
}

SemaphoreHandle_t lcd_sync;
void lcd_dis_entry(void *argument)
{
    uint32_t task_cnt = 0;
    lcd_sync = xSemaphoreCreateBinary();
    while(1)
    {
        xSemaphoreTake(lcd_sync, 50);
        LCD_ShowPicture(0, 0, 176, 220, lcd_disp[0][0]);
        xSemaphoreGive(lcd_sync);
        osDelay(50);
    }
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
