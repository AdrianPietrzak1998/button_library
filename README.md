# Button Library for Embedded Systems

A robust, feature-rich button handling library for embedded systems with advanced state machine implementation, debouncing, and multi-click detection.

## Features

✅ **State Machine Based** - Clean, predictable button behavior  
✅ **Advanced Debouncing** - Eliminates mechanical bounce (press & release)  
✅ **Long Press Detection** - Configurable long press threshold  
✅ **Auto-Repeat** - Continuous callbacks while button held  
✅ **Multi-Click Support** - Double and triple click detection (normal & combined modes)  
✅ **Reverse Logic** - Support for active-high and active-low buttons  
✅ **Idle Detection** - Callback for button inactivity  
✅ **Error Handling** - All functions return status codes  
✅ **NULL-Safe** - Built-in pointer validation  
✅ **Portable** - Works with HAL or bare-metal  

---

## Quick Start

### 1. Basic Setup

```c
#include "button.h"

// Global tick variable (updated in SysTick interrupt)
volatile uint32_t g_system_tick = 0;

// Button instance
button_t my_button;

int main(void) {
    // Register tick source
    BTN_tick_variable_register(&g_system_tick);
    
    // Initialize button (default timings: 50ms debounce, 500ms long press, 300ms repeat)
    ButtonInitKeyDefault(&my_button, GPIOA, GPIO_PIN_0, NON_REVERSE, 1);
    
    // Register callbacks
    ButtonRegisterPressCallback(&my_button, on_button_press);
    ButtonRegisterReleaseCallback(&my_button, on_button_release);
    
    while(1) {
        ButtonTask(&my_button);  // Call regularly in main loop
        // ... other tasks
    }
}

void on_button_press(uint16_t button_id) {
    printf("Button %d pressed!\n", button_id);
}

void on_button_release(uint16_t button_id) {
    printf("Button %d released!\n", button_id);
}
```

### 2. With Custom Timings

```c
// Custom timings: 100ms debounce, 1000ms long press, 200ms repeat
ButtonInitKey(&my_button, GPIOB, GPIO_PIN_5, 100, 1000, 200, NON_REVERSE, 2);
```

---

## State Machine

The library uses a well-defined state machine for reliable button handling:

```
┌─────────────────────────────────────────────────────────────┐
│                       Button States                          │
└─────────────────────────────────────────────────────────────┘

    IDLE ──[press]──> DEBOUNCE ──[stable]──> PRESSED
     ↑                    │                      │
     │               [unstable]            [hold > long_time]
     │                    │                      │
     │                    ↓                      ↓
     │                  IDLE                  REPEAT ─┐
     │                                           │    │ [hold > repeat_time]
     │                                      [release] │
     │                                           │    │
     │                                           ↓    ↓
     └─────────[release]────── DEBOUNCE_RELEASE* ────┘
                                     │
                              [stable release]
                                     │
                                     ↓
                     ┌───────────────┴────────────────┐
                     │                                 │
                  RELEASE                   RELEASE_AFTER_REPEAT*
                     │                                 │
                     └────────> IDLE <─────────────────┘

* Optional states (configurable via macros)
```

### State Descriptions

| State | Description |
|-------|-------------|
| `IDLE` | Button is not pressed, waiting for input |
| `DEBOUNCE` | Filtering mechanical bounce after press detected |
| `PRESSED` | Button press confirmed, waiting for long press or release |
| `REPEAT` | Long press detected, triggering repeat callbacks |
| `DEBOUNCE_RELEASE` | Filtering mechanical bounce on release (optional) |
| `RELEASE` | Normal button release |
| `RELEASE_AFTER_REPEAT` | Release after repeat state (optional) |

---

## API Reference

### Initialization

#### `BTN_tick_variable_register()`
```c
BTN_operate_status BTN_tick_variable_register(BTN_TIME_t *Variable);
```
**Must be called first!** Registers the system tick variable.

**Parameters:**
- `Variable` - Pointer to system tick counter (incremented in interrupt)

**Returns:** `BTN_OK` or `BTN_ERROR`

---

#### `ButtonInitKey()`
```c
BTN_operate_status ButtonInitKey(button_t *Key, GPIO_TypeDef *GpioPort, 
                                 uint16_t GpioPin, BTN_TIME_t TimerDebounce,
                                 BTN_TIME_t TimerLongPressed, BTN_TIME_t TimerRepeat,
                                 ReverseLogicGpio_t ReverseLogic, uint16_t Number);
```
Initialize button with custom parameters.

**Parameters:**
- `Key` - Pointer to button structure
- `GpioPort` - GPIO port (e.g., GPIOA)
- `GpioPin` - GPIO pin (e.g., GPIO_PIN_0)
- `TimerDebounce` - Debounce time in ms (recommended: 20-100)
- `TimerLongPressed` - Long press threshold in ms (typical: 500-2000)
- `TimerRepeat` - Repeat interval in ms (typical: 100-500)
- `ReverseLogic` - `NON_REVERSE` (active-low) or `REVERSE` (active-high)
- `Number` - Button identifier (passed to callbacks)

**Returns:** `BTN_OK` or `BTN_ERROR`

---

#### `ButtonInitKeyDefault()`
```c
BTN_operate_status ButtonInitKeyDefault(button_t *Key, GPIO_TypeDef *GpioPort,
                                        uint16_t GpioPin, ReverseLogicGpio_t ReverseLogic,
                                        uint16_t Number);
```
Initialize button with default timings (50ms debounce, 500ms long press, 300ms repeat).

---

### State Machine

#### `ButtonTask()`
```c
BTN_operate_status ButtonTask(button_t *Key);
```
**Must be called regularly!** Processes button state machine. Call in main loop or timer interrupt.

**Returns:** `BTN_OK` or `BTN_ERROR`

---

### Callback Registration

All callback functions have signature: `void callback(uint16_t button_number)`

#### `ButtonRegisterPressCallback()`
Triggered when button press is confirmed (after debounce).

#### `ButtonRegisterLongPressedCallback()`
Triggered once when button held longer than `TimerLongPressed`.

#### `ButtonRegisterRepeatCallback()`
Triggered repeatedly every `TimerRepeat` ms while button is held.

#### `ButtonRegisterReleaseCallback()`
Triggered when button released after short press.

#### `ButtonRegisterReleaseAfterRepeatCallback()`
Triggered when button released after long press/repeat (requires `BTN_RELEASE_AFTER_REPEAT`).

#### `ButtonRegisterDoubleClickCallback()`
Triggered on double-click (requires `BTN_MULTIPLE_CLICK`).

#### `ButtonRegisterTripleClickCallback()`
Triggered on triple-click (requires `BTN_MULTIPLE_CLICK`).

**Example:**
```c
void my_callback(uint16_t btn_id) {
    switch(btn_id) {
        case 1: handle_button1(); break;
        case 2: handle_button2(); break;
    }
}

ButtonRegisterPressCallback(&button, my_callback);
```

---

### Configuration Functions

#### `ButtonSetDebounceTime()`
```c
BTN_operate_status ButtonSetDebounceTime(button_t *Key, BTN_TIME_t Milliseconds);
```
Change debounce time at runtime.

#### `ButtonSetLongPressedTime()`
Change long press threshold.

#### `ButtonSetRepeatTime()`
Change repeat interval.

#### `ButtonSetReleaseDebounceTime()`
Change release debounce time (requires `BTN_DOUBLE_DEBOUNCING`).

#### `ButtonSetMultipleClickTime()`
Change time window for multi-click detection (requires `BTN_MULTIPLE_CLICK`).

---

### Multi-Click Configuration

#### `ButtonSetMultipleClick()`
```c
BTN_operate_status ButtonSetMultipleClick(button_t *Key, 
                                          MultipleClickMode_t Mode,
                                          BTN_TIME_t TimerBetweenClick);
```

**Modes:**
- `BTN_MULTIPLE_CLICK_OFF` - Disable multi-click
- `BTN_MULTIPLE_CLICK_NORMAL_MODE` - Immediate callbacks for each click
- `BTN_MULTIPLE_CLICK_COMBINED_MODE` - Delayed callbacks after click sequence

**Normal Mode Behavior:**
```
Click 1: ButtonPressed(1)
Click 2: ButtonPressed(2) + ButtonDoubleClick(1)
Click 3: ButtonPressed(3) + ButtonTripleClick(1)
```

**Combined Mode Behavior:**
```
Click 1: [wait] → ButtonPressed(1)
Click 2: [wait] → ButtonDoubleClick(1)
Click 3: [wait] → ButtonTripleClick(1)
```

**Example:**
```c
ButtonSetMultipleClick(&btn, BTN_MULTIPLE_CLICK_NORMAL_MODE, 300);
ButtonRegisterDoubleClickCallback(&btn, on_double_click);
ButtonRegisterTripleClickCallback(&btn, on_triple_click);
```

---

### Idle Detection

#### `ButtonSetNonUsed()`
```c
BTN_operate_status ButtonSetNonUsed(button_t *Key, BTN_TIME_t Milliseconds,
                                    void *Callback);
```
Trigger callback after button is idle for specified time. Useful for sleep/timeout.

**Example:**
```c
void on_idle(uint16_t btn) {
    enter_low_power_mode();
}

ButtonSetNonUsed(&btn, 30000, on_idle);  // 30 seconds
```

---

## Configuration Options (`button_cfg.h`)

### Feature Flags

```c
#define BTN_RELEASE_AFTER_REPEAT 1  // Enable separate release callback after repeat
#define BTN_DOUBLE_DEBOUNCING 1     // Enable debouncing on release
#define BTN_MULTIPLE_CLICK 1        // Enable multi-click detection
#define BTN_DEFAULT_INIT 1          // Enable ButtonInitKeyDefault()
#define BTN_NON_USED_CALLBACK 1     // Enable idle detection
```

### Default Timings (when using `ButtonInitKeyDefault`)

```c
#define BTN_DEFAULT_TIME_DEBOUNCE 50      // ms
#define BTN_DEFAULT_TIME_LONG_PRESS 500   // ms
#define BTN_DEFAULT_TIME_REPEAT 300       // ms
```

### Platform Configuration

```c
#define BTN_FORCE_NON_HAL 1         // Use direct register access instead of HAL
#define BTN_USER_READ_PIN_ROUTINE 0 // Use custom GPIO read function
#define BTN_GPIO_PORT_T GPIO_TypeDef
#define BTN_GPIO_PIN_T uint16_t
```

---

## Advanced Usage Examples

### Example 1: Multiple Buttons

```c
button_t btn_up, btn_down, btn_enter;

void init_buttons(void) {
    BTN_tick_variable_register(&system_tick);
    
    ButtonInitKeyDefault(&btn_up, GPIOA, GPIO_PIN_0, NON_REVERSE, 1);
    ButtonInitKeyDefault(&btn_down, GPIOA, GPIO_PIN_1, NON_REVERSE, 2);
    ButtonInitKeyDefault(&btn_enter, GPIOA, GPIO_PIN_2, NON_REVERSE, 3);
    
    ButtonRegisterPressCallback(&btn_up, menu_up);
    ButtonRegisterPressCallback(&btn_down, menu_down);
    ButtonRegisterPressCallback(&btn_enter, menu_select);
}

void main_loop(void) {
    while(1) {
        ButtonTask(&btn_up);
        ButtonTask(&btn_down);
        ButtonTask(&btn_enter);
    }
}
```

### Example 2: Volume Control with Repeat

```c
button_t vol_up, vol_down;

void init_volume_buttons(void) {
    ButtonInitKeyDefault(&vol_up, GPIOB, GPIO_PIN_0, NON_REVERSE, 10);
    ButtonInitKeyDefault(&vol_down, GPIOB, GPIO_PIN_1, NON_REVERSE, 11);
    
    // Single press and auto-repeat
    ButtonRegisterPressCallback(&vol_up, volume_change);
    ButtonRegisterRepeatCallback(&vol_up, volume_change);
    
    ButtonRegisterPressCallback(&vol_down, volume_change);
    ButtonRegisterRepeatCallback(&vol_down, volume_change);
    
    // Faster repeat for volume
    ButtonSetRepeatTime(&vol_up, 100);
    ButtonSetRepeatTime(&vol_down, 100);
}

void volume_change(uint16_t btn) {
    if(btn == 10) volume++;
    else volume--;
    update_display();
}
```

### Example 3: Power Button with Long Press

```c
button_t power_btn;

void init_power_button(void) {
    ButtonInitKeyDefault(&power_btn, GPIOC, GPIO_PIN_13, NON_REVERSE, 99);
    
    ButtonRegisterPressCallback(&power_btn, toggle_backlight);
    ButtonRegisterLongPressedCallback(&power_btn, power_off_dialog);
}

void toggle_backlight(uint16_t btn) {
    backlight_enabled = !backlight_enabled;
}

void power_off_dialog(uint16_t btn) {
    show_shutdown_confirmation();
}
```

### Example 4: Secret Menu (Triple Click)

```c
button_t settings_btn;

void init_settings_button(void) {
    ButtonInitKeyDefault(&settings_btn, GPIOD, GPIO_PIN_8, NON_REVERSE, 50);
    
    ButtonSetMultipleClick(&settings_btn, BTN_MULTIPLE_CLICK_NORMAL_MODE, 400);
    
    ButtonRegisterPressCallback(&settings_btn, normal_settings);
    ButtonRegisterTripleClickCallback(&settings_btn, advanced_settings);
}

void normal_settings(uint16_t btn) {
    open_settings_menu();
}

void advanced_settings(uint16_t btn) {
    open_developer_menu();  // Hidden feature!
}
```

---

## Best Practices

### ✅ DO

- **Call `ButtonTask()` regularly** (every 1-10ms recommended)
- **Register tick variable before initializing buttons**
- **Check return values** in critical applications
- **Use debounce times appropriate for your buttons** (mechanical: 20-100ms, membrane: 10-50ms)
- **Test multi-click timings** with real users
- **Initialize callbacks to NULL** if not used (done automatically)

### ❌ DON'T

- **Don't call `ButtonTask()` from multiple contexts** without synchronization
- **Don't use very short debounce times** (<10ms) - wastes CPU
- **Don't ignore `BTN_ERROR` returns** in production code
- **Don't modify button structure directly** - use API functions
- **Don't forget to call `BTN_tick_variable_register()`** before initialization

---

## Troubleshooting

### Button not responding
- ✓ Check `BTN_tick_variable_register()` was called
- ✓ Verify `ButtonTask()` is being called regularly
- ✓ Check GPIO initialization (pull-up/down resistors)
- ✓ Verify correct `ReverseLogic` setting

### False triggers / bouncing
- ✓ Increase debounce time (`ButtonSetDebounceTime()`)
- ✓ Enable `BTN_DOUBLE_DEBOUNCING` for noisy buttons
- ✓ Check for electrical noise (add hardware filtering)

### Multi-click not working
- ✓ Verify `BTN_MULTIPLE_CLICK` is enabled in `button_cfg.h`
- ✓ Check `TimerBetweenClick` is appropriate (200-500ms typical)
- ✓ Ensure callbacks are registered
- ✓ Try `BTN_MULTIPLE_CLICK_NORMAL_MODE` first (simpler)

### Long press triggers too early/late
- ✓ Adjust with `ButtonSetLongPressedTime()`
- ✓ Typical range: 500-2000ms

---

## Testing

The library includes a comprehensive test suite (`button_test.c`). To run tests:

```bash
gcc -DUNIT_TESTING button.c button_test.c -o button_test
./button_test
```

**Test Coverage:**
- ✅ Tick registration
- ✅ Initialization (normal & default)
- ✅ Simple press/release
- ✅ Debouncing
- ✅ Long press detection
- ✅ Repeat functionality
- ✅ Release after repeat
- ✅ Double/triple click (normal & combined modes)
- ✅ State machine transitions
- ✅ Reverse logic
- ✅ Multiple independent buttons
- ✅ Runtime configuration changes
- ✅ Timer wraparound handling

---

## Memory Usage

**Per button instance:**
- Without optional features: ~40 bytes
- With all features enabled: ~72 bytes

**Code size (ARM Cortex-M, -Os):**
- Core functionality: ~1.2 KB
- With all features: ~2.5 KB

---

## Platform Support

**Tested on:**
- ✅ STM32 (HAL & LL)
- ✅ ESP32
- ✅ Nordic nRF52
- ✅ Generic ARM Cortex-M

**Requirements:**
- C99 or later
- System tick counter (1ms resolution recommended)
- GPIO read capability

---

## License

Mozilla Public License 2.0 (MPL-2.0)

---

## Author

**Adrian Pietrzak**  
GitHub: [@AdrianPietrzak1998](https://github.com/AdrianPietrzak1998)

---

## Changelog

### v2.0.0 (2025-01-19)
- ✨ Added comprehensive error handling (BTN_ERROR returns)
- ✨ Added NULL pointer validation
- ✨ Added tick registration API
- ✨ Fixed multi-click counting bug (now starts at 0)
- 🐛 Fixed uninitialized callback pointers
- 📝 Improved documentation
- ✅ Added comprehensive test suite

### v1.0.0 (2022-05-07)
- 🎉 Initial release
