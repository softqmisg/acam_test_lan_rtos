/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
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
#include "tim.h"
#include "lwip.h"
#include "api.h"
#include "httpserver-netconn.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern struct netif gnetif;
char buffer[256];
osThreadId  tcpclientHandle;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId backlightHandle;
osMessageQId backlight_valueHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void StarttcpclientTask(void const *argument);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void backlightTask(void const * argument);

extern void MX_USB_DEVICE_Init(void);
extern void MX_USB_HOST_Init(void);
extern void MX_LWIP_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

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

  /* Create the queue(s) */
  /* definition and creation of backlight_value */
  osMessageQDef(backlight_value, 100, uint8_t);
  backlight_valueHandle = osMessageCreate(osMessageQ(backlight_value), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 256);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of backlight */
  osThreadDef(backlight, backlightTask, osPriorityLow, 0, 256);
  backlightHandle = osThreadCreate(osThread(backlight), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
//  osThreadDef(tcpclientTask,StarttcpclientTask,osPriorityBelowNormal,0,256);
//    tcpclientHandle = osThreadCreate(osThread(tcpclientTask), NULL);
  osThreadDef(httpTask,http_server_netconn_thread,osPriorityBelowNormal,0,DEFAULT_THREAD_STACKSIZE);
  osThreadCreate(osThread(httpTask), NULL);

  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();

  /* init code for USB_HOST */
  MX_USB_HOST_Init();

  /* init code for LWIP */
  MX_LWIP_Init();
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
//  http_server_netconn_init();
  printf("StartDefaultTask\n\r");
  int16_t desiredbrightness=10;
  int16_t direction=1;

  for(;;)
  {

	  desiredbrightness+=direction;
	  if(desiredbrightness>90) direction=-1;
	  if(desiredbrightness==10) direction=1;
//	  printf("Send:brightness=%d,direction=%d\n",desiredbrightness,direction);
	  osMessagePut(backlight_valueHandle, desiredbrightness, osWaitForever);
//	  osMessageQueuePut(backlight_valueHandle, (void *)&desiredbrightness, 0, 1000);
	  osDelay(10);
//	  osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_backlightTask */
/**
* @brief Function implementing the backlight thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_backlightTask */
void backlightTask(void const * argument)
{
  /* USER CODE BEGIN backlightTask */
  /* Infinite loop */
	osEvent event;
	uint16_t backlight_value;
	uint16_t pulse_width;
	printf("backlighttask start\n\r");
  for(;;)
  {
//	  status=osMessageQueueGet(backlight_valueHandle,(uint16_t *) &backlight_value,NULL, 1000);
	  event=osMessageGet(backlight_valueHandle, osWaitForever);
//	  if(status==osOK)
	  if(event.status==osEventMessage)
	  {
//		  printf("RCV:%d\n",backlight_value);
		  backlight_value=(uint16_t)event.value.v;
		  pulse_width=(uint16_t) (__HAL_TIM_GET_AUTORELOAD(&htim4)* backlight_value) / 100;
			__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_4,pulse_width);
			HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);
	  }
    osDelay(1);
  }
  /* USER CODE END backlightTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void StarttcpclientTask(void const *argument)
{

	ip_addr_t local_ip;
	ip_addr_t remote_ip;
	struct netconn *nc;
	struct netbuf *nb;
	err_t res;
	uint16_t len=0;
	printf("tcpclientTask!\r\n");
	while(gnetif.ip_addr.addr==0) osDelay(1);
	local_ip=gnetif.ip_addr;
	printf("Assigned IP:%s\n\r",ip4addr_ntoa(&local_ip));

	ip4addr_aton("192.168.1.11",&remote_ip);
	nc=netconn_new(NETCONN_TCP);
	if(nc==NULL){
		printf("new Connection Error!\r\n");
		while(1) osDelay(1);
	}
	res=netconn_bind(nc,&local_ip,0);
	if(res!=ERR_OK){
		printf("bind Connection Error!\r\n");
		while(1) osDelay(1);
	}
	osDelay(20000);

	while((res=netconn_connect(nc,&remote_ip,17))!=ERR_OK)
	{
		printf("connect to Connection Error,%d!\r\n",res);
		osDelay(2000);
	}
	printf("connect to Connection OK!\r\n");
	len=sprintf(buffer,"Hello World From Nucleo-F7");
	res=netconn_write(nc,buffer,len,NETCONN_NOCOPY );
	if(res!=ERR_OK){
		printf("Write Error!\r\n");
		while(1) osDelay(1);
	}
	res=netconn_recv(nc,&nb);
	if(res!=ERR_OK){
		printf("Recv Error!\r\n");
		while(1) osDelay(1);
	}
	len=netbuf_len(nb);
	if(len>0){
		netbuf_copy(nb,buffer,len);
		printf("Recieved %d Byte:\r\n%s\r\n",len,buffer);
	}
	netconn_close(nc);
	netconn_delete(nc);

	for(;;)
	{
		osDelay(1);
	}
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
