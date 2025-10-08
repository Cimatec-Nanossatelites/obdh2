#include <stdint.h>

#define FLASH_INSTRUCTION_NORMAL_READ 0x03
#define FLASH_INSTRUCTION_PAGE_PROGRAM 0x02
#define FLASH_INSTRUCTION_SECTOR_ERASE 0xD7
#define FLASH_INSTRUCTION_32KB_BLOCK_ERASE 0x52
#define FLASH_INSTRUCTION_64KB_BLOCK_ERASE 0xD8
#define FLASH_INSTRUCTION_CHIP_ERASE 0xC7
#define FLASH_INSTRUCTION_WRITE_ENABLE 0x06
#define FLASH_INSTRUCTION_WRITE_DISABLE 0x04
#define FLASH_INSTRUCTION_WRITE_STATUS_REGISTER 0x01
#define FLASH_INSTRUCTION_READ_STATUS_REGISTER 0x05
#define FLASH_INSTRUCTION_READ_FUNCTION_REGISTER 0x48

#define FLASH_STATUS_BIT_WIP 0x01 /* Write in progress bit: 1 = Write cicle in progress <-> 0 = device is ready */

typedef enum {
  DEV_EXT_FLASH_OK = 0x00000000,
  DEV_EXT_FLASH_INIT_ERROR,
  DEV_EXT_FLASH_CMD_ERROR,
  DEV_EXT_FLASH_WRITE_ERROR,
  DEV_EXT_FLASH_READ_ERROR,
  DEV_EXT_FLASH_ERASE_ERROR
} dev_ext_flash_return_t;

int Dev_Mem_External_Flash_Init(void);

/**
 * @brief  Random read routine.
 * @param  Address      Memory physical address
 * @param  Data         Pointer to the area who will receive the read data.
 * @param  Size         Number of bytes to be read.
 * @retval Result : Result of Operation.
 *                  This parameter can be one of the following values:
 *                  @arg ANSWERED_REQUEST: All ok.
 *                  @arg Else: Some error happened.
 * @note
 */
dev_ext_flash_return_t Dev_Mem_External_Flash_Read(uint32_t Address, uint8_t *Data, uint32_t Size);


/**
 * @brief  Random read routine.
 * @param  Flash_Address    Start address to written
 * @param  Page_Data        Pointer to the data who will be written on the memory.
 * @param  Data_Size        Amount of bytes to be written on the page. Must less or equal than 256 bytes.
 * @retval Result : Result of Operation.
 *                  This parameter can be one of the following values:
 *                  @arg ANSWERED_REQUEST: All ok.
 *                  @arg Else: Some error happened.
 * @note This function allows the program up to 256 bytes of data to be programmed into memory
 * in a single write operation.
 */
dev_ext_flash_return_t Dev_Mem_External_Flash_Write(uint32_t Flash_Address, uint8_t *Flash_Data, uint32_t Data_Size);

/**
 * @brief Sector Erase routine.
 * @param  Sector_Address Sector number
 * @retval Result : Result of Operation.
 *         			This parameter can be one of the following values:
 *             		@arg ANSWERED_REQUEST: All ok.
 *             		@arg Else: Some error happened.
 * @note Erases a 4 Kbyte sector
 */
dev_ext_flash_return_t Dev_Mem_External_Flash_Erase_Sector(uint32_t Sector_Address);

