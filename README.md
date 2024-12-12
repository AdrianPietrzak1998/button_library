
# Button Library - README

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

## Multiple Click Modes

### 1. **Normal Mode** (`BTN_MULTIPLE_CLICK_NORMAL_MODE`)
In this mode, the library tracks how many clicks occur within a short period (defined by `TimerBetweenClick`). If two clicks occur within this time, a `double-click` callback is triggered. Similarly, three clicks within the same time window trigger a `triple-click` callback.

**How it works**:
- The button waits for clicks within the specified time (`TimerBetweenClick`).
- If two clicks are detected, the `ButtonDoubleClick` callback is called.
- If three clicks are detected, the `ButtonTripleClick` callback is called.

### 2. **Combined Mode** (`BTN_MULTIPLE_CLICK_COMBINED_MODE`)
In this mode, the button can handle multiple clicks in a more advanced way, allowing for mixed presses. For example, a click can be combined with a long press, and multiple short presses can trigger different actions.

**Important Considerations**:
- **Additional Delay**: In combined mode, there is an additional delay due to the combined press detection. This can introduce some latency, which means the release callback may be triggered **before** the press callback in certain cases.
- **Callback Order**: Because of the delay in combined mode, you might see the release callback being triggered before the press callback, so ensure your callbacks can handle this order.

### Example:
```c
// Set the button to combined mode with a 500 ms interval between clicks
ButtonSetMultipleClick(&myButton, BTN_MULTIPLE_CLICK_COMBINED_MODE, 500);

// Register callbacks for the different click types
ButtonRegisterDoubleClickCallback(&myButton, myDoubleClickCallback);
ButtonRegisterTripleClickCallback(&myButton, myTripleClickCallback);
```

## Function List and Usage

### 1. **ButtonInitKeyDefault**
```c
void ButtonInitKeyDefault(button_t *Key, GPIO_TypeDef *GpioPort, uint16_t GpioPin, ReverseLogicGpio_t ReverseLogic, uint16_t Number);
```
- Initializes the button structure with default settings.
- Parameters:
  - `Key`: Pointer to the `button_t` structure.
  - `GpioPort`: GPIO port to which the button is connected.
  - `GpioPin`: GPIO pin number.
  - `ReverseLogic`: Logic level for button press (use `NON_REVERSE` or `REVERSE`).
  - `Number`: The button number (used to identify the button).

### 2. **ButtonSetDebounceTime**
```c
void ButtonSetDebounceTime(button_t *Key, uint32_t Miliseconds);
```
- Sets the debounce time for the button.
- Parameters:
  - `Key`: Pointer to the button structure.
  - `Miliseconds`: Debounce time in milliseconds.

### 3. **ButtonSetLongPressedTime**
```c
void ButtonSetLongPressedTime(button_t *Key, uint32_t Miliseconds);
```
- Sets the long press time for the button.
- Parameters:
  - `Key`: Pointer to the button structure.
  - `Miliseconds`: Long press time in milliseconds.

### 4. **ButtonSetRepeatTime**
```c
void ButtonSetRepeatTime(button_t *Key, uint32_t Miliseconds);
```
- Sets the repeat time for the button press.
- Parameters:
  - `Key`: Pointer to the button structure.
  - `Miliseconds`: Repeat time in milliseconds.

### 5. **ButtonRegisterPressCallback**
```c
void ButtonRegisterPressCallback(button_t *Key, void *Callback);
```
- Registers a callback for the button press event.
- Parameters:
  - `Key`: Pointer to the button structure.
  - `Callback`: Function to be called when the button is pressed.

### 6. **ButtonRegisterLongPressedCallback**
```c
void ButtonRegisterLongPressedCallback(button_t *Key, void *Callback);
```
- Registers a callback for the long press event.
- Parameters:
  - `Key`: Pointer to the button structure.
  - `Callback`: Function to be called when the button is long pressed.

### 7. **ButtonRegisterRepeatCallback**
```c
void ButtonRegisterRepeatCallback(button_t *Key, void *Callback);
```
- Registers a callback for the repeat event.
- Parameters:
  - `Key`: Pointer to the button structure.
  - `Callback`: Function to be called when the button is repeated.

### 8. **ButtonRegisterReleaseCallback**
```c
void ButtonRegisterReleaseCallback(button_t *Key, void *Callback);
```
- Registers a callback for the release event.
- Parameters:
  - `Key`: Pointer to the button structure.
  - `Callback`: Function to be called when the button is released.

### 9. **ButtonRegisterReleaseAfterRepeatCallback**
```c
void ButtonRegisterReleaseAfterRepeatCallback(button_t *Key, void *Callback);
```
- Registers a callback for the release after repeat event.
- Parameters:
  - `Key`: Pointer to the button structure.
  - `Callback`: Function to be called when the button is released after repeat.

### 10. **ButtonSetMultipleClick**
```c
void ButtonSetMultipleClick(button_t *Key, MultipleClickMode_t MultipleClickMode, uint32_t TimerBetweenClick);
```
- Sets the multiple click mode for the button.
- Parameters:
  - `Key`: Pointer to the button structure.
  - `MultipleClickMode`: Mode for multiple clicks (`BTN_MULTIPLE_CLICK_NORMAL_MODE`, `BTN_MULTIPLE_CLICK_COMBINED_MODE`).
  - `TimerBetweenClick`: Time in milliseconds between clicks.

## Conclusion
This library offers flexible and easy-to-use functions for managing button input in embedded systems. With support for multiple click modes, debouncing, long press detection, and repeat functionality, it is well-suited for a wide range of applications.

