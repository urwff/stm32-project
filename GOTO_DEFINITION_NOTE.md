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