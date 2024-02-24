#include "windows.h"
#include <stdio.h>
#include <stdlib.h>

#include "ebtn.h"

static LARGE_INTEGER freq, sys_start_time;
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

static int windows_get_match_key(uint16_t key_id)
{
    int key = 0;
    switch (key_id)
    {
    case USER_BUTTON_0:
        key = '0';
        break;
    case USER_BUTTON_1:
        key = '1';
        break;
    case USER_BUTTON_2:
        key = '2';
        break;
    case USER_BUTTON_3:
        key = '3';
        break;
    case USER_BUTTON_4:
        key = '4';
        break;
    case USER_BUTTON_5:
        key = '5';
        break;
    case USER_BUTTON_6:
        key = '6';
        break;
    case USER_BUTTON_7:
        key = '7';
        break;
    case USER_BUTTON_8:
        key = '8';
        break;
    case USER_BUTTON_9:
        key = '9';
        break;
    }

    return key;
}

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
    return GetAsyncKeyState(windows_get_match_key(btn->key_id)) < 0;
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
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
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
        color = FOREGROUND_RED | FOREGROUND_BLUE;
    }
    else if (evt == EBTN_EVT_ONPRESS)
    {
        s = "ONPRESS";
        color = FOREGROUND_GREEN;
    }
    else if (evt == EBTN_EVT_ONRELEASE)
    {
        s = "ONRELEASE";
        color = FOREGROUND_BLUE;
    }
    else if (evt == EBTN_EVT_ONCLICK)
    {
        s = "ONCLICK";
        color = FOREGROUND_RED | FOREGROUND_GREEN;
    }
    else
    {
        s = "UNKNOWN";
        color = FOREGROUND_RED;
    }

    SetConsoleTextAttribute(hConsole, color);
    printf("[%7u][%6u] ID(hex):%4x, evt: %10s, keep-alive cnt: %3u, click cnt: %3u\r\n", (unsigned)get_tick(), (unsigned)diff_time, btn->key_id, s,
           (unsigned)ebtn_keepalive_get_count(btn), (unsigned)ebtn_click_get_count(btn));
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

/**
 * \brief           Example function
 */
int example_user(void)
{
    uint32_t time_last;
    printf("Application running\r\n");
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&sys_start_time);

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
        Sleep(5);
    }
    return 0;
}

/**
 * \brief           Get current tick in ms from start of program
 * \return          uint32_t: Tick in ms
 */
static uint32_t get_tick(void)
{
    LONGLONG ret;
    LARGE_INTEGER now;

    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&now);
    ret = now.QuadPart - sys_start_time.QuadPart;
    return (uint32_t)((ret * 1000) / freq.QuadPart);
}
