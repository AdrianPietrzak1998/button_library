/*
 * button.c
 *
 *  Created on: May 7, 2022
 *      Author: Adrian
 */

#include "main.h"
#include "button.h"

#define BTN_LIB_TICK uwTick //Miliseconds timer 32-bit

#define	DEFAULT_TIME_DEBOUNCE 50
#define	DEFAULT_TIME_LONG_PRESS 500
#define	DEFAULT_TIME_REPEAT 300

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
#if BTN_DEFAULT_INIT
void ButtonInitKeyDefault(button_t * Key, GPIO_TypeDef *GpioPort, uint16_t GpioPin,
		ReverseLogicGpio_t ReverseLogic, uint16_t Number)
{
	Key->State = IDLE;
	Key->GpioPort = GpioPort;
	Key->GpioPin = GpioPin;
	Key->TimerDebounce = DEFAULT_TIME_DEBOUNCE;
	Key->TimerLongPressed = DEFAULT_TIME_LONG_PRESS;
	Key->TimerRepeat = DEFAULT_TIME_REPEAT;

	Key->ReverseLogic = ReverseLogic;
	Key->NumberBtn = Number;
}
#endif

#if BTN_MULTIPLE_CLICK
static void MultipleClickDebounce(button_t * Key)
{
	if(Key->MultipleClickMode == BTN_MULTIPLE_CLICK_OFF)
	{
//		Key->State = PRESSED;
		Key->LastTick = BTN_LIB_TICK;
		if(Key->ButtonPressed != NULL)
		{
			Key->ButtonPressed(Key->NumberBtn);
		}
		return;
	}
	else if(Key->MultipleClickMode == NORMAL_MODE)
	{
//		Key->State = PRESSED;
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
	else if(Key->MultipleClickMode == COMBINED_MODE && Key->ClickCounterCycle == 0)
	{
		Key->ClickCounterCycle = 1;
		if(BTN_LIB_TICK - Key->LastClickTick <= Key->TimerBetweenClick)
		{
			Key->ClickCounter++;
			if(Key->ClickCounter > 3)
			{
				Key->ClickCounter = 0;
				return;
			}
		}
		else Key->ClickCounter = 1;

	}
}

static void multipleClikIdle(button_t * Key)
{
	if(Key->MultipleClickMode != COMBINED_MODE) return;
	Key->CombinedModeRepeatPressEx = 0;
	Key->ClickCounterCycle = 0;
	if(BTN_LIB_TICK - Key->LastClickTick > Key->TimerBetweenClick)
	{
		switch(Key->ClickCounter)
		{
		case 1:
			if(Key->ButtonPressed != NULL) Key->ButtonPressed(Key->NumberBtn);
			break;
		case 2:
			if(Key->ButtonDoubleClick != NULL) Key->ButtonDoubleClick(Key->NumberBtn);
			break;
		case 3:
			if(Key->ButtonTripleClick != NULL) Key->ButtonTripleClick(Key->NumberBtn);
			break;
		default:
			break;
		}
		Key->ClickCounter = 0;
	}
}

static void multipleClikRepeat(button_t * Key)
{
	if(Key->ButtonPressed != NULL && !Key->CombinedModeRepeatPressEx)
	{
		Key->ButtonPressed(Key->NumberBtn);
		Key->CombinedModeRepeatPressEx = 1;
	}
	Key->ClickCounter = 0;
}
#endif


//States routine
static void ButtonIdleRoutine(button_t *Key)
{
#if BTN_MULTIPLE_CLICK
	multipleClikIdle(Key);
#endif
	if(ReadState(Key->GpioPort, Key->GpioPin) == (Key->ReverseLogic==0)?BTN_RESET:BTN_SET)
	{
		Key->LastTick = BTN_LIB_TICK;
		Key->State = DEBOUNCE;
	}
}

static void ButtonDebounceRoutine(button_t *Key)
{
	if((BTN_LIB_TICK - Key->LastTick) >= Key->TimerDebounce)
	{
		if(ReadState(Key->GpioPort, Key->GpioPin) == (Key->ReverseLogic==0)?BTN_RESET:BTN_SET)
		{

#if BTN_MULTIPLE_CLICK
			MultipleClickDebounce(Key);
			Key->State = PRESSED;
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

static void ButtonPressedRoutine(button_t *Key)
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

static void ButtonRepeatRoutine(button_t *Key)
{
#if BTN_MULTIPLE_CLICK
	multipleClikRepeat(Key);
#endif
	if(ReadState(Key->GpioPort, Key->GpioPin) == (Key->ReverseLogic==0)?BTN_SET:BTN_RESET)
	{
#if !BTN_RELEASE_AFTER_REPEAT
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

static void ButtonReleaseRoutine(button_t *Key)
{
	if(Key->ButtonRelease != NULL)
	{
		Key->ButtonRelease(Key->NumberBtn);
	}
	Key->State = IDLE;
}
#if BTN_RELEASE_AFTER_REPEAT
static void ButtonReleaseAfterRepeatRoutine(button_t *Key)
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

#if BTN_RELEASE_AFTER_REPEAT
	case RELEASE_AFTER_REPEAT:
		ButtonReleaseAfterRepeatRoutine(Key);
		break;
#endif
	}
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

#if BTN_RELEASE_AFTER_REPEAT
void ButtonRegisterReleaseAfterRepeatCallback(button_t *Key, void *Callback)
{
	Key->ButtonReleaseAfterRepeat = Callback;
}
#endif
#if BTN_MULTIPLE_CLICK
void ButtonRegisterDoubleClickCallback(button_t *Key, void *Callback)
{
	Key->ButtonDoubleClick = Callback;
}
void ButtonRegisterTripleClickCallback(button_t *Key, void *Callback)
{
	Key->ButtonTripleClick = Callback;
}
#endif


#ifdef HAL_TO_DEFINE
#define USE_HAL_DRIVER
#endif
