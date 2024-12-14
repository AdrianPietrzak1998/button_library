/*
 * button_cfg.h
 *
 *  Created on: Dec 14, 2024
 *      Author: Adrian Pietrzak
 */

#ifndef BUTTON_LIBRARY_BUTTON_CFG_H_
#define BUTTON_LIBRARY_BUTTON_CFG_H_

/**
  * @brief Preprocessor macros for button functionality configuration.
  *
  * These macros control the behavior and features of the button functionality, such as enabling
  * multiple click detection, long press handling, debounce timing, and more. Adjusting these
  * macros will modify the way the button logic is processed in the system.
  */

/**
 * @def BTN_RELEASE_AFTER_REPEAT
 * @brief Enables or disables the release after repeat state for the button.
 * If set to 1, the button will trigger a release_after_repeat event after the repeat action.
 */
#define BTN_RELEASE_AFTER_REPEAT 1

/**
 * @def BTN_DOUBLE_DEBOUNCING
 * @brief Enables or disables the debouncing for button release.
 *
 * If set to 1, the button will apply debouncing logic not only on press but also on release.
 * This ensures that any mechanical bouncing or signal noise during the release of the button
 * is filtered out, providing more stable and reliable button behavior.
 *
 * If set to 0, the debouncing will only be applied during the press event.
 */
#define BTN_DOUBLE_DEBOUNCING 1

/**
 * @def BTN_MULTIPLE_CLICK
 * @brief Enables or disables multiple click detection functionality.
 * If set to 1, the button will support handling multiple clicks within a specified time.
 */
#define BTN_MULTIPLE_CLICK 1

/**
 * @def BTN_MULTIPLE_CLICK_COMBINED_TO_MUCH_AS_TRIPLE
 * @brief Defines the behavior for handling too many clicks in combined mode.
 * If set to 1, multiple clicks beyond triple will be treated as a triple click.
 */
#define BTN_MULTIPLE_CLICK_COMBINED_TO_MUCH_AS_TRIPLE 1

/**
 * @def BTN_DEFAULT_INIT
 * @brief Enables or disables the use of default button initialization values.
 * If set to 1, the default values for debounce, long press, and repeat will be used in
 * the ButtonInitKeyDefault initialization function.
 */
#define BTN_DEFAULT_INIT 1

/**
 * @def BTN_FORCE_NON_HAL
 * @brief Forces the use of non-HAL GPIO drivers.
 * If set to 1, the code will use direct register access for GPIO operations instead of the HAL driver.
 */
#define BTN_FORCE_NON_HAL 1

#if BTN_DEFAULT_INIT
/**
 * @def BTN_DEFAULT_TIME_DEBOUNCE
 * @brief Default debounce time in milliseconds.
 * This defines how long the button's signal is allowed to stabilize before detecting a valid press.
 */
#define BTN_DEFAULT_TIME_DEBOUNCE 50

/**
 * @def BTN_DEFAULT_TIME_LONG_PRESS
 * @brief Default time for detecting a long press in milliseconds.
 * A long press is detected if the button is held for at least this duration.
 */
#define BTN_DEFAULT_TIME_LONG_PRESS 500

/**
 * @def BTN_DEFAULT_TIME_REPEAT
 * @brief Default time for repeat actions in milliseconds.
 * This defines how frequently the button repeats its action after being held down.
 */
#define BTN_DEFAULT_TIME_REPEAT 300
#endif

/**
 * @def BTN_LIB_TICK
 * @brief Defines the variable used as the timer source for the library.
 *
 * This variable must provide the current time in milliseconds as an `unsigned` value.
 * It is used for timing operations such as debounce, long press, and repeat detection.
 *
 * @note Ensure that this timer increments continuously in milliseconds and does not overflow too quickly.
 * For STM32 HAL-based projects, `uwTick` is commonly used as it is updated in the SysTick interrupt.
 */
#define BTN_LIB_TICK uwTick //Miliseconds timer

/**
 * @def BTN_USER_READ_PIN_ROUTINE
 * @brief Enables or disables a custom user-defined GPIO read routine.
 *
 * - If set to `1`: The library will call a user-defined routine to read the GPIO pin state.
 *   The user must implement this routine by overriding the `BTN_READ_PIN()` macro.
 * - If set to `0`: The library will use its default method to read the GPIO pin state
 *   (e.g., `HAL_GPIO_ReadPin()` for STM32 or direct register access for bare-metal).
 *
 * @note Use this option if the default GPIO read routine does not fit your platform or requirements.
 */
#define BTN_USER_READ_PIN_ROUTINE 0

/**
 * @def BTN_TIMER_T
 * @brief Defines the data type used for timer values within the library.
 *
 * This type must be an `unsigned` integer that can represent time in milliseconds.
 * It is used for internal timing variables such as debounce time, long press duration, and repeat intervals.
 *
 * @default `uint32_t` is recommended for most platforms as it provides sufficient range for timing.
 * @note Ensure that the chosen type matches the resolution and range of your timer source.
 */
#define BTN_TIMER_T uint32_t

/**
 * @def BTN_GPIO_PORT_T
 * @brief Defines the data type for the GPIO port parameter.
 *
 * This type represents the GPIO port to which the button is connected.
 * For STM32, this is typically `GPIO_TypeDef`.
 *
 * @default `GPIO_TypeDef` is used for STM32 HAL-based projects.
 * @note Adjust this type if your platform uses a different representation for GPIO ports.
 */
#define BTN_GPIO_PORT_T GPIO_TypeDef

/**
 * @def BTN_GPIO_PIN_T
 * @brief Defines the data type for the GPIO pin parameter.
 *
 * This type represents the GPIO pin to which the button is connected.
 * For STM32, this is typically `uint16_t`.
 *
 * @default `uint16_t` is used for STM32 HAL-based projects.
 * @note Ensure this type matches the width of your platform's GPIO pin identifiers.
 */
#define BTN_GPIO_PIN_T uint16_t



#endif /* BUTTON_LIBRARY_BUTTON_CFG_H_ */
