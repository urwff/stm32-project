/**
 * @file w25q32_pro.c
 * @brief 专业级的W25Q32 Flash驱动程序源文件。
 * @version 1.1
 * @date 2025-11-29
 *
 * @note
 * 本驱动程序设计思想:
 * 1. 健壮性: 通过超时机制和参数校验，避免程序卡死或因非法参数导致硬件错误。
 * 2. 易用性: 封装底层操作细节 (如写使能、等待)，提供简洁的API，并通过返回状态码提供明确反馈。
 * 3. 可移植性: 将硬件相关的SPI操作抽象为几个静态函数，方便移植到不同平台 (HAL, LL, 或其他)。
 * 4. 状态管理: 使用W25Q32_State_t结构体来管理设备状态，初始化后可快速访问设备信息。
 */

#include "w25q32.h"
#include "spi.h" // 包含项目自定义的SPI头文件

//======================================================================
//        硬件适配层: SPI底层功能实现
//======================================================================

/**
 * @brief  片选使能 (拉低CS引脚)。
 * @note   这是适配层的一部分，将驱动的内部逻辑与具体硬件操作解耦。
 */
static void SPI_CS_Select(void) {
    Hal_SPI_Start(); // 映射到项目中的 Hal_SPI_Start() 函数，该函数应负责拉低CS线。
}

/**
 * @brief  片选失能 (拉高CS引脚)。
 * @note   这是适配层的一部分，将驱动的内部逻辑与具体硬件操作解耦。
 */
static void SPI_CS_Deselect(void) {
    Hal_SPI_Stop(); // 映射到项目中的 Hal_SPI_Stop() 函数，该函数应负责拉高CS线。
}

/**
 * @brief  通过SPI总线发送并接收一个字节。
 * @param  byte 要发送的字节。
 * @return uint8_t 从设备接收到的字节。
 * @note   这是适配层的一部分，SPI是全双工通信，发送一个字节的同时必然会接收一个字节。
 */
static uint8_t SPI_TransmitReceive(uint8_t byte) {
    return Hal_SPI_SwapByte(byte); // 映射到项目中的 Hal_SPI_SwapByte() 函数。
}


//======================================================================
//                内部辅助函数的声明 (Private Helper Prototypes)
//======================================================================

// 发送“写使能”指令
static void W25Q32_WriteEnable(void);
// 发送“写失能”指令
static void W25Q32_WriteDisable(void);
// 读取状态寄存器1
static uint8_t W25Q32_ReadStatusRegister1(void);
// 等待Flash内部操作完成，防止在擦写过程中执行新指令
static W25Q32_Status_t W25Q32_WaitForWriteEnd(void);


//======================================================================
//                 公共API函数的实现 (Public API Implementations)
//======================================================================

/**
 * @brief  初始化W25Q32芯片，读取并验证ID，填充状态结构体。
 * @param  state 指向W25Q32状态结构体的指针，用于存储芯片信息。
 * @return W25Q32_Status_t 操作状态码 (W25Q32_OK 表示成功)。
 */
W25Q32_Status_t W25Q32_Init(W25Q32_State_t *state) {
    // 1. 检查传入的指针是否有效，防止空指针引起程序崩溃。
    if (state == 0) {
        return W25Q32_INVALID_PARAM;
    }

    // 2. SPI硬件本身的初始化 (SPI_Init()) 应该在调用此函数前完成。
    //    先将CS线拉高，确保芯片处于非选中状态。
    SPI_CS_Deselect();
    
    // 3. 发送“从掉电模式唤醒”指令，这是一个好习惯，可确保芯片处于可操作状态。
    W25Q32_ReleasePowerDown();

    // 4. 读取 JEDEC ID (制造商ID, 设备ID)。
    SPI_CS_Select(); // 片选使能
    SPI_TransmitReceive(W25Q32_CMD_JEDEC_ID);             // 发送 0x9F 读JEDEC ID指令。
    state->manufacturer_id = SPI_TransmitReceive(0xFF);    // 发送一个虚拟字节，以接收制造商ID。
    uint8_t memory_type = SPI_TransmitReceive(0xFF);       // 接收存储器类型。
    uint8_t capacity = SPI_TransmitReceive(0xFF);            // 接收容量信息。
    SPI_CS_Deselect(); // 片选失能

    // 5. 组合ID信息。
    state->jedec_id = (memory_type << 8) | capacity;
    state->device_id = capacity;

    // 6. 校验芯片型号是否为W25Q32。
    if (state->manufacturer_id != W25Q32_EXPECTED_MANUFACTURER_ID || state->jedec_id != W25Q32_EXPECTED_JEDEC_ID_PART) {
        return W25Q32_CHIP_NOT_FOUND; // 如果ID不匹配，返回错误。
    }
    
    // 7. 根据芯片总容量，计算页、扇区、块的数量，方便上层应用使用。
    state->page_count = W25Q32_TOTAL_SIZE_BYTES / W25Q32_PAGE_SIZE;
    state->sector_count = W25Q32_TOTAL_SIZE_BYTES / W25Q32_SECTOR_SIZE;
    state->block_64k_count = W25Q32_TOTAL_SIZE_BYTES / W25Q32_BLOCK_64K_SIZE;

    // 8. 读取芯片的64位唯一ID (可选，但对于产品追溯等很有用)。
    SPI_CS_Select();
    SPI_TransmitReceive(W25Q32_CMD_READ_UNIQUE_ID); // 发送 0x4B 读唯一ID指令。
    for (uint8_t i = 0; i < 4; i++) { // 根据数据手册，需要先发送4个虚拟字节。
        SPI_TransmitReceive(0xFF);
    }
    // 循环8次，接收8个字节的唯一ID。
    state->unique_id = 0;
    for (uint8_t i = 0; i < 8; i++) {
        state->unique_id <<= 8; // 左移8位，为下一个字节腾出空间。
        state->unique_id |= SPI_TransmitReceive(0xFF); // 读入一个字节并拼接到unique_id中。
    }
    SPI_CS_Deselect();

    // 9. 所有步骤完成，返回成功。
    return W25Q32_OK;
}

/**
 * @brief  擦除整个芯片。
 * @warning 这个操作非常耗时(可能长达数十秒)，会阻塞程序，请谨慎使用。
 * @return W25Q32_Status_t 操作状态码。
 */
W25Q32_Status_t W25Q32_ChipErase(void) {
    if (W25Q32_WaitForWriteEnd() != W25Q32_OK) return W25Q32_TIMEOUT; // 等待芯片空闲
    W25Q32_WriteEnable(); // 使能写入
    SPI_CS_Select();
    SPI_TransmitReceive(W25Q32_CMD_CHIP_ERASE); // 发送 0xC7 整片擦除指令
    SPI_CS_Deselect();
    return W25Q32_WaitForWriteEnd(); // 等待擦除完成，这会花费很长时间
}

/**
 * @brief  擦除一个4KB的扇区。
 * @param  sector_num 要擦除的扇区号 (对于W25Q32是 0-1023)。
 * @return W25Q32_Status_t 操作状态码。
 */
W25Q32_Status_t W25Q32_SectorErase_4KB(uint32_t sector_num) {
    // 1. 校验扇区号是否在有效范围内。
    if (sector_num >= (W25Q32_TOTAL_SIZE_BYTES / W25Q32_SECTOR_SIZE)) {
        return W25Q32_INVALID_PARAM;
    }
    // 2. 等待上一个操作完成。
    if (W25Q32_WaitForWriteEnd() != W25Q32_OK) return W25Q32_TIMEOUT;
    // 3. 使能写入。
    W25Q32_WriteEnable();

    // 4. 计算目标地址 (扇区号 * 4096)。
    uint32_t address = sector_num * W25Q32_SECTOR_SIZE;
    
    // 5. 发送指令和地址。
    SPI_CS_Select();
    SPI_TransmitReceive(W25Q32_CMD_SECTOR_ERASE_4KB);   // 发送 0x20 扇区擦除指令。
    SPI_TransmitReceive((address >> 16) & 0xFF);     // 发送地址的高8位。
    SPI_TransmitReceive((address >> 8) & 0xFF);      // 发送地址的中8位。
    SPI_TransmitReceive(address & 0xFF);             // 发送地址的低8位。
    SPI_CS_Deselect();
    
    // 6. 等待本次擦除操作完成。
    return W25Q32_WaitForWriteEnd();
}

/**
 * @brief  页编程 (向一个页写入数据)。页是W25Q32最小的编程单位。
 * @param  page_num       要写入的页号 (0-16383)。
 * @param  offset_in_page 页内偏移地址 (0-255)，即从页的第几个字节开始写。
 * @param  data           指向要写入数据的指针。
 * @param  size           要写入的字节数。
 * @return W25Q32_Status_t 操作状态码。
 */
W25Q32_Status_t W25Q32_PageProgram(uint32_t page_num, uint16_t offset_in_page, uint8_t *data, uint32_t size) {
    // 1. 校验所有参数是否合法。
    if (page_num >= (W25Q32_TOTAL_SIZE_BYTES / W25Q32_PAGE_SIZE) || offset_in_page >= W25Q32_PAGE_SIZE || data == 0) {
        return W25Q32_INVALID_PARAM;
    }
    // 2. 如果要写入的数据长度超过了当前页的剩余空间，则自动截断，防止写到下一页。
    if (size > W25Q32_PAGE_SIZE - offset_in_page) {
        size = W25Q32_PAGE_SIZE - offset_in_page;
    }
    // 3. 如果要写入的长度为0，直接返回成功。
    if (size == 0) {
        return W25Q32_OK;
    }

    // 4. 等待空闲并使能写入。
    if (W25Q32_WaitForWriteEnd() != W25Q32_OK) return W25Q32_TIMEOUT;
    W25Q32_WriteEnable();

    // 5. 计算绝对物理地址。
    uint32_t address = (page_num * W25Q32_PAGE_SIZE) + offset_in_page;
    
    // 6. 发送指令和地址。
    SPI_CS_Select();
    SPI_TransmitReceive(W25Q32_CMD_PAGE_PROGRAM);    // 发送 0x02 页编程指令。
    SPI_TransmitReceive((address >> 16) & 0xFF);   // 发送地址的高8位。
    SPI_TransmitReceive((address >> 8) & 0xFF);    // 发送地址的中8位。
    SPI_TransmitReceive(address & 0xFF);           // 发送地址的低8位。

    // 7. 循环发送要写入的数据。
    for (uint32_t i = 0; i < size; i++) {
        SPI_TransmitReceive(data[i]);
    }

    SPI_CS_Deselect();
    
    // 8. 等待写入操作完成。
    return W25Q32_WaitForWriteEnd();
}

/**
 * @brief  从Flash的任意地址读取任意长度的数据。
 * @param  address 要读取的24位起始地址。
 * @param  data    指向存储读取数据的缓冲区的指针。
 * @param  size    要读取的字节数。
 * @return W25Q32_Status_t 操作状态码。
 */
W25Q32_Status_t W25Q32_ReadData(uint32_t address, uint8_t *data, uint32_t size) {
    // 1. 参数校验。
    if (address + size > W25Q32_TOTAL_SIZE_BYTES || data == 0) {
        return W25Q32_INVALID_PARAM;
    }
    if (size == 0) {
        return W25Q32_OK;
    }

    // 2. 等待芯片空闲。
    if (W25Q32_WaitForWriteEnd() != W25Q32_OK) return W25Q32_TIMEOUT;

    // 3. 发送指令和地址。
    SPI_CS_Select();
    SPI_TransmitReceive(W25Q32_CMD_READ_DATA);     // 发送 0x03 读数据指令。
    SPI_TransmitReceive((address >> 16) & 0xFF); // 发送地址的高8位。
    SPI_TransmitReceive((address >> 8) & 0xFF);  // 发送地址的中8位。
    SPI_TransmitReceive(address & 0xFF);         // 发送地址的低8位。

    // 4. 循环接收数据。
    for (uint32_t i = 0; i < size; i++) {
        data[i] = SPI_TransmitReceive(0xFF); // 接收数据时，主机需要持续发送时钟，所以我们发送虚拟(dummy)字节0xFF。
    }
    SPI_CS_Deselect();
    
    return W25Q32_OK;
}


//======================================================================
//                 内部辅助函数的实现 (Private Helper Implementations)
//======================================================================

/**
 * @brief  发送“写使能”指令 (0x06)。
 * @note   在每次擦除或写入操作之前，都必须先发送此指令。
 */
static void W25Q32_WriteEnable(void) {
    SPI_CS_Select();
    SPI_TransmitReceive(W25Q32_CMD_WRITE_ENABLE);
    SPI_CS_Deselect();
}

/**
 * @brief  发送“写失能”指令 (0x04)。
 * @note   通常在写操作完成后调用，以保护Flash数据不被意外改写。
 */
static void W25Q32_WriteDisable(void) {
    SPI_CS_Select();
    SPI_TransmitReceive(W25Q32_CMD_WRITE_DISABLE);
    SPI_CS_Deselect();
}

/**
 * @brief  读取状态寄存器1的值。
 * @return uint8_t: 状态寄存器1 (SR1) 的值。
 */
static uint8_t W25Q32_ReadStatusRegister1(void) {
    uint8_t reg_val;
    SPI_CS_Select();
    SPI_TransmitReceive(W25Q32_CMD_READ_STATUS_REG1); // 发送 0x05 读状态寄存器指令。
    reg_val = SPI_TransmitReceive(0xFF);             // 发送虚拟字节，接收寄存器的值。
    SPI_CS_Deselect();
    return reg_val;
}

/**
 * @brief  等待Flash内部操作完成 (通过轮询状态寄存器的BUSY位)。
 * @return W25Q32_Status_t 操作状态码 (W25Q32_OK 或 W25Q32_TIMEOUT)。
 */
static W25Q32_Status_t W25Q32_WaitForWriteEnd(void) {
    // 定义一个超时计数器。volatile关键字防止编译器优化掉这个变量。
    // 这个值不需要精确，只要足够大，能覆盖最耗时的操作(如扇区擦除)即可。
    volatile uint32_t timeout = 4000000;

    // 循环查询状态寄存器1的第0位 (BUSY位)。
    // 只要BUSY位为1，就表示芯片正在进行内部擦写操作。
    while ((W25Q32_ReadStatusRegister1() & W25Q32_SR1_BUSY_BIT)) {
        // 每次循环都将超时计数器减1。
        if (--timeout == 0) {
            return W25Q32_TIMEOUT; // 如果计数器减到0，芯片仍然繁忙，则返回超时错误。
        }
    }
    return W25Q32_OK; // BUSY位变为0，芯片空闲，返回成功。
}

/**
 * @brief  将设备置于掉电模式以降低功耗。
 * @note   掉电模式下，大部分功能被禁用，功耗降至最低。
 */
void W25Q32_PowerDown(void) {
    SPI_CS_Select();
    SPI_TransmitReceive(W25Q32_CMD_POWER_DOWN);
    SPI_CS_Deselect();

    // 等待一小段时间确保芯片进入掉电模式
    // 根据数据手册，通常需要几微秒时间
    volatile uint32_t delay = 1000;
    while (delay--) {
        __NOP();
    }
}

/**
 * @brief  从掉电模式唤醒设备。
 * @note   唤醒后需要一定的延时才能进行正常操作。
 */
void W25Q32_ReleasePowerDown(void) {
    SPI_CS_Select();
    SPI_TransmitReceive(W25Q32_CMD_RELEASE_POWER_DOWN);
    SPI_CS_Deselect();

    // 根据数据手册，唤醒需要一定的延时
    // 这里使用简单的软件延时，实际应用中可以用HAL_Delay
    volatile uint32_t delay = 10000;
    while (delay--) {
        __NOP();
    }
}
