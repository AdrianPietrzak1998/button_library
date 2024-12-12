/*
 * button.h
 *
 *  Created on: May 7, 2022
 *      Author: Adrian
 */

#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

// States of state machine
#define RELEASE_AFTER_REPEAT_EN 1
#define MULTIPLE_CLICK 1



typedef enum{
	IDLE = 0,
	DEBOUNCE,
	PRESSED,
	REPEAT,
	RELEASE
#if RELEASE_AFTER_REPEAT_EN
	,RELEASE_AFTER_REPEAT
#endif
} BUTTON_STATE;

typedef enum{
	NON_REVERSE = 0,
	REVERSE
}ReverseLogicGpio_t;

typedef enum{
	MULTIPLE_CLICK_OFF = 0,
	NORMAL_MODE,
	COMBINED_MODE
}MultipleClickMode_t;

// Struct for buttons
typedef struct
{
	BUTTON_STATE  State;

	GPIO_TypeDef *GpioPort;
	uint16_t      GpioPin;

	uint32_t      LastTick;
	uint32_t      TimerDebounce;
	uint32_t	  TimerLongPressed;
	uint32_t	  TimerRepeat;

	ReverseLogicGpio_t ReverseLogic;
	uint16_t	  	   NumberBtn;

	void(*ButtonPressed)(uint16_t);
	void(*ButtonLongPressed)(uint16_t);
	void(*ButtonRepeat)(uint16_t);
	void(*ButtonRelease)(uint16_t);
#if RELEASE_AFTER_REPEAT_EN
	void(*ButtonReleaseAfterRepeat)(uint16_t);
#endif
#if MULTIPLE_CLICK
	void(*ButtonDoubleClick)(uint16_t);
	void(*ButtonTripleClick)(uint16_t);

	MultipleClickMode_t MultipleClickMode;
	uint8_t 			ClickCounter:6;
	uint8_t				ClickCounterCycle:1;
	uint8_t				CombinedModeRepeatPressEx:1;
	uint32_t	  		TimerBetweenClick;
	uint32_t			LastClickTick;
#endif

} button_t;

// Public function
void ButtonInitKey(button_t * Key, GPIO_TypeDef *GpioPort, uint16_t GpioPin, uint32_t TimerDebounce, uint32_t TimerLongPressed,
					uint32_t TimerRepeat, ReverseLogicGpio_t ReverseLogic, uint16_t Number); // Initialization for state machine
#if MULTIPLE_CLICK
void ButtonSetMultipleClick(button_t * Key, MultipleClickMode_t MultipleClickMode, uint32_t TimerBetweenClick);
#endif
void ButtonTask(button_t *Key); //Task for working state machine

//Registers functions need Callback as a pointer
void ButtonRegisterPressCallback(button_t *Key, void *Callback); //For first press key
void ButtonRegisterLongPressedCallback(button_t *Key, void *Callback); //If key was long pressed
void ButtonRegisterRepeatCallback(button_t *Key, void *Callback); //While key is long pressed
void ButtonRegisterReleaseCallback(button_t *Key, void *Callback); //If key was released
#if RELEASE_AFTER_REPEAT_EN
void ButtonRegisterReleaseAfterRepeatCallback(button_t *Key, void *Callback);
#endif
#if MULTIPLE_CLICK
void ButtonRegisterDoubleClickCallback(button_t *Key, void *Callback);
void ButtonRegisterTripleClickCallback(button_t *Key, void *Callback);
#endif

void ButtonSetDebounceTime(button_t * Key, uint32_t Miliseconds);
void ButtonSetLongPressedTime(button_t *Key, uint32_t Miliseconds);
void ButtonSetRepeatTime(button_t *Key, uint32_t Miliseconds);

#endif /* INC_BUTTON_H_ */
