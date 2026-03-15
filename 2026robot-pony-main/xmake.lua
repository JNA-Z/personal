-- xmake.lua for 2026robot (STM32F767)
-- Usage:
-- 1) Ensure arm-none-eabi toolchain is installed and in PATH (or provide SDK path to xmake):
--    xmake f -p cross --sdk=/path/to/gcc-arm-none-eabi
-- 2) Build: xmake -v
-- 3) Rebuild: xmake -r
-- 3) Clean: xmake clean
-- 4) Flash: use your preferred flasher (OpenOCD / st-flash / pyocd / stlink)


-- 1. ==========全局配置==========

-- 强制设置默认平台为交叉编译
set_config("plat", "cross")
set_config("arch", "arm")

-- 设置工程名称、版本
set_project("2026robot")
set_version("1.0.0")
set_xmakever("3.0.0")

-- 允许 c11 和 c++17 标准
set_languages("c11", "cxx17")

-- 添加调试/发布模式规则
add_rules("mode.debug", "mode.release")

-- 自动更新 VSCode 的智能提示配置
add_rules("plugin.compile_commands.autoupdate", {outputdir = "."})


-- 2. ===========引入外部模块==========

-- 加载自定义工具链
includes("arm-none-eabi-custom.lua") 

-- 加载 HAL, CMSIS, Bin转换规则
includes("stm32f7xx-rules.lua")          


-- 3. ===========定义 Target==========


target("2026robot")
    set_kind("binary")
    
    -- 使用 arm-none-eabi-custom.lua 里定义的工具链
    set_toolchains("arm-none-eabi-custom")

    -- 输出elf文件
    set_filename("2026robot.elf")

    -- 调用规则 (stm32f7xx-rules.lua )
    
    -- A. 自动转换 .elf 为 .bin 和 .hex
    add_rules("util.convert_bin_hex")

    -- B. 注入 CMSIS 头文件路径 
    add_rules("stm32.f7xx.cmsis")

    -- C. 注入 HAL 库驱动 
    add_rules("stm32.f7xx.hal")

    -- D. 注入 FreeRTOS (如果你的工程没开 FreeRTOS，可以把这行注释掉)
    add_rules("stm32.f7xx.hal.freertos")

    -- 添加宏定义、启动文件以及源文件

    -- A. 宏定义（根据芯片型号而定）
    add_defines("STM32F767xx")

    -- B. 启动文件（由CubeMX生成位于根目录下）
    add_files("startup_stm32f767xx.s")

    -- C. CubeMX 生成的核心代码
    -- add_files("Core/Src/*.c")
    
    -- D. 用户代码 (src 目录，递归搜索所有.c文件加入工程)
    add_files("src/bsp/*.c")
    add_files("src/sys/*.c")
    add_files("src/gen_src/*.c")
    add_files("src/log/*.c")

    -- 添加头文件路径
    -- add_includedirs("Core/Inc")
    add_includedirs("src")
    add_includedirs("src/gen_inc")

    -- 添加头文件搜索路径
    add_includedirs("src/bsp")
    add_includedirs("src/sys")

    -- 7. 编译与链接选项 (Cortex-M4 + Hard FPU)

    add_cflags(
        "-mcpu=cortex-m4", 
        "-mthumb", 
        "-mfpu=fpv4-sp-d16", 
        "-mfloat-abi=hard",  -- 强开硬浮点
        "-std=c11", 
        "-Wall", 
        "-fdata-sections", 
        "-ffunction-sections",
        "-g",                -- 生成调试信息
        "-gdwarf-2",
        {force = true}
    )
    -- 给C++编译器同样的选项，并确保使用C++17标准
    add_cxxflags(
        "-mcpu=cortex-m4",
        "-mthumb",
        "-mfpu=fpv4-sp-d16",
        "-mfloat-abi=hard",
        "-std=c++17",
        "-Wall",
        "-fdata-sections",
        "-ffunction-sections",
        "-g",
        "-gdwarf-2",
        {force = true}
    )

    add_asflags(
        "-mcpu=cortex-m4", 
        "-mthumb", 
        "-mfpu=fpv4-sp-d16", 
        "-mfloat-abi=hard",
        {force = true}
    )
    
    add_ldflags(
        "-mcpu=cortex-m4", 
        "-mthumb", 
        "-mfpu=fpv4-sp-d16", 
        "-mfloat-abi=hard",
        "-Wl,--gc-sections", 
        "-Wl,-Map=build/output.map", 
        "-TSTM32F767XX_FLASH.ld",  
        "-lc", "-lm", "-lnosys", 
        "--specs=nano.specs", 
        "-u _printf_float",
        "-u _scanf_float",
        {linker = true, force = true}
    )
    -- 添加C++链接选项，确保链接到C++标准库和支持
    add_ldflags(
        "-lstdc++",
        "-lsupc++",
        {linker = true, force = true}
    )

    -- 调试/发布模式差异化配置
    if is_mode("debug") then
        add_defines("DEBUG")
        set_optimize("none")
        set_symbols("debug")
    else
        set_optimize("fast")
    end

    -- 打印编译的文件信息
    -- after_load(function (target)
    --     print("================================")
    --     print("📁 加入编译的源文件:")
    --     print("================================")
    --     -- 获取所有源文件列表并打印
    --     local source_files = target:sourcefiles()
    --     for _, file in ipairs(source_files) do
    --         print("  - " .. file)
    --     end

    --     print("\n================================")
    --     print("🔍 头文件搜索路径:")
    --     print("================================")
    --     -- 获取所有头文件搜索路径（包括系统路径和用户添加的路径）
    --     -- 使用 target:get("includedirs") 可以获取直接配置的路径
    --     local include_dirs = target:get("includedirs")
    --     if include_dirs then
    --         for _, dir in ipairs(include_dirs) do
    --             print("  - " .. dir)
    --         end
    --     else
    --         print("  (没有配置额外的头文件搜索路径)")
    --     end

    --     -- 如果你想查看包括工具链等在内的所有最终搜索路径，可以使用 target:toolchain("cxx"):includedirs()
    --     -- 但这部分通常比较复杂，上述方法已能满足大部分调试需求。
    -- end)


-- Notes:
-- - Make sure `arm-none-eabi-gcc`/`objcopy` are in PATH or configure xmake toolchain with `xmake f`.
-- - If you use a different linker script path/name, update the -T option accordingly.
-- - Flashing is intentionally left to user tooling (OpenOCD / st-flash / ST-Link GUI). You can add a script or xmake target for flashing if desired.
