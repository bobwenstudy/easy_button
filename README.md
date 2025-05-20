# 简介

在嵌入式裸机开发中，经常有按键的管理需求，GitHub上已经有蛮多成熟的按键驱动了，但是由于这样那样的问题，最终还是自己实现了一套。本项目地址：[bobwenstudy/easy_button (github.com)](https://github.com/bobwenstudy/easy_button)。
一个网友的分享，MCU应用实践可以参考：[easy_button-Application](https://github.com/Sighthesia/easy_button-Application/tree/main)

项目开发过程中参考了如下几个项目[murphyzhao/FlexibleButton: 灵活的按键处理库（Flexible Button）| 按键驱动 | 支持单击、双击、连击、长按、自动消抖 | 灵活适配中断和低功耗 | 按需实现组合按键 (github.com)](https://github.com/murphyzhao/FlexibleButton)，[0x1abin/MultiButton: Button driver for embedded system (github.com)](https://github.com/0x1abin/MultiButton)和[MaJerle/lwbtn: Lightweight button handler for embedded systems (github.com)](https://github.com/MaJerle/lwbtn)。

其中核心的按键管理机制借鉴的是[lwbtn](https://github.com/MaJerle/lwbtn)，并在其基础上做了比较多的改动，部分事件上报行为和原本处理有些不同。

## 对比分析

下面从几个维度来对比几个开源库的差异。

注意：分析纯属个人观点，如有异议请随时与我沟通。

|                                 | [easy_button](https://github.com/bobwenstudy/easy_button) | [FlexibleButton](https://github.com/murphyzhao/FlexibleButton) | [MultiButton](https://github.com/0x1abin/MultiButton) | [lwbtn](https://github.com/MaJerle/lwbtn) |
| ------------------------------- | --------------------------------------------------------- | ------------------------------------------------------------ | ----------------------------------------------------- | ----------------------------------------- |
| 最大支持按键数                  | 无限                                                      | 32                                                           | 无限                                                  | 无限                                      |
| 按键时间参数独立配置            | 支持                                                      | 支持                                                         | 部分支持                                              | 支持                                      |
| 单个按键RAM Size（Bytes）       | 20(ebtn_btn_t)                                            | 28(flex_button_t)                                            | 44(Button)                                            | 48(lwbtn_btn_t)                           |
| 支持组合按键                    | 支持                                                      | 不支持                                                       | 不支持                                                | 不支持                                    |
| 支持静态注册（可以省Code Size） | 支持                                                      | 不支持                                                       | 不支持                                                | 支持                                      |
| 支持动态注册                    | 支持                                                      | 支持                                                         | 支持                                                  | 不支持                                    |
| 单击最大次数                    | 无限                                                      | 无限                                                         | 2                                                     | 无限                                      |
| 长按种类                        | 无限                                                      | 1                                                            | 1                                                     | 无限                                      |
| 批量扫描支持                    | 支持                                                      | 不支持                                                       | 不支持                                                | 不支持                                    |

可以看出easy_button功能是最全的，并且使用的RAM Size也是最小的，这个在键盘之类有很多按键场景下非常有意义。



## 组合按键支持

现有的项目基本都不支持组合按键，基本都是要求用户根据ID在应用层将多个按键作为一个ID来实现，虽然这样也能实现组合按键的功能需要。

但是这样的实现逻辑不够优雅，并且扫描按键行为的逻辑不可避免有重复的部分，增加了mips。

本项目基于[bit_array_static](https://github.com/bobwenstudy/bit_array_static)实现了优雅的组合按键处理机制，无需重复定义按键扫描逻辑，驱动会利用已经读取到的按键状态来实现组合按键的功能逻辑。



## 长按支持

实际项目中会遇到各种功能需求，如长按3s是功能A，长按5s是功能B，长按30s是功能C。通过`keepalive_cnt`和`time_keepalive_period`的设计，能够支持各种场景的长按功能需要。

如定义`time_keepalive_period=1000`，那么每隔1s会上报一个`KEEPALIVE(EBTN_EVT_KEEPALIVE)`事件，应用层在收到上报事件后，当`keepalive_cnt==3`时，执行功能A；当`keepalive_cnt==5`时，执行功能B；当`keepalive_cnt==30`时，执行功能C。



## 批量扫描支持

现有的按键库都是一个个按键扫描再单独处理，这个在按键比较少的时候，比较好管理，但是在多按键场景下，尤其是矩阵键盘下，这个会大大增加扫描延迟，通过批量扫描支持，可以先在用户层将所有按键状态记录好（用户层根据具体应用优化获取速度），而后一次性将当前状态传给（`ebtn_process_with_curr_state`）驱动。

嵌入式按键处理驱动，支持单击、双击、多击、自动消抖、长按、长长按、超长按 | 低功耗支持 | 组合按键支持 | 静态/动态注册支持



## 简易但灵活的事件类型

参考[lwbtn](https://github.com/MaJerle/lwbtn)实现，当有按键事件发生时，所上报的事件类型只有4种，通过`click_cnt`和`keepalive_cnt`来支持灵活的按键点击和长按功能需要，这样的设计大大简化了代码行为，也大大降低了后续维护成本。

如果用户觉得不好用，也可以在该驱动基础上再封装出自己所需的驱动。

```c
typedef enum
{
    EBTN_EVT_ONPRESS = 0x00, /*!< On press event - sent when valid press is detected */
    EBTN_EVT_ONRELEASE,      /*!< On release event - sent when valid release event is detected (from
                                active to inactive) */
    EBTN_EVT_ONCLICK,   /*!< On Click event - sent when valid sequence of on-press and on-release
                           events occurs */
    EBTN_EVT_KEEPALIVE, /*!< Keep alive event - sent periodically when button is active */
} ebtn_evt_t;
```



## 关于低功耗

参考[Flexible Button](https://github.com/murphyzhao/FlexibleButton)，本按键库是通过不间断扫描的方式来检查按键状态，因此会一直占用 CPU 资源，这对低功耗应用场景是不友好的。为了降低正常工作模式下的功耗，建议合理配置扫描周期（5ms - 20ms），扫描间隙里 CPU 可以进入轻度睡眠。

一般MCU都有深度睡眠模式，这是CPU只能被IO切换唤醒，所以驱动为大家提供了`int ebtn_is_in_process(void)`接口来判断是否可以进入深度睡眠模式。






# 代码结构

代码结构如下所示：

- **ebtn**：驱动库，主要包含BitArray管理和EasyButton管理。
- **example_user.c**：捕获windows的0-9作为按键输入，测试用户交互的例程。
- **example_test.c**：模拟一些场景的按键事件，对驱动进行测试。
- **main.c**：程序主入口，配置进行测试模式函数用户交互模式。
- **build.mk**和**Makefile**：Makefile编译环境。
- **README.md**：说明文档

```shell
easy_button
 ├── ebtn
 │   ├── bit_array.h
 │   ├── ebtn.c
 │   └── ebtn.h
 ├── build.mk
 ├── example_user.c
 └── example_test.c
 ├── main.c
 ├── Makefile
 └── README.md
```







# 使用说明

## 使用简易步骤

Step1：定义KEY_ID、按键参数和按键数组和组合按键数组。

```c

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
        EBTN_BUTTON_INIT(USER_BUTTON_0, &defaul_ebtn_param),
        EBTN_BUTTON_INIT(USER_BUTTON_1, &defaul_ebtn_param),
        EBTN_BUTTON_INIT(USER_BUTTON_2, &defaul_ebtn_param),
        EBTN_BUTTON_INIT(USER_BUTTON_3, &defaul_ebtn_param),
        EBTN_BUTTON_INIT(USER_BUTTON_4, &defaul_ebtn_param),
        EBTN_BUTTON_INIT(USER_BUTTON_5, &defaul_ebtn_param),
};


static ebtn_btn_combo_t btns_combo[] = {
        EBTN_BUTTON_COMBO_INIT(USER_BUTTON_COMBO_0, &defaul_ebtn_param),
        EBTN_BUTTON_COMBO_INIT(USER_BUTTON_COMBO_1, &defaul_ebtn_param),
};
```

Step2：初始化按键驱动

```c
ebtn_init(btns, EBTN_ARRAY_SIZE(btns), btns_combo, EBTN_ARRAY_SIZE(btns_combo),
              prv_btn_get_state, prv_btn_event);
```

Step3：配置组合按键comb_key，必须在按键注册完毕后再配置，不然需要`ebtn_combo_btn_add_btn_by_idx`用这个接口。

```c
ebtn_combo_btn_add_btn(&btns_combo[0], USER_BUTTON_0);
ebtn_combo_btn_add_btn(&btns_combo[0], USER_BUTTON_1);

ebtn_combo_btn_add_btn(&btns_combo[1], USER_BUTTON_2);
ebtn_combo_btn_add_btn(&btns_combo[1], USER_BUTTON_3);
```

Step4：动态注册所需按键，并配置comb_key。

```c
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
```

Step5：启动按键扫描，具体实现可以用定时器做，也可以启任务或者轮询处理。需要注意需要将当前系统时钟`get_tick()`传给驱动接口`ebtn_process`。

```c
while (1)
{
    /* Process forever */
    ebtn_process(get_tick());

    /* Artificial sleep to offload win process */
    Sleep(5);
}
```

具体可以参考`example_user.c`和`example_test.c`的实现。





## key_id和key_idx的说明

为了更好的实现**组合按键**以及**批量扫描**的支持，驱动引入了BitArray来管理按键的历史状态和组合按键信息。这样就间接引入了key_index的概念，其代表独立按键在驱动的位置，该值不可直接设置，是按照一定规则隐式定义的。

key_id是用户定义的，用于标识按键的，该值可以随意更改，但是尽量保证该值独立。

如下图所示，驱动有2个静态注册的按键，还有3个动态注册的按键。每个按键的key_id是随意定义的，但是key_idx却是驱动内部隐式定义的，先是静态数组，而后按照动态数组顺先依次定义。

**注意**：由于组合按键也会用到key_idx的信息，所以动态按键目前并不提供删除按键的行为，这个可能引发一些风险。

![image-20240223103317921](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240223103317921.png)



## 结构体说明

### 按键配置参数结构体说明-ebtn_btn_param_t

按键根据不同的时间触发不同的事件，目前每个按键可以配置的参数如下。

| 名称                   | 说明                                                         |
| ---------------------- | ------------------------------------------------------------ |
| time_debounce          | 防抖处理，按下防抖超时，配置为0，代表不启动                  |
| time_debounce_release  | 防抖处理，松开防抖超时，配置为0，代表不启动                  |
| time_click_pressed_min | 按键超时处理，按键最短时间，配置为0，代表不检查最小值        |
| time_click_pressed_max | 按键超时处理，按键最长时间，配置为0xFFFF，代表不检查最大值，用于区分长按和按键事件。 |
| time_click_multi_max   | 多击处理，两个按键之间认为是连击的超时时间                   |
| time_keepalive_period  | 长按处理，长按周期，每个周期增加keepalive_cnt计数            |
| max_consecutive        | 最大连击次数，配置为0，代表不进行连击检查。                  |



```c
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
```





### 按键控制结构体说明-ebtn_btn_t

每个按键有一个管理结构体，用于记录按键当前状态，按键参数等信息。

| 名称                | 说明                                                         |
| ------------------- | ------------------------------------------------------------ |
| key_id              | 用户定义的key_id信息，该值建议唯一                           |
| flags               | 用于记录一些状态，目前只支持`EBTN_FLAG_ONPRESS_SENT`和`EBTN_FLAG_IN_PROCESS` |
| time_change         | 记录按键按下或者松开状态的时间点                             |
| time_state_change   | 记录按键状态切换时间点（并不考虑防抖，单纯记录状态切换时间点） |
| keepalive_last_time | 长按最后一次上报长按时间的时间点，用于管理keepalive_cnt      |
| click_last_time     | 点击最后一次松开状态的时间点，用于管理click_cnt              |
| keepalive_cnt       | 长按的KEEP_ALIVE次数                                         |
| click_cnt           | 多击的次数                                                   |
| param               | 按键时间参数，指向ebtn_btn_param_t，方便节省RAM，并且多个按键可公用一组参数 |





```c
typedef struct ebtn_btn
{
    uint16_t key_id;         /*!< User defined custom argument for callback function purpose */
    uint16_t flags;          /*!< Private button flags management */
    ebtn_time_t time_change; /*!< Time in ms when button state got changed last time after valid
                             debounce */
    ebtn_time_t time_state_change; /*!< Time in ms when button state got changed last time */

    ebtn_time_t keepalive_last_time; /*!< Time in ms of last send keep alive event */
    ebtn_time_t
            click_last_time; /*!< Time in ms of last successfully detected (not sent!) click event
                              */

    uint16_t keepalive_cnt; /*!< Number of keep alive events sent after successful on-press
                            detection. Value is reset after on-release */
    uint16_t click_cnt;     /*!< Number of consecutive clicks detected, respecting maximum timeout
                        between     clicks */

    const ebtn_btn_param_t *param;
} ebtn_btn_t;
```





### 组合按键控制结构体说明-ebtn_btn_combo_t

每个组合按键有一个管理结构体，用于记录组合按键组合配置参数，以及按键信息。

| 名称     | 说明                              |
| -------- | --------------------------------- |
| comb_key | 用独立按键的key_idx设置的BitArray |
| btn      | ebtn_btn_t管理对象，管理按键状态  |



```c
typedef struct ebtn_btn_combo
{
    BIT_ARRAY_DEFINE(
            comb_key,
            EBTN_MAX_KEYNUM); /*!< select key index - `1` means active, `0` means inactive */

    ebtn_btn_t btn;
} ebtn_btn_combo_t;
```





### 动态注册按键控制结构体说明-ebtn_btn_dyn_t

动态注册需要维护一个列表，所以需要一个next指针。

| 名称 | 说明                             |
| ---- | -------------------------------- |
| next | 用于链表链接每个节点             |
| btn  | ebtn_btn_t管理对象，管理按键状态 |



```c
typedef struct ebtn_btn_dyn
{
    struct ebtn_btn_dyn *next;

    ebtn_btn_t btn;
} ebtn_btn_dyn_t;
```





### 动态注册组合按键控制结构体说明-ebtn_btn_combo_dyn_t

动态注册需要维护一个列表，所以需要一个next指针。

| 名称 | 说明                                       |
| ---- | ------------------------------------------ |
| next | 用于链表链接每个节点                       |
| btn  | ebtn_btn_combo_t管理对象，管理组合按键状态 |



```c
typedef struct ebtn_btn_combo_dyn
{
    struct ebtn_btn_combo_dyn *next; /*!< point to next combo-button */

    ebtn_btn_combo_t btn;
} ebtn_btn_combo_dyn_t;
```







### 按键驱动管理结构体-ebtn_t

按键驱动需要管理所有静态注册和动态注册的按键和组合按键信息，并且记录接口以及最后的按键状态。

| 名称               | 说明                           |
| ------------------ | ------------------------------ |
| btns               | 管理静态注册按键的指针         |
| btns_cnt           | 记录静态注册按键的个数         |
| btns_combo         | 管理静态注册组合按键的指针     |
| btns_combo_cnt     | 记录静态注册组合按键的个数     |
| btn_dyn_head       | 管理动态注册按键的列表指针     |
| btn_combo_dyn_head | 管理动态注册组合按键的列表指针 |
| evt_fn             | 事件上报的回调接口             |
| get_state_fn       | 按键状态获取的回调接口         |
| old_state          | 记录按键上一次状态             |



```c
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

    BIT_ARRAY_DEFINE(
            old_state,
            EBTN_MAX_KEYNUM); /*!< Old button state - `1` means active, `0` means inactive */
} ebtn_t;
```



## 操作API



### 核心API

主要的就是初始化和运行接口，加上动态注册接口。

```c
void ebtn_process(ebtn_time_t mstime);
int ebtn_init(ebtn_btn_t *btns, uint16_t btns_cnt, ebtn_btn_combo_t *btns_combo,
              uint16_t btns_combo_cnt, ebtn_get_state_fn get_state_fn, ebtn_evt_fn evt_fn);
int ebtn_register(ebtn_btn_dyn_t *button);
int ebtn_combo_register(ebtn_btn_combo_dyn_t *button);
```



### 组合按键注册key的API

用于给组合按键绑定btn使用，最终都是关联到`key_idx`上。

**注意**，`key_id`注册接口必需先确保对应的Button已经注册到驱动中。

```c
void ebtn_combo_btn_add_btn_by_idx(ebtn_btn_combo_t *btn, int idx);
void ebtn_combo_btn_remove_btn_by_idx(ebtn_btn_combo_t *btn, int idx);
void ebtn_combo_btn_add_btn(ebtn_btn_combo_t *btn, uint16_t key_id);
void ebtn_combo_btn_remove_btn(ebtn_btn_combo_t *btn, uint16_t key_id);
```



### 其他API

一些工具函数，按需使用。

```c
void ebtn_process_with_curr_state(bit_array_t *curr_state, ebtn_time_t mstime);

int ebtn_get_total_btn_cnt(void);
int ebtn_get_btn_index_by_key_id(uint16_t key_id);
ebtn_btn_t *ebtn_get_btn_by_key_id(uint16_t key_id);
int ebtn_get_btn_index_by_btn(ebtn_btn_t *btn);
int ebtn_get_btn_index_by_btn_dyn(ebtn_btn_dyn_t *btn);

int ebtn_is_btn_active(const ebtn_btn_t *btn);
int ebtn_is_btn_in_process(const ebtn_btn_t *btn);
int ebtn_is_in_process(void);
```



其中`ebtn_is_in_process()`可以用于超低功耗业务场景，这时候MCU只有靠IO翻转唤醒。





# 按键核心处理逻辑说明

这里参考[用户手册 — LwBTN 文档 (majerle.eu)](https://docs.majerle.eu/projects/lwbtn/en/latest/user-manual/index.html#how-it-works)对本驱动的按键实现机制进行说明。

在驱动运行中，应用程序可以会接收到如下事件：

- `EBTN_EVT_ONPRESS`(简称：`ONPRESS`)，每当输入从非活动状态变为活动状态并且最短去抖动时间过去时，都会将事件发送到应用程序
- `EBTN_EVT_ONRELEASE`(简称：`ONRELEASE`)，每当输入发送 `ONPRESS`事件时，以及当输入从活动状态变为非活动状态时，都会将事件发送到应用程序
- `EBTN_EVT_KEEPALIVE`(简称：`KEEPALIVE`)，事件在 `ONPRESS` 和`ONRELEASE`事件之间定期发送
- `EBTN_EVT_ONCLICK`(简称：`ONCLICK`)，事件在`ONRELEASE`后发送，并且仅当活动按钮状态在有效单击事件的允许窗口内时发送。

## ONPRESS事件

`ONPRESS` 事件是检测到按键处于活动状态时的第一个事件。 由于嵌入式系统的性质和连接到设备的各种按钮，有必要过滤掉潜在的噪音，以忽略无意的多次按下。 这是通过检查输入至少在一些最短时间内处于稳定水平来完成的，通常称为*消抖时间*，通常需要大约`20ms` 。

按键*消抖时间*分为按下消抖时间`time_debounce`和松开消抖时间`time_debounce_release`。

![image-20240223135908798](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240223135908798.png)

## ONRELEASE事件

当按键从活动状态变为非活动状态时，才会立即触发 `ONRELEASE`事件，前提是在此之前检测到`ONPRESS` 事件。也就是 `ONRELEASE`事件是伴随着`ONPRESS` 事件发生的。

![image-20240223143215840](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240223143215840.png)

## ONCLICK事件

`ONCLICK`事件在多个事件组合后触发：

- 应正确检测到`ONPRESS` 事件，表示按钮已按下
- 应检测到`ONRELEASE`事件，表示按钮已松开
- `ONPRESS`和`ONRELEASE`事件之间的时间必须在时间窗口内，也就是在`time_click_pressed_min`和`time_click_pressed_max`之间时。

当满足条件时，在`ONRELEASE`事件之后的`time_click_multi_max`时间，发送`ONCLICK`事件。

![image-20240223143426179](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240223143426179.png)

下面显示了在 Windows 测试下的单击事件演示。

![image-20240223173405665](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240223173405665.png)





## Multi-Click事件

实际需求除了单击需求外，还需要满足多击需求。本驱动是靠`time_click_multi_max`来满足此功能，虽然有多次点击，但是只发送**一次 `ONCLICK`事件**。

注意：想象一下，有一个按钮可以在单击时切换一盏灯，并在双击时关闭房间中的所有灯。 通过超时功能和单次点击通知，用户将只收到**一次点击**，并且会根据连续按压次数值，来执行适当的操作。

下面是**Multi-Click**的简化图，忽略了消抖时间。`click_cnt`表示检测到的**Multi-Click** 事件数，将在最终的`ONCLICK`事件中上报。

需要注意前一个按键的`ONRELEASE`事件和下次的`ONPRESS`事件间隔时间应小于`time_click_multi_max`，`ONCLICK`事件会在最后一次按键的`ONRELEASE`事件之后`time_click_multi_max`时间上报。

![image-20240223144701688](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240223144701688.png)

下面显示了在 Windows 测试下的三击事件演示。

![image-20240223173435824](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240223173435824.png)



## KEEPALIVE事件

`KEEPALIVE`事件在 `ONPRESS`事件和`ONRELEASE`事件之间定期发送，它可用于长按处理，根据过程中有多少`KEEPALIVE`事件以及`time_keepalive_period`可以实现各种复杂的长按功能需求。

需要注意这里根据配置的时间参数的不同，可能会出现`KEEPALIVE`事件和`ONCLICK`事件在一次按键事件都上报的情况。这个情况一般发生在按下保持时间（`ONPRESS`事件和`ONRELEASE`事件之间）大于`time_keepalive_period`却小于`time_click_pressed_max`的场景下。

![image-20240223173042135](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240223173042135.png)

下面显示了在 Windows 测试下的`KEEPALIVE`事件和`ONCLICK`事件在一次按键事件出现的演示。

![image-20240223173002558](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240223173002558.png)





而当按下保持时间大于`time_click_pressed_max`时，就不会上报`ONCLICK`事件，如下图所示。

![image-20240227112945641](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240227112945641.png)



## 其他边界场景

在`example_test.c`中对一些场景进行了覆盖性测试，具体可以看代码实现，测试都符合预期。

**注意**：time_overflow相关的case需要`EBTN_CONFIG_TIMER_16`宏，不然测试时间太长了。







# 测试说明

## 环境搭建

目前测试暂时只支持Windows编译，最终生成exe，可以直接在PC上跑。

目前需要安装如下环境：
- GCC环境，笔者用的msys64+mingw，用于编译生成exe，参考这个文章安装即可。[Win7下msys64安装mingw工具链 - Milton - 博客园 (cnblogs.com)](https://www.cnblogs.com/milton/p/11808091.html)。


## 编译说明

本项目都是由makefile组织编译的，编译整个项目只需要执行`make all`即可。


也就是可以通过如下指令来编译工程：

```shell
make all
```

而后运行执行`make run`即可运行例程，例程默认运行测试例程，覆盖绝大多数场景，从结果上看测试通过。

```shell
PS D:\workspace\github\easy_button> make run
Building   : "output/main.exe"
Start Build Image.
objcopy -v -O binary output/main.exe output/main.bin
copy from `output/main.exe' [pei-i386] to `output/main.bin' [binary]
objdump --source --all-headers --demangle --line-numbers --wide output/main.exe > output/main.lst
Print Size
   text    data     bss     dec     hex filename
  49616    6572    2644   58832    e5d0 output/main.exe
./output/main.exe
Test running
[     20][    20] ID(hex):   0, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   0
[     42][    22] ID(hex):   0, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   0
[    242][   200] ID(hex):   0, evt:   ONCLICK, keep-alive cnt:   0, click cnt:   1
Testing test_sequence_single_click ......................................... pass
[     20][    20] ID(hex):   0, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   0
[     42][    22] ID(hex):   0, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   0
[    163][   121] ID(hex):   0, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   1
[    184][    21] ID(hex):   0, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   1
[    384][   200] ID(hex):   0, evt:   ONCLICK, keep-alive cnt:   0, click cnt:   2
Testing test_sequence_double_click ......................................... pass
[     20][    20] ID(hex):   0, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   0
[     42][    22] ID(hex):   0, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   0
[    163][   121] ID(hex):   0, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   1
[    184][    21] ID(hex):   0, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   1
[    305][   121] ID(hex):   0, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   2
[    326][    21] ID(hex):   0, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   2
[    526][   200] ID(hex):   0, evt:   ONCLICK, keep-alive cnt:   0, click cnt:   3
Testing test_sequence_triple_click ......................................... pass
[     20][    20] ID(hex):   0, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   0
[     42][    22] ID(hex):   0, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   0
[    241][   199] ID(hex):   0, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   1
[    262][    21] ID(hex):   0, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   1
[    462][   200] ID(hex):   0, evt:   ONCLICK, keep-alive cnt:   0, click cnt:   2
Testing test_sequence_double_click_critical_time ........................... pass
[     20][    20] ID(hex):   0, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   0
[     42][    22] ID(hex):   0, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   0
[    243][   201] ID(hex):   0, evt:   ONCLICK, keep-alive cnt:   0, click cnt:   1
[    243][     0] ID(hex):   0, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   0
[    264][    21] ID(hex):   0, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   0
[    464][   200] ID(hex):   0, evt:   ONCLICK, keep-alive cnt:   0, click cnt:   1
Testing test_sequence_double_click_critical_time_over ...................... pass
[     20][    20] ID(hex):   0, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   0
[     42][    22] ID(hex):   0, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   0
[    163][   121] ID(hex):   0, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   1
[    464][   301] ID(hex):   0, evt:   ONCLICK, keep-alive cnt:   0, click cnt:   1
[    663][   199] ID(hex):   0, evt: KEEPALIVE, keep-alive cnt:   1, click cnt:   0
[   1163][   500] ID(hex):   0, evt: KEEPALIVE, keep-alive cnt:   2, click cnt:   0
[   1663][   500] ID(hex):   0, evt: KEEPALIVE, keep-alive cnt:   3, click cnt:   0
[   1667][     4] ID(hex):   0, evt: ONRELEASE, keep-alive cnt:   3, click cnt:   0
Testing test_sequence_click_with_keepalive ................................. pass
[     20][    20] ID(hex):   0, evt:   ONPRESS, keep-alive cnt:   3, click cnt:   0
[     32][    12] ID(hex):   0, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   0
Testing test_sequence_click_with_short ..................................... pass
[     20][    20] ID(hex):   0, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   0
[     32][    12] ID(hex):   0, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   0
[    153][   121] ID(hex):   0, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   0
[    174][    21] ID(hex):   0, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   0
[    295][   121] ID(hex):   0, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   1
[    316][    21] ID(hex):   0, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   1
[    516][   200] ID(hex):   0, evt:   ONCLICK, keep-alive cnt:   0, click cnt:   2
Testing test_sequence_click_with_short_with_multi .......................... pass
[     20][    20] ID(hex):   0, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   0
[     42][    22] ID(hex):   0, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   0
[    163][   121] ID(hex):   0, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   1
[    184][    21] ID(hex):   0, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   1
[    305][   121] ID(hex):   0, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   2
[    316][    11] ID(hex):   0, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   2
[    316][     0] ID(hex):   0, evt:   ONCLICK, keep-alive cnt:   0, click cnt:   2
Testing test_sequence_multi_click_with_short ............................... pass
[     60][    60] ID(hex):   0, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   0
[     81][    21] ID(hex):   0, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   0
[    281][   200] ID(hex):   0, evt:   ONCLICK, keep-alive cnt:   0, click cnt:   1
Testing test_sequence_onpress_debounce ..................................... pass
[  65547][ 65547] ID(hex):   0, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   0
[  65568][    21] ID(hex):   0, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   0
[  65768][   200] ID(hex):   0, evt:   ONCLICK, keep-alive cnt:   0, click cnt:   1
Testing test_sequence_time_overflow_onpress_debounce ....................... pass
[  65527][ 65527] ID(hex):   0, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   0
[  65548][    21] ID(hex):   0, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   0
[  65748][   200] ID(hex):   0, evt:   ONCLICK, keep-alive cnt:   0, click cnt:   1
Testing test_sequence_time_overflow_onpress ................................ pass
[  65507][ 65507] ID(hex):   0, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   0
[  65528][    21] ID(hex):   0, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   0
[  65728][   200] ID(hex):   0, evt:   ONCLICK, keep-alive cnt:   0, click cnt:   1
Testing test_sequence_time_overflow_onrelease_muti ......................... pass
[  65267][ 65267] ID(hex):   0, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   0
[  65767][   500] ID(hex):   0, evt: KEEPALIVE, keep-alive cnt:   1, click cnt:   0
[  65789][    22] ID(hex):   0, evt: ONRELEASE, keep-alive cnt:   1, click cnt:   0
Testing test_sequence_time_overflow_keepalive .............................. pass
[     20][    20] ID(hex):   1, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   0
[    164][   144] ID(hex):   1, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   0
[    364][   200] ID(hex):   1, evt:   ONCLICK, keep-alive cnt:   0, click cnt:   1
Testing test_sequence_onrelease_debounce ................................... pass
[     20][    20] ID(hex):   1, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   0
[    102][    82] ID(hex):   1, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   0
[    123][    21] ID(hex):   1, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   1
[    204][    81] ID(hex):   1, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   1
[    404][   200] ID(hex):   1, evt:   ONCLICK, keep-alive cnt:   0, click cnt:   2
Testing test_sequence_onrelease_debounce_over .............................. pass
[  65497][ 65497] ID(hex):   1, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   0
[  65578][    81] ID(hex):   1, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   0
[  65778][   200] ID(hex):   1, evt:   ONCLICK, keep-alive cnt:   0, click cnt:   1
Testing test_sequence_onrelease_debounce_time_overflow ..................... pass
[     20][    20] ID(hex):   2, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   0
[    120][   100] ID(hex):   2, evt: KEEPALIVE, keep-alive cnt:   1, click cnt:   0
[    203][    83] ID(hex):   2, evt: ONRELEASE, keep-alive cnt:   1, click cnt:   0
[    403][   200] ID(hex):   2, evt:   ONCLICK, keep-alive cnt:   1, click cnt:   1
Testing test_sequence_keepalive_with_click ................................. pass
[     20][    20] ID(hex):   2, evt:   ONPRESS, keep-alive cnt:   1, click cnt:   0
[    120][   100] ID(hex):   2, evt: KEEPALIVE, keep-alive cnt:   1, click cnt:   0
[    220][   100] ID(hex):   2, evt: KEEPALIVE, keep-alive cnt:   2, click cnt:   0
[    304][    84] ID(hex):   2, evt: ONRELEASE, keep-alive cnt:   2, click cnt:   0
[    504][   200] ID(hex):   2, evt:   ONCLICK, keep-alive cnt:   2, click cnt:   1
Testing test_sequence_keepalive_with_click_double .......................... pass
[     20][    20] ID(hex):   3, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   0
[    102][    82] ID(hex):   3, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   0
[    223][   121] ID(hex):   3, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   1
[    304][    81] ID(hex):   3, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   1
[    425][   121] ID(hex):   3, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   2
[    506][    81] ID(hex):   3, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   2
[    506][     0] ID(hex):   3, evt:   ONCLICK, keep-alive cnt:   0, click cnt:   3
Testing test_sequence_max_click_3 .......................................... pass
[     20][    20] ID(hex):   3, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   0
[    102][    82] ID(hex):   3, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   0
[    223][   121] ID(hex):   3, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   1
[    304][    81] ID(hex):   3, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   1
[    425][   121] ID(hex):   3, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   2
[    506][    81] ID(hex):   3, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   2
[    506][     0] ID(hex):   3, evt:   ONCLICK, keep-alive cnt:   0, click cnt:   3
[    627][   121] ID(hex):   3, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   0
[    708][    81] ID(hex):   3, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   0
[    908][   200] ID(hex):   3, evt:   ONCLICK, keep-alive cnt:   0, click cnt:   1
Testing test_sequence_max_click_3_over ..................................... pass
[     20][    20] ID(hex):   4, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   0
[    102][    82] ID(hex):   4, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   0
[    123][    21] ID(hex):   4, evt:   ONCLICK, keep-alive cnt:   0, click cnt:   1
[    123][     0] ID(hex):   4, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   0
[    204][    81] ID(hex):   4, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   0
[    225][    21] ID(hex):   4, evt:   ONCLICK, keep-alive cnt:   0, click cnt:   1
[    225][     0] ID(hex):   4, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   0
[    306][    81] ID(hex):   4, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   0
[    307][     1] ID(hex):   4, evt:   ONCLICK, keep-alive cnt:   0, click cnt:   1
Testing test_sequence_click_multi_max_0 .................................... pass
[     20][    20] ID(hex):   5, evt:   ONPRESS, keep-alive cnt:   0, click cnt:   0
[    303][   283] ID(hex):   5, evt: ONRELEASE, keep-alive cnt:   0, click cnt:   0
[    503][   200] ID(hex):   5, evt:   ONCLICK, keep-alive cnt:   0, click cnt:   1
Testing test_sequence_keep_alive_0 ......................................... pass
Executing 'run: all' complete!
```

当然可以用windows的按键进行交互测试，详见`example_user.c`的处理，`main.c`选择调用`example_user()`。













