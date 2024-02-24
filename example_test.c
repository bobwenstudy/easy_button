#include <stdio.h>
#include <stdlib.h>

#include "windows.h"

#include "ebtn.h"

//
// Tests
//
const char *suite_name;
char suite_pass;
int suites_run = 0, suites_failed = 0, suites_empty = 0;
int tests_in_suite = 0, tests_run = 0, tests_failed = 0;

#define QUOTE(str) #str
#define ASSERT(x)                                                                                                                                              \
    {                                                                                                                                                          \
        tests_run++;                                                                                                                                           \
        tests_in_suite++;                                                                                                                                      \
        if (!(x))                                                                                                                                              \
        {                                                                                                                                                      \
            printf("failed assert [%s:%i] %s\n", __FILE__, __LINE__, QUOTE(x));                                                                                \
            suite_pass = 0;                                                                                                                                    \
            tests_failed++;                                                                                                                                    \
        }                                                                                                                                                      \
    }

void SUITE_START(const char *name)
{
    suite_pass = 1;
    suite_name = name;
    suites_run++;
    tests_in_suite = 0;
}

void SUITE_END(void)
{
    printf("Testing %s ", suite_name);
    size_t suite_i;
    for (suite_i = strlen(suite_name); suite_i < 80 - 8 - 5; suite_i++)
        printf(".");
    printf("%s\n", suite_pass ? " pass" : " fail");
    if (!suite_pass)
        suites_failed++;
    if (!tests_in_suite)
        suites_empty++;
}

typedef enum
{
    USER_BUTTON_default = 0,
    USER_BUTTON_onrelease_debounce,
    USER_BUTTON_keepalive_with_click,
    USER_BUTTON_max_click_3,
    USER_BUTTON_click_multi_max_0,
    USER_BUTTON_keep_alive_0,
    USER_BUTTON_MAX,

    USER_BUTTON_COMBO_0 = 0x100,
    USER_BUTTON_COMBO_1,
    USER_BUTTON_COMBO_2,
    USER_BUTTON_COMBO_3,
    USER_BUTTON_COMBO_MAX,
} user_button_t;

/**
 * \brief           Input state information
 */
typedef struct
{
    uint16_t key_id;   /*!< Select Key id */
    uint8_t state;     /*!< Input state -> 1 = active, 0 = inactive */
    uint32_t duration; /*!< Time until this state is enabled */
} btn_test_time_t;

/**
 * \brief           Event sequence
 */
typedef struct
{
    ebtn_evt_t evt;        /*!< Event type */
    uint8_t keepalive_cnt; /*!< Number of keep alive events while button is active */
    uint8_t conseq_clicks; /*!< Number of consecutive clicks detected */
} btn_test_evt_t;

typedef struct
{
    const char *test_name;
    uint16_t test_key_id;
    int test_sequence_cnt;
    btn_test_time_t *test_sequence; /*!< Input state -> 1 = active, 0 = inactive */
    int test_events_cnt;
    const btn_test_evt_t *test_events; /*!< Time until this state is enabled */
} btn_test_arr_t;

#define TEST_ARRAY_DEFINE(_key_id, _seq, _evt)                                                                                                                 \
    {                                                                                                                                                          \
        .test_key_id = _key_id, .test_name = #_seq, .test_sequence_cnt = EBTN_ARRAY_SIZE(_seq), .test_sequence = _seq,                                         \
        .test_events_cnt = EBTN_ARRAY_SIZE(_evt), .test_events = _evt                                                                                          \
    }

/* Max number of ms to demonstrate */
#define MAX_TIME_MS 0x3FFFF

#define EBTN_PARAM_TIME_DEBOUNCE_PRESS(_param)   _param.time_debounce
#define EBTN_PARAM_TIME_DEBOUNCE_RELEASE(_param) _param.time_debounce_release
#define EBTN_PARAM_TIME_CLICK_MIN(_param)        _param.time_click_pressed_min
#define EBTN_PARAM_TIME_CLICK_MAX(_param)        _param.time_click_pressed_max
#define EBTN_PARAM_TIME_CLICK_MULTI_MAX(_param)  _param.time_click_multi_max
#define EBTN_PARAM_TIME_KEEPALIVE_PERIOD(_param) _param.time_keepalive_period
#define EBTN_PARAM_CLICK_MAX_CONSECUTIVE(_param) _param.max_consecutive

static const ebtn_btn_param_t param_default = EBTN_PARAMS_INIT(20, 0, 20, 300, 200, 500, 10);

static const ebtn_btn_param_t param_onrelease_debounce = EBTN_PARAMS_INIT(20, 80, 0, 300, 200, 500, 10);

static const ebtn_btn_param_t param_keepalive_with_click = EBTN_PARAMS_INIT(20, 80, 0, 400, 200, 100, 10);

static const ebtn_btn_param_t param_max_click_3 = EBTN_PARAMS_INIT(20, 80, 0, 400, 200, 100, 3);

static const ebtn_btn_param_t param_click_multi_max_0 = EBTN_PARAMS_INIT(20, 80, 0, 400, 0, 100, 3);

static const ebtn_btn_param_t param_keep_alive_0 = EBTN_PARAMS_INIT(20, 80, 0, 400, 200, 0, 3);

/* List of used buttons -> test case */
static ebtn_btn_t btns[] = {EBTN_BUTTON_INIT(USER_BUTTON_default, &param_default),
                            EBTN_BUTTON_INIT(USER_BUTTON_onrelease_debounce, &param_onrelease_debounce),
                            EBTN_BUTTON_INIT(USER_BUTTON_keepalive_with_click, &param_keepalive_with_click),
                            EBTN_BUTTON_INIT(USER_BUTTON_max_click_3, &param_max_click_3),
                            EBTN_BUTTON_INIT(USER_BUTTON_click_multi_max_0, &param_click_multi_max_0),
                            EBTN_BUTTON_INIT(USER_BUTTON_keep_alive_0, &param_keep_alive_0)};

static volatile uint32_t test_processed_time_current;

/* Set button state -> used for test purposes */
#define BTN_STATE_RAW(_key_id_, _state_, _duration_)                                                                                                           \
    {                                                                                                                                                          \
        .key_id = (_key_id_), .state = (_state_), .duration = (_duration_)                                                                                     \
    }

#define BTN_STATE(_state_, _duration_) BTN_STATE_RAW(USER_BUTTON_default, _state_, _duration_)

/* On-Press event */
#define BTN_EVENT_ONPRESS()                                                                                                                                    \
    {                                                                                                                                                          \
        .evt = EBTN_EVT_ONPRESS                                                                                                                                \
    }
/* On-Release event */
#define BTN_EVENT_ONRELEASE()                                                                                                                                  \
    {                                                                                                                                                          \
        .evt = EBTN_EVT_ONRELEASE                                                                                                                              \
    }
/* On-Click event */
#define BTN_EVENT_ONCLICK(_conseq_clicks_)                                                                                                                     \
    {                                                                                                                                                          \
        .evt = EBTN_EVT_ONCLICK, .conseq_clicks = (_conseq_clicks_)                                                                                            \
    }
/* On-Click event */
#define BTN_EVENT_KEEPALIVE(_keepalive_cnt_)                                                                                                                   \
    {                                                                                                                                                          \
        .evt = EBTN_EVT_KEEPALIVE, .keepalive_cnt = (_keepalive_cnt_)                                                                                          \
    }

/*
 * Simulate click event
 */
static btn_test_time_t test_sequence_single_click[] = {
        /*
         * Step 1:
         * Go to active state and stay there for a period
         * of minimum debounce time and minimum
         * time input must be active to later detect click event
         *
         * Step 2:
         * Go low and stay inactive until time to report click has elapsed.
         * Include debounce timing for release event.
         *
         * Add +1 to the end, to force click event,
         * and not to go to "consecutive clicks" if any further tests are added in this sequence
         */
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) + EBTN_PARAM_TIME_CLICK_MIN(param_default)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_default) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_default)),
};

static const btn_test_evt_t test_events_single_click[] = {
        BTN_EVENT_ONPRESS(),
        BTN_EVENT_ONRELEASE(),
        BTN_EVENT_ONCLICK(1),
};

static btn_test_time_t test_sequence_double_click[] = {
        /*
         * Repeat above steps, this time w/o +1 at the end.
         *
         * Simulate "2" consecutive clicks and report final "click" event at the end of the
         * sequence, with "2" consecutive clicks in the report info
         */
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) + EBTN_PARAM_TIME_CLICK_MIN(param_default)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_default) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_default) / 2),
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) + EBTN_PARAM_TIME_CLICK_MIN(param_default)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_default) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_default)),
};

static const btn_test_evt_t test_events_double_click[] = {
        BTN_EVENT_ONPRESS(), BTN_EVENT_ONRELEASE(), BTN_EVENT_ONPRESS(), BTN_EVENT_ONRELEASE(), BTN_EVENT_ONCLICK(2),
};

static btn_test_time_t test_sequence_triple_click[] = {
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) + EBTN_PARAM_TIME_CLICK_MIN(param_default)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_default) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_default) / 2),
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) + EBTN_PARAM_TIME_CLICK_MIN(param_default)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_default) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_default) / 2),
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) + EBTN_PARAM_TIME_CLICK_MIN(param_default)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_default) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_default)),
};

static const btn_test_evt_t test_events_triple_click[] = {
        BTN_EVENT_ONPRESS(), BTN_EVENT_ONRELEASE(), BTN_EVENT_ONPRESS(),  BTN_EVENT_ONRELEASE(),
        BTN_EVENT_ONPRESS(), BTN_EVENT_ONRELEASE(), BTN_EVENT_ONCLICK(3),
};

static btn_test_time_t test_sequence_double_click_critical_time[] = {
        /*
         * Repeat above steps, this time w/o +1 at the end.
         *
         * Simulate "2" consecutive clicks and report final "click" event at the end of the
         * sequence, with "2" consecutive clicks in the report info
         */
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) + EBTN_PARAM_TIME_CLICK_MIN(param_default)),
        /* Hold button in release state for time that is max for 2 clicks - time that we will
            indicate in the next press state -> this is the frequency between detected events */
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_default) +
                             EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_default)
                             /* Decrease by active time in next step */
                             - (EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default)) - 2),
        /* Active time */
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) + EBTN_PARAM_TIME_CLICK_MIN(param_default)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_default) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_default)),
};

static const btn_test_evt_t test_events_double_click_critical_time[] = {
        BTN_EVENT_ONPRESS(), BTN_EVENT_ONRELEASE(), BTN_EVENT_ONPRESS(), BTN_EVENT_ONRELEASE(), BTN_EVENT_ONCLICK(2),
};

static btn_test_time_t test_sequence_double_click_critical_time_over[] = {
        /*
         * This test shows how to handle case when 2 clicks are being executed,
         * but time between 2 release events is larger than maximum
         * allowed time for consecutive clicks.
         *
         * In this case, 2 onclick events are sent,
         * both with consecutive clicks counter set to 1
         */
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) + EBTN_PARAM_TIME_CLICK_MIN(param_default)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_default) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_default) -
                             (EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default))),
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) + EBTN_PARAM_TIME_CLICK_MIN(param_default)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_default) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_default)),
};

static const btn_test_evt_t test_events_double_click_critical_time_over[] = {
        BTN_EVENT_ONPRESS(),
        BTN_EVENT_ONRELEASE(),
        /* This one is to handle click for first sequence */
        BTN_EVENT_ONCLICK(1),
        BTN_EVENT_ONPRESS(),
        BTN_EVENT_ONRELEASE(),
        /* This one is to handle click for second sequence */
        BTN_EVENT_ONCLICK(1),
};

static btn_test_time_t test_sequence_click_with_keepalive[] = {
        /*
         * Make a click event, followed by the longer press.
         * Simulate "long press" w/ previous click, that has click counter set to 1
         */
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) + EBTN_PARAM_TIME_CLICK_MIN(param_default)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_default) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_default) / 2),
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default)),
        BTN_STATE(1, EBTN_PARAM_TIME_KEEPALIVE_PERIOD(param_default)),
        BTN_STATE(1, EBTN_PARAM_TIME_KEEPALIVE_PERIOD(param_default)),
        BTN_STATE(1, EBTN_PARAM_TIME_KEEPALIVE_PERIOD(param_default)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_default) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_default)),
};

static const btn_test_evt_t test_events_click_with_keepalive[] = {
        BTN_EVENT_ONPRESS(),
        BTN_EVENT_ONRELEASE(),
        BTN_EVENT_ONPRESS(),
        /* This one is to handle click before long press */
        BTN_EVENT_ONCLICK(1),
        BTN_EVENT_KEEPALIVE(1),
        BTN_EVENT_KEEPALIVE(2),
        BTN_EVENT_KEEPALIVE(3),
        BTN_EVENT_ONRELEASE(),
};

static btn_test_time_t test_sequence_click_with_short[] = {
        /* Make with short press (shorter than minimum required) */
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) + EBTN_PARAM_TIME_CLICK_MIN(param_default) / 2),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_default) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_default)),
};

static const btn_test_evt_t test_events_click_with_short[] = {
        BTN_EVENT_ONPRESS(),
        BTN_EVENT_ONRELEASE(),
};

static btn_test_time_t test_sequence_click_with_short_with_multi[] = {
        /* Make with short press (shorter than minimum required) */
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) + EBTN_PARAM_TIME_CLICK_MIN(param_default) / 2),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_default) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_default) / 2),
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) + EBTN_PARAM_TIME_CLICK_MIN(param_default)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_default) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_default) / 2),
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) + EBTN_PARAM_TIME_CLICK_MIN(param_default)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_default) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_default)),
};

static const btn_test_evt_t test_events_click_with_short_with_multi[] = {
        /* This one is short... */
        BTN_EVENT_ONPRESS(), BTN_EVENT_ONRELEASE(), BTN_EVENT_ONPRESS(),  BTN_EVENT_ONRELEASE(),
        BTN_EVENT_ONPRESS(), BTN_EVENT_ONRELEASE(), BTN_EVENT_ONCLICK(2),
};

static btn_test_time_t test_sequence_multi_click_with_short[] = {
        /* Make 2 clicks, and 3rd one with short press (shorter than minimum required) */
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) + EBTN_PARAM_TIME_CLICK_MIN(param_default)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_default) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_default) / 2),
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) + EBTN_PARAM_TIME_CLICK_MIN(param_default)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_default) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_default) / 2),
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) + EBTN_PARAM_TIME_CLICK_MIN(param_default) / 2),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_default)),
};

static const btn_test_evt_t test_events_multi_click_with_short[] = {
        BTN_EVENT_ONPRESS(),
        BTN_EVENT_ONRELEASE(),
        BTN_EVENT_ONPRESS(),
        BTN_EVENT_ONRELEASE(),
        /* This one is short... */
        BTN_EVENT_ONPRESS(),
        BTN_EVENT_ONRELEASE(),
        BTN_EVENT_ONCLICK(2),
};

static btn_test_time_t test_sequence_onpress_debounce[] = {
        BTN_STATE(0, 0),
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) / 2),
        BTN_STATE(0, 1),
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) / 2),
        BTN_STATE(0, 1),
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) / 2),
        BTN_STATE(0, 1),
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) + EBTN_PARAM_TIME_CLICK_MIN(param_default)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_default) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_default) + 1),
};

static const btn_test_evt_t test_events_onpress_debounce[] = {
        BTN_EVENT_ONPRESS(),
        BTN_EVENT_ONRELEASE(),
        BTN_EVENT_ONCLICK(1),
};

// for test overflow, make sure enable macro 'EBTN_CONFIG_TIMER_16' or compile with 'make all
// CFLAGS=-DEBTN_CONFIG_TIMER_16'
static btn_test_time_t test_sequence_time_overflow_onpress_debounce[] = {
        BTN_STATE(0, 0x0ffff - (EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) / 2)),
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) + EBTN_PARAM_TIME_CLICK_MIN(param_default)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_default) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_default)),
};

static const btn_test_evt_t test_events_time_overflow_onpress_debounce[] = {
        BTN_EVENT_ONPRESS(),
        BTN_EVENT_ONRELEASE(),
        BTN_EVENT_ONCLICK(1),
};

static btn_test_time_t test_sequence_time_overflow_onpress[] = {
        BTN_STATE(0, 0x0ffff - (EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) + EBTN_PARAM_TIME_CLICK_MIN(param_default) / 2)),
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) + EBTN_PARAM_TIME_CLICK_MIN(param_default)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_default) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_default)),
};

static const btn_test_evt_t test_events_time_overflow_onpress[] = {
        BTN_EVENT_ONPRESS(),
        BTN_EVENT_ONRELEASE(),
        BTN_EVENT_ONCLICK(1),
};

static btn_test_time_t test_sequence_time_overflow_onrelease_muti[] = {
        BTN_STATE(0, 0x0ffff - (EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) + EBTN_PARAM_TIME_CLICK_MIN(param_default) +
                                EBTN_PARAM_TIME_CLICK_MIN(param_default) / 2)),
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) + EBTN_PARAM_TIME_CLICK_MIN(param_default)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_default) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_default)),
};

static const btn_test_evt_t test_events_time_overflow_onrelease_muti[] = {
        BTN_EVENT_ONPRESS(),
        BTN_EVENT_ONRELEASE(),
        BTN_EVENT_ONCLICK(1),
};

static btn_test_time_t test_sequence_time_overflow_keepalive[] = {
        BTN_STATE(0, 0x0ffff - (EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) + EBTN_PARAM_TIME_CLICK_MIN(param_default) +
                                EBTN_PARAM_TIME_KEEPALIVE_PERIOD(param_default) / 2)),
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_default) + EBTN_PARAM_TIME_CLICK_MIN(param_default)),
        BTN_STATE(1, EBTN_PARAM_TIME_KEEPALIVE_PERIOD(param_default)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_default) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_default)),
};

static const btn_test_evt_t test_events_time_overflow_keepalive[] = {
        BTN_EVENT_ONPRESS(),
        BTN_EVENT_KEEPALIVE(1),
        BTN_EVENT_ONRELEASE(),
};

///
/// Test onrelease debounce
///

static btn_test_time_t test_sequence_onrelease_debounce[] = {
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_onrelease_debounce) + EBTN_PARAM_TIME_CLICK_MIN(param_onrelease_debounce)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_onrelease_debounce) / 2),
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_onrelease_debounce)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_onrelease_debounce) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_onrelease_debounce)),
};

static const btn_test_evt_t test_events_onrelease_debounce[] = {
        BTN_EVENT_ONPRESS(),
        BTN_EVENT_ONRELEASE(),
        BTN_EVENT_ONCLICK(1),
};

static btn_test_time_t test_sequence_onrelease_debounce_over[] = {
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_onrelease_debounce) + EBTN_PARAM_TIME_CLICK_MIN(param_onrelease_debounce)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_onrelease_debounce)),
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_onrelease_debounce) + EBTN_PARAM_TIME_CLICK_MIN(param_onrelease_debounce)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_onrelease_debounce) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_onrelease_debounce)),
};

static const btn_test_evt_t test_events_onrelease_debounce_over[] = {
        BTN_EVENT_ONPRESS(), BTN_EVENT_ONRELEASE(), BTN_EVENT_ONPRESS(), BTN_EVENT_ONRELEASE(), BTN_EVENT_ONCLICK(2),
};

static btn_test_time_t test_sequence_onrelease_debounce_time_overflow[] = {
        BTN_STATE(0, 0x0ffff - (EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_onrelease_debounce) + EBTN_PARAM_TIME_CLICK_MIN(param_onrelease_debounce) +
                                EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_onrelease_debounce) / 2)),
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_onrelease_debounce) + EBTN_PARAM_TIME_CLICK_MIN(param_onrelease_debounce)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_onrelease_debounce) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_onrelease_debounce)),
};

static const btn_test_evt_t test_events_onrelease_debounce_time_overflow[] = {
        BTN_EVENT_ONPRESS(),
        BTN_EVENT_ONRELEASE(),
        BTN_EVENT_ONCLICK(1),
};

///
/// Test keepalive with click debounce
///
static btn_test_time_t test_sequence_keepalive_with_click[] = {
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_keepalive_with_click) + EBTN_PARAM_TIME_CLICK_MIN(param_keepalive_with_click)),
        BTN_STATE(1, EBTN_PARAM_TIME_KEEPALIVE_PERIOD(param_keepalive_with_click)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_keepalive_with_click) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_keepalive_with_click)),
};

static const btn_test_evt_t test_events_keepalive_with_click[] = {
        BTN_EVENT_ONPRESS(),
        BTN_EVENT_KEEPALIVE(1),
        BTN_EVENT_ONRELEASE(),
        BTN_EVENT_ONCLICK(1),
};

static btn_test_time_t test_sequence_keepalive_with_click_double[] = {
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_keepalive_with_click) + EBTN_PARAM_TIME_CLICK_MIN(param_keepalive_with_click)),
        BTN_STATE(1, EBTN_PARAM_TIME_KEEPALIVE_PERIOD(param_keepalive_with_click)),
        BTN_STATE(1, EBTN_PARAM_TIME_KEEPALIVE_PERIOD(param_keepalive_with_click)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_keepalive_with_click) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_keepalive_with_click)),
};

static const btn_test_evt_t test_events_keepalive_with_click_double[] = {
        BTN_EVENT_ONPRESS(), BTN_EVENT_KEEPALIVE(1), BTN_EVENT_KEEPALIVE(2), BTN_EVENT_ONRELEASE(), BTN_EVENT_ONCLICK(1),
};

///
/// Test max multi click with 3
///
static btn_test_time_t test_sequence_max_click_3[] = {
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_max_click_3) + EBTN_PARAM_TIME_CLICK_MIN(param_max_click_3)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_max_click_3) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_max_click_3) / 2),
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_max_click_3) + EBTN_PARAM_TIME_CLICK_MIN(param_max_click_3)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_max_click_3) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_max_click_3) / 2),
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_max_click_3) + EBTN_PARAM_TIME_CLICK_MIN(param_max_click_3)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_max_click_3) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_max_click_3)),
};

static const btn_test_evt_t test_events_max_click_3[] = {
        BTN_EVENT_ONPRESS(), BTN_EVENT_ONRELEASE(), BTN_EVENT_ONPRESS(),  BTN_EVENT_ONRELEASE(),
        BTN_EVENT_ONPRESS(), BTN_EVENT_ONRELEASE(), BTN_EVENT_ONCLICK(3),
};

static btn_test_time_t test_sequence_max_click_3_over[] = {
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_max_click_3) + EBTN_PARAM_TIME_CLICK_MIN(param_max_click_3)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_max_click_3) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_max_click_3) / 2),
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_max_click_3) + EBTN_PARAM_TIME_CLICK_MIN(param_max_click_3)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_max_click_3) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_max_click_3) / 2),
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_max_click_3) + EBTN_PARAM_TIME_CLICK_MIN(param_max_click_3)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_max_click_3) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_max_click_3) / 2),
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_max_click_3) + EBTN_PARAM_TIME_CLICK_MIN(param_max_click_3)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_max_click_3) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_max_click_3)),
};

static const btn_test_evt_t test_events_max_click_3_over[] = {
        BTN_EVENT_ONPRESS(),   BTN_EVENT_ONRELEASE(), BTN_EVENT_ONPRESS(), BTN_EVENT_ONRELEASE(), BTN_EVENT_ONPRESS(),
        BTN_EVENT_ONRELEASE(), BTN_EVENT_ONCLICK(3),  BTN_EVENT_ONPRESS(), BTN_EVENT_ONRELEASE(), BTN_EVENT_ONCLICK(1),
};

///
/// Test click multi max with 0
///
static btn_test_time_t test_sequence_click_multi_max_0[] = {
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_click_multi_max_0) + EBTN_PARAM_TIME_CLICK_MIN(param_click_multi_max_0)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_click_multi_max_0) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_click_multi_max_0) / 2),
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_click_multi_max_0) + EBTN_PARAM_TIME_CLICK_MIN(param_click_multi_max_0)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_click_multi_max_0) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_click_multi_max_0) / 2),
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_click_multi_max_0) + EBTN_PARAM_TIME_CLICK_MIN(param_click_multi_max_0)),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_click_multi_max_0) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_click_multi_max_0)),
};

static const btn_test_evt_t test_events_click_multi_max_0[] = {
        BTN_EVENT_ONPRESS(),  BTN_EVENT_ONRELEASE(), BTN_EVENT_ONCLICK(1),  BTN_EVENT_ONPRESS(),  BTN_EVENT_ONRELEASE(),
        BTN_EVENT_ONCLICK(1), BTN_EVENT_ONPRESS(),   BTN_EVENT_ONRELEASE(), BTN_EVENT_ONCLICK(1),
};

///
/// Test max keepalive with 0
///
static btn_test_time_t test_sequence_keep_alive_0[] = {
        BTN_STATE(1, EBTN_PARAM_TIME_DEBOUNCE_PRESS(param_keep_alive_0) + EBTN_PARAM_TIME_CLICK_MIN(param_keep_alive_0)),
        BTN_STATE(1, EBTN_PARAM_TIME_CLICK_MAX(param_keep_alive_0) / 2),
        BTN_STATE(0, EBTN_PARAM_TIME_DEBOUNCE_RELEASE(param_keep_alive_0) + EBTN_PARAM_TIME_CLICK_MULTI_MAX(param_keep_alive_0)),
};

static const btn_test_evt_t test_events_keep_alive_0[] = {
        BTN_EVENT_ONPRESS(),
        BTN_EVENT_ONRELEASE(),
        BTN_EVENT_ONCLICK(1),
};

static btn_test_arr_t test_list[] = {
        TEST_ARRAY_DEFINE(USER_BUTTON_default, test_sequence_single_click, test_events_single_click),
        TEST_ARRAY_DEFINE(USER_BUTTON_default, test_sequence_double_click, test_events_double_click),
        TEST_ARRAY_DEFINE(USER_BUTTON_default, test_sequence_triple_click, test_events_triple_click),
        TEST_ARRAY_DEFINE(USER_BUTTON_default, test_sequence_double_click_critical_time, test_events_double_click_critical_time),
        TEST_ARRAY_DEFINE(USER_BUTTON_default, test_sequence_double_click_critical_time_over, test_events_double_click_critical_time_over),
        TEST_ARRAY_DEFINE(USER_BUTTON_default, test_sequence_click_with_keepalive, test_events_click_with_keepalive),
        TEST_ARRAY_DEFINE(USER_BUTTON_default, test_sequence_click_with_short, test_events_click_with_short),
        TEST_ARRAY_DEFINE(USER_BUTTON_default, test_sequence_click_with_short_with_multi, test_events_click_with_short_with_multi),
        TEST_ARRAY_DEFINE(USER_BUTTON_default, test_sequence_multi_click_with_short, test_events_multi_click_with_short),
        TEST_ARRAY_DEFINE(USER_BUTTON_default, test_sequence_onpress_debounce, test_events_onpress_debounce),
        TEST_ARRAY_DEFINE(USER_BUTTON_default, test_sequence_time_overflow_onpress_debounce, test_events_time_overflow_onpress_debounce),
        TEST_ARRAY_DEFINE(USER_BUTTON_default, test_sequence_time_overflow_onpress, test_events_time_overflow_onpress),
        TEST_ARRAY_DEFINE(USER_BUTTON_default, test_sequence_time_overflow_onrelease_muti, test_events_time_overflow_onrelease_muti),
        TEST_ARRAY_DEFINE(USER_BUTTON_default, test_sequence_time_overflow_keepalive, test_events_time_overflow_keepalive),

        TEST_ARRAY_DEFINE(USER_BUTTON_onrelease_debounce, test_sequence_onrelease_debounce, test_events_onrelease_debounce),
        TEST_ARRAY_DEFINE(USER_BUTTON_onrelease_debounce, test_sequence_onrelease_debounce_over, test_events_onrelease_debounce_over),
        TEST_ARRAY_DEFINE(USER_BUTTON_onrelease_debounce, test_sequence_onrelease_debounce_time_overflow, test_events_onrelease_debounce_time_overflow),

        TEST_ARRAY_DEFINE(USER_BUTTON_keepalive_with_click, test_sequence_keepalive_with_click, test_events_keepalive_with_click),
        TEST_ARRAY_DEFINE(USER_BUTTON_keepalive_with_click, test_sequence_keepalive_with_click_double, test_events_keepalive_with_click_double),

        TEST_ARRAY_DEFINE(USER_BUTTON_max_click_3, test_sequence_max_click_3, test_events_max_click_3),
        TEST_ARRAY_DEFINE(USER_BUTTON_max_click_3, test_sequence_max_click_3_over, test_events_max_click_3_over),

        TEST_ARRAY_DEFINE(USER_BUTTON_click_multi_max_0, test_sequence_click_multi_max_0, test_events_click_multi_max_0),

        TEST_ARRAY_DEFINE(USER_BUTTON_keep_alive_0, test_sequence_keep_alive_0, test_events_keep_alive_0),
};

static btn_test_arr_t *select_test_item;

/* Get button state for given current time */
static uint8_t prv_get_state_for_time(uint16_t key_id, uint32_t time)
{
    uint8_t state = 0;
    uint32_t duration = 0;

    if (select_test_item->test_key_id == key_id)
    {
        // printf("time: %d, key_id: %d\n", time, key_id);
        for (size_t i = 0; i < select_test_item->test_sequence_cnt; ++i)
        {
            if (select_test_item->test_sequence[i].duration == 0)
            {
                continue;
            }
            duration += select_test_item->test_sequence[i].duration + 1; /* Advance time, need add 1 for state switch time. */
            // printf("i: %d, duration: %d\n", i, duration);
            if (time <= duration)
            {
                state = select_test_item->test_sequence[i].state;
                // printf("i: %d, time: %d, duration: %d, state: %d\n", i, time, duration, state);
                // printf("state: %d\n", state);
                break;
            }
        }
    }

    return state;
}

static uint32_t test_get_state_total_duration(void)
{
    uint32_t duration = 0;

    for (size_t i = 0; i < select_test_item->test_sequence_cnt; ++i)
    {
        if (select_test_item->test_sequence[i].duration == 0)
        {
            continue;
        }
        duration += select_test_item->test_sequence[i].duration + 1; /* Advance time, need add 1 for state switch time. */
    }

    return duration;
}

/* Get button state */
static uint8_t prv_btn_get_state(struct ebtn_btn *btn)
{
    uint8_t state = prv_get_state_for_time(btn->key_id, test_processed_time_current);

    (void)btn;
    return state;
}

static uint32_t test_processed_event_time_prev;
static uint32_t test_processed_array_index = 0;
/* Process button event */
static void prv_btn_event(struct ebtn_btn *btn, ebtn_evt_t evt)
{
    const char *s;
    uint32_t color, keepalive_cnt = 0, diff_time;
    const btn_test_evt_t *test_evt_data = NULL;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    if (test_processed_array_index >= select_test_item->test_events_cnt)
    {
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
        printf("[%7u] ERROR! Array index is out of bounds!\r\n", (unsigned)test_processed_time_current);
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }
    else
    {
        test_evt_data = &select_test_item->test_events[test_processed_array_index];
    }

    /* Handle timing */
    diff_time = test_processed_time_current - test_processed_event_time_prev;
    test_processed_event_time_prev = test_processed_time_current;
    keepalive_cnt = btn->keepalive_cnt;

    /* Event type must match */
    ASSERT((test_evt_data != NULL) && (test_evt_data->evt == evt));

    /* Get event string */
    if (evt == EBTN_EVT_KEEPALIVE)
    {
        s = "KEEPALIVE";
        color = FOREGROUND_RED | FOREGROUND_BLUE;

        ASSERT((test_evt_data != NULL) && (test_evt_data->keepalive_cnt == keepalive_cnt));
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

        ASSERT((test_evt_data != NULL) && (test_evt_data->conseq_clicks == btn->click_cnt));
    }
    else
    {
        s = "UNKNOWN";
        color = FOREGROUND_RED;
    }

    SetConsoleTextAttribute(hConsole, color);
    printf("[%7u][%6u] ID(hex):%4x, evt:%10s, keep-alive cnt: %3u, click cnt: %3u\r\n", (unsigned)test_processed_time_current, (unsigned)diff_time, btn->key_id,
           s, (unsigned)keepalive_cnt, (unsigned)btn->click_cnt);

    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    ++test_processed_array_index; /* Go to next step in next event */
}

/**
 * \brief           Test function
 */
int example_test(void)
{
    printf("Test running\r\n");

    for (int index = 0; index < EBTN_ARRAY_SIZE(test_list); index++)
    {
        select_test_item = &test_list[index];

        SUITE_START(select_test_item->test_name);

        // printf("\n");

        // init variable
        test_processed_event_time_prev = 0;
        test_processed_array_index = 0;

        /* Define buttons */
        ebtn_init(btns, EBTN_ARRAY_SIZE(btns), NULL, 0, prv_btn_get_state, prv_btn_event);

        /* Counter simulates ms tick */
        for (uint32_t i = 0; i < MAX_TIME_MS; ++i)
        {
            test_processed_time_current = i; /* Set current time used in callback */
            ebtn_process(i);                 /* Now run processing */

            // printf("time: %d, end: %d, in_process(): %d/%d\n", i, test_processed_array_index >= select_test_item->test_events_cnt
            //     , ebtn_is_btn_in_process(ebtn_get_btn_by_key_id(select_test_item->test_key_id)), ebtn_is_in_process());
            // check end
            if (test_processed_array_index >= select_test_item->test_events_cnt)
            {
                uint32_t duration = test_get_state_total_duration();
                if (i > duration + 1)
                {
                    ASSERT(!ebtn_is_btn_in_process(ebtn_get_btn_by_key_id(select_test_item->test_key_id)));
                    ASSERT(!ebtn_is_in_process());
                }
            }
        }
        ASSERT(test_processed_array_index == select_test_item->test_events_cnt);

        // printf("\n");

        SUITE_END();
    }
    return 0;
}
