-- 规则：STM32F7xx HAL 驱动
-- 触发：配置阶段（on_load）
-- 作用：将厂商 HAL 的源文件与头文件路径注入目标，确保 HAL 与工程同编译
-- 说明：会排除模板文件以避免重复定义和链接错误。
rule("stm32.f7xx.hal")
    on_load(function (target)
        -- 这里的路径对应你实际的工程结构
        local src_dir = path.join(os.scriptdir(), "Drivers/STM32F7xx_HAL_Driver/Src")
        local inc_dir = path.join(os.scriptdir(), "Drivers/STM32F7xx_HAL_Driver/Inc")
        -- 使用通配符 *.c，自动包含所有驱动
        target:add("files", path.join(src_dir, "*.c"))
        
        -- 排除模板文件，防止重复定义报错
        target:remove("files", path.join(src_dir, "*_template.c"))
        -- 排除由 CubeMX 生成的特殊 ramfunc 文件（保留 HAL 的 extension 实现如 *_ex.c）
        target:remove("files", path.join(src_dir, "stm32f7xx_hal_flash_ramfunc.c"))

        -- 添加hal库头文件路径
        target:add("includedirs", {inc_dir, path.join(inc_dir, "Legacy")})

        -- 添加宏
        target:add("defines", "USE_HAL_DRIVER")
    end)
rule_end()


-- 规则：STM32F7xx 的 CMSIS 头文件路径
-- 触发：配置阶段（on_load）
-- 作用：仅添加处理器/CMSIS 的头文件目录（**不**注入 CubeMX 生成的 system/startup 源文件以避免冲突）。
rule("stm32.f7xx.cmsis")
    on_load(function (target)
        -- 添加头文件路径
        local base_dir = path.join(os.scriptdir(), "Drivers/CMSIS/Device/ST/STM32F7xx")
        local device_inc_dir = path.join(base_dir, "Include")
        local cmsis_inc_dir = path.join(os.scriptdir(), "Drivers/CMSIS/Include")

        target:add("includedirs", {device_inc_dir, cmsis_inc_dir})
        -- 这里不要添加 system_stm32f7xx.c CubeMX 已经在 Core/Src 里生成了，这里加了就重复了
    end)
rule_end()


-- 规则：由 CubeMX 提供的 FreeRTOS 源（兼容 CMSIS）
-- 触发：配置阶段（on_load）
-- 作用：添加 FreeRTOS 源码与头路径（包含 portable 与 CMSIS-RTOS V2），将 RTOS 编入目标中
-- 说明：CubeMX 可能在 Core/Src 中生成应用相关的钩子/文件，项目应显式维护这些文件。
rule("stm32.f7xx.hal.freertos")
    on_load(function (target)
        -- 获取当前脚本目录，确保路径是绝对路径，防止找不到文件
        local root_dir = os.scriptdir()
        
        -- 添加源文件
        target:add("files", {
            path.join(root_dir, "Middlewares/Third_Party/FreeRTOS/Source/*.c"),
            path.join(root_dir, "Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2/*.c"),
            path.join(root_dir, "Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c"),
            -- path.join(root_dir, "Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1/*.c")
            path.join(root_dir, "gcc/freertos/ARM_CM7/r0p1/*.c")
        })

        -- 添加头文件路径
        target:add("includedirs", {
            path.join(root_dir, "Middlewares/Third_Party/FreeRTOS/Source/include"),
            path.join(root_dir, "Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2"),
            -- path.join(root_dir, "Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1")
            path.join(root_dir, "gcc/freertos/ARM_CM7/r0p1")
        })
    end)
rule_end()
