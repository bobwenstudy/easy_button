#ifndef _EBTN_H
#define _EBTN_H

#include <stdint.h>
#include <string.h>

#include "bit_array.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// #define EBTN_CONFIG_TIMER_16

// here can change to uint16_t, if you want reduce RAM size.
#ifdef EBTN_CONFIG_TIMER_16
typedef uint16_t ebtn_time_t;
typedef int16_t ebtn_time_sign_t;
#else
typedef uint32_t ebtn_time_t;
typedef int32_t ebtn_time_sign_t;
#endif

/* Forward declarations */
struct ebtn_btn;
struct ebtn;

#define EBTN_MAX_KEYNUM (64)

/**
 * \brief           List of button events
 *
 */
typedef enum
{
    EBTN_EVT_ONPRESS = 0x00, /*!< On press event - sent when valid press is detected */
    EBTN_EVT_ONRELEASE,      /*!< On release event - sent when valid release event is detected (from
                                active to inactive) */
    EBTN_EVT_ONCLICK,        /*!< On Click event - sent when valid sequence of on-press and on-release
                                events occurs */
    EBTN_EVT_KEEPALIVE,      /*!< Keep alive event - sent periodically when button is active */
} ebtn_evt_t;

#define EBTN_EVT_MASK_ONPRESS   (1 << EBTN_EVT_ONPRESS)
#define EBTN_EVT_MASK_ONRELEASE (1 << EBTN_EVT_ONRELEASE)
#define EBTN_EVT_MASK_ONCLICK   (1 << EBTN_EVT_ONCLICK)
#define EBTN_EVT_MASK_KEEPALIVE (1 << EBTN_EVT_KEEPALIVE)

#define EBTN_EVT_MASK_ALL (EBTN_EVT_MASK_ONPRESS | EBTN_EVT_MASK_ONRELEASE | EBTN_EVT_MASK_ONCLICK | EBTN_EVT_MASK_KEEPALIVE)

/**
 * @brief  Returns the difference between two absolute times: time1-time2.
 * @param[in]  time1: Absolute time expressed in internal time units.
 * @param[in]  time2: Absolute time expressed in internal time units.
 * @return resulting signed relative time expressed in internal time units.
 */
static inline ebtn_time_sign_t ebtn_timer_sub(ebtn_time_t time1, ebtn_time_t time2)
{
    return time1 - time2;
}

// test time overflow error
// #define ebtn_timer_sub(time1, time2) (time1 - time2)

/**
 * \brief           Button event function callback prototype
 * \param[in]       btn: Button instance from array for which event occured
 * \param[in]       evt: Event type
 */
typedef void (*ebtn_evt_fn)(struct ebtn_btn *btn, ebtn_evt_t evt);

/**
 * \brief           Get button/input state callback function
 *
 * \param[in]       btn: Button instance from array to read state
 * \return          `1` when button is considered `active`, `0` otherwise
 */
typedef uint8_t (*ebtn_get_state_fn)(struct ebtn_btn *btn);

/**
 * \brief           Button Params structure
 */
typedef struct ebtn_btn_param
{
    /**
     * \brief           Minimum debounce time for press event in units of milliseconds
     *
     * This is the time when the input shall have stable active level to detect valid *onpress*
     * event.
     *
     * When value is set to `> 0`, input must be in active state for at least
     * minimum milliseconds time, before valid *onpress* event is detected.
     *
     * \note            If value is set to `0`, debounce is not used and *press* event will be
     * triggered immediately when input states goes to *inactive* state.
     *
     *                  To be safe not using this feature, external logic must ensure stable
     * transition at input level.
     *
     */
    uint16_t time_debounce; /*!< Debounce time in milliseconds */

    /**
     * \brief           Minimum debounce time for release event in units of milliseconds
     *
     * This is the time when the input shall have minimum stable released level to detect valid
     * *onrelease* event.
     *
     * This setting can be useful if application wants to protect against
     * unwanted glitches on the line when input is considered "active".
     *
     * When value is set to `> 0`, input must be in inactive low for at least
     * minimum milliseconds time, before valid *onrelease* event is detected
     *
     * \note            If value is set to `0`, debounce is not used and *release* event will be
     * triggered immediately when input states goes to *inactive* state
     *
     */
    uint16_t time_debounce_release; /*!< Debounce time in milliseconds for release event  */

    /**
     * \brief           Minimum active input time for valid click event, in milliseconds
     *
     * Input shall be in active state (after debounce) at least this amount of time to even consider
     * the potential valid click event. Set the value to `0` to disable this feature
     *
     */
    uint16_t time_click_pressed_min; /*!< Minimum pressed time for valid click event */

    /**
     * \brief           Maximum active input time for valid click event, in milliseconds
     *
     * Input shall be pressed at most this amount of time to still trigger valid click.
     * Set to `-1` to allow any time triggering click event.
     *
     * When input is active for more than the configured time, click even is not detected and is
     * ignored.
     *
     */
    uint16_t time_click_pressed_max; /*!< Maximum pressed time for valid click event*/

    /**
     * \brief           Maximum allowed time between last on-release and next valid on-press,
     *                  to still allow multi-click events, in milliseconds
     *
     * This value is also used as a timeout length to send the *onclick* event to application from
     * previously detected valid click events.
     *
     * If application relies on multi consecutive clicks, this is the max time to allow user
     * to trigger potential new click, or structure will get reset (before sent to user if any
     * clicks have been detected so far)
     *
     */
    uint16_t time_click_multi_max; /*!< Maximum time between 2 clicks to be considered consecutive
                                      click */

    /**
     * \brief           Keep-alive event period, in milliseconds
     *
     * When input is active, keep alive events will be sent through this period of time.
     * First keep alive will be sent after input being considered
     * active.
     *
     */
    uint16_t time_keepalive_period; /*!< Time in ms for periodic keep alive event */

    /**
     * \brief           Maximum number of allowed consecutive click events,
     *                  before structure gets reset to default value.
     *
     * \note            When consecutive value is reached, application will get notification of
     * clicks. This can be executed immediately after last click has been detected, or after
     * standard timeout (unless next on-press has already been detected, then it is send to
     * application just before valid next press event).
     *
     */
    uint16_t max_consecutive; /*!< Max number of consecutive clicks */
} ebtn_btn_param_t;

#define EBTN_PARAMS_INIT(_time_debounce, _time_debounce_release, _time_click_pressed_min, _time_click_pressed_max, _time_click_multi_max,                      \
                         _time_keepalive_period, _max_consecutive)                                                                                             \
    {                                                                                                                                                          \
        .time_debounce = _time_debounce, .time_debounce_release = _time_debounce_release, .time_click_pressed_min = _time_click_pressed_min,                   \
        .time_click_pressed_max = _time_click_pressed_max, .time_click_multi_max = _time_click_multi_max, .time_keepalive_period = _time_keepalive_period,     \
        .max_consecutive = _max_consecutive                                                                                                                    \
    }

#define EBTN_BUTTON_INIT_RAW(_key_id, _param, _mask)                                                                                                           \
    {                                                                                                                                                          \
        .key_id = _key_id, .param = _param, .event_mask = _mask,                                                                                               \
    }

#define EBTN_BUTTON_INIT(_key_id, _param) EBTN_BUTTON_INIT_RAW(_key_id, _param, EBTN_EVT_MASK_ALL)

#define EBTN_BUTTON_DYN_INIT(_key_id, _param)                                                                                                                  \
    {                                                                                                                                                          \
        .next = NULL, .btn = EBTN_BUTTON_INIT(_key_id, _param),                                                                                                \
    }

#define EBTN_BUTTON_COMBO_INIT_RAW(_key_id, _param, _mask)                                                                                                     \
    {                                                                                                                                                          \
        .comb_key = {0}, .btn = EBTN_BUTTON_INIT_RAW(_key_id, _param, _mask),                                                                                  \
    }

#define EBTN_BUTTON_COMBO_INIT(_key_id, _param)                                                                                                                \
    {                                                                                                                                                          \
        .comb_key = {0}, .btn = EBTN_BUTTON_INIT(_key_id, _param),                                                                                             \
    }

#define EBTN_BUTTON_COMBO_DYN_INIT(_key_id, _param)                                                                                                            \
    {                                                                                                                                                          \
        .next = NULL, .btn = EBTN_BUTTON_COMBO_INIT(_key_id, _param),                                                                                          \
    }

#define EBTN_ARRAY_SIZE(_arr) sizeof(_arr) / sizeof((_arr)[0])

/**
 * \brief           Button structure
 */
typedef struct ebtn_btn
{
    uint16_t key_id;    /*!< User defined custom argument for callback function purpose */
    uint8_t flags;      /*!< Private button flags management */
    uint8_t event_mask; /*!< Private button event mask management */

    ebtn_time_t time_change;       /*!< Time in ms when button state got changed last time after valid
                                   debounce */
    ebtn_time_t time_state_change; /*!< Time in ms when button state got changed last time */

    ebtn_time_t keepalive_last_time; /*!< Time in ms of last send keep alive event */
    ebtn_time_t click_last_time;     /*!< Time in ms of last successfully detected (not sent!) click event
                                      */

    uint16_t keepalive_cnt; /*!< Number of keep alive events sent after successful on-press
                            detection. Value is reset after on-release */
    uint16_t click_cnt;     /*!< Number of consecutive clicks detected, respecting maximum timeout
                        between clicks */

    const ebtn_btn_param_t *param;
} ebtn_btn_t;

/**
 * \brief           ComboButton structure
 */
typedef struct ebtn_btn_combo
{
    BIT_ARRAY_DEFINE(comb_key, EBTN_MAX_KEYNUM); /*!< select key index - `1` means active, `0` means inactive */

    ebtn_btn_t btn;
} ebtn_btn_combo_t;

/**
 * \brief           Dynamic Button structure
 */
typedef struct ebtn_btn_dyn
{
    struct ebtn_btn_dyn *next; /*!< point to next button */

    ebtn_btn_t btn;
} ebtn_btn_dyn_t;

/**
 * \brief           Dynamic ComboButton structure
 */
typedef struct ebtn_btn_combo_dyn
{
    struct ebtn_btn_combo_dyn *next; /*!< point to next combo-button */

    ebtn_btn_combo_t btn;
} ebtn_btn_combo_dyn_t;

/**
 * \brief           easy_button group structure
 */
typedef struct ebtn
{
    ebtn_btn_t *btns;             /*!< Pointer to buttons array */
    uint16_t btns_cnt;            /*!< Number of buttons in array */
    ebtn_btn_combo_t *btns_combo; /*!< Pointer to comb-buttons array */
    uint16_t btns_combo_cnt;      /*!< Number of comb-buttons in array */

    ebtn_btn_dyn_t *btn_dyn_head;             /*!< Pointer to btn-dynamic list */
    ebtn_btn_combo_dyn_t *btn_combo_dyn_head; /*!< Pointer to btn-combo-dynamic list */

    ebtn_evt_fn evt_fn;             /*!< Pointer to event function */
    ebtn_get_state_fn get_state_fn; /*!< Pointer to get state function */

    BIT_ARRAY_DEFINE(old_state, EBTN_MAX_KEYNUM); /*!< Old button state - `1` means active, `0` means inactive */
} ebtn_t;

/**
 * \brief           Button processing function, that reads the inputs and makes actions accordingly.
 *
 *
 * \param[in]       mstime: Current system time in milliseconds
 */
void ebtn_process(ebtn_time_t mstime);

/**
 * \brief           Button processing function, with all button input state.
 *
 * \param[in]       curr_state: Current all button input state
 * \param[in]       mstime: Current system time in milliseconds
 */
void ebtn_process_with_curr_state(bit_array_t *curr_state, ebtn_time_t mstime);

/**
 * \brief           Check if button is active.
 * Active is considered when initial debounce period has been a pass.
 * This is the period between on-press and on-release events.
 *
 * \param[in]       btn: Button handle to check
 * \return          `1` if active, `0` otherwise
 */
int ebtn_is_btn_active(const ebtn_btn_t *btn);

/**
 * \brief           Check if button is in process.
 * Used for low-power processing, indicating that the buttons are temporarily idle, and embedded systems can consider entering deep sleep.
 *
 * \param[in]       btn: Button handle to check
 * \return          `1` if in process, `0` otherwise
 */
int ebtn_is_btn_in_process(const ebtn_btn_t *btn);

/**
 * \brief           Check if some button is in process.
 * Used for low-power processing, indicating that the buttons are temporarily idle, and embedded systems can consider entering deep sleep.
 *
 * \return          `1` if in process, `0` otherwise
 */
int ebtn_is_in_process(void);

/**
 * \brief           Initialize button manager
 * \param[in]       btns: Array of buttons to process
 * \param[in]       btns_cnt: Number of buttons to process
 * \param[in]       btns_combo: Array of combo-buttons to process
 * \param[in]       btns_combo_cnt: Number of combo-buttons to process
 * \param[in]       get_state_fn: Pointer to function providing button state on demand.
 * \param[in]       evt_fn: Button event function callback
 *
 * \return          `1` on success, `0` otherwise
 */
int ebtn_init(ebtn_btn_t *btns, uint16_t btns_cnt, ebtn_btn_combo_t *btns_combo, uint16_t btns_combo_cnt, ebtn_get_state_fn get_state_fn, ebtn_evt_fn evt_fn);

/**
 * @brief Register a dynamic button
 *
 * @param button: Dynamic button structure instance
 * \return          `1` on success, `0` otherwise
 */
int ebtn_register(ebtn_btn_dyn_t *button);

/**
 * \brief           Register a dynamic combo-button
 * \param[in]       button: Dynamic combo-button structure instance
 *
 * \return          `1` on success, `0` otherwise
 */
int ebtn_combo_register(ebtn_btn_combo_dyn_t *button);

/**
 * \brief           Get the current total button cnt
 *
 * \return          size of button.
 */
int ebtn_get_total_btn_cnt(void);

/**
 * \brief           Get the internal key_idx of the key_id
 * \param[in]       key_id: key_id
 *
 * \return          '-1' on error, other is key_idx
 */
int ebtn_get_btn_index_by_key_id(uint16_t key_id);

/**
 * \brief           Get the internal btn instance of the key_id, here is the button instance, and what is dynamically registered is also to obtain its button
 * instance
 *
 * \param[in]       key_id: key_id
 *
 * \return          'NULL' on error, other is button instance
 */
ebtn_btn_t *ebtn_get_btn_by_key_id(uint16_t key_id);

/**
 * \brief           Get the internal key_idx of the button
 * \param[in]       btn: Button
 *
 * \return          '-1' on error, other is key_idx
 */
int ebtn_get_btn_index_by_btn(ebtn_btn_t *btn);

/**
 * \brief           Get the internal key_idx of the dynamic button
 * \param[in]       btn: Button
 *
 * \return          '-1' on error, other is key_idx
 */
int ebtn_get_btn_index_by_btn_dyn(ebtn_btn_dyn_t *btn);

/**
 * \brief           Bind combo-button key with key_idx
 * \param[in]       btn: Combo Button
 * \param[in]       idx: key_idx
 *
 */
void ebtn_combo_btn_add_btn_by_idx(ebtn_btn_combo_t *btn, int idx);

/**
 * \brief           Remove combo-button key with key_idx
 * \param[in]       btn: Combo Button
 * \param[in]       idx: key_idx
 *
 */
void ebtn_combo_btn_remove_btn_by_idx(ebtn_btn_combo_t *btn, int idx);

/**
 * \brief           Bind combo-button key with key_id, make sure key_id(button) is already register.
 * \param[in]       btn: Combo Button
 * \param[in]       key_id: key_id
 *
 */
void ebtn_combo_btn_add_btn(ebtn_btn_combo_t *btn, uint16_t key_id);

/**
 * \brief           Remove combo-button key with key_id, make sure key_id(button) is already
 * register. \param[in]       btn: Combo Button \param[in]       key_id: key_id
 *
 */
void ebtn_combo_btn_remove_btn(ebtn_btn_combo_t *btn, uint16_t key_id);

/**
 * \brief           Get keep alive period for specific button
 * \param[in]       btn: Button instance to get keep alive period for
 * \return          Keep alive period in `ms`
 */
#define ebtn_keepalive_get_period(btn) ((btn)->time_keepalive_period)

/**
 * \brief           Get actual number of keep alive counts since the last on-press event.
 *                  It is set to `0` if btn isn't pressed
 * \param[in]       btn: Button instance to get keep alive period for
 * \return          Number of keep alive events since on-press event
 */
#define ebtn_keepalive_get_count(btn) ((btn)->keepalive_cnt)

/**
 * \brief           Get number of keep alive counts for specific required time in milliseconds.
 *                  It will calculate number of keepalive ticks specific button shall make,
 *                  before requested time is reached.
 *
 * Result of the function can be used with \ref ebtn_keepalive_get_count which returns
 * actual number of keep alive counts since last on-press event of the button.
 *
 * \note            Value is always integer aligned, with granularity of one keepalive time period
 * \note            Implemented as macro, as it may be optimized by compiler when static keep alive
 * is used
 *
 * \param[in]       btn: Button to use for check
 * \param[in]       ms_time: Time in ms to calculate number of keep alive counts
 * \return          Number of keep alive counts
 */
#define ebtn_keepalive_get_count_for_time(btn, ms_time) ((ms_time) / ebtn_keepalive_get_period(btn))

/**
 * \brief           Get number of consecutive click events on a button
 * \param[in]       btn: Button instance to get number of clicks
 * \return          Number of consecutive clicks on a button
 */
#define ebtn_click_get_count(btn) ((btn)->click_cnt)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _EBTN_H */
