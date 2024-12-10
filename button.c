/*
 * button.c
 *
 *  Created on: May 7, 2022
 *      Author: Adrian
 */

#include "main.h"
#include "button.h"

#define BTN_LIB_TICK uwTick //Miliseconds timer 32-bit

#define BTN_FORCE_NON_HAL 1

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

static uint8_t ReadState(const GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
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
}

#if MULTIPLE_CLICK
static void MultipleClickDebounce(button_t * Key)
{
	if(Key->MultipleClickMode == MULTIPLE_CLICK_OFF)
	{
		Key->State = PRESSED;
		Key->LastTick = BTN_LIB_TICK;
		if(Key->ButtonPressed != NULL)
		{
			Key->ButtonPressed(Key->NumberBtn);
		}
		return;
	}
	else if(Key->MultipleClickMode == NORMAL_MODE)
	{
		Key->State = PRESSED;
		Key->LastTick = BTN_LIB_TICK;
		if(Key->ButtonPressed != NULL)
		{
			Key->ButtonPressed(Key->NumberBtn);
		}
		if(BTN_LIB_TICK - Key->LastClickTick <= Key->TimerBetweenClick)
		{
			Key->ClickCounter++;
			if(Key->ClickCounter > 2)
			{
				Key->ClickCounter = 0;
				return;
			}
			switch(Key->ClickCounter)
			{
			case 1:
				if(Key->ButtonDoubleClick != NULL) Key->ButtonDoubleClick(Key->NumberBtn);
				break;
			case 2:
				if(Key->ButtonTripleClick != NULL) Key->ButtonTripleClick(Key->NumberBtn);
				break;
			default:
				break;
			}
		}
		else Key->ClickCounter = 0;
	}
	else if(Key->MultipleClickMode == COMBINED_MODE)
	{

	}
}
#endif

//Button init
void ButtonInitKey(button_t * Key, GPIO_TypeDef *GpioPort, uint16_t GpioPin, uint32_t TimerDebounce,
			uint32_t TimerLongPressed, uint32_t TimerRepeat, ReverseLogicGpio_t ReverseLogic, uint16_t Number)
{
	Key->State = IDLE;
	Key->GpioPort = GpioPort;
	Key->GpioPin = GpioPin;
	Key->TimerDebounce = TimerDebounce;
	Key->TimerLongPressed = TimerLongPressed;
	Key->TimerRepeat = TimerRepeat;

	Key->ReverseLogic = ReverseLogic;
	Key->NumberBtn = Number;
}
//Time settings function
void ButtonSetDebounceTime(button_t * Key, uint32_t Miliseconds)
{
	Key->TimerDebounce = Miliseconds;
}

void ButtonSetLongPressedTime(button_t *Key, uint32_t Miliseconds)
{
	Key->TimerLongPressed = Miliseconds;
}

void ButtonSetRepeatTime(button_t *Key, uint32_t Miliseconds)
{
	Key->TimerRepeat = Miliseconds;
}

//Callbacks
void ButtonRegisterPressCallback(button_t *Key, void *Callback)
{
	Key->ButtonPressed = Callback;
}

void ButtonRegisterLongPressedCallback(button_t *Key, void *Callback)
{
	Key->ButtonLongPressed = Callback;
}

void ButtonRegisterRepeatCallback(button_t *Key, void *Callback)
{
	Key->ButtonRepeat = Callback;
}

void ButtonRegisterReleaseCallback(button_t *Key, void *Callback)
{
	Key->ButtonRelease = Callback;
}

#if RELEASE_AFTER_REPEAT_EN
void ButtonRegisterReleaseAfterRepeatCallback(button_t *Key, void *Callback)
{
	Key->ButtonReleaseAfterRepeat = Callback;
}
#endif
#if MULTIPLE_CLICK
void ButtonRegisterDoubleClickCallback(button_t *Key, void *Callback)
{
	Key->ButtonDoubleClick = Callback;
}
void ButtonRegisterTripleClickCallback(button_t *Key, void *Callback)
{
	Key->ButtonTripleClick = Callback;
}
#endif
//States routine
void ButtonIdleRoutine(button_t *Key)
{
	if(ReadState(Key->GpioPort, Key->GpioPin) == (Key->ReverseLogic==0)?BTN_RESET:BTN_SET)
	{
		Key->LastTick = BTN_LIB_TICK;
		Key->State = DEBOUNCE;
	}
}

void ButtonDebounceRoutine(button_t *Key)
{
	if((BTN_LIB_TICK - Key->LastTick) >= Key->TimerDebounce)
	{
		if(ReadState(Key->GpioPort, Key->GpioPin) == (Key->ReverseLogic==0)?BTN_RESET:BTN_SET)
		{

#if MULTIPLE_CLICK
			MultipleClickDebounce(Key);
			Key->LastClickTick = BTN_LIB_TICK;
#else
			Key->State = PRESSED;
			Key->LastTick = BTN_LIB_TICK;
			if(Key->ButtonPressed != NULL)
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

void ButtonPressedRoutine(button_t *Key)
{
	if(ReadState(Key->GpioPort, Key->GpioPin) == (Key->ReverseLogic==0)?BTN_SET:BTN_RESET)
	{
		Key->State = RELEASE;
	}
	else if(BTN_LIB_TICK - Key->LastTick >= Key->TimerLongPressed)
	{
		Key->State = REPEAT;
		Key->LastTick = BTN_LIB_TICK;
		if(Key->ButtonLongPressed != NULL)
		{
			Key->ButtonLongPressed(Key->NumberBtn);
		}
	}
}

void ButtonRepeatRoutine(button_t *Key)
{
	if(ReadState(Key->GpioPort, Key->GpioPin) == (Key->ReverseLogic==0)?BTN_SET:BTN_RESET)
	{
#if !RELEASE_AFTER_REPEAT_EN
		Key->State = RELEASE;
#else
		Key->State = RELEASE_AFTER_REPEAT;
#endif
	}
	else if(BTN_LIB_TICK - Key->LastTick >= Key->TimerRepeat)
	{
		Key->LastTick = BTN_LIB_TICK;
		if(Key->ButtonRepeat != NULL)
		{
			Key->ButtonRepeat(Key->NumberBtn);
		}
	}
}

void ButtonReleaseRoutine(button_t *Key)
{
	if(Key->ButtonRelease != NULL)
	{
		Key->ButtonRelease(Key->NumberBtn);
	}
	Key->State = IDLE;
}
#if RELEASE_AFTER_REPEAT_EN
void ButtonReleaseAfterRepeatRoutine(button_t *Key)
{
	if(Key->ButtonReleaseAfterRepeat != NULL)
	{
		Key->ButtonReleaseAfterRepeat(Key->NumberBtn);
	}
	Key->State = IDLE;
}
#endif

//State machines
void ButtonTask(button_t *Key)
{
	switch(Key->State)
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

	case RELEASE:
		ButtonReleaseRoutine(Key);
		break;

#if RELEASE_AFTER_REPEAT_EN
	case RELEASE_AFTER_REPEAT:
		ButtonReleaseAfterRepeatRoutine(Key);
		break;
#endif
	}
}

#ifdef HAL_TO_DEFINE
#define USE_HAL_DRIVER
#endif
