/*
 * button.c
 *
 *  Created on: May 7, 2022
 *      Author: Adrian
 */

#include "main.h"
#include "button.h"



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
//States routine
void ButtonIdleRoutine(button_t *Key)
{
	if(HAL_GPIO_ReadPin(Key->GpioPort, Key->GpioPin) == (Key->ReverseLogic==0)?GPIO_PIN_RESET:GPIO_PIN_SET)
	{
		Key->LastTick = HAL_GetTick();
		Key->State = DEBOUNCE;
	}
}

void ButtonDebounceRoutine(button_t *Key)
{
	if((HAL_GetTick() - Key->LastTick) >= Key->TimerDebounce)
	{
		if(HAL_GPIO_ReadPin(Key->GpioPort, Key->GpioPin) == (Key->ReverseLogic==0)?GPIO_PIN_RESET:GPIO_PIN_SET)
		{
			Key->State = PRESSED;
			Key->LastTick = HAL_GetTick();
			if(Key->ButtonPressed != NULL)
			{
				Key->ButtonPressed(Key->NumberBtn);
			}
		}
		else
		{
			Key->State = IDLE;
		}
	}
}

void ButtonPressedRoutine(button_t *Key)
{
	if(HAL_GPIO_ReadPin(Key->GpioPort, Key->GpioPin) == (Key->ReverseLogic==0)?GPIO_PIN_SET:GPIO_PIN_RESET)
	{
		Key->State = RELEASE;
	}
	else if(HAL_GetTick() - Key->LastTick >= Key->TimerLongPressed)
	{
		Key->State = REPEAT;
		Key->LastTick = HAL_GetTick();
		if(Key->ButtonLongPressed != NULL)
		{
			Key->ButtonLongPressed(Key->NumberBtn);
		}
	}
}

void ButtonRepeatRoutine(button_t *Key)
{
	if(HAL_GPIO_ReadPin(Key->GpioPort, Key->GpioPin) == (Key->ReverseLogic==0)?GPIO_PIN_SET:GPIO_PIN_RESET)
	{
#if !RELEASE_AFTER_REPEAT_EN
		Key->State = RELEASE;
#else
		Key->State = RELEASE_AFTER_REPEAT;
#endif
	}
	else if(HAL_GetTick() - Key->LastTick >= Key->TimerRepeat)
	{
		Key->LastTick = HAL_GetTick();
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
