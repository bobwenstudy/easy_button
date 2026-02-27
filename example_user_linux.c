#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <termios.h>
#include "ebtn.h"
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <pthread.h>
#include <stdatomic.h>
#include <errno.h>      // 新增：用于错误处理
#include <string.h>     // 新增：用于字符串操作
#include <sys/types.h>  // 新增：系统类型定义
#include "libscl/SCL_map.h"
SCL_map_t   btn_info_map;
#define KEY_DEVICE "/dev/input/event1"  // 按键设备节点
static uint32_t get_tick(void);
typedef enum
{
    USER_BUTTON_0 = 0,
    USER_BUTTON_1,
    USER_BUTTON_2,
    USER_BUTTON_3,
    USER_BUTTON_4,
    USER_BUTTON_5,
    USER_BUTTON_6,
    USER_BUTTON_7,
    USER_BUTTON_8,
    USER_BUTTON_9,
    USER_BUTTON_INVALID,
    USER_BUTTON_MAX,

    USER_BUTTON_COMBO_0 = 0x100,
    USER_BUTTON_COMBO_1,
    USER_BUTTON_COMBO_2,
    USER_BUTTON_COMBO_3,
    USER_BUTTON_COMBO_MAX,
} user_button_t;
static struct termios old_termios;
typedef struct key_btn_info_t
{
    int key_value;
    char key_status;
}key_btn_info_t;
user_button_t get_key_input(void) {
	static int fd = -1;
	struct input_event ev;
	ssize_t n;

	// 首次调用时打开设备
	if (fd < 0) {
		printf("Trying to open input device %s\n", KEY_DEVICE);
		fd = open(KEY_DEVICE, O_RDONLY);
		if (fd < 0) {
			perror("Failed to open input device");
			return USER_BUTTON_INVALID;
		}
		printf("Successfully opened input device %s\n", KEY_DEVICE);
	}

	// 读取输入事件
	n = read(fd, &ev, sizeof(ev));
	if (n == -1) {
		if (errno != EAGAIN) {
			perror("Error reading input device");
			close(fd);
			fd = -1;
		}
		return USER_BUTTON_INVALID;
	}
	else if (n != sizeof(ev)) {
		fprintf(stderr, "Read incomplete input event (%zd bytes)\n", n);
		return USER_BUTTON_INVALID;
	}

	// printf("Input event: type=%d code=%d value=%d\n", ev.type, ev.code, ev.value);
    int key_value = USER_BUTTON_INVALID;
    int key_status = 0;
	if (ev.type == EV_KEY) {
        key_status = ev.value;
        switch (ev.code) {
            case KEY_0:
                key_value = USER_BUTTON_0;
                break;
            case KEY_1:
                key_value = USER_BUTTON_1;
                break;
            case KEY_2:
                key_value = USER_BUTTON_2;
                break;
            case KEY_3:
                key_value = USER_BUTTON_3;
                break;
            case KEY_4:
                key_value = USER_BUTTON_4;
                break;
            case KEY_5:
                key_value = USER_BUTTON_5;
                break;
            case KEY_6:
                key_value = USER_BUTTON_6;
                break;
            case KEY_7:
                key_value = USER_BUTTON_7;
                break;
            case KEY_8:
                key_value = USER_BUTTON_8;
                break;
            case KEY_9:
                key_value = USER_BUTTON_9;
                break;
            default:
                printf("Unmapped key code: %d\n", ev.code);
                break;
        }
        if (key_value != USER_BUTTON_INVALID){
            key_btn_info_t *key_btn_info = NULL;
            key_btn_info = scl_map_access(btn_info_map, key_value);
            if (key_btn_info == NULL){
                key_btn_info = malloc(sizeof(key_btn_info_t));
                memset(key_btn_info, 0, sizeof(key_btn_info_t));
                key_btn_info->key_value = key_value;
                key_btn_info->key_status = key_status;
                scl_map_insert(btn_info_map, key_value, key_btn_info);
            }
            key_btn_info->key_value = key_value;
            key_btn_info->key_status = key_status;
            return key_value;
        }
	}
	return USER_BUTTON_INVALID;
}




/* User defined settings */
static const ebtn_btn_param_t defaul_ebtn_param = EBTN_PARAMS_INIT(20, 0, 20, 300, 200, 500, 10);

static ebtn_btn_t btns[] = {
        // For key_idx double test, need full key map size
        EBTN_BUTTON_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),
        EBTN_BUTTON_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),
        EBTN_BUTTON_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),
        EBTN_BUTTON_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),

        EBTN_BUTTON_INIT(USER_BUTTON_0, &defaul_ebtn_param),
        EBTN_BUTTON_INIT(USER_BUTTON_1, &defaul_ebtn_param),
        EBTN_BUTTON_INIT(USER_BUTTON_2, &defaul_ebtn_param),
        EBTN_BUTTON_INIT(USER_BUTTON_3, &defaul_ebtn_param),

        // For key_idx double test, need full key map size
        EBTN_BUTTON_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),
        EBTN_BUTTON_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),
        EBTN_BUTTON_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),
        EBTN_BUTTON_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),

        EBTN_BUTTON_INIT(USER_BUTTON_4, &defaul_ebtn_param),
        EBTN_BUTTON_INIT(USER_BUTTON_5, &defaul_ebtn_param),

        // For key_idx double test, need full key map size
        EBTN_BUTTON_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),
        EBTN_BUTTON_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),
        EBTN_BUTTON_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),
        EBTN_BUTTON_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),

        EBTN_BUTTON_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),
        EBTN_BUTTON_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),
        EBTN_BUTTON_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),
        EBTN_BUTTON_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),

        EBTN_BUTTON_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),
        EBTN_BUTTON_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),
        EBTN_BUTTON_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),
        EBTN_BUTTON_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),

        EBTN_BUTTON_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),
        EBTN_BUTTON_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),
        EBTN_BUTTON_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),
        EBTN_BUTTON_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),
};

static ebtn_btn_dyn_t btns_dyn[] = {
        // For key_idx double test, need full key map size
        EBTN_BUTTON_DYN_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),
        EBTN_BUTTON_DYN_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),
        EBTN_BUTTON_DYN_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),
        EBTN_BUTTON_DYN_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),

        EBTN_BUTTON_DYN_INIT(USER_BUTTON_6, &defaul_ebtn_param),
        EBTN_BUTTON_DYN_INIT(USER_BUTTON_7, &defaul_ebtn_param),

        // For key_idx double test, need full key map size
        EBTN_BUTTON_DYN_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),
        EBTN_BUTTON_DYN_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),
        EBTN_BUTTON_DYN_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),
        EBTN_BUTTON_DYN_INIT(USER_BUTTON_INVALID, &defaul_ebtn_param),

        EBTN_BUTTON_DYN_INIT(USER_BUTTON_8, &defaul_ebtn_param),
        EBTN_BUTTON_DYN_INIT(USER_BUTTON_9, &defaul_ebtn_param),
};

uint32_t last_time_keys[USER_BUTTON_MAX - USER_BUTTON_0] = {0};

static ebtn_btn_combo_t btns_combo[] = {
        EBTN_BUTTON_COMBO_INIT(USER_BUTTON_COMBO_0, &defaul_ebtn_param),
        EBTN_BUTTON_COMBO_INIT(USER_BUTTON_COMBO_1, &defaul_ebtn_param),
};

static ebtn_btn_combo_dyn_t btns_combo_dyn[] = {
        EBTN_BUTTON_COMBO_DYN_INIT(USER_BUTTON_COMBO_2, &defaul_ebtn_param),
        EBTN_BUTTON_COMBO_DYN_INIT(USER_BUTTON_COMBO_3, &defaul_ebtn_param),
};

uint32_t last_time_keys_combo[USER_BUTTON_COMBO_MAX - USER_BUTTON_COMBO_0] = {0};



/**
 * \brief           Get input state callback
 * \param           btn: Button instance
 * \return          `1` if button active, `0` otherwise
 */
uint8_t prv_btn_get_state(struct ebtn_btn *btn)
{
    /*
     * Function will return negative number if button is pressed,
     * or zero if button is releases
     */
    key_btn_info_t *key_btn_info = scl_map_access(btn_info_map, btn->key_id);
    if (key_btn_info){
        return key_btn_info->key_status;
    }
    return 0;
}

/**
 * \brief           Button event
 *
 * \param           btn: Button instance
 * \param           evt: Button event
 */
void prv_btn_event(struct ebtn_btn *btn, ebtn_evt_t evt)
{
    const char *s;
    uint32_t color, keepalive_cnt = 0;
    uint32_t diff_time = 0;
    uint32_t *diff_time_ptr = NULL;
    if (btn->key_id < USER_BUTTON_MAX)
    {
        diff_time_ptr = &last_time_keys[btn->key_id - USER_BUTTON_0];
    }
    else
    {
        diff_time_ptr = &last_time_keys[btn->key_id - USER_BUTTON_COMBO_0];
    }
    diff_time = get_tick() - *diff_time_ptr;

    /* This is for purpose of test and timing validation */
    if (diff_time > 2000)
    {
        diff_time = 0;
    }

    *diff_time_ptr = get_tick(); /* Set current date as last one */

    /* Get event string */
    if (evt == EBTN_EVT_KEEPALIVE)
    {
        s = "KEEPALIVE";
    }
    else if (evt == EBTN_EVT_ONPRESS)
    {
        s = "ONPRESS";
    }
    else if (evt == EBTN_EVT_ONRELEASE)
    {
        s = "ONRELEASE";
    }
    else if (evt == EBTN_EVT_ONCLICK)
    {
        s = "ONCLICK";
    }
    else
    {
        s = "UNKNOWN";
    }

    printf("[%7u][%6u] ID(hex):%4x, evt: %10s, keep-alive cnt: %3u, click cnt: %3u\r\n", (unsigned)get_tick(), (unsigned)diff_time, btn->key_id, s,
           (unsigned)ebtn_keepalive_get_count(btn), (unsigned)ebtn_click_get_count(btn));
}

void* key_listener_thread(void* arg) {
	(void)arg;

	while (1) {
		int key = get_key_input(); // 使用上述任一方法
		if (key != USER_BUTTON_INVALID) {
			// atomic_store(&key_pressed, key);
		}
		usleep(10000); // 10ms间隔
	}

	return NULL;
}
static pthread_t key_thread;

/**
 * \brief           Example function
 */
int example_user(void)
{
    uint32_t time_last;
    printf("Application running\r\n");
    btn_info_map = scl_map_new();
    pthread_create(&key_thread, NULL, key_listener_thread, NULL);
	pthread_detach(key_thread);

    /* Define buttons */
    ebtn_init(btns, EBTN_ARRAY_SIZE(btns), btns_combo, EBTN_ARRAY_SIZE(btns_combo), prv_btn_get_state, prv_btn_event);

    ebtn_combo_btn_add_btn(&btns_combo[0], USER_BUTTON_0);
    ebtn_combo_btn_add_btn(&btns_combo[0], USER_BUTTON_1);

    ebtn_combo_btn_add_btn(&btns_combo[1], USER_BUTTON_2);
    ebtn_combo_btn_add_btn(&btns_combo[1], USER_BUTTON_3);

    // dynamic register
    for (int i = 0; i < (EBTN_ARRAY_SIZE(btns_dyn)); i++)
    {
        ebtn_register(&btns_dyn[i]);
    }

    ebtn_combo_btn_add_btn(&btns_combo_dyn[0].btn, USER_BUTTON_4);
    ebtn_combo_btn_add_btn(&btns_combo_dyn[0].btn, USER_BUTTON_5);

    ebtn_combo_btn_add_btn(&btns_combo_dyn[1].btn, USER_BUTTON_6);
    ebtn_combo_btn_add_btn(&btns_combo_dyn[1].btn, USER_BUTTON_7);

    for (int i = 0; i < (EBTN_ARRAY_SIZE(btns_combo_dyn)); i++)
    {
        ebtn_combo_register(&btns_combo_dyn[i]);
    }

    while (1)
    {
        /* Process forever */
        ebtn_process(get_tick());

        /* Artificial sleep to offload win process */
        usleep(5000);
    }
    return 0;
}

/**
 * \brief           Get current tick in ms from start of program
 * \return          uint32_t: Tick in ms
 */
static uint32_t get_tick(void)
{
	struct timespec ts;
	clock_gettime (CLOCK_MONOTONIC_RAW, &ts);
	return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}
