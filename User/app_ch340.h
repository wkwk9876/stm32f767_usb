#include "main.h"
#include "sdram_alloc.h"

#define RECV_BUFF_SIZE		(128)



typedef enum {
	CH340_APPLICATION_IDLE = 0,
	CH340_APPLICATION_START,
	CH340_APPLICATION_READY,
	CH340_APPLICATION_READY_CHECK,
	CH340_APPLICATION_SETTING_LINECODE,
	CH340_APPLICATION_RUNNING,
	CH340_APPLICATION_DISCONNECT,
}ApplicationTypeDef;

typedef struct _LOGBuffState {
	sdram_buffer_map	*buf;
	__IO uint32_t     	p_read;
	__IO uint32_t     	p_write;
}CDC_LOG_BUFF_STATE;


typedef struct _ch340_app
{
	osMessageQId 			AppliEvent;
	ApplicationTypeDef 		Appli_state;
	CDC_LOG_BUFF_STATE 		log_buf;
	char 					recv_buf[RECV_BUFF_SIZE + 1];
	volatile unsigned char	g_stop_flag;
	unsigned int 			rx_total_num;
	unsigned int 			tx_total_num;
	CDC_LineCodingTypeDef 	LineCoding;
	CDC_LineCodingTypeDef 	DefaultLineCoding;
	osThreadId				CH340_Send_Thread_id;
	osThreadId				LOG_Task_id;
}ch340_app;

static void Start_CH340_Application_Thread(void const *argument);

