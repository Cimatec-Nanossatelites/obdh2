#include "spi_flash.h"
#include <msp430.h>
#include <config/config.h>
#include <drivers/spi/spi.h>
#include <stdint.h>

#define FLASH_SPI_PORT          SPI_PORT_0
#define FLASH_SPI_MODE          SPI_MODE_0
#define FLASH_SPI_CLK_HZ        CONFIG_SPI_PORT_0_SPEED_BPS
#define FLASH_SPI_CS_PIN        SPI_CS_2


dev_ext_flash_return_t Dev_Mem_External_Flash_Read_Status(uint8_t *Status);
dev_ext_flash_return_t Dev_Mem_External_Flash_Enable_Write(bool enable_disable);


int Dev_Mem_External_Flash_Init(void)
{
    spi_config_t conf = {0};

    conf.speed_hz   = FLASH_SPI_CLK_HZ;
    conf.mode       = FLASH_SPI_MODE;

    return spi_init(FLASH_SPI_PORT, conf);
}


dev_ext_flash_return_t Dev_Mem_External_Flash_Read(uint32_t Address, uint8_t *Data, uint32_t Size) {
  uint8_t Memory_Status;
  uint8_t TXData[4];


  TXData[0] = FLASH_INSTRUCTION_NORMAL_READ;
  TXData[1] = Address >> 16 & 0xFF;
  TXData[2] = Address >> 8 & 0xFF;
  TXData[3] = Address & 0xFF;

  spi_select_slave(FLASH_SPI_PORT, FLASH_SPI_CS_PIN, true);
  
  if(spi_write(FLASH_SPI_PORT, SPI_CS_NONE, TXData, 4) != 0)
    return DEV_EXT_FLASH_CMD_ERROR;

  if(spi_read(FLASH_SPI_PORT, SPI_CS_NONE, Data, Size) != 0)
    return DEV_EXT_FLASH_READ_ERROR;

  spi_select_slave(FLASH_SPI_PORT, FLASH_SPI_CS_PIN, false);

  return DEV_EXT_FLASH_OK;
}

dev_ext_flash_return_t Dev_Mem_External_Flash_Write(uint8_t ID, uint32_t Flash_Address, uint8_t *Flash_Data, uint32_t Data_Size) {
  dev_ext_flash_return_t MEM_RetCode;
  uint8_t Memory_Status;
  uint8_t TXData[4];
  // uint16_t Wait_Write_Counter;

  do {
    Dev_Mem_External_Flash_Read_Status(&Memory_Status);  // Blocking Function
  } while (Memory_Status & FLASH_STATUS_BIT_WIP);
  // goto return_error;

  
  if (Dev_Mem_External_Flash_Enable_Write(true) != DEV_EXT_FLASH_OK)
    return DEV_EXT_FLASH_WRITE_ERROR;

  // Calculate the physical sector address
  TXData[0] = FLASH_INSTRUCTION_PAGE_PROGRAM;
  TXData[1] = Flash_Address >> 16 & 0xFF;
  TXData[2] = Flash_Address >> 8 & 0xFF;
  TXData[3] = Flash_Address & 0xFF;
  //memcpy(TXData + 4, Flash_Data, Data_Size);
  
  spi_select_slave(FLASH_SPI_PORT, FLASH_SPI_CS_PIN, true);
  
  if(spi_write(FLASH_SPI_PORT, SPI_CS_NONE, TXData, 4) != 0)
    return DEV_EXT_FLASH_WRITE_ERROR;

  if(spi_write(FLASH_SPI_PORT, SPI_CS_NONE, Flash_Data, Data_Size) != 0)
    return DEV_EXT_FLASH_WRITE_ERROR;

  spi_select_slave(FLASH_SPI_PORT, FLASH_SPI_CS_PIN, false);

  return DEV_EXT_FLASH_OK;
}


dev_ext_flash_return_t Dev_Mem_External_Flash_Erase_Sector(uint8_t ID, uint32_t Sector_Address) {
  dev_ext_flash_return_t MEM_RetCode;
  uint32_t Sector_Physical_Address;
  uint8_t Memory_Status;
  uint8_t TXData[4];  

  do {
    Dev_Mem_External_Flash_Read_Status(&Memory_Status);  // Blocking Function
  } while (Memory_Status & FLASH_STATUS_BIT_WIP);
  

  if (Dev_Mem_External_Flash_Enable_Write(true) != DEV_EXT_FLASH_OK)
    return DEV_EXT_FLASH_WRITE_ERROR;


  // Calculate the physical sector address
  Sector_Physical_Address = Sector_Address;
  TXData[0] = FLASH_INSTRUCTION_SECTOR_ERASE;
  TXData[1] = Sector_Physical_Address >> 16 & 0xFF;
  TXData[2] = Sector_Physical_Address >> 8;
  TXData[3] = Sector_Physical_Address & 0xFF;

  if(spi_write(FLASH_SPI_PORT, FLASH_SPI_CS_PIN, TXData, 4) != 0)
    return DEV_EXT_FLASH_CMD_ERROR;
 
  return DEV_EXT_FLASH_OK;
}

dev_ext_flash_return_t Dev_Mem_External_Flash_Read_Status(uint8_t *Status) {
    uint8_t cmd[2] = {FLASH_INSTRUCTION_READ_STATUS_REGISTER, 0x00};
    uint8_t ans[2] = {0};

    if (spi_transfer(FLASH_SPI_PORT, FLASH_SPI_CS_PIN, cmd, ans, 2) == 0)
    {
       *Status = ans[1];
        return DEV_EXT_FLASH_OK;
    }

    return DEV_EXT_FLASH_READ_ERROR;
}

dev_ext_flash_return_t Dev_Mem_External_Flash_Enable_Write(bool enable_disable) {
    uint8_t cmd = FLASH_INSTRUCTION_WRITE_ENABLE;

    if (spi_write(FLASH_SPI_PORT, FLASH_SPI_CS_PIN, &cmd, 1) == 0)
    {
        return DEV_EXT_FLASH_OK;
    }
    return DEV_EXT_FLASH_CMD_ERROR;
}