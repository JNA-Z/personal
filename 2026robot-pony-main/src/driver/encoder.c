#include "encoder.h"

static const encoder_port_t* g_encoder_port = NULL;

void encoder_bind_port(const encoder_port_t* port)
{
    g_encoder_port = port;
}

bool encoder_port_is_registered(void)
{
    return (g_encoder_port != NULL);
}

/**
 * @brief 初始化编码器
 * 
 * @param self 编码器句柄
 */
void encoder_init(encoder_t* self)
{
    switch (self->mode) {
        case ENCODER_MODE_QUADRATURE:
            if (self->encoder_tim == NULL) {
                debug_print("[encoder] error: encoder_tim is NULL in quadrature mode\n");
                return;
            }
            if (self->gate_tim == NULL) {
                debug_print("[encoder] warning: gate_tim is NULL in quadrature mode\n");
            }
            break;
        case ENCODER_MODE_SINGLE_M:
            break;
        case ENCODER_MODE_SINGLE_T:
            break;
        default:
            break;
    }

    self->prev_value = 0;
    self->value = 0;

    // 默认直接启动编码器以及门控定时器
    g_encoder_port->tim_base_start(self->encoder_tim);
    if (self->gate_tim) {
        g_encoder_port->tim_base_start(self->gate_tim);
    }
    g_encoder_port->tim_start(self->encoder_tim, self->channel);
}

/**
 * @brief 获取编码器计数值
 * 
 * @param self 编码器句柄
 * @return u32 编码器计数值
 * ENCODER_MODE_QUADRATURE与ENCODER_MODE_SINGLE_M为计数器值
 * ENCODER_MODE_SINGLE_T模式下为捕获值
 */
u32 encoder_get_count(encoder_t* self)
{
    return g_encoder_port->read_tim_cnt(self->encoder_tim, self->channel); 
}

/**
 * @brief 编码器中断处理入口，该入口应当放入中断回调中
 * 
 * @param self 
 */
void encoder_irq(encoder_t* self)
{
    u32 val;
    switch (self->mode) {
        case ENCODER_MODE_QUADRATURE:
        case ENCODER_MODE_SINGLE_M:
            // 该入口应当放到窗定时器的更新中断中，定时取出计数值并清零
            self->value = self->polarity * (i16)g_encoder_port->read_tim_cnt(self->encoder_tim, self->channel);
            g_encoder_port->clear_tim_cnt(self->encoder_tim, self->channel);
            break;        
        case ENCODER_MODE_SINGLE_T:
            // 该入口应当放在捕获比较中断中，通过计算相邻两次的差值
            val = g_encoder_port->read_tim_cnt(self->encoder_tim, self->channel);
            self->value = self->polarity * (val - self->prev_value);
            self->prev_value = val;
            break;
        default:
            break;
    }

}



