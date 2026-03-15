/**
 * @file door_manager.h
 * @brief 门状态管理器
 * @note 封装门的开/关/未知状态，支持出发和回家两个方向
 */

#ifndef __DOOR_MANAGER_H
#define __DOOR_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 门编号枚举
 * ============================================================================ */
typedef enum {
    DOOR_ID_1 = 1,      /* 1号门 */
    DOOR_ID_2,          /* 2号门 */
    DOOR_ID_3,          /* 3号门 */
    DOOR_ID_4,          /* 4号门 */
    DOOR_ID_MAX         /* 门数量上限 */
} door_id_t;

/* ============================================================================
 * 行驶方向枚举
 * ============================================================================ */
typedef enum {
    DOOR_DIR_GO = 0,    /* 出发方向（探险） */
    DOOR_DIR_BACK = 1   /* 回家方向 */
} door_dir_t;

/* ============================================================================
 * 门状态枚举
 * ============================================================================ */
typedef enum {
    DOOR_STATE_UNKNOWN = 0, /* 未探测，未知 */
    DOOR_STATE_OPEN,        /* 开放 */
    DOOR_STATE_CLOSED       /* 关闭 */
} door_state_t;

/* ============================================================================
 * 初始化函数
 * ============================================================================ */

/**
 * @brief 初始化门管理器，将所有门状态设为未知
 */
void door_manager_init(void);

/* ============================================================================
 * 状态操作函数
 * ============================================================================ */

/**
 * @brief 设置某个方向的门状态
 * @param door 门编号 (DOOR_ID_1 ~ DOOR_ID_4)
 * @param dir 行驶方向
 * @param state 门状态
 */
void door_set_state(door_id_t door, door_dir_t dir, door_state_t state);

/**
 * @brief 获取某个方向的门状态
 * @param door 门编号 (DOOR_ID_1 ~ DOOR_ID_4)
 * @param dir 行驶方向
 * @return 门状态
 */
door_state_t door_get_state(door_id_t door, door_dir_t dir);

/* ============================================================================
 * 便捷判断函数（内联）
 * ============================================================================ */

/**
 * @brief 判断门是否关闭
 */
static inline bool door_is_closed(door_id_t door, door_dir_t dir) {
    return door_get_state(door, dir) == DOOR_STATE_CLOSED;
}

/**
 * @brief 判断门是否开放
 */
static inline bool door_is_open(door_id_t door, door_dir_t dir) {
    return door_get_state(door, dir) == DOOR_STATE_OPEN;
}

/**
 * @brief 判断门状态是否未知
 */
static inline bool door_is_unknown(door_id_t door, door_dir_t dir) {
    return door_get_state(door, dir) == DOOR_STATE_UNKNOWN;
}

/**
 * @brief 判断门是否可通行（开放或未知）
 * @note 未知状态视为可通行，与原逻辑一致
 */
static inline bool door_is_passable(door_id_t door, door_dir_t dir) {
    door_state_t state = door_get_state(door, dir);
    return state == DOOR_STATE_OPEN || state == DOOR_STATE_UNKNOWN;
}

#ifdef __cplusplus
}
#endif

#endif /* __DOOR_MANAGER_H */
