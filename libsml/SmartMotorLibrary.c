/**
 * @file libsml/SmartMotorLibrary.c
 * @brief Source file for SML functions.
 *
 * Copyright (c) 2014-2015 Olympic Steel Eagles. All rights reserved.
 * Portions of this file may contain elements from the PROS API.
 * See include/API.h for additional notice.
 ********************************************************************/

#include "main.h"
#include "sml/SmartMotorLibrary.h"
#include "lcd/LCDFunctions.h"

#define MOTOR_SKEWER_DELTAT	50

static Motor Motors[10];
static Mutex Mutexes[10];
static TaskHandle MotorManagerTaskHandle;

/**
 * @brief The default recalculate function for RecalculateCommanded (takes input and returns it)
 *        This method is only accessible to this file for organizational purposes and may be opened to other files.
 * @param in
 *        Input value
 */
static int DefaultRecalculate(int in)
{
	return in;
}

/**
 * @brief Initializes the Motor Manager Task by creating the Motor Mutexes and starting the task.
 */
void InitializeMotorManager()
{
	for (int i = 0; i < 10; i++)
	{
		Mutexes[i] = mutexCreate();
		Motors[i].RecalculateCommanded = &DefaultRecalculate;
	}
	MotorManagerTaskHandle = taskCreate(MotorManagerTask, TASK_DEFAULT_STACK_SIZE, NULL, TASK_PRIORITY_DEFAULT+1);
}

/**
 * @brief Kills the motor manager task, if it exists
 */
void StopMotorManager()
{
	if (MotorManagerTaskHandle != NULL) // passing NULL kills current thread, so don't allow that to happen
		taskDelete(MotorManagerTaskHandle);
}

/**
 * @brief The motor manager task processes all the motors and determines if a change needs to be made to the motor speed and executes the change if necessary
 *        This task is initialized by the InitializeMotorManager method. Do not manually create this task.
 */
void MotorManagerTask(void *none)
{
	while (true)
	{
		for (int i = 0; i < 10; i++)
		{
			if (!mutexTake(Mutexes[i], MUTEX_TAKE_TIMEOUT)) // Grab mutex if possible, if it's not available (being changed by MotorSet()), skip the motor check.
				continue;

			if (motorGet(i+1) != Motors[i].RecalculateCommanded(Motors[i].commanded)) // Motor has not been set to target
			{
				int current = motorGet(i+1);
				int command = Motors[i].RecalculateCommanded(Motors[i].commanded);
				double skew = Motors[i].skewPerMsec;

				if (abs(command - current) < (skew * (millis() - Motors[i].lastUpdate))) // If skew is less than required delta-PWM, set commanded to output
					motorSet(i+1, command);

				

				// Add appropriate motor skew value to current speed
				else
					motorSet(i+1, (current + (int)(skew * (millis() - Motors[i].lastUpdate) * (command - current > 0 ? 1 : -1))));

			}
			Motors[i].lastUpdate = millis();
			mutexGive(Mutexes[i]);
		}
		delay(MOTOR_SKEWER_DELTAT);
	}
}

/**
 * @brief Change the motor speed
 *
 * @param channel
 *        The port of the motor (1-10)
 *
 * @param set
 *        The PWM value of the motor to set it to. Will be checked and forced back into the bounds [-127,127]
 *
 * @param immediate
 *        Will change the speed of the motor immediately, bypassing the motor manager ramping if set to true.
 */
bool MotorSet(int channel, int set, bool immediate)
{
	if (channel > 10 || channel < 1)
		return false;
	if (abs(set) > 127)
		set = 127 * (set / abs(set));

	channel--;

	if (!mutexTake(Mutexes[channel], MUTEX_TAKE_TIMEOUT))
		return false;
	Motors[channel].commanded = set * (Motors[channel].inverted ? 1 : -1);

	if (immediate)
		motorSet(channel + 1, set * (Motors[channel].inverted ? 1 : -1));
	mutexGive(Mutexes[channel]);
	return true;
}

/**
 * @brief Returns the normalized commanded speed of the motor
 *
 * @param channel
 *			The port of the motor [1,10]
 */
int MotorGet(int channel)
{
	if (channel > 10 || channel < 1)
		return 0;
	channel--;

	if (!mutexTake(Mutexes[channel], MUTEX_TAKE_TIMEOUT))
		return 0;

	int ret = Motors[channel].commanded * (Motors[channel].inverted ? 1 : -1);

	mutexGive(Mutexes[channel]);
	return ret;
}

/**
 * @brief Configures a motor port with inversion and skew
 *
 * @param channel
 *        The port of the motor [1,10]
 *
 * @param inverted
 *        If the motor port is inverted, set to true (127 will become -127 and vice versa)
 *
 * @param skewPerMsec
 *        The acceleration of the motor in dPWM/millisecond
 */
void MotorConfigure(int channel, bool inverted, double skewPerMsec)
{
	if (channel < 0 || channel > 11)
		return;

	channel--;

	if (!mutexTake(Mutexes[channel], MUTEX_TAKE_TIMEOUT))
		return;

	Motors[channel].channel = channel + 1;
	Motors[channel].inverted = inverted;
	Motors[channel].skewPerMsec = skewPerMsec;
	Motors[channel].RecalculateCommanded = &DefaultRecalculate;

	mutexGive(Mutexes[channel]);
}

/**
 * @brief Sets the recalculate commanded to the provided function pointer.
 *        Raw input values will be recalculated using the function.
 *        Example usage: convert raw speed to tune to a speed of an encoder (i.e. consistent speed)
 *
 * @param channel
 *        The port of the motor [1,10]
 *
 * @param func
 *        A pointer to a function taking an integer input (the raw input) and returning the corrected output as an int
 */
void MotorChangeRecalculateCommanded(int channel, int(*func)(int))
{
	if (channel < 0 || channel > 11)
		return;

	channel--;

	if (!mutexTake(Mutexes[channel], MUTEX_TAKE_TIMEOUT))
		return;

	Motors[channel].RecalculateCommanded = func;

	mutexGive(Mutexes[channel]);
}