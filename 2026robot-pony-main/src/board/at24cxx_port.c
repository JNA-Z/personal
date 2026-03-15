#include "at24cxx_port.h"
#include "../driver/at24cxx.h"
#include "../util/datatype.h"
#include "../driver/debug.h"

void at24cxx_port_i2c_write(void* hi2c, u16 dev_addr, u16 mem_addr, u16 mem_addr_size, u8* data,
                            u16 data_size, u32 timeout)
{
    // HAL_I2C_Mem_Write(i2c, dev_addr, mem_addr, mem_addr_size, data, data_size, timeout);
}

void at24cxx_port_i2c_read(void* i2c_extra_data, u16 dev_addr, u16 mem_addr, u16 mem_addr_size, u8* data, u16 data_size,
                           u32 timeout)
{
    // HAL_I2C_Mem_Read(i2c, dev_addr, mem_addr, mem_addr_size, data, data_size, timeout);
}

static const at24cxx_port_t g_at24cxx_port = {
    .i2c_write = at24cxx_port_i2c_write,
    .i2c_read  = at24cxx_port_i2c_read,
};

bool at24cxx_port_init(void)
{
    at24cxx_bind_port(&g_at24cxx_port);
    return at24cxx_port_is_registered();
}

at24cxx_t g_at24c256;

void test_at24cxx(void)
{
    at24cxx_write_byte(&g_at24c256, 0, 0x66);
    // HAL_Delay(1000);
    u8 data = 0;
    at24cxx_read_byte(&g_at24c256, 0, &data);

    debug_print("data=%x\n", data);
}
