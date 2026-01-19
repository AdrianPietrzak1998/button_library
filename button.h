/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Author: Adrian Pietrzak
 * GitHub: https://github.com/AdrianPietrzak1998
 * Created: May 7, 2022
 */

#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

#include "button_cfg.h"
#include <stdint.h>

/**
 * @brief Selects the time source for the state machine tick.
 *
 * If set to 1, the state machine uses a user-provided function to retrieve the
 * current time. If set to 0, the state machine uses a pointer to a global time
 * variable.
 */
#define BT_TICK_FROM_FUNC 0

/**
 * @brief Time base configuration for the state machine module.
 *
 * By default, the system tick type is `uint32_t`. To use a custom type, define
 * `BTN_TIME_BASE_TYPE_CUSTOM` as the desired type (e.g. `uint16_t`, `uint64_t`,
 * etc.) and define the appropriate `_IS_*` macro (e.g.
 * `BTN_TIME_BASE_TYPE_CUSTOM_IS_UINT16`) to allow the code to determine
 * `BTN_MAX_TIMEOUT`. Only unsigned integer types are supported. variables are
 * recommended for time bases for proper overflow handling.
 */
#ifndef BTN_TIME_BASE_TYPE_CUSTOM

#define BTN_MAX_TIMEOUT UINT32_MAX
typedef volatile uint32_t BTN_TIME_t;

#else

typedef BTN_TIME_BASE_TYPE_CUSTOM BTN_TIME_t;

#if defined(SM_TIME_BASE_TYPE_CUSTOM_IS_UINT8)
#define BTN_MAX_TIMEOUT UINT8_MAX
#elif defined(SM_TIME_BASE_TYPE_CUSTOM_IS_UINT16)
#define BTN_MAX_TIMEOUT UINT16_MAX
#elif defined(SM_TIME_BASE_TYPE_CUSTOM_IS_UINT32)
#define BTN_MAX_TIMEOUT UINT32_MAX
#elif defined(SM_TIME_BASE_TYPE_CUSTOM_IS_UINT64)
#define BTN_MAX_TIMEOUT UINT64_MAX
#else
#error "BTN_MAX_TIMEOUT: Unknown BTN_TIME_BASE_TYPE_CUSTOM or missing _IS_* define"
#endif

#endif

/**
 * @brief Enum for button states.
 *
 * This enumeration defines the various states of the button during its
 * lifecycle. It helps track and manage the button's state transitions, such as
 * when it is idle, pressed, or released.
 */
typedef enum
{
    IDLE = 0, /**< Button is in the idle state, no action is taking place. */
    DEBOUNCE, /**< Button is in the debounce state, waiting for the signal to
                 stabilize. */
    PRESSED,  /**< Button has been pressed and is being handled. */
    REPEAT,   /**< Button is being held down and the repeat action is being
                 triggered. */
    RELEASE,  /**< Button has been released. */
#if BTN_DOUBLE_DEBOUNCING
    DEBOUNCE_RELEASE, /**< Button is in the debounce state, waiting for the signal
                         to stabilize. */
#endif
#if BTN_RELEASE_AFTER_REPEAT
    RELEASE_AFTER_REPEAT /**< Button is released after repeat action, if enabled.
                          */
#endif
} ButtonState_t;

/**
 * @brief Enum for GPIO reverse logic.
 *
 * This enumeration defines the logic level for GPIO pin readings.
 * It allows the configuration of whether the button's state is inverted or not.
 */
typedef enum
{
    NON_REVERSE = 0, /**< No reverse logic, the button state follows the actual
                        GPIO state. */
    REVERSE          /**< Reverse logic, the button state is inverted from the actual GPIO
                        state. */
} ReverseLogicGpio_t;

/**
 * @brief Enum for multiple click modes.
 *
 * This enumeration defines the different modes available for handling multiple
 * clicks on a button. It allows the configuration of how the button reacts to
 * multiple consecutive presses.
 */
typedef enum
{
    BTN_MULTIPLE_CLICK_OFF = 0,      /**< No multiple click handling (disabled). */
    BTN_MULTIPLE_CLICK_NORMAL_MODE,  /**< Normal mode for multiple click detection.
                                      */
    BTN_MULTIPLE_CLICK_COMBINED_MODE /**< Combined mode for handling click cycles.
                                      */
} MultipleClickMode_t;

/**
 * @brief Enum for operation status codes.
 * This enumeration defines the status codes returned by button library
 * functions to indicate success or failure of operations.
 * @note BTN_OK indicates a successful operation, while BTN_ERROR indicates an
 * error occurred during the operation.
 */
typedef enum
{
    BTN_OK = 0,   /**< Operation completed successfully. */
    BTN_ERROR = 1 /**< An error occurred during the operation. */
} BTN_operate_status;

/**
 * @brief Button structure used for managing the state and behavior of a button.
 *
 * This structure holds all the necessary information for managing a button,
 * including its current state, GPIO settings, timers for debouncing, long
 * press, repeat, and multiple clicks, as well as function pointers for button
 * event callbacks (e.g., pressed, long pressed, release, etc.).
 *
 * @note The structure is intended to be used with functions for handling button
 * debouncing, state transitions, and event callbacks.
 */
typedef struct
{
    ButtonState_t State;             /**< Current state of the button. */
    GPIO_TypeDef *GpioPort;          /**< GPIO port where the button is connected. */
    BTN_GPIO_PIN_T GpioPin;          /**< GPIO pin where the button is connected. */
    BTN_TIME_t LastTick;             /**< Timestamp of the last button event. */
    BTN_TIME_t TimerDebounce;        /**< Debounce time in milliseconds. */
    BTN_TIME_t TimerLongPressed;     /**< Time threshold for a long press. */
    BTN_TIME_t TimerRepeat;          /**< Repeat time threshold for repeated presses. */
    ReverseLogicGpio_t ReverseLogic; /**< Logic level inversion for the button (if
                                        applicable). */
    uint16_t NumberBtn;              /**< Identifier for the button (used in callbacks). */
#if BTN_DOUBLE_DEBOUNCING
    ButtonState_t StateBeforeRelease;  /**< Previous button state before entering
                                          the release state. */
    BTN_TIME_t TimerSecondDebounce;    /**< Debounce time for the release state in
                                          milliseconds. */
    BTN_TIME_t LastTickSecondDebounce; /**< Timestamp of the last event during the
                                          release debounce phase. */
#endif
    void (*ButtonPressed)(uint16_t);     /**< Callback function for button press event. */
    void (*ButtonLongPressed)(uint16_t); /**< Callback function for long press event. */
    void (*ButtonRepeat)(uint16_t);      /**< Callback function for repeat press event. */
    void (*ButtonRelease)(uint16_t);     /**< Callback function for button release event. */
#if BTN_RELEASE_AFTER_REPEAT
    void (*ButtonReleaseAfterRepeat)(uint16_t); /**< Callback function for release event after repeat. */
#endif
#if BTN_MULTIPLE_CLICK
    void (*ButtonDoubleClick)(uint16_t); /**< Callback function for double-click event. */
    void (*ButtonTripleClick)(uint16_t); /**< Callback function for triple-click event. */

    MultipleClickMode_t MultipleClickMode; /**< Mode for multiple click handling. */
    uint8_t ClickCounter : 6;              /**< Counter for tracking number of clicks. */
    uint8_t ClickCounterCycle : 1;         /**< Flag indicating cycle completion. */
    uint8_t CombinedModeRepeatPressEx : 1; /**< Flag for combined mode repeat
                                              press. */
    uint32_t TimerBetweenClick;            /**< Time between multiple clicks. */
    uint32_t LastClickTick;                /**< Timestamp of the last click event. */
#endif
#if BTN_NON_USED_CALLBACK
    BTN_TIME_t TimerNonUsed;
    void (*ButtonNonUsed)(uint16_t);
#endif
} button_t;

/**
 * @brief Registers the time source for the button library tick mechanism.
 *
 * This function is used to provide the button library with a time base for
 * managing timeouts, delays, and transition timing. Depending on the
 * configuration (`BT_TICK_FROM_FUNC`), the user must provide either a function
 * that returns the current time or a pointer to a variable representing the
 * current time.
 *
 * - If `BT_TICK_FROM_FUNC` is set to 1, call `BT_tick_function_register()` with
 * a function that returns the current tick value.
 * - If `BT_TICK_FROM_FUNC` is set to 0, call `BT_tick_variable_register()` with
 * a pointer to a variable that is periodically updated with the current tick
 * value.
 *
 * @param Function Pointer to the function returning the current time tick (only
 * if `BT_TICK_FROM_FUNC` is 1).
 * @param Variable Pointer to the time tick variable (only if
 * `BT_TICK_FROM_FUNC` is 0).
 * @return BT_OK if registration was successful; an error code otherwise.
 */
#if SM_TICK_FROM_FUNC
BTN_operate_status BTN_tick_function_register(BTN_TIME_t (*Function)(void));
#else
BTN_operate_status BTN_tick_variable_register(BTN_TIME_t *Variable);
#endif

/* ========================== Initialization Functions =========================
 */
/**
 * @brief Initializes a button structure with the provided parameters.
 * @param Key Pointer to the button structure to initialize.
 * @param GpioPort GPIO port where the button is connected.
 * @param GpioPin GPIO pin where the button is connected.
 * @param TimerDebounce Debounce time in milliseconds to filter button noise.
 * @param TimerLongPressed Time in milliseconds to detect a long press.
 * @param TimerRepeat Time in milliseconds for repeated press events.
 * @param ReverseLogic Indicates whether the GPIO uses reverse logic
 * (active-low).
 * @param Number Button identifier passed to callback functions.
 * @retval Status of the initialization:
 *         - `BTN_OK` if initialization is successful.
 *         - `BTN_ERROR` if there was an error during initialization.
 */
BTN_operate_status ButtonInitKey(button_t *Key, GPIO_TypeDef *GpioPort, uint16_t GpioPin, BTN_TIME_t TimerDebounce,
                                 BTN_TIME_t TimerLongPressed, BTN_TIME_t TimerRepeat, ReverseLogicGpio_t ReverseLogic,
                                 uint16_t Number); // Initialization for state machine
#if BTN_DEFAULT_INIT
/**
 * @brief Initializes a button structure with default timing values.
 * @param Key Pointer to the button structure to initialize.
 * @param GpioPort GPIO port where the button is connected.
 * @param GpioPin GPIO pin where the button is connected.
 * @param ReverseLogic Indicates whether the GPIO uses reverse logic
 * (active-low).
 * @param Number Button identifier passed to callback functions.
 *
 * This function behaves similarly to `ButtonInitKey`, but uses predefined
 * default values for debounce time, long press time, and repeat time. These
 * values are defined as macros in the header file.
 * @retval Status of the initialization:
 *         - `BTN_OK` if initialization is successful.
 *         - `BTN_ERROR` if there was an error during initialization.
 */
BTN_operate_status ButtonInitKeyDefault(button_t *Key, GPIO_TypeDef *GpioPort, uint16_t GpioPin,
                                        ReverseLogicGpio_t ReverseLogic, uint16_t Number);
#endif

#if BTN_MULTIPLE_CLICK
/**
 * @brief Sets the multiple click mode and the timer between clicks for the
 * button.
 *
 * This function configures the button to operate in different multiple click
 * modes, such as normal or combined, and sets the time between consecutive
 * clicks.
 *
 * @param Key Pointer to the button structure.
 * @param MultipleClickMode The mode to handle multiple clicks (e.g.,
 * BTN_MULTIPLE_CLICK_OFF, BTN_MULTIPLE_CLICK_NORMAL_MODE).
 * @param TimerBetweenClick Time in milliseconds between two clicks that are
 * considered part of the same multiple click event.
 * @retval Status of the configuration:
 *         - `BTN_OK` if the configuration was successful.
 *         - `BTN_ERROR` if there was an error during configuration.
 */
BTN_operate_status ButtonSetMultipleClick(button_t *Key, MultipleClickMode_t MultipleClickMode,
                                          uint32_t TimerBetweenClick);
#endif

#if BTN_NON_USED_CALLBACK
/**
 * @brief Configures the non-used (idle) behavior for a button.
 *
 * This function sets the duration of inactivity (in milliseconds) required to
 * trigger the non-used callback for a specific button. It also assigns the
 * callback function to be called when the button is considered unused.
 *
 * @param Key Pointer to the button instance.
 * @param Miliseconds Time in milliseconds after which the button is considered
 * non-used. If set to 0, the non-used functionality is disabled for this
 * button.
 * @param Callback Pointer to the function to be executed when the button is
 * unused.
 * @return BTN_OK if the configuration was successful; an error code otherwise.
 *
 * @note This feature is available only if `BTN_NON_USED_CALLBACK` is set to 1.
 */
BTN_operate_status ButtonSetNonUsed(button_t *Key, BTN_TIME_t Miliseconds, void *Callback);
#endif

/* =================================== State machine
 * ================================== */
/**
 * @brief Handles the button state machine and triggers appropriate routines.
 *
 * This function manages the button state transitions and calls the
 * corresponding routine based on the button's current state. It should be
 * called in the main loop or system task.
 *
 * @param Key Pointer to the button structure being processed.
 *
 * @retval Status of the button task:
 *         - `BTN_OK` if the task executed successfully.
 *         - `BTN_ERROR` if there was an error during execution.
 */
BTN_operate_status ButtonTask(button_t *Key); // Task for working state machine

/* ========================== Callback Registration Functions
 * ========================= */
/**
 * @brief Registers a callback function for the button press event.
 *
 * This function registers a user-defined callback function to be called when
 * the button is pressed.
 *
 * @param Key Pointer to the button structure.
 * @param Callback Pointer to the callback function to be executed when the
 * button is pressed.
 * @retval Status of the registration:
 *         - `BTN_OK` if registration was successful.
 *         - `BTN_ERROR` if there was an error during registration.
 */
BTN_operate_status ButtonRegisterPressCallback(button_t *Key, void *Callback);

/**
 * @brief Registers a callback function for the button long press event.
 *
 * This function registers a user-defined callback function to be called when
 * the button is held down for longer than the specified long press time.
 *
 * @param Key Pointer to the button structure.
 * @param Callback Pointer to the callback function to be executed when a long
 * press is detected.
 * @retval Status of the registration:
 *         - `BTN_OK` if registration was successful.
 *         - `BTN_ERROR` if there was an error during registration.
 */
BTN_operate_status ButtonRegisterLongPressedCallback(button_t *Key, void *Callback);

/**
 * @brief Registers a callback function for the button repeat event.
 *
 * This function registers a user-defined callback function to be called when
 * the button is held down and the repeat event occurs after the repeat time is
 * reached.
 *
 * @param Key Pointer to the button structure.
 * @param Callback Pointer to the callback function to be executed during
 * repeat.
 * @retval Status of the registration:
 *         - `BTN_OK` if registration was successful.
 *         - `BTN_ERROR` if there was an error during registration.
 */
BTN_operate_status ButtonRegisterRepeatCallback(button_t *Key, void *Callback);

/**
 * @brief Registers a callback function for the button release event.
 *
 * This function registers a user-defined callback function to be called when
 * the button is released.
 *
 * @param Key Pointer to the button structure.
 * @param Callback Pointer to the callback function to be executed when the
 * button is released.
 * @retval Status of the registration:
 *         - `BTN_OK` if registration was successful.
 *         - `BTN_ERROR` if there was an error during registration.
 */
BTN_operate_status ButtonRegisterReleaseCallback(button_t *Key, void *Callback);

#if BTN_RELEASE_AFTER_REPEAT
/**
 * @brief Registers a callback function for the button release after repeat
 * event.
 *
 * This function registers a user-defined callback function to be called when
 * the button is released after a repeat event has occurred.
 *
 * @param Key Pointer to the button structure.
 * @param Callback Pointer to the callback function to be executed when the
 * button is released after repeat.
 * @retval Status of the registration:
 *         - `BTN_OK` if registration was successful.
 *         - `BTN_ERROR` if there was an error during registration.
 */
BTN_operate_status ButtonRegisterReleaseAfterRepeatCallback(button_t *Key, void *Callback);
#endif

#if BTN_MULTIPLE_CLICK
/**
 * @brief Registers a callback function for the button double-click event.
 *
 * This function registers a user-defined callback function to be called when
 * the button is double-clicked (two button presses within a specified time).
 *
 * @param Key Pointer to the button structure.
 * @param Callback Pointer to the callback function to be executed during
 * double-click.
 * @retval Status of the registration:
 *         - `BTN_OK` if registration was successful.
 *         - `BTN_ERROR` if there was an error during registration.
 */
BTN_operate_status ButtonRegisterDoubleClickCallback(button_t *Key, void *Callback);

/**
 * @brief Registers a callback function for the button triple-click event.
 *
 * This function registers a user-defined callback function to be called when
 * the button is triple-clicked (three button presses within a specified time).
 *
 * @param Key Pointer to the button structure.
 * @param Callback Pointer to the callback function to be executed during
 * triple-click.
 * @retval Status of the registration:
 *         - `BTN_OK` if registration was successful.
 *         - `BTN_ERROR` if there was an error during registration.
 */
BTN_operate_status ButtonRegisterTripleClickCallback(button_t *Key, void *Callback);
#endif

/* ========================== Time Settings Functions =========================
 */
/**
 * @brief Sets the debounce time for the button.
 *
 * This function configures the debounce time, which determines the minimum time
 * between button presses that are considered separate events. This helps in
 * avoiding false triggering due to contact bounce.
 *
 * @param Key Pointer to the button structure.
 * @param Miliseconds Debounce time in milliseconds.
 * @retval Status of the configuration:
 *         - `BTN_OK` if the configuration was successful.
 *         - `BTN_ERROR` if there was an error during configuration.
 */
BTN_operate_status ButtonSetDebounceTime(button_t *Key, BTN_TIME_t Miliseconds);

#if BTN_DOUBLE_DEBOUNCING
/**
 * @brief Sets the debounce time for button release handling.
 *
 * This function configures the debounce time used during the release state
 * when double debouncing is enabled (`BTN_DOUBLE_DEBOUNCING`). It determines
 * how long the button signal must stabilize after being released before
 * transitioning to the final state.
 *
 * @param Key Pointer to the button structure.
 * @param Miliseconds Debounce time in milliseconds.
 * @retval Status of the configuration:
 *         - `BTN_OK` if the configuration was successful.
 *         - `BTN_ERROR` if there was an error during configuration.
 */
BTN_operate_status ButtonSetReleaseDebounceTime(button_t *Key, BTN_TIME_t Miliseconds);
#endif

/**
 * @brief Sets the long press time for the button.
 *
 * This function configures the time threshold for detecting a long press event.
 * If the button is held for longer than the specified time, the long press
 * action will be triggered.
 *
 * @param Key Pointer to the button structure.
 * @param Miliseconds Time for long press detection in milliseconds.
 * @retval Status of the configuration:
 *         - `BTN_OK` if the configuration was successful.
 *         - `BTN_ERROR` if there was an error during configuration.
 */
BTN_operate_status ButtonSetLongPressedTime(button_t *Key, BTN_TIME_t Miliseconds);

/**
 * @brief Sets the repeat time for the button.
 *
 * This function configures the repeat time. When the button is held down, the
 * repeat event will be triggered periodically after the specified time.
 *
 * @param Key Pointer to the button structure.
 * @param Miliseconds Time for repeat detection in milliseconds.
 * @retval Status of the configuration:
 *         - `BTN_OK` if the configuration was successful.
 *         - `BTN_ERROR` if there was an error during configuration.
 */
BTN_operate_status ButtonSetRepeatTime(button_t *Key, BTN_TIME_t Miliseconds);

#if BTN_MULTIPLE_CLICK
/**
 * @brief Sets the time between multiple clicks.
 *
 * This function configures the maximum time allowed between multiple button
 * presses that are considered as distinct clicks (e.g., double click or triple
 * click).
 *
 * @param Key Pointer to the button structure.
 * @param Miliseconds Time between consecutive clicks in milliseconds.
 * @retval Status of the configuration:
 *         - `BTN_OK` if the configuration was successful.
 *         - `BTN_ERROR` if there was an error during configuration.
 */
BTN_operate_status ButtonSetMultipleClickTime(button_t *Key, BTN_TIME_t Miliseconds);
#endif

#endif /* INC_BUTTON_H_ */
