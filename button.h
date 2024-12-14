/*
 * button.h
 *
 *  Created on: May 7, 2022
 *      Author: Adrian Pietrzak
 */

#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

#include <stdint.h>
#include "button_cfg.h"

/**
  * @brief Enum for button states.
  *
  * This enumeration defines the various states of the button during its lifecycle.
  * It helps track and manage the button's state transitions, such as when it is idle, pressed, or released.
  */
typedef enum
{
    IDLE = 0,           /**< Button is in the idle state, no action is taking place. */
    DEBOUNCE,           /**< Button is in the debounce state, waiting for the signal to stabilize. */
    PRESSED,            /**< Button has been pressed and is being handled. */
    REPEAT,             /**< Button is being held down and the repeat action is being triggered. */
    RELEASE,            /**< Button has been released. */
#if BTN_DOUBLE_DEBOUNCING
	DEBOUNCE_RELEASE,   /**< Button is in the debounce state, waiting for the signal to stabilize. */
#endif
#if BTN_RELEASE_AFTER_REPEAT
    RELEASE_AFTER_REPEAT /**< Button is released after repeat action, if enabled. */
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
    NON_REVERSE = 0,    /**< No reverse logic, the button state follows the actual GPIO state. */
    REVERSE             /**< Reverse logic, the button state is inverted from the actual GPIO state. */
} ReverseLogicGpio_t;

/**
  * @brief Enum for multiple click modes.
  *
  * This enumeration defines the different modes available for handling multiple clicks on a button.
  * It allows the configuration of how the button reacts to multiple consecutive presses.
  */
typedef enum
{
    BTN_MULTIPLE_CLICK_OFF = 0,            /**< No multiple click handling (disabled). */
    BTN_MULTIPLE_CLICK_NORMAL_MODE,        /**< Normal mode for multiple click detection. */
    BTN_MULTIPLE_CLICK_COMBINED_MODE      /**< Combined mode for handling click cycles. */
} MultipleClickMode_t;


/**
  * @brief Button structure used for managing the state and behavior of a button.
  *
  * This structure holds all the necessary information for managing a button, including its current
  * state, GPIO settings, timers for debouncing, long press, repeat, and multiple clicks, as well as
  * function pointers for button event callbacks (e.g., pressed, long pressed, release, etc.).
  *
  * @note The structure is intended to be used with functions for handling button debouncing, state
  *       transitions, and event callbacks.
  */
typedef struct
{
    ButtonState_t   State;                /**< Current state of the button. */
    GPIO_TypeDef   *GpioPort;              /**< GPIO port where the button is connected. */
    BTN_GPIO_PIN_T  GpioPin;               /**< GPIO pin where the button is connected. */
    BTN_TIMER_T     LastTick;              /**< Timestamp of the last button event. */
    BTN_TIMER_T     TimerDebounce;         /**< Debounce time in milliseconds. */
    BTN_TIMER_T     TimerLongPressed;      /**< Time threshold for a long press. */
    BTN_TIMER_T     TimerRepeat;           /**< Repeat time threshold for repeated presses. */
    ReverseLogicGpio_t ReverseLogic;     /**< Logic level inversion for the button (if applicable). */
    uint16_t        NumberBtn;             /**< Identifier for the button (used in callbacks). */
#if BTN_DOUBLE_DEBOUNCING
    ButtonState_t StateBeforeRelease;     /**< Previous button state before entering the release state. */
    BTN_TIMER_T	  TimerSecondDebounce;    /**< Debounce time for the release state in milliseconds. */
    BTN_TIMER_T	  LastTickSecondDebounce; /**< Timestamp of the last event during the release debounce phase. */
#endif
    void(*ButtonPressed)(uint16_t);     /**< Callback function for button press event. */
    void(*ButtonLongPressed)(uint16_t); /**< Callback function for long press event. */
    void(*ButtonRepeat)(uint16_t);      /**< Callback function for repeat press event. */
    void(*ButtonRelease)(uint16_t);     /**< Callback function for button release event. */
#if BTN_RELEASE_AFTER_REPEAT
    void(*ButtonReleaseAfterRepeat)(uint16_t); /**< Callback function for release event after repeat. */
#endif
#if BTN_MULTIPLE_CLICK
    void(*ButtonDoubleClick)(uint16_t); /**< Callback function for double-click event. */
    void(*ButtonTripleClick)(uint16_t); /**< Callback function for triple-click event. */

    MultipleClickMode_t MultipleClickMode; /**< Mode for multiple click handling. */
    uint8_t             ClickCounter:6;     /**< Counter for tracking number of clicks. */
    uint8_t             ClickCounterCycle:1;/**< Flag indicating cycle completion. */
    uint8_t             CombinedModeRepeatPressEx:1; /**< Flag for combined mode repeat press. */
    uint32_t            TimerBetweenClick; /**< Time between multiple clicks. */
    uint32_t            LastClickTick;     /**< Timestamp of the last click event. */
#endif
} button_t;


/* ========================== Initialization Functions ========================= */
/**
  * @brief Initializes a button structure with the provided parameters.
  * @param Key Pointer to the button structure to initialize.
  * @param GpioPort GPIO port where the button is connected.
  * @param GpioPin GPIO pin where the button is connected.
  * @param TimerDebounce Debounce time in milliseconds to filter button noise.
  * @param TimerLongPressed Time in milliseconds to detect a long press.
  * @param TimerRepeat Time in milliseconds for repeated press events.
  * @param ReverseLogic Indicates whether the GPIO uses reverse logic (active-low).
  * @param Number Button identifier passed to callback functions.
  * @retval None
  */
void ButtonInitKey(button_t * Key, GPIO_TypeDef *GpioPort, uint16_t GpioPin, BTN_TIMER_T TimerDebounce, BTN_TIMER_T TimerLongPressed,
			BTN_TIMER_T TimerRepeat, ReverseLogicGpio_t ReverseLogic, uint16_t Number); // Initialization for state machine
#if BTN_DEFAULT_INIT
/**
  * @brief Initializes a button structure with default timing values.
  * @param Key Pointer to the button structure to initialize.
  * @param GpioPort GPIO port where the button is connected.
  * @param GpioPin GPIO pin where the button is connected.
  * @param ReverseLogic Indicates whether the GPIO uses reverse logic (active-low).
  * @param Number Button identifier passed to callback functions.
  *
  * This function behaves similarly to `ButtonInitKey`, but uses predefined default
  * values for debounce time, long press time, and repeat time. These values are
  * defined as macros in the header file.
  * @retval None
  */
void ButtonInitKeyDefault(button_t * Key, GPIO_TypeDef *GpioPort, uint16_t GpioPin, ReverseLogicGpio_t ReverseLogic, uint16_t Number);
#endif


#if BTN_MULTIPLE_CLICK
/**
  * @brief Sets the multiple click mode and the timer between clicks for the button.
  *
  * This function configures the button to operate in different multiple click modes,
  * such as normal or combined, and sets the time between consecutive clicks.
  *
  * @param Key Pointer to the button structure.
  * @param MultipleClickMode The mode to handle multiple clicks (e.g., BTN_MULTIPLE_CLICK_OFF, BTN_MULTIPLE_CLICK_NORMAL_MODE).
  * @param TimerBetweenClick Time in milliseconds between two clicks that are considered part of the same multiple click event.
  * @retval None
  */
void ButtonSetMultipleClick(button_t * Key, MultipleClickMode_t MultipleClickMode, uint32_t TimerBetweenClick);
#endif

/* =================================== State machine ================================== */
/**
  * @brief Handles the button state machine and triggers appropriate routines.
  *
  * This function manages the button state transitions and calls the corresponding routine
  * based on the button's current state. It should be called in the main loop or system task.
  *
  * @param Key Pointer to the button structure being processed.
  *
  * @retval None
  */
void ButtonTask(button_t *Key); //Task for working state machine

/* ========================== Callback Registration Functions ========================= */
/**
  * @brief Registers a callback function for the button press event.
  *
  * This function registers a user-defined callback function to be called when the button
  * is pressed.
  *
  * @param Key Pointer to the button structure.
  * @param Callback Pointer to the callback function to be executed when the button is pressed.
  * @retval None
  */
void ButtonRegisterPressCallback(button_t *Key, void *Callback);

/**
  * @brief Registers a callback function for the button long press event.
  *
  * This function registers a user-defined callback function to be called when the button
  * is held down for longer than the specified long press time.
  *
  * @param Key Pointer to the button structure.
  * @param Callback Pointer to the callback function to be executed when a long press is detected.
  * @retval None
  */
void ButtonRegisterLongPressedCallback(button_t *Key, void *Callback);

/**
  * @brief Registers a callback function for the button repeat event.
  *
  * This function registers a user-defined callback function to be called when the button
  * is held down and the repeat event occurs after the repeat time is reached.
  *
  * @param Key Pointer to the button structure.
  * @param Callback Pointer to the callback function to be executed during repeat.
  * @retval None
  */
void ButtonRegisterRepeatCallback(button_t *Key, void *Callback);

/**
  * @brief Registers a callback function for the button release event.
  *
  * This function registers a user-defined callback function to be called when the button
  * is released.
  *
  * @param Key Pointer to the button structure.
  * @param Callback Pointer to the callback function to be executed when the button is released.
  * @retval None
  */
void ButtonRegisterReleaseCallback(button_t *Key, void *Callback);

#if BTN_RELEASE_AFTER_REPEAT
/**
  * @brief Registers a callback function for the button release after repeat event.
  *
  * This function registers a user-defined callback function to be called when the button
  * is released after a repeat event has occurred.
  *
  * @param Key Pointer to the button structure.
  * @param Callback Pointer to the callback function to be executed when the button is released after repeat.
  * @retval None
  */
void ButtonRegisterReleaseAfterRepeatCallback(button_t *Key, void *Callback);
#endif

#if BTN_MULTIPLE_CLICK
/**
  * @brief Registers a callback function for the button double-click event.
  *
  * This function registers a user-defined callback function to be called when the button
  * is double-clicked (two button presses within a specified time).
  *
  * @param Key Pointer to the button structure.
  * @param Callback Pointer to the callback function to be executed during double-click.
  * @retval None
  */
void ButtonRegisterDoubleClickCallback(button_t *Key, void *Callback);

/**
  * @brief Registers a callback function for the button triple-click event.
  *
  * This function registers a user-defined callback function to be called when the button
  * is triple-clicked (three button presses within a specified time).
  *
  * @param Key Pointer to the button structure.
  * @param Callback Pointer to the callback function to be executed during triple-click.
  * @retval None
  */
void ButtonRegisterTripleClickCallback(button_t *Key, void *Callback);
#endif

/* ========================== Time Settings Functions ========================= */
/**
  * @brief Sets the debounce time for the button.
  *
  * This function configures the debounce time, which determines the minimum time
  * between button presses that are considered separate events. This helps in avoiding
  * false triggering due to contact bounce.
  *
  * @param Key Pointer to the button structure.
  * @param Miliseconds Debounce time in milliseconds.
  * @retval None
  */
void ButtonSetDebounceTime(button_t * Key, BTN_TIMER_T Miliseconds);

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
 */
void ButtonSetReleaseDebounceTime(button_t * Key, BTN_TIMER_T Miliseconds);
#endif


/**
  * @brief Sets the long press time for the button.
  *
  * This function configures the time threshold for detecting a long press event.
  * If the button is held for longer than the specified time, the long press action
  * will be triggered.
  *
  * @param Key Pointer to the button structure.
  * @param Miliseconds Time for long press detection in milliseconds.
  * @retval None
  */
void ButtonSetLongPressedTime(button_t *Key, BTN_TIMER_T Miliseconds);


/**
  * @brief Sets the repeat time for the button.
  *
  * This function configures the repeat time. When the button is held down, the
  * repeat event will be triggered periodically after the specified time.
  *
  * @param Key Pointer to the button structure.
  * @param Miliseconds Time for repeat detection in milliseconds.
  * @retval None
  */
void ButtonSetRepeatTime(button_t *Key, BTN_TIMER_T Miliseconds);

#if BTN_MULTIPLE_CLICK
/**
  * @brief Sets the time between multiple clicks.
  *
  * This function configures the maximum time allowed between multiple button presses
  * that are considered as distinct clicks (e.g., double click or triple click).
  *
  * @param Key Pointer to the button structure.
  * @param Miliseconds Time between consecutive clicks in milliseconds.
  * @retval None
  */
void ButtonSetMultipleClickTime(button_t *Key, BTN_TIMER_T Miliseconds);
#endif

#endif /* INC_BUTTON_H_ */
