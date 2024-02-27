/*
 * HC_05_UART_USB.h
 *
 *  Created on: Jan 14, 2024
 *      Author: Adam Iwachow
 *
 *
 *     Poor usb-uart bridge
 */

#ifndef INC_HC_05_UART_USB_H_
#define INC_HC_05_UART_USB_H_

#include "usart.h"

#define LINE_MAX_LENGTH	 64

static char line_buffer[LINE_MAX_LENGTH + 2];
static uint32_t line_length;
uint8_t uart_rx_buffer;

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if(huart == &huart2){

		 if (uart_rx_buffer == '\n' || uart_rx_buffer == '\r'){

		    if (line_length > 0) {

		    line_buffer[line_length] = uart_rx_buffer;
		    line_buffer[line_length + 1] = '\n';
		    line_buffer[line_length + 2] = '\0';
		    HAL_UART_Transmit(&huart1, (uint8_t*)line_buffer, strlen(line_buffer), HAL_MAX_DELAY);


		      line_length = 0;
		    }
		 }
		  else {
		    if (line_length >= LINE_MAX_LENGTH) {

		      line_length = 0;
		    }
		    line_buffer[line_length++] = uart_rx_buffer;
		  }


	}

	if(huart == &huart1)
	{
		 if (uart_rx_buffer == '\r' || uart_rx_buffer == '\n') {
			    if (line_length > 0) {

			    	line_buffer[line_length] = uart_rx_buffer;
			    	line_buffer[line_length + 1] = '\n';
			    	line_buffer[line_length + 2] = '\0';
			    	HAL_UART_Transmit(&huart2, (uint8_t*)line_buffer, strlen(line_buffer), HAL_MAX_DELAY);

			      line_length = 0;
			    }
			  }
			  else {
			    if (line_length >= LINE_MAX_LENGTH) {

			      line_length = 0;
			    }
			    line_buffer[line_length++] = uart_rx_buffer;
			  }

	}

	HAL_UARTEx_ReceiveToIdle_IT(&huart2, &uart_rx_buffer, 1);
	HAL_UARTEx_ReceiveToIdle_IT(&huart1, &uart_rx_buffer, 1);
}

void INIT_USB_UART_BRIDGE(void)
{

	HAL_UARTEx_ReceiveToIdle_IT(&huart2, &uart_rx_buffer, 1);
	HAL_UARTEx_ReceiveToIdle_IT(&huart1, &uart_rx_buffer, 1);

}

#endif /* INC_HC_05_UART_USB_H_ */
