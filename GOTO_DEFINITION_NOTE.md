# VS Code 无法跳转函数的原因

## 背景
- 在 `Core/Src/w24c02.c` 中 Ctrl+点击 `Driver_I2C_*` 等符号无法跳转到定义。
- VS Code 的 C/C++/clangd 后端依赖 `build/Debug/compile_commands.json` 描述所有源码的编译方式来提供跳转。

## 根因
- 项目 `CMakeLists.txt` 的 `target_sources` 里意外传入了目录 `Core/Src`，CMake 会忽略它，只保留显式列出的少量 `.c` 文件（`main.c`、`my_usart.c`、`w24c02.c` 等）。
- 由于 `my_i2c.c`、`gpio.c` 等绝大部分源码未参与目标构建，它们没有出现在 `compile_commands.json`；clangd 缺少编译指令，自然无法索引到这些符号的定义，所以跳转失效。

## 处理办法
1. 在 `CMakeLists.txt` 中使用 `file(GLOB CORE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/*.c")` 收集 `Core/Src` 目录下全部 `.c` 文件。
2. 将 `target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${CORE_SOURCES})` 添加到可执行目标，确保所有源码都会进编译数据库。
3. 重新执行 `CMake: Configure` / `cmake -S . -B build/Debug` 并 `CMake: Build`，让新的 `compile_commands.json` 生效。
4. 若 VS Code 仍无感知，可运行一次 “C/C++: Reset IntelliSense Database” 或重启窗口，clangd 会重新扫描并恢复跳转能力。

## 后续建议
- 每次新增 `.c` 文件时无需再手动改 CMake，但如果使用 `GLOB`，记得在添加/删除文件后重新配置 CMake，以便更新编译数据库。
- 若以后需要精细控制，也可以改为手写列表，只要保证所有源文件都被列入 `target_sources`。

## 2025-11-26 错误记录
- 构建时报 `multiple definition of SystemCoreClock/AHBPrescTable/APBPrescTable/SystemInit/SystemCoreClockUpdate`，来自 `Core/Src/system_stm32f1xx.c` 被编译两次。
- 根因：顶层 `CMakeLists.txt` 在 `CORE_SOURCES` 中包含了 `system_stm32f1xx.c`，同时 STM32CubeMX 的 `STM32_Drivers` 目标也会编译该文件。
- 处理：在顶层 `CMakeLists.txt` 中对 `CORE_SOURCES` 执行 `list(REMOVE_ITEM ... system_stm32f1xx.c)`，让该文件只由 `STM32_Drivers` 目标编译，消除重复定义并恢复链接。
