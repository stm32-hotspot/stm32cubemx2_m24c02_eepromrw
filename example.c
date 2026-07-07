/**
  ******************************************************************************
  * file           : example.c
  * brief          : example program body
  ******************************************************************************
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "example.h"
#include "m24c02.h"
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

#define SIZE256    256U  /* Size in bytes of test data buffers */
/* Private variables ---------------------------------------------------------*/
m24c02_object_t *pM24c020; /* Pointer to the M24C02 driver object instance */
/* Sample data for R/W */
uint8_t tx256[SIZE256] = "abcdefghE-EEPROM-Expansion Firmware library EEPROM driver example : This firmware provides "
                         "a basic example of how to use the X-Nucleo-eXpansion firmware library. This block of data is "
                         "specially written to test the data write function of EEPROM (SPI/I2C)  ";
uint8_t rx256[SIZE256] = {0};        /* Receive buffer for multi-byte read tests    */
uint8_t Transmit_Buff[256] = {0};    /* Temporary buffer used to test memory       */

/* Private function prototypes -----------------------------------------------*/
#if DEBUG_APPLI
#define PRINTF_APPLI(...) printf(__VA_ARGS__)  /* Debug printing enabled  */
#else
#define PRINTF_APPLI(...)                      /* Debug printing disabled */
#endif /* DEBUG_APPLI */

/**
  * ########## Step 1 ##########
  * The init of M24C02 is triggered by the application code
  */
app_status_t app_init(void)
{
  app_status_t return_status = EXEC_STATUS_ERROR;
  /* Get the M24C02 object pointer from board support layer */
  pM24c020 = MX_M24C02_getobject();
  /* Initialize the M24C02 driver instance 0 */
  if (m24c02_drv_init(pM24c020, MX_M24C02) != 0)
  {
    PRINTF("[ERROR] Step 1: M24C02 EEPROM init error\r\n");
    goto _app_init_exit;
  }
  PRINTF("[INFO] Step 1: M24C02 EEPROM init completed\r\n");

  /* Initialization completed successfully */
  return_status = EXEC_STATUS_INIT_OK;

_app_init_exit:
  /* Return application level init status */
  return return_status;

}

/**
  * ########## Step 2 ##########
  * Perform Read and Write operations.
  * The values are displayed on the terminal.
  * output: EXEC_STATUS_OK if OK, EXEC_STATUS_ERROR in case of error
  */
app_status_t app_process(void)
{

  app_status_t return_status = EXEC_STATUS_OK;
  app_status_t return_status_sb = EXEC_STATUS_UNKNOWN; /* Result of single byte test */
  app_status_t return_status_td = EXEC_STATUS_UNKNOWN; /* Result of full data test   */
  app_status_t return_status_tp = EXEC_STATUS_UNKNOWN; /* Result of page test        */

  /* Single byte access test */
  return_status_sb = EEPRMA2TestSingleByte();

  /* Full buffer read/write test */
  return_status_td = EEPRMA2TestData();

  /* Page oriented read/write test */
  return_status_tp = EEPRMA2TestPage();

  /* Aggregate results from all tests */
  if ((return_status_sb == EXEC_STATUS_OK)
      && (return_status_td == EXEC_STATUS_OK)
      && (return_status_tp == EXEC_STATUS_OK))
  {
    printf("\n\nAll test cases PASSED.\r\n");
    return_status = EXEC_STATUS_OK;
  }
  else
  {
    printf("\n\nTest cases FAILED.\r\n");
    return_status = EXEC_STATUS_ERROR;
  }

  /* Return global application test status */
  return return_status;
}

/** ########## Step 3 ##########
  * In this example, app_deinit is never called and is provided as a reference only.
  */
app_status_t app_deinit(void)
{
  /* Deinitialize M24C02 driver instance and related resources */
  if (m24c02_drv_deinit(pM24c020) != 0)
  {
    PRINTF("[ERROR] Step 3: EEPROM deinit error\r\n");
    return EXEC_STATUS_ERROR;
  }

  /* Deinitialization completed successfully */
  return EXEC_STATUS_OK;
}


/**
  * @brief  Test single byte data transfer.
  * @param  None
  * @retval app_status_t
  */
app_status_t EEPRMA2TestSingleByte(void)
{

  app_status_t ret_val = EXEC_STATUS_OK;

  PRINTF("\n\n***************************************************************\r\n");
  printf("                   -- I2C EEPROM TEST SINGLE BYTE-- \r\n");
  PRINTF("***************************************************************\r\n");

  uint8_t tx = 0x89;      /* Single byte to be written to EEPROM          */
  uint8_t rx = 0xFF;      /* Single byte read back from EEPROM            */
  uint16_t target_addr = 0x00; /* Target address for single byte access   */
  uint16_t nbyte = 1;     /* Number of bytes for read operation           */

  /* Read current value at target address before test */
  if (m24c02_drv_read_data_addr8(pM24c020, &rx, target_addr, nbyte) == 0)
  {
    printf("Read Memory Data : 0x%x at Address : 0x%x\r\n", rx, target_addr);
  }

  /* Write test byte to target address */
  int32_t w_ret = m24c02_drv_write_byte_addr8(pM24c020, &tx, target_addr);

  /* Read back the same address after write */
  int32_t r_ret = m24c02_drv_read_data_addr8(pM24c020, &rx, target_addr, nbyte);

  if ((w_ret == 0) && (r_ret == 0))
  {
    if (rx == tx)
    {
      printf("TestByte | Target: %s | Address: 0x%u | TX: 0x%x | RX: 0x%x | Result: PASSED \r\n",
             "M24C02", target_addr, tx, rx);
    }
    else
    {
      printf("TestByte | Target: %s | Address: %u | Result: FAILED \r\n", "M24C02", target_addr);
      ret_val = EXEC_STATUS_ERROR;
    }
  }
  else
  {
    printf("TestByte | Target: %s | Write or Read Operation FAILED \r\n", "M24C02");
    ret_val = EXEC_STATUS_ERROR;
  }

  /* Return outcome of single byte test */
  return ret_val;
}

/**
  * @brief  Test full buffer data transfer.
  * @param  None
  * @retval app_status_t
  */
app_status_t EEPRMA2TestData(void)
{

  app_status_t ret_val = EXEC_STATUS_OK;
  uint16_t idx;

  PRINTF("\n\n***************************************************************\r\n");
  printf("                   -- I2C EEPROM TEST DATA-- \r\n");
  PRINTF("***************************************************************\r\n");

  uint16_t target_addr = 0x00; /* Start address for multi-byte transfer */
  memset(rx256, 0x00, SIZE256);

  /* Dump memory contents before write operation */
  printf("\n\nMemory contents before write: \r\n");
  if (m24c02_drv_read_data_addr8(pM24c020, rx256, target_addr, SIZE256) == 0)
  {
    for (idx = 0; idx < SIZE256; idx++)
    {
      PRINTF("0x%x ", rx256[idx]);
    }
  }

  /* Write complete buffer in page sized chunks across address space */
  int32_t w_ret = m24c02_drv_write_data_addr8(pM24c020, tx256, target_addr, M24_PAGE_SIZE, SIZE256);

  /* Read back the same number of bytes starting at same address */
  int32_t r_ret = m24c02_drv_read_data_addr8(pM24c020, rx256, target_addr, SIZE256);

  if ((w_ret == 0) && (r_ret == 0))
  {
    printf("\n\nMemory contents after write: \r\n");
    /* Compare transmitted and received buffers byte by byte */
    for (idx = 0; idx < M24_MEMORY_SIZE; idx++)
    {
      if (tx256[idx] == rx256[idx])
      {
        PRINTF("%c", rx256[idx]);
      }
      else
      {
        break;
      }
    }

    if (idx == M24_MEMORY_SIZE)
    {
      printf("\nAll data to M24C02 written successfully!\r\n");
    }
    else
    {
      printf("Error in M24C02 write.\r\n");
      return EXEC_STATUS_ERROR;
    }

    /* Clear memory contents back to 0xFF for clean state */
    target_addr = 0;
    printf("\nReset memory to 0xFF from Address:0x%2.2X \r\n", target_addr);
    memset(Transmit_Buff, 0xFF, sizeof(Transmit_Buff));

    if (m24c02_drv_write_data_addr8(pM24c020, Transmit_Buff, target_addr, M24_PAGE_SIZE, SIZE256) != 0)
    {
      ret_val = EXEC_STATUS_ERROR;
    }
    else
    {
      /* Read back after clear and optionally dump contents */
      if (m24c02_drv_read_data_addr8(pM24c020, rx256, target_addr, SIZE256) == 0)
      {
        for (idx = 0; idx < SIZE256; idx++)
        {
          PRINTF("0x%x ", rx256[idx]);
        }
      }
      printf("\nMemory contents of M24C02 cleared to 0xFF \r\n");
    }

  }
  else
  {
    printf("M24C02 Test Memory Data: FAILED \r\n");
    ret_val = EXEC_STATUS_ERROR;
  }

  /* Return outcome of multi-byte data test */
  return ret_val;
}


/**
  * @brief  Test page oriented data transfer.
  * @param  None
  * @retval app_status_t
  */
app_status_t EEPRMA2TestPage(void)
{

  PRINTF("\n\n***************************************************************\r\n");
  printf("                   -- I2C EEPROM TEST PAGE-- \r\n");
  PRINTF("***************************************************************\r\n");

  app_status_t ret_val = EXEC_STATUS_OK;
  unsigned int idx;
  uint8_t tx[M24_PAGE_SIZE] = {0}; /* Transmit buffer for one page        */
  uint8_t rx[M24_PAGE_SIZE] = {0}; /* Receive buffer for one page         */
  unsigned int target_addr = 0x00;     /* Page start address for test         */
  uint16_t nbyte = M24_PAGE_SIZE;  /* Number of bytes for one page access */

  /* Prepare page buffer with constant pattern 0x45 */
  memset(tx, 0x45, M24_PAGE_SIZE);

  /* Write one page at target address */
  int32_t w_ret = m24c02_drv_write_page_addr8(pM24c020, tx, target_addr, nbyte);

  /* Read back same page into receive buffer */
  int32_t r_ret = m24c02_drv_read_page_addr8(pM24c020, rx, target_addr, nbyte);

  if ((w_ret == 0) && (r_ret == 0))
  {
    /* Compare page contents, intentionally loop beyond page for robustness check */
    for (idx = 0; idx < M24_MEMORY_SIZE; idx++)
    {
      if (tx[idx] == rx[idx])
      {
        PRINTF("0x%x ", rx[idx]);
      }
      else
      {
        break;
      }
    }

    if (idx == M24_PAGE_SIZE)
    {
      printf("\nAll data to M24C02 written successfully!\r\n");
    }
    else
    {
      printf("Error in M24C02 write.\r\n");
      return EXEC_STATUS_ERROR;
    }

    /* Clear tested page back to 0xFF */
    target_addr = 0;
    printf("\nReset memory to 0xFF from Address:0x%2.2X \r\n", target_addr);
    memset(tx, 0xFF, sizeof(tx));

    if (m24c02_drv_write_page_addr8(pM24c020, tx, target_addr, M24_PAGE_SIZE) != 0)
    {
      ret_val = EXEC_STATUS_ERROR;
    }
    else
    {
      printf("\nMemory contents of M24C02 cleared to 0xFF \r\n");
    }
  }
  else
  {
    printf("Test PAGE | Target: %s| Write or Read Operation FAILED \r\n", "M24C02");
    ret_val = EXEC_STATUS_ERROR;
  }

  /* Return outcome of page mode test */
  return ret_val;
}