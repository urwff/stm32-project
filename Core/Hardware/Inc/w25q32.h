#ifndef __W25Q32_H
#define __W25Q32_H

#include <stdint.h>

//======================================================================
//                          Typedefs and Enums
//======================================================================

/**
 * @brief  W25Q32 Status/Error Codes
 */
typedef enum {
    W25Q32_OK = 0,
    W25Q32_ERROR = 1,
    W25Q32_BUSY = 2,
    W25Q32_TIMEOUT = 3,
    W25Q32_INVALID_PARAM = 4,
    W25Q32_CHIP_NOT_FOUND = 5
} W25Q32_Status_t;

/**
 * @brief  W25Q32 State and Identification Struct
 */
typedef struct {
    uint8_t  manufacturer_id;
    uint16_t jedec_id;
    uint8_t  device_id;
    uint64_t unique_id;
    
    // Calculated properties
    uint32_t page_count;
    uint32_t sector_count;
    uint32_t block_64k_count;
} W25Q32_State_t;


//======================================================================
//                         Constant Definitions
//======================================================================

// --- Memory Size ---
#define W25Q32_PAGE_SIZE             256
#define W25Q32_SECTOR_SIZE           4096  // 4KB
#define W25Q32_BLOCK_64K_SIZE        65536 // 64KB
#define W25Q32_TOTAL_SIZE_BYTES      4194304 // 4MB

// --- W25Q32 Command Opcodes ---
#define W25Q32_CMD_WRITE_ENABLE          0x06
#define W25Q32_CMD_WRITE_DISABLE         0x04
#define W25Q32_CMD_READ_STATUS_REG1      0x05
#define W25Q32_CMD_PAGE_PROGRAM          0x02
#define W25Q32_CMD_SECTOR_ERASE_4KB      0x20
#define W25Q32_CMD_BLOCK_ERASE_64KB      0xD8
#define W25Q32_CMD_CHIP_ERASE            0xC7
#define W25Q32_CMD_READ_DATA             0x03
#define W25Q32_CMD_JEDEC_ID              0x9F
#define W25Q32_CMD_READ_UNIQUE_ID        0x4B
#define W25Q32_CMD_POWER_DOWN            0xB9
#define W25Q32_CMD_RELEASE_POWER_DOWN    0xAB

// --- Status Register 1 Bits ---
#define W25Q32_SR1_BUSY_BIT              0x01 // Erase/Write In Progress

// --- Expected JEDEC ID ---
#define W25Q32_EXPECTED_MANUFACTURER_ID  0xEF
#define W25Q32_EXPECTED_JEDEC_ID_PART    0x4016 // Memory Type + Capacity


//======================================================================
//                        Public Function Prototypes
//======================================================================

/**
 * @brief  Initializes the W25Q32 chip and populates the state structure.
 * @param  state: Pointer to a W25Q32_State_t structure to hold chip info.
 * @return W25Q32_Status_t status code.
 */
W25Q32_Status_t W25Q32_Init(W25Q32_State_t *state);

/**
 * @brief  Erases the entire chip.
 * @note   This can take a significant amount of time.
 * @return W25Q32_Status_t status code.
 */
W25Q32_Status_t W25Q32_ChipErase(void);

/**
 * @brief  Erases a 4KB sector.
 * @param  sector_num: The sector number to erase (0 to 1023).
 * @return W25Q32_Status_t status code.
 */
W25Q32_Status_t W25Q32_SectorErase_4KB(uint32_t sector_num);

/**
 * @brief  Erases a 64KB block.
 * @param  block_num: The block number to erase (0 to 63).
 * @return W25Q32_Status_t status code.
 */
W25Q32_Status_t W25Q32_BlockErase_64KB(uint32_t block_num);

/**
 * @brief  Writes data to a page.
 * @param  page_num: The page number to write to (0 to 16383).
 * @param  offset_in_page: Start offset within the page (0-255).
 * @param  data: Pointer to the data to write.
 * @param  size: Number of bytes to write.
 * @return W25Q32_Status_t status code.
 */
W25Q32_Status_t W25Q32_PageProgram(uint32_t page_num, uint16_t offset_in_page, uint8_t *data, uint32_t size);

/**
 * @brief  Reads data from the flash memory.
 * @param  address: The 24-bit starting address to read from.
 * @param  data: Pointer to the buffer to store read data.
 * @param  size: Number of bytes to read.
 * @return W25Q32_Status_t status code.
 */
W25Q32_Status_t W25Q32_ReadData(uint32_t address, uint8_t *data, uint32_t size);

/**
 * @brief  Puts the device in power-down mode.
 */
void W25Q32_PowerDown(void);

/**
 * @brief  Wakes the device from power-down mode.
 */
void W25Q32_ReleasePowerDown(void);

#endif // __W25Q32_PRO_H
