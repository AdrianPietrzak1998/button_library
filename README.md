
# Button Library

## Introduction
This library provides an easy-to-use interface for handling button input in embedded systems. It supports a state machine for buttons with features like debouncing, long-press detection, repeat functionality, and multiple click modes. The library is designed to work with GPIO pins on a microcontroller, with options to customize debounce times, long press durations, and actions for various button states.

## Basic Setup
To use the button library in your project, follow these steps:

1. **Include the Header File**:
   Make sure to include the `button.h` header file in your source code:
   ```c
   #include "button.h"
   ```

2. **Create a Button Object**:
   Define a `button_t` structure to represent each button:
   ```c
   button_t myButton;
   ```

3. **Initialize the Button**:
   Call the `ButtonInitKeyDefault()` function to initialize the button with default parameters:
   ```c
   ButtonInitKeyDefault(&myButton, GPIOB, GPIO_PIN_0, NON_REVERSE, 1);
   ```
   This function will set the button's state to `IDLE` and initialize other parameters like debounce time, long press time, etc. You can modify the time settings using `ButtonSetDebounceTime()`, `ButtonSetLongPressedTime()`, and `ButtonSetRepeatTime()`.

4. **Register Callbacks**:
   You can register callback functions for different button actions (e.g., press, long press, release):
   ```c
   ButtonRegisterPressCallback(&myButton, myPressCallback);
   ButtonRegisterLongPressedCallback(&myButton, myLongPressCallback);
   ButtonRegisterRepeatCallback(&myButton, myRepeatCallback);
   ButtonRegisterReleaseCallback(&myButton, myReleaseCallback);
   ```

5. **Call the Button Task**:
   To handle the button state machine, call `ButtonTask()` in the main loop or task scheduler:
   ```c
   ButtonTask(&myButton);
   ```

## Callback Functions

The library provides several callback functions that you can define in your application to handle button events. One of the most important callback functions is `ButtonPressed`, which is triggered when the button is pressed. This callback is passed the button number (`uint16_t`), which allows you to distinguish between different buttons if you are managing multiple buttons.

### Example Callback Function: `ButtonPressed`

Here’s an example of how to define a callback function for the `ButtonPressed` event, using the button number to differentiate between multiple buttons:

```c
void ButtonPressedCallback(uint16_t ButtonNumber)
{
    switch(ButtonNumber)
    {
        case 1:
            // Handle button 1 press
            break;
        case 2:
            // Handle button 2 press
            break;
        case 3:
            // Handle button 3 press
            break;
        default:
            // Handle default case
            break;
    }
}
```

## Multiple Click Modes

### 1. **Normal Mode** (`BTN_MULTIPLE_CLICK_NORMAL_MODE`)

In this mode, the library tracks how many clicks occur within a short period (defined by `TimerBetweenClick`). If two clicks occur within this time, a `double-click` callback is triggered. Similarly, three clicks within the same time window trigger a `triple-click` callback.

**How it works**:
- The button waits for clicks within the specified time (`TimerBetweenClick`).
- If two clicks are detected, the `ButtonDoubleClick` callback is called.
- If three clicks are detected, the `ButtonTripleClick` callback is called.

**Important Considerations**:
- **Multiple Callback Triggers**: In **Normal Mode**, additional callbacks may be triggered:
  - For a **double-click**, besides triggering the `ButtonDoubleClick` callback, the `ButtonPressed` callback will be called twice (once for each press) and the `ButtonRelease` callback will be called twice (once for each release).
  - For a **triple-click**, the `ButtonPressed` and `ButtonRelease` callbacks will be called three times each, and additionally, the `ButtonDoubleClick` and `ButtonTripleClick` callbacks will be triggered once each.

This behavior results from the fact that **Normal Mode** processes each individual click within the specified time, leading to multiple callback invocations for each press and release in a sequence.

### 2. **Combined Mode** (`BTN_MULTIPLE_CLICK_COMBINED_MODE`)

In this mode, the button can handle multiple clicks in a more advanced way, allowing for mixed presses. For example, a click can be combined with a long press, and multiple short presses can trigger different actions.

**How it works**:
- The library tracks clicks as they occur within a specified time (`TimerBetweenClick`).
- However, the callbacks are triggered only after all clicks in the sequence have been detected, with an added delay corresponding to the maximum time between clicks.

**Important Considerations**:
- **Additional Delay**: In combined mode, the callbacks are triggered after all clicks are detected, with the delay increased by the maximum time set in `TimerBetweenClick`. This introduces a latency before the callback is invoked.
- **Callback Order**: Due to this added delay, in **Combined Mode**, the callbacks will be invoked in the exact order as the clicks occurred, without overlap. The `ButtonPressed` callback will only be triggered once for each click, and the release callback (`ButtonRelease`) will also occur after the final click.

**Behavior**:
- Unlike **Normal Mode**, the callbacks in **Combined Mode** are delayed, ensuring that only the appropriate callback is triggered for each click, without duplicates.

### Example:
```c
// Set the button to combined mode with a 500 ms interval between clicks
ButtonSetMultipleClick(&myButton, BTN_MULTIPLE_CLICK_COMBINED_MODE, 500);

// Register callbacks for the different click types
ButtonRegisterDoubleClickCallback(&myButton, myDoubleClickCallback);
ButtonRegisterTripleClickCallback(&myButton, myTripleClickCallback);
```

## Configuration Macros for Button Functionality

These macros control various features of the button functionality, such as enabling multiple click detection, long press handling, debounce timing, and more. You can modify these macros to customize the behavior of the button library according to your needs.

### 1. **BTN_RELEASE_AFTER_REPEAT**
- **Description**: Enables or disables the "release after repeat" functionality for the button.
  - **When Enabled**: If set to `1`, the button will trigger a different release callback after a long press compared to a short press. This means that the release callback will be different depending on whether the button was held for a long press or just briefly pressed.
  - **When Disabled**: If set to `0`, the same release callback will be triggered for both short and long presses when the button is released, regardless of how long it was held.
  - **Use Case**: This option is useful when you want to differentiate between the release of a short press and a long press, allowing for more specific behavior based on the duration of the press.

---

### 2. **BTN_MULTIPLE_CLICK**
- **Description**: Enables or disables multiple click detection functionality.
  - **When Enabled**: If set to `1`, the button will handle multiple clicks within a specified time.
  - **When Disabled**: If set to `0`, the library will ignore multiple clicks, reducing code size and improving performance.
  - **Use Case**: If you don’t need to track multiple clicks (e.g., double-click or triple-click detection), disable this option to optimize memory usage and processing speed.

---

### 3. **BTN_MULTIPLE_CLICK_COMBINED_TO_MUCH_AS_TRIPLE**
- **Description**: Defines the behavior when there are too many clicks in combined mode.
  - **When Enabled**: If set to `1`, any clicks beyond the third click will be ignored.
  - **When Disabled**: If set to `0`, the click count will reset to zero after the third click, and the counting will start again.
  - **Use Case**: This option helps control how clicks are counted in combined mode, either by limiting the number of clicks or by restarting the count after too many presses.

---

### 4. **BTN_DEFAULT_INIT**
- **Description**: Enables or disables the use of default initialization values for button configuration.
  - **When Enabled**: If set to `1`, the library provides a simpler initialization function that assigns default time values for debounce, long press, and repeat. These default values can be changed later.
  - **When Disabled**: If set to `0`, you must manually configure the debounce, long press, and repeat times using the appropriate functions.
  - **Use Case**: Enable this option if you want to simplify button initialization by using predefined time values for debounce, long press, and repeat, without needing to manually set them. However, you still have the ability to change these values as needed. The full initialization function remains available at all times for more advanced configurations.

---

### 5. **BTN_FORCE_NON_HAL**
- **Description**: Forces the use of non-HAL GPIO drivers (direct register access).
  - **When Enabled**: If set to `1`, the library will bypass the HAL (Hardware Abstraction Layer) and use direct register access for GPIO operations.
  - **When Disabled**: If set to `0`, the library will use the HAL GPIO functions, which may be more portable but less efficient.
  - **Use Case**: Enable this option if you want to improve performance by avoiding the overhead of HAL and directly accessing the GPIO registers.

---

## Default Time Configurations (Used if `BTN_DEFAULT_INIT` is enabled)

When `BTN_DEFAULT_INIT` is enabled, the following default times are used for button press actions during initialization with `ButtonInitKeyDefault`:
### 1. **BTN_DEFAULT_TIME_DEBOUNCE**
- **Description**: Defines the debounce time in milliseconds.
### 2. **BTN_DEFAULT_TIME_LONG_PRESS**
- **Description**: Defines the long press time in milliseconds.
### 3. **BTN_DEFAULT_TIME_REPEAT**
- **Description**: Defines the repeat time in milliseconds.

