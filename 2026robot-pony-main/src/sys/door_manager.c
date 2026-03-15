/**
 * @file door_manager.c
 * @brief 门状态管理器实现
 */

#include "door_manager.h"


/* 出发方向门状态 */
static door_state_t s_go_states[DOOR_ID_MAX + 1] = {DOOR_STATE_UNKNOWN};
/* 回家方向门状态 */
static door_state_t s_back_states[DOOR_ID_MAX + 1] = {DOOR_STATE_UNKNOWN};

/* ============================================================================
 * 初始化函数
 * ============================================================================ */
void door_manager_init(void)
{
    for (int i = 0; i <= DOOR_ID_MAX; i++) {
        s_go_states[i]   = DOOR_STATE_UNKNOWN;
        s_back_states[i] = DOOR_STATE_UNKNOWN;
    }
}

/* ============================================================================
 * 状态操作函数
 * ============================================================================ */
void door_set_state(door_id_t door, door_dir_t dir, door_state_t state)
{
    if (door < DOOR_ID_1 || door > DOOR_ID_MAX) {
        return;
    }

    if (dir == DOOR_DIR_GO) {
        s_go_states[door] = state;
    }
    else {
        s_back_states[door] = state;
    }
}

door_state_t door_get_state(door_id_t door, door_dir_t dir)
{
    if (door < DOOR_ID_1 || door > DOOR_ID_MAX) {
        return DOOR_STATE_UNKNOWN;
    }

    if (dir == DOOR_DIR_GO) {
        return s_go_states[door];
    }
    else {
        return s_back_states[door];
    }
}
