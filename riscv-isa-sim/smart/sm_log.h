/*addedd by li yang, date 2025-07-02
 */
#ifndef SM_LOG_H
#define SM_LOG_H

#define SM_LOG_LEVEL 3
// #define SM_LOG_DEBUG 4
#define SM_LOG_L_INFO 3
#define SM_LOG_L_WARN 2
#define SM_LOG_L_ERROR 1

#define C_NONE "\e[0m"
#define C_RED "\e[0;31m"
#define C_GREEN "\e[0;32m"
#define C_YELLOW "\e[0;43m"
#define C_BLUE "\e[0;34m"

#define sm_log_info(fmt, args...) \
    do { \
        if (SM_LOG_LEVEL >= SM_LOG_L_INFO) { \
            printf(C_GREEN "[mdl] " fmt C_NONE "\n", ##args); \
        } \
    } while (0)

#define sm_log_warn(fmt, args...) \
    do { \
        if (SM_LOG_LEVEL >= SM_LOG_L_WARN) { \
            printf(C_YELLOW "[mdl] " fmt C_NONE "\n", ##args); \
        } \
    } while (0)

#define sm_log_error(fmt, args...) \
    do { \
        if (SM_LOG_LEVEL >= SM_LOG_L_ERROR) { \
            printf(C_RED "[mdl] " fmt C_NONE "\n", ##args); \
        } \
    } while (0)

#endif // SM_LOG_H