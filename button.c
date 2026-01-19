/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Author: Adrian Pietrzak
 * GitHub: https://github.com/AdrianPietrzak1998
 * Created: May 7, 2022
 */
#include "button.h"

#if BTN_FORCE_NON_HAL
#undef USE_HAL_DRIVER
#define HAL_TO_DEFINE
#endif

#ifdef USE_HAL_DRIVER
#define BTN_SET GPIO_PIN_SET
#define BTN_RESET GPIO_PIN_RESET
#else
#define BTN_SET 1
#define BTN_RESET 0
#endif

#if SM_TICK_FROM_FUNC
static BTN_TIME_t (*BTN_get_tick)(void) = NULL;
#define BTN_GET_TICK ((SM_get_tick != NULL) ? BTN_get_tick() : ((BTN_TIME_t)0))
BTN_operate_status BTN_tick_function_register(BTN_TIME_t (*Function)(void))
{
    if (Function == NULL)
    {
        return BTN_ERROR;
    }

    BTN_get_tick = Function;
    return BTN_OK;
}
#else
BTN_TIME_t *BTN_tick = NULL;
#define BTN_GET_TICK (*(BTN_tick))
BTN_operate_status BTN_tick_variable_register(BTN_TIME_t *Variable)
{
    if (Variable == NULL)
    {
        return BTN_ERROR;
    }

    BTN_tick = Variable;
    return BTN_OK;
}
#endif

/* ========================== Helper Functions ========================= */
/**
 * @brief Reads the state of a specified GPIO pin.
 *
 * This function reads the input state of the specified GPIO pin. It checks the
 * pin's state and returns either `BTN_SET` or `BTN_RESET`. The function adapts
 * depending on whether the HAL driver is used or direct register access is
 * used.
 *
 * @param GPIOx Pointer to the GPIO port (e.g., GPIOA, GPIOB).
 * @param GPIO_Pin The GPIO pin number (e.g., GPIO_PIN_0, GPIO_PIN_1).
 * @return The state of the pin:
 *         - `BTN_SET` if the pin is high.
 *         - `BTN_RESET` if the pin is low.
 */
static uint8_t ReadState(BTN_GPIO_PORT_T *GPIOx, BTN_GPIO_PIN_T GPIO_Pin)
{
#if BTN_USER_READ_PIN_ROUTINE

#else
#ifdef USE_HAL_DRIVER
    return HAL_GPIO_ReadPin(GPIOx, GPIO_Pin);
#else
    if ((GPIOx->IDR & GPIO_Pin) != 0U)
    {
        return BTN_SET;
    }
    else
    {
        return BTN_RESET;
    }
#endif
#endif
}

#if BTN_MULTIPLE_CLICK
/**
 * @brief Helper function for handling multiple button clicks during the
 * debounce state.
 *
 * This function processes multiple button clicks based on the current multiple
 * click mode and the time elapsed between successive clicks. It updates the
 * click counter and triggers appropriate callback functions for single, double,
 * or triple clicks, depending on the configuration.
 *
 * Multiple click modes:
 * - `BTN_MULTIPLE_CLICK_OFF`: No multiple click handling, processes single
 * clicks only.
 * - `BTN_MULTIPLE_CLICK_NORMAL_MODE`: Handles single, double, and triple clicks
 * with individual callbacks.
 * - `BTN_MULTIPLE_CLICK_COMBINED_MODE`: Aggregates multiple clicks into a
 * combined mode with optional maximum click handling.
 *
 * @param Key Pointer to the button structure for which multiple clicks are
 * being processed.
 *
 * @note This function is static and used internally within the debounce state
 * logic. It is called automatically as part of the button state machine.
 *
 * @return None
 */
static void MultipleClickDebounce(button_t *Key)
{
    if (Key->MultipleClickMode == BTN_MULTIPLE_CLICK_OFF)
    {
        Key->LastTick = BTN_GET_TICK;
        if (Key->ButtonPressed != NULL)
        {
            Key->ButtonPressed(Key->NumberBtn);
        }
        return;
    }
    else if (Key->MultipleClickMode == BTN_MULTIPLE_CLICK_NORMAL_MODE)
    {
        Key->LastTick = BTN_GET_TICK;
        if (Key->ButtonPressed != NULL)
        {
            Key->ButtonPressed(Key->NumberBtn);
        }
        if (BTN_GET_TICK - Key->LastClickTick <= Key->TimerBetweenClick)
        {
            Key->ClickCounter++;
            if (Key->ClickCounter > 3)
            {
                Key->ClickCounter = 0;
                return;
            }
            switch (Key->ClickCounter)
            {
            case 2:
                if (Key->ButtonDoubleClick != NULL)
                    Key->ButtonDoubleClick(Key->NumberBtn);
                break;
            case 3:
                if (Key->ButtonTripleClick != NULL)
                    Key->ButtonTripleClick(Key->NumberBtn);
                break;
            default:
                break;
            }
        }
        else
            Key->ClickCounter = 0;
    }
    else if (Key->MultipleClickMode == BTN_MULTIPLE_CLICK_COMBINED_MODE && Key->ClickCounterCycle == 0)
    {
        Key->ClickCounterCycle = 1;
        if (BTN_GET_TICK - Key->LastClickTick <= Key->TimerBetweenClick)
        {
            Key->ClickCounter++;
            if (Key->ClickCounter > 3)
            {
#if BTN_MULTIPLE_CLICK_COMBINED_TO_MUCH_AS_TRIPLE
                Key->ClickCounter = 3;
#else
                Key->ClickCounter = 0;
#endif
            }
        }
        else
            Key->ClickCounter = 1;
    }
}
/**
 * @brief Helper function for handling multiple button clicks during the repeat
 * state.
 *
 * This function processes multiple button clicks when the button is in the
 * repeat state and the multiple click mode is set to
 * `BTN_MULTIPLE_CLICK_COMBINED_MODE`. It evaluates the number of clicks
 * registered within the allowed time frame (`TimerBetweenClick`) and triggers
 * the appropriate callback functions for single, double, or triple clicks.
 *
 * Actions performed:
 * - Resets the combined mode repeat press flag and click cycle counter.
 * - Checks the elapsed time since the last click (`LastClickTick`).
 * - Invokes the callback function corresponding to the number of clicks
 * detected.
 *
 * @param Key Pointer to the button structure for which multiple clicks are
 * being processed.
 *
 * @note This function is static and is part of the internal button state
 * machine logic. It only processes clicks when the combined mode is active.
 *
 * @return None
 */
static void multipleClikIdle(button_t *Key)
{
    if (Key->MultipleClickMode != BTN_MULTIPLE_CLICK_COMBINED_MODE)
        return;
    Key->CombinedModeRepeatPressEx = 0;
    Key->ClickCounterCycle = 0;
    if (BTN_GET_TICK - Key->LastClickTick > Key->TimerBetweenClick)
    {
        switch (Key->ClickCounter)
        {
        case 1:
            if (Key->ButtonPressed != NULL)
                Key->ButtonPressed(Key->NumberBtn);
            break;
        case 2:
            if (Key->ButtonDoubleClick != NULL)
                Key->ButtonDoubleClick(Key->NumberBtn);
            break;
        case 3:
            if (Key->ButtonTripleClick != NULL)
                Key->ButtonTripleClick(Key->NumberBtn);
            break;
        default:
            break;
        }
        Key->ClickCounter = 0;
    }
}

/**
 * @brief Helper function for handling multiple button clicks during the repeat
 * state.
 *
 * This function manages the behavior of the button in
 * `BTN_MULTIPLE_CLICK_COMBINED_MODE` during the repeat state. It ensures the
 * single press callback (`ButtonPressed`) is executed only once per repeat
 * cycle and resets the click counter.
 *
 * Actions performed:
 * - Invokes the single press callback if it hasn't already been triggered in
 * the current repeat cycle.
 * - Sets the `CombinedModeRepeatPressEx` flag to prevent duplicate callback
 * executions.
 * - Resets the `ClickCounter` to ensure no additional clicks are processed in
 * this state.
 *
 * @param Key Pointer to the button structure for which the repeat state is
 * being processed.
 *
 * @note This function is static and part of the internal button state machine
 * logic. It is specifically used when combined click mode is active.
 *
 * @return None
 */
static void multipleClikRepeat(button_t *Key)
{
    if (Key->ButtonPressed != NULL && !Key->CombinedModeRepeatPressEx)
    {
        Key->ButtonPressed(Key->NumberBtn);
        Key->CombinedModeRepeatPressEx = 1;
    }
    Key->ClickCounter = 0;
}
#endif

static void ButtonResetInstance(button_t *Key)
{
    for (uint8_t i = 0; i < sizeof(button_t); i++)
    {
        ((uint8_t *)Key)[i] = 0;
    }
}

/* ========================== Initialization Functions =========================
 */
/**
 * @brief Initializes a button structure with the provided parameters.
 *
 * This function sets the initial state of the button, configures its associated
 * GPIO port and pin, and initializes timers for debounce, long press, and
 * repeat events. It also sets the button's reverse logic mode and assigns a
 * unique identifier.
 *
 * @param Key Pointer to the button structure to initialize.
 * @param GpioPort GPIO port where the button is connected.
 * @param GpioPin GPIO pin where the button is connected.
 * @param TimerDebounce Debounce time in milliseconds to filter button noise.
 * @param TimerLongPressed Time in milliseconds to detect a long press.
 * @param TimerRepeat Time in milliseconds for repeated press events.
 * @param ReverseLogic Indicates whether the GPIO uses reverse logic
 * (active-low).
 * @param Number Button identifier passed to callback functions.
 * @return Status of the initialization:
 *         - `BTN_OK` if initialization is successful.
 *         - `BTN_ERROR` if there was an error during initialization.
 */
BTN_operate_status ButtonInitKey(button_t *Key, BTN_GPIO_PORT_T *GpioPort, BTN_GPIO_PIN_T GpioPin,
                                 BTN_TIME_t TimerDebounce, BTN_TIME_t TimerLongPressed, BTN_TIME_t TimerRepeat,
                                 ReverseLogicGpio_t ReverseLogic, uint16_t Number)
{
    if (Key == NULL || GpioPort == NULL)
    {
        return BTN_ERROR;
    }
    ButtonResetInstance(Key);
    Key->State = IDLE;
    Key->GpioPort = GpioPort;
    Key->GpioPin = GpioPin;
    Key->TimerDebounce = TimerDebounce;
    Key->TimerLongPressed = TimerLongPressed;
    Key->TimerRepeat = TimerRepeat;

    Key->ReverseLogic = ReverseLogic;
    Key->NumberBtn = Number;

#if BTN_DOUBLE_DEBOUNCING
    Key->TimerSecondDebounce = TimerDebounce;
#endif

#if BTN_MULTIPLE_CLICK
    Key->MultipleClickMode = BTN_MULTIPLE_CLICK_OFF;
#endif

#if BTN_NON_USED_CALLBACK
    Key->TimerNonUsed = 0;
#endif
    return BTN_OK;
}

#if BTN_DEFAULT_INIT
/**
 * @brief Initializes a button structure with default timing values.
 *
 * This function sets the initial state of the button, configures its associated
 * GPIO port and pin, and initializes timers for debounce, long press, and
 * repeat events using predefined default values. It also sets the button's
 * reverse logic mode and assigns a unique identifier.
 *
 * Default timing values:
 * - Debounce time: `BTN_DEFAULT_TIME_DEBOUNCE`
 * - Long press time: `BTN_DEFAULT_TIME_LONG_PRESS`
 * - Repeat time: `BTN_DEFAULT_TIME_REPEAT`
 *
 * @param Key Pointer to the button structure to initialize.
 * @param GpioPort GPIO port where the button is connected.
 * @param GpioPin GPIO pin where the button is connected.
 * @param ReverseLogic Indicates whether the GPIO uses reverse logic
 * (active-low).
 * @param Number Button identifier passed to callback functions.
 * @return Status of the initialization:
 *         - `BTN_OK` if initialization is successful.
 *         - `BTN_ERROR` if there was an error during initialization.
 */
BTN_operate_status ButtonInitKeyDefault(button_t *Key, BTN_GPIO_PORT_T *GpioPort, BTN_GPIO_PIN_T GpioPin,
                                        ReverseLogicGpio_t ReverseLogic, uint16_t Number)
{
    if (Key == NULL || GpioPort == NULL)
    {
        return BTN_ERROR;
    }

    ButtonResetInstance(Key);
    Key->State = IDLE;
    Key->GpioPort = GpioPort;
    Key->GpioPin = GpioPin;
    Key->TimerDebounce = BTN_DEFAULT_TIME_DEBOUNCE;
    Key->TimerLongPressed = BTN_DEFAULT_TIME_LONG_PRESS;
    Key->TimerRepeat = BTN_DEFAULT_TIME_REPEAT;

    Key->ReverseLogic = ReverseLogic;
    Key->NumberBtn = Number;

#if BTN_DOUBLE_DEBOUNCING
    Key->TimerSecondDebounce = BTN_DEFAULT_TIME_DEBOUNCE;
#endif

#if BTN_MULTIPLE_CLICK
    Key->MultipleClickMode = BTN_MULTIPLE_CLICK_OFF;
#endif

#if BTN_NON_USED_CALLBACK
    Key->TimerNonUsed = 0;
#endif
    return BTN_OK;
}
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
 * @return Status of the configuration:
 *         - `BTN_OK` if the configuration was successful.
 *         - `BTN_ERROR` if there was an error during configuration.
 */
BTN_operate_status ButtonSetMultipleClick(button_t *Key, MultipleClickMode_t MultipleClickMode,
                                          BTN_TIME_t TimerBetweenClick)
{
    if (Key == NULL)
    {
        return BTN_ERROR;
    }

    Key->MultipleClickMode = MultipleClickMode;
    Key->TimerBetweenClick = TimerBetweenClick;
    return BTN_OK;
}
#endif

#if BTN_NON_USED_CALLBACK
/**
 * @brief Configures the non-used (idle) behavior for a button.
 *
 * This function sets the time threshold for considering a button as unused and
 * assigns a callback function to be executed when the button remains idle for
 * the specified duration.
 *
 * @param Key Pointer to the button instance.
 * @param Miliseconds Time in milliseconds after which the button is considered
 * non-used. If set to 0, the non-used functionality is disabled for this
 * button.
 * @param Callback Pointer to the function to be executed when the button is
 * unused.
 * @return Status of the configuration:
 *         - `BTN_OK` if the configuration was successful.
 *         - `BTN_ERROR` if there was an error during configuration.
 *
 * @note Ensure that the `BTN_NON_USED_CALLBACK` macro is enabled in the library
 * configuration to use this functionality.
 */
BTN_operate_status ButtonSetNonUsed(button_t *Key, BTN_TIME_t Miliseconds, void *Callback)
{
    if (Key == NULL)
    {
        return BTN_ERROR;
    }

    Key->TimerNonUsed = Miliseconds;
    Key->ButtonNonUsed = Callback;
    return BTN_OK;
}
#endif

/* ========================== Button State Handlers ==========================
 */
/**
 * @brief Handles the idle state of the button.
 *
 * This function processes the button's behavior when it is in the idle state.
 * It checks for a button press event, transitions the button state to debounce
 * if a press is detected, and handles any multiple click logic if enabled.
 *
 * Actions performed:
 * - Invokes the `multipleClikIdle` function if multiple click handling is
 * enabled (`BTN_MULTIPLE_CLICK`).
 * - Reads the current GPIO pin state and compares it with the expected state
 * based on the reverse logic configuration.
 * - If a press is detected, updates the `LastTick` timestamp and transitions
 * the button state to `DEBOUNCE`.
 *
 * @param Key Pointer to the button structure being processed.
 *
 * @note This function is static and part of the internal state machine for
 * button handling.
 *
 * @return None
 */
static void ButtonIdleRoutine(button_t *Key)
{
#if BTN_MULTIPLE_CLICK
    multipleClikIdle(Key);
#endif
    if (ReadState(Key->GpioPort, Key->GpioPin) == (Key->ReverseLogic == 0) ? BTN_RESET : BTN_SET)
    {
        Key->LastTick = BTN_GET_TICK;
        Key->State = DEBOUNCE;
    }
#if BTN_NON_USED_CALLBACK
    if (Key->TimerNonUsed && (BTN_GET_TICK - Key->LastTick >= Key->TimerNonUsed))
    {
        Key->LastTick = BTN_GET_TICK;
        if (NULL != Key->ButtonNonUsed)
            Key->ButtonNonUsed(Key->NumberBtn);
    }
#endif
}

/**
 * @brief Handles the debounce state of the button.
 *
 * This function processes the button's behavior during the debounce state. It
 * verifies if the debounce timer has elapsed and determines whether the button
 * press is valid based on the GPIO pin state. Depending on the configuration,
 * it transitions the state to `PRESSED` or back to `IDLE` and triggers the
 * appropriate callbacks if required.
 *
 * Actions performed:
 * - Checks if the debounce timer (`TimerDebounce`) has elapsed.
 * - Reads the GPIO pin state and compares it with the expected state based on
 * reverse logic.
 * - If a valid press is detected:
 *   - Handles multiple clicks if enabled (`BTN_MULTIPLE_CLICK`) by invoking
 * `MultipleClickDebounce`.
 *   - Updates timestamps (`LastTick` or `LastClickTick`) and transitions the
 * state to `PRESSED`.
 *   - Executes the single press callback (`ButtonPressed`) if multiple click
 * handling is disabled.
 * - If no press is detected, transitions the state back to `IDLE`.
 *
 * @param Key Pointer to the button structure being processed.
 *
 * @note This function is static and part of the internal state machine for
 * button handling. Multiple click handling is only processed if
 * `BTN_MULTIPLE_CLICK` is enabled.
 *
 * @return None
 */
static void ButtonDebounceRoutine(button_t *Key)
{
    if ((BTN_GET_TICK - Key->LastTick) >= Key->TimerDebounce)
    {
        if (ReadState(Key->GpioPort, Key->GpioPin) == (Key->ReverseLogic == 0) ? BTN_RESET : BTN_SET)
        {

#if BTN_MULTIPLE_CLICK
            MultipleClickDebounce(Key);
            Key->State = PRESSED;
            Key->LastClickTick = BTN_GET_TICK;
#else
            Key->State = PRESSED;
            Key->LastTick = BTN_GET_TICK;
            if (Key->ButtonPressed != NULL)
            {
                Key->ButtonPressed(Key->NumberBtn);
            }
#endif
        }
        else
        {
            Key->State = IDLE;
        }
    }
}

/**
 * @brief Handles the pressed state of the button.
 *
 * This function processes the button's behavior during the pressed state. It
 * checks for either a release event or the expiration of the long press timer.
 * Based on the conditions, it transitions the button state to `RELEASE` or
 * `REPEAT` and triggers the long press callback if applicable.
 *
 * Actions performed:
 * - Checks if the button is released (based on the GPIO pin state and reverse
 * logic).
 *   - If released, transitions the state to `RELEASE`.
 * - Checks if the long press timer (`TimerLongPressed`) has elapsed.
 *   - If elapsed, transitions the state to `REPEAT`, updates the `LastTick`
 * timestamp, and invokes the long press callback (`ButtonLongPressed`) if it is
 * set.
 *
 * @param Key Pointer to the button structure being processed.
 *
 * @note This function is static and part of the internal state machine for
 * button handling.
 *
 * @return None
 */

static void ButtonPressedRoutine(button_t *Key)
{
    if (ReadState(Key->GpioPort, Key->GpioPin) == (Key->ReverseLogic == 0) ? BTN_SET : BTN_RESET)
    {
#if BTN_DOUBLE_DEBOUNCING
        Key->StateBeforeRelease = Key->State;
        Key->State = DEBOUNCE_RELEASE;
        Key->LastTickSecondDebounce = BTN_GET_TICK;
#else
        Key->State = RELEASE;
#endif
    }
    else if (BTN_GET_TICK - Key->LastTick >= Key->TimerLongPressed)
    {
        Key->State = REPEAT;
        Key->LastTick = BTN_GET_TICK;
        if (Key->ButtonLongPressed != NULL)
        {
            Key->ButtonLongPressed(Key->NumberBtn);
        }
    }
}

/**
 * @brief Handles the repeat state of the button.
 *
 * This function processes the button's behavior during the repeat state. It
 * checks for the button release or long press expiration, and handles the
 * repeat callback if the repeat timer has elapsed. Additionally, it handles
 * multiple click logic if enabled.
 *
 * Actions performed:
 * - Invokes the `multipleClikRepeat` function if multiple click handling is
 * enabled (`BTN_MULTIPLE_CLICK`).
 * - Checks if the button is released (based on the GPIO pin state and reverse
 * logic).
 *   - If released, transitions the state to either `RELEASE` or
 * `RELEASE_AFTER_REPEAT` based on the configuration.
 * - Checks if the repeat timer (`TimerRepeat`) has elapsed.
 *   - If elapsed, updates the `LastTick` timestamp and invokes the repeat
 * callback (`ButtonRepeat`) if set.
 *
 * @param Key Pointer to the button structure being processed.
 *
 * @note This function is static and part of the internal state machine for
 * button handling. Multiple click handling is only processed if
 * `BTN_MULTIPLE_CLICK` is enabled.
 *
 * @return None
 */

static void ButtonRepeatRoutine(button_t *Key)
{
#if BTN_MULTIPLE_CLICK
    multipleClikRepeat(Key);
#endif
    if (ReadState(Key->GpioPort, Key->GpioPin) == (Key->ReverseLogic == 0) ? BTN_SET : BTN_RESET)
    {
#if BTN_DOUBLE_DEBOUNCING
        Key->StateBeforeRelease = Key->State;
        Key->State = DEBOUNCE_RELEASE;
        Key->LastTickSecondDebounce = BTN_GET_TICK;
#else
#if !BTN_RELEASE_AFTER_REPEAT
        Key->State = RELEASE;
#else
        Key->State = RELEASE_AFTER_REPEAT;
#endif
#endif
    }
    else if (BTN_GET_TICK - Key->LastTick >= Key->TimerRepeat)
    {
        Key->LastTick = BTN_GET_TICK;
        if (Key->ButtonRepeat != NULL)
        {
            Key->ButtonRepeat(Key->NumberBtn);
        }
    }
}

#if BTN_DOUBLE_DEBOUNCING
/**
 * @brief Handles the debounce logic for the button release state.
 *
 * This routine checks if the button is stable (debounced) after being released.
 * It determines the next state of the button based on its previous state
 * (`StateBeforeRelease`) and the current GPIO pin logic level.
 *
 * @param Key Pointer to the button structure being processed.
 */
static void ButtonDebounceReleaseRoutine(button_t *Key)
{
    if ((BTN_GET_TICK - Key->LastTickSecondDebounce) >= Key->TimerSecondDebounce)
    {
        if (ReadState(Key->GpioPort, Key->GpioPin) != (Key->ReverseLogic == 0) ? BTN_SET : BTN_RESET)
        {
            Key->State = Key->StateBeforeRelease;
        }
        else
        {
#if BTN_RELEASE_AFTER_REPEAT
            if (Key->StateBeforeRelease == PRESSED)
            {
                Key->State = RELEASE;
            }
            else if (Key->StateBeforeRelease == REPEAT)
            {
                Key->State = RELEASE_AFTER_REPEAT;
            }
#else
            Key->State = RELEASE;
#endif
        }
    }
}
#endif

/**
 * @brief Handles the release state of the button.
 *
 * This function processes the button's behavior during the release state. It
 * triggers the release callback if defined and transitions the button state
 * back to `IDLE`.
 *
 * Actions performed:
 * - Invokes the release callback (`ButtonRelease`) if set, passing the button
 * number as an argument.
 * - Transitions the button state to `IDLE` after handling the release.
 *
 * @param Key Pointer to the button structure being processed.
 *
 * @note This function is static and part of the internal state machine for
 * button handling.
 *
 * @return None
 */

static void ButtonReleaseRoutine(button_t *Key)
{
    if (Key->ButtonRelease != NULL)
    {
        Key->ButtonRelease(Key->NumberBtn);
    }
    Key->State = IDLE;
}

#if BTN_RELEASE_AFTER_REPEAT
/**
 * @brief Handles the release state after repeat for the button.
 *
 * This function processes the button's behavior after a repeat cycle has
 * completed. It triggers the release callback for this state if defined and
 * transitions the button state back to `IDLE`.
 *
 * Actions performed:
 * - Invokes the release after repeat callback (`ButtonReleaseAfterRepeat`) if
 * set, passing the button number as an argument.
 * - Transitions the button state to `IDLE` after handling the release.
 *
 * @param Key Pointer to the button structure being processed.
 *
 * @note This function is static and part of the internal state machine for
 * button handling, specifically for the state after repeat when
 * `BTN_RELEASE_AFTER_REPEAT` is enabled.
 *
 * @return None
 */

static void ButtonReleaseAfterRepeatRoutine(button_t *Key)
{
    if (Key->ButtonReleaseAfterRepeat != NULL)
    {
        Key->ButtonReleaseAfterRepeat(Key->NumberBtn);
    }
    Key->State = IDLE;
}
#endif

/* =================================== State machine
 * ================================== */
/**
 * @brief Handles the button state machine and triggers appropriate routines
 * based on the button's current state.
 *
 * This function is responsible for managing the button state transitions and
 * calling the corresponding routine for each state (e.g., IDLE, DEBOUNCE,
 * PRESSED, REPEAT, RELEASE, RELEASE_AFTER_REPEAT). It should be invoked in the
 * main loop or the main system task to continuously check and process the
 * button's state.
 *
 * Actions performed:
 * - Evaluates the current button state (`State`) and calls the corresponding
 * state handling function:
 *   - `ButtonIdleRoutine` for the `IDLE` state.
 *   - `ButtonDebounceRoutine` for the `DEBOUNCE` state.
 *   - `ButtonPressedRoutine` for the `PRESSED` state.
 *   - `ButtonRepeatRoutine` for the `REPEAT` state.
 *   - `ButtonDebounceReleaseRoutine` for the `DEBOUNCE_RELEASE` state.
 *   - `ButtonReleaseRoutine` for the `RELEASE` state.
 *   - `ButtonReleaseAfterRepeatRoutine` for the `RELEASE_AFTER_REPEAT` state,
 * if enabled.
 *
 * @param Key Pointer to the button structure being processed.
 *
 * @note This function should be called in the main loop or system task to
 * ensure the button state machine is regularly processed. The specific state
 * handling functions are invoked based on the current state of the button.
 *
 * @return Status of the button operation:
 *         - `BTN_OK` if the operation was successful.
 *         - `BTN_ERROR` if there was an error during the operation.
 */

BTN_operate_status ButtonTask(button_t *Key)
{
    if (Key == NULL)
    {
        return BTN_ERROR;
    }

    switch (Key->State)
    {
    case IDLE:
        ButtonIdleRoutine(Key);
        break;

    case DEBOUNCE:
        ButtonDebounceRoutine(Key);
        break;

    case PRESSED:
        ButtonPressedRoutine(Key);
        break;

    case REPEAT:
        ButtonRepeatRoutine(Key);
        break;

#if BTN_DOUBLE_DEBOUNCING
    case DEBOUNCE_RELEASE:
        ButtonDebounceReleaseRoutine(Key);
        break;
#endif

    case RELEASE:
        ButtonReleaseRoutine(Key);
        break;

#if BTN_RELEASE_AFTER_REPEAT
    case RELEASE_AFTER_REPEAT:
        ButtonReleaseAfterRepeatRoutine(Key);
        break;
#endif
    }
    return BTN_OK;
}

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
 * @return Status of the operation:
 *         - `BTN_OK` if the operation was successful.
 *         - `BTN_ERROR` if there was an error during the operation.
 */
BTN_operate_status ButtonSetDebounceTime(button_t *Key, BTN_TIME_t Miliseconds)
{
    if (Key == NULL)
    {
        return BTN_ERROR;
    }
    Key->TimerDebounce = Miliseconds;
    return BTN_OK;
}

#if BTN_DOUBLE_DEBOUNCING
/**
 * @brief Configures the debounce time for release handling of a button.
 *
 * This function assigns a debounce time in milliseconds to the
 * `TimerSecondDebounce` field of the given button structure. It is used during
 * the double debounce process to ensure the button signal has stabilized after
 * release.
 *
 * @param Key Pointer to the button structure being configured.
 * @param Miliseconds Debounce time in milliseconds.
 * @return Status of the operation:
 *         - `BTN_OK` if the operation was successful.
 *         - `BTN_ERROR` if there was an error during the operation.
 *
 * @note This function is only available when `BTN_DOUBLE_DEBOUNCING` is
 * enabled.
 */
BTN_operate_status ButtonSetReleaseDebounceTime(button_t *Key, BTN_TIME_t Miliseconds)
{
    if (Key == NULL)
    {
        return BTN_ERROR;
    }

    Key->TimerSecondDebounce = Miliseconds;
    return BTN_OK;
}
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
 * @return Status of the operation:
 *         - `BTN_OK` if the operation was successful.
 *         - `BTN_ERROR` if there was an error during the operation.
 */
BTN_operate_status ButtonSetLongPressedTime(button_t *Key, BTN_TIME_t Miliseconds)
{
    if (Key == NULL)
    {
        return BTN_ERROR;
    }

    Key->TimerLongPressed = Miliseconds;
    return BTN_OK;
}

/**
 * @brief Sets the repeat time for the button.
 *
 * This function configures the repeat time. When the button is held down, the
 * repeat event will be triggered periodically after the specified time.
 *
 * @param Key Pointer to the button structure.
 * @param Miliseconds Time for repeat detection in milliseconds.
 * @return Status of the operation:
 *         - `BTN_OK` if the operation was successful.
 *         - `BTN_ERROR` if there was an error during the operation.
 */
BTN_operate_status ButtonSetRepeatTime(button_t *Key, BTN_TIME_t Miliseconds)
{
    if (Key == NULL)
    {
        return BTN_ERROR;
    }

    Key->TimerRepeat = Miliseconds;
    return BTN_OK;
}

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
 * @return Status of the operation:
 *         - `BTN_OK` if the operation was successful.
 *         - `BTN_ERROR` if there was an error during the operation.
 */
BTN_operate_status ButtonSetMultipleClickTime(button_t *Key, BTN_TIME_t Miliseconds)
{
    if (Key == NULL)
    {
        return BTN_ERROR;
    }

    Key->TimerBetweenClick = Miliseconds;
    return BTN_OK;
}
#endif

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
 * @return Status of the operation:
 *         - `BTN_OK` if the operation was successful.
 *         - `BTN_ERROR` if there was an error during the operation.
 */
BTN_operate_status ButtonRegisterPressCallback(button_t *Key, void *Callback)
{
    if (Key == NULL)
    {
        return BTN_ERROR;
    }

    Key->ButtonPressed = Callback;
    return BTN_OK;
}

/**
 * @brief Registers a callback function for the button long press event.
 *
 * This function registers a user-defined callback function to be called when
 * the button is held down for longer than the specified long press time.
 *
 * @param Key Pointer to the button structure.
 * @param Callback Pointer to the callback function to be executed when a long
 * press is detected.
 * @return Status of the operation:
 *         - `BTN_OK` if the operation was successful.
 *         - `BTN_ERROR` if there was an error during the operation.
 */
BTN_operate_status ButtonRegisterLongPressedCallback(button_t *Key, void *Callback)
{
    if (Key == NULL)
    {
        return BTN_ERROR;
    }
    Key->ButtonLongPressed = Callback;
    return BTN_OK;
}

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
 * @return Status of the operation:
 *         - `BTN_OK` if the operation was successful.
 *         - `BTN_ERROR` if there was an error during the operation.
 */
BTN_operate_status ButtonRegisterRepeatCallback(button_t *Key, void *Callback)
{
    if (Key == NULL)
    {
        return BTN_ERROR;
    }

    Key->ButtonRepeat = Callback;
    return BTN_OK;
}

/**
 * @brief Registers a callback function for the button release event.
 *
 * This function registers a user-defined callback function to be called when
 * the button is released.
 *
 * @param Key Pointer to the button structure.
 * @param Callback Pointer to the callback function to be executed when the
 * button is released.
 * @return Status of the operation:
 *         - `BTN_OK` if the operation was successful.
 *         - `BTN_ERROR` if there was an error during the operation.
 */
BTN_operate_status ButtonRegisterReleaseCallback(button_t *Key, void *Callback)
{
    if (Key == NULL)
    {
        return BTN_ERROR;
    }

    Key->ButtonRelease = Callback;
    return BTN_OK;
}

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
 * @return Status of the operation:
 *         - `BTN_OK` if the operation was successful.
 *         - `BTN_ERROR` if there was an error during the operation.
 */
BTN_operate_status ButtonRegisterReleaseAfterRepeatCallback(button_t *Key, void *Callback)
{
    if (Key == NULL)
    {
        return BTN_ERROR;
    }

    Key->ButtonReleaseAfterRepeat = Callback;
    return BTN_OK;
}
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
 * @return Status of the operation:
 *         - `BTN_OK` if the operation was successful.
 *         - `BTN_ERROR` if there was an error during the operation.
 */
BTN_operate_status ButtonRegisterDoubleClickCallback(button_t *Key, void *Callback)
{
    if (Key == NULL)
    {
        return BTN_ERROR;
    }

    Key->ButtonDoubleClick = Callback;
    return BTN_OK;
}

/**
 * @brief Registers a callback function for the button triple-click event.
 *
 * This function registers a user-defined callback function to be called when
 * the button is triple-clicked (three button presses within a specified time).
 *
 * @param Key Pointer to the button structure.
 * @param Callback Pointer to the callback function to be executed during
 * triple-click.
 * @return Status of the operation:
 *         - `BTN_OK` if the operation was successful.
 *         - `BTN_ERROR` if there was an error during the operation.
 */
BTN_operate_status ButtonRegisterTripleClickCallback(button_t *Key, void *Callback)
{
    if (Key == NULL)
    {
        return BTN_ERROR;
    }

    Key->ButtonTripleClick = Callback;
    return BTN_OK;
}
#endif

#ifdef HAL_TO_DEFINE
#define USE_HAL_DRIVER
#endif
