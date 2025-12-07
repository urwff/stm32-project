# `printf` 重定向总结：`fputc` vs `_write`

本文档总结了在不同嵌入式编译器环境下重定向 `printf` 函数输出到串口的常用方法。

---

## 1. Keil MDK (ARMCC 编译器)

在 Keil MDK 环境中，C 库的 `printf` 函数最终会通过调用 `fputc` 来输出 **单个字符**。

- **需要重写的函数**: `fputc`
- **实现方式**: 在 `fputc` 函数内部，实现通过串口发送一个字节的逻辑。

### 示例代码:
```c
#include <stdio.h>
#include "stm32f1xx.h" // 根据项目包含正确的头文件

// 假设已有发送单字节的函数
void UART_Transmit_Byte(char byte); 

int fputc(int ch, FILE *f)
{
  UART_Transmit_Byte(ch); // 将字符发送到串口
  return ch;
}
```

---

## 2. GCC 工具链 (如 STM32CubeIDE, ARM-GCC)

在使用 GCC 和 newlib/newlib-nano C 库的环境中，`printf` 会将输出缓冲为 **一个数据块**，然后调用底层“系统调用 (syscall)” `_write` 来一次性发送。

- **需要重写的函数**: `_write`
- **实现方式**: 在 `_write` 函数内部，处理一个指向数据块的指针和其长度，通过串口将整个数据块发送出去。

### 示例代码:
```c
#include <sys/unistd.h> // for STDOUT_FILENO, STDERR_FILENO

// 假设已有发送字符串的函数
void UART_Transmit_String(const uint8_t* str, int len);

int _write(int file, char *ptr, int len)
{
  // 只处理标准输出 (stdout) 和标准错误 (stderr)
  if (file == STDOUT_FILENO || file == STDERR_FILENO)
  {
    // 调用串口发送函数
    UART_Transmit_String((uint8_t*)ptr, len);
    return len; // 返回成功写入的字节数
  }
  return -1; // 其他文件描述符返回错误
}
```

---

**结论**:
- **Keil**: 重写 `fputc`，处理单个字符。
- **GCC**: 重写 `_write`，处理字符串（数据块）。

对于当前这个基于 CMake 和 GCC 的项目，应采用重写 `_write` 的方法。

---

# ST-LINK 第二次烧录失败问题排查记录

**问题现象** (2025-12-06):  
第一次烧录成功后能正常运行，但第二次尝试烧录时 ST-LINK 报错：
```
Target no device found
Error in initializing ST-LINK device.
Reason: No device found on target.
```

**问题根因**:  
代码中 `can_driver.c` 的 `CAN_GPIO_Init()` 函数在配置 `AFIO->MAPR` 寄存器时，使用了不安全的位操作方式：
```c
// ❌ 错误写法 (第一版代码)
AFIO->MAPR |= AFIO_MAPR_CAN_REMAP_1;   // 直接 OR 操作
AFIO->MAPR &= ~AFIO_MAPR_CAN_REMAP_0;  // 直接 AND 操作
```

这种写法**可能意外修改 `AFIO->MAPR` 寄存器的其他位**，特别是 **SWJ_CFG[2:0]** (bit 26-24)，这些位控制着 **JTAG/SWD 调试接口的使能状态**。

**执行流程**:
1. 第一次烧录：代码还未执行，调试接口正常，烧录成功
2. 芯片上电运行：执行到 `CAN_GPIO_Init()` → 误修改 SWJ_CFG → SWD 被禁用
3. 第二次烧录：ST-LINK 无法找到调试接口 → 连接失败

---

## 解决方案

### ✅ 正确的寄存器修改方法：读-改-写 (Read-Modify-Write)

修改任何包含多个功能位的寄存器时，必须：
1. 读取当前寄存器值
2. 使用掩码清除目标位域
3. 设置新值
4. 写回寄存器

**正确代码**:
```c
// ✅ 修复后的代码
uint32_t mapr_temp = AFIO->MAPR;               // 1. 读取
mapr_temp &= ~AFIO_MAPR_CAN_REMAP;             // 2. 清除 CAN_REMAP 字段
mapr_temp |= AFIO_MAPR_CAN_REMAP_REMAP2;       // 3. 设置新值 (10 = PB8/PB9)
AFIO->MAPR = mapr_temp;                        // 4. 写回
```

这样可以**保证只修改 CAN_REMAP[1:0] 位域，不影响 SWJ_CFG 等其他配置位**。

---

## 应急恢复方法

如果芯片已经被锁死（SWD 已禁用），可以通过以下方法恢复：

### 方法 1: Connect Under Reset (推荐)
在 STM32CubeProgrammer 或 CubeIDE 调试配置中：
1. 勾选 **"Connect under reset"** 选项
2. 点击 Connect/Download
3. 在芯片复位瞬间建立连接（此时代码未执行）

### 方法 2: 手动按住复位
1. 按住开发板的 **RESET 按钮**
2. 在 STM32CubeProgrammer 中点击 **Connect**
3. 连接成功后再松开 RESET

### 方法 3: 完全擦除 Flash (慎用)
```powershell
STM32_Programmer_CLI.exe -c port=SWD mode=UR -e all
```
⚠️ 注意：会清除所有代码！

---

## 经验教训

1. **永远不要盲目使用 `|=` 和 `&=` 修改多功能寄存器**
2. **修改 `AFIO->MAPR` 等配置寄存器时，务必使用读-改-写模式**
3. **特别注意 SWJ_CFG 位域，禁用调试接口会导致芯片"变砖"**
4. **开发阶段建议默认使用 "Connect under reset" 模式进行烧录**

---

# ST-LINK 连接失败与救砖记录 (2025-12-07)

**问题现象**:
VSCode 调试时报错 `bound doStepConnectToTarget Failed: localhost:61234: Connection timed out`。
CubeProgrammer 连接报错 `Error: Unable to get core ID` 和 `Error: No STM32 target found`，即使电压识别正常 (3.36V)。

**原因分析**:
芯片处于由于代码错误（如看门狗复位死循环、错误配置 SWD 引脚、进入低功耗模式等）导致的“死锁”状态，或者是 SWD 接线接触不良。

**解决方案 (最终成功)**:
使用了 **全片擦除 (Mass Erase)** 恢复了芯片。

**操作步骤**:
1.  **BOOT0 跳线法 (本次使用)**:
    1.  拔下 **BOOT0** 跳线帽，插到 **3.3V (High)** 端。
    2.  按一下复位键 (此时芯片从系统区启动，不运行用户坏代码)。
    3.  连接调试器 (OpenOCD / CubeProgrammer) -> **连接成功**。
    4.  执行 **Full Chip Erase**。
    5.  **重要**: 擦除后将 BOOT0 插回 **GND (Low)**。

2.  **或者按住复位键法**:
    1.  在 CubeProgrammer 点击 Connect 的同时（或稍前），按住板子复位键。
    2.  见 Log 刚开始输出时迅速松开复位键。