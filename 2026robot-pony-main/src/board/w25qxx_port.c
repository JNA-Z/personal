#include "w25qxx_port.h"
#include "../driver/w25qxx.h"

// SPI_HandleTypeDef* g_hspi;
// GPIO_TypeDef*      g_hcs_port;
// u16           g_hcs_pin;

static void w25qxx_write_pin(void* gpio_port, u16 pin, u8 value)
{
    // HAL_GPIO_WritePin((GPIO_TypeDef*)gpio_port, pin, (GPIO_PinState)value);
}

static void w25qxx_spi_transmit(void* hspi, u8* data, usize size, u32 timeout)
{
    // HAL_SPI_Transmit((SPI_HandleTypeDef*)hspi, data, size, timeout);
}

static void w25qxx_spi_receive(void* hspi, u8* data, usize size, u32 timeout)
{
    // HAL_SPI_Receive((SPI_HandleTypeDef*)hspi, data, size, timeout);
}

static const w25qxx_port_t g_w25qxx_port={
    .write_pin    = w25qxx_write_pin,
    .spi_transmit = w25qxx_spi_transmit,
    .spi_receive  = w25qxx_spi_receive,
};

bool w25qxx_port_init(void)
{
    w25qxx_bind_port(&g_w25qxx_port);

    return w25qxx_port_is_registered();
}
