/**
 * @file libsml/SingleThreadPIDController.c
 * @brief Source file for single-threaded PIDController functions.	
 *
 * Copyright (c) 2014-2015 Olympic Steel Eagles. All rights reserved.
 * Portions of this file may contain elements from the PROS API.
 * See include/API.h for additional notice.
 *********************************************************************/

#include "main.h"
#include "sml/SingleThreadPIDController.h"
#include <math.h>

/**
 * @brief Creates a PIDController struct based off of the parameters
 *
 * @param Execute
 *        A function pointer to the output function of type void (*func)(int,bool). Takes a parameter int, the output, and a bool, the immediate value
 *        Ex. void ChassisSet(int val, bool immediate); PIDControllerCreate(&ChassisSet, ...);
 *
 * @param Call
 *        A function pointer to the input function of type int (*func)(void). Takes no parameters and returns the input
 *        See ex. for void(*Execute)(int, bool)
 *
 * @param Kp
 *        The proportional constant
 *
 * @param Ki
 *        The integral constant
 *
 * @param Kd
 *        The derivative constant
 *
 * @param MaxIntegral
 *        Maximum value of the integral to prevent integral windup
 *
 * @param MinIntegral
 *        Minimum value of the integral to prevent integral windup, usually the same magnitude as MinIntegral
 *
 * @param AcceptableTolerance
 *        Tolerance of the controller
 *
 * @returns Returns a PIDController struct representing the controller supplied in the parameters
 */
PIDController PIDControllerCreate(void(*Execute)(int, bool), int(*Call)(void), double Kp, double Ki, double Kd, int MaxIntegral, int MinIntegral, int AcceptableTolerance)
{
	PIDController controller;
	controller.Execute = Execute;
	controller.Call = Call;
	controller.Kp = Kp;
	controller.Ki = Ki;
	controller.Kd = Kd;
	controller.MaxIntegral = MaxIntegral;
	controller.MinIntegral = MinIntegral;
	controller.AcceptableTolerance = AcceptableTolerance;
	return controller;
}

/**
 * @brief Resets the PIDController by setting the goal, integral, and prevError to 0
 *
 * @param controller
 *		A pointer to a PIDController struct containing the necessary constants and container values
 */
void PIDControllerReset(PIDController *controller)
{
	controller->Goal = 0;
	controller->integral = 0;
	controller->prevError = 0;
}

/**
 * @brief Computes and returns the output of one pass of the PID Controller using the controller.Goal and controller.Call()
 *
 * @param controller
 *		A pointer to a PIDController struct containing the necessary constants and container values
 * 
 * @returns Returns the output computed by running the controller and using the standard (non-rinsed/modified) goal - current error
 */
int PIDControllerCompute(PIDController *controller)
{
	return PIDControllerComputer(controller, controller->Goal - controller->Call());
}

/**
 * @brief Computes and returns the PID controller with the given error. Will not use controller.Call()
 *        Integral, prevError, and prevTime will continue to pass through each iteration.
 *
 * @param controller
 *        A pointer to a PIDController struct containing the necessary constants and container values
 *
 * @param error
 *        Provided error usually Input - Goal
 *
 * @returns Returns the controller's output based off of the supplied error.
 */
int PIDControllerComputer(PIDController *controller, int error)
{
	controller->integral += error;
	if (controller->integral < controller->MinIntegral)
		controller->integral = controller->MinIntegral;
	else if (controller->integral > controller->MaxIntegral)
		controller->integral = controller->MaxIntegral;
	if (abs(error) < abs(controller->AcceptableTolerance)) // 
		controller->integral = 0;


	long derivative = (error - controller->prevError) /
		((micros() - controller->prevTime) * 1000000); // get true estimated instantaneous change in ticks/sec

	int out = (int)((controller->Kp * error) + (controller->Ki * controller->integral) + (controller->Kd * derivative));


	if (abs(error) < abs(controller->AcceptableTolerance))
		out = 0;

	controller->prevTime = micros();
	controller->prevError = error;

	return out;
}

/**
 * @brief Computes and executes one pass of the PID Controller
 *
 * @param controller
 *        A pointer to a PIDController struct containing the necessary constants and container values
 *
 * @return True or false if the input parameter is within the acceptable tolerance of the goal
 */
bool PIDControllerExecuteContinuous(PIDController *controller)
{
	controller->Execute(PIDControllerCompute(controller), false);
	if (abs(controller->Goal - controller->Call()) < controller->AcceptableTolerance)
		return true;
	else return false;
}

/**
 * @brief Computes and executes the PIDController to completion
 *
 * @param controller
 *        A pointer to a PIDController struct containing the necessary constants and container values
 */
void PIDControllerExecuteCompletion(PIDController *controller)
{
	while (PIDControllerExecuteContinuous(controller))
		delay(DEFAULT_INTERVAL);
}

/**
 * @brief Resets the PIDController and sets the goal to parameter
 *
 * @param controller
 *         A pointer to a PIDController struct containing the necessary constants and container values
 *
 * @param goal
 *        The goal value
 */
void PIDControllerSetGoal(PIDController *controller, int goal)
{
	PIDControllerReset(controller);
	controller->Goal = goal;
}