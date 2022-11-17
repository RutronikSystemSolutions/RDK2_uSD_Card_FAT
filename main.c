/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the RutDevKit-uSD_Card_FAT
*              Application for ModusToolbox.
*
* Related Document: See README.md
*
*
*  Created on: 2022-10-28
*  Company: Rutronik Elektronische Bauelemente GmbH
*  Address: Jonavos g. 30, Kaunas 44262, Lithuania
*  Author: GDR
*
*******************************************************************************
* (c) 2019-2021, Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software"), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*
* Rutronik Elektronische Bauelemente GmbH Disclaimer: The evaluation board
* including the software is for testing purposes only and,
* because it has limited functions and limited resilience, is not suitable
* for permanent use under real conditions. If the evaluation board is
* nevertheless used under real conditions, this is done at one’s responsibility;
* any liability of Rutronik is insofar excluded
*******************************************************************************/

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "stdlib.h"
#include "cy_retarget_io.h"
#include "ff.h"
#include "diskio.h"

#define ALLOC_SIZE 0x10000

void handle_error(void);

FATFS  SDFatFS;
TCHAR SDPath = DEV_SD;

int main(void)
{
    cy_rslt_t result;
    FRESULT fs_result;
    char *buffer = NULL, *memory = NULL;
    FIL SDFile;       /* File object for SD */

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS)
    {
    	handle_error();
    }

    __enable_irq();

    /*Initialize LEDs*/
    result = cyhal_gpio_init( LED1, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    if (result != CY_RSLT_SUCCESS)
    {handle_error();}
    result = cyhal_gpio_init( LED2, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);
    if (result != CY_RSLT_SUCCESS)
    {handle_error();}

    /*Enable debug output via KitProg UART*/
    result = cy_retarget_io_init( KITPROG_TX, KITPROG_RX, CY_RETARGET_IO_BAUDRATE);
    if (result != CY_RSLT_SUCCESS)
    {
        handle_error();
    }
    printf("\x1b[2J\x1b[;H");
    printf("RDK2 uSD Card with the FAT File System Example has Started.\r\n");

    /*Mount the media and write with a new test file*/
    fs_result = f_mount(&SDFatFS, &SDPath, 1);
    if (fs_result == FR_OK)
    {
    	cyhal_gpio_write(LED1, CYBSP_LED_STATE_ON);
        printf("FAT File System: Media mounted successfully.\r\n");

    	/*Allocate memory buffer*/
    	buffer = (char*)malloc(ALLOC_SIZE);
    	if(buffer == NULL)
    	{
    	    printf("Dynamic memory allocation failure.\r\n");
    	    handle_error();
    	}

        fs_result = f_unlink("TEST.TXT");
        if ((fs_result == FR_NO_FILE) || (fs_result == FR_OK) )
        {
        	fs_result = f_open(&SDFile, "TEST.TXT", FA_WRITE | FA_CREATE_ALWAYS);
        	if (fs_result == FR_OK)
        	{
        		memset(buffer, 'A', ALLOC_SIZE);
        		UINT bw;
        		f_write(&SDFile, buffer, ALLOC_SIZE, &bw);
        		f_close(&SDFile);
        	}
        	else
        	{
        		printf("Could not create file.\r\n");
        		handle_error();
        	}
            printf("A file \"TEST.TXT\" for the test has been created.\r\n");
         }
        else
        {
          printf("Error deleting file.\r\n");
          handle_error();
        }
    }
    else
    {
    	printf("Could not mount media.\r\n");
    	handle_error();
    }

    printf("A file \"TEST.TXT\" will be read indefinitely from now.\r\n");
    printf("The test will stop in case of error.\r\n");

    for (;;)
    {
    	/*Open and read test file*/
    	cyhal_gpio_write(LED1, CYBSP_LED_STATE_ON);
        fs_result = f_open(&SDFile, "TEST.TXT", FA_READ);
        {
          if(fs_result == FR_OK)
          {
            UINT br = 0;
            memset(buffer, 0x00, ALLOC_SIZE);
            f_read(&SDFile, buffer, ALLOC_SIZE, &br);
            f_close(&SDFile);
            if((br==0) || (br<ALLOC_SIZE))
            {
              printf("Could not read file.\r\n");
              handle_error();
            }
          }
          else
          {
            printf("Could not open file.\r\n");
            handle_error();
          }
        }

        /*Check the data in the memory*/
        memory = buffer;
        for(uint32_t i = 0; i < ALLOC_SIZE; i++)
        {
          if(*memory != 'A')
          {
            printf("Memory read error.\r\n");
            handle_error();
          }
          memory++;
        }
        cyhal_gpio_write(LED1, CYBSP_LED_STATE_OFF);
    }
}

void handle_error(void)
{
     /* Disable all interrupts. */
    __disable_irq();
    cyhal_gpio_write(LED1, CYBSP_LED_STATE_OFF);
    cyhal_gpio_write(LED2, CYBSP_LED_STATE_ON);
    CY_ASSERT(0);
}


/* [] END OF FILE */
