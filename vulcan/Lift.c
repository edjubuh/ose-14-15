/**
 * @file vulcan/lift.c		
 * @brief Source file for lift functions.
 *
 * Copyright(c) 2014-2015 Olympic Steel Eagles.All rights reserved. <br>
 * Portions of this file may contain elements from the PROS API. <br>
 * See include/API.h for additional notice.
 ********************************************************************************/

#include <string.h>

#include "main.h"
#include "vulcan/Lift.h"

#include "sml/SmartMotorLibrary.h"
#include "sml/MasterSlavePIDController.h"
#include "sml/SingleThreadPIDController.h"

#include "vulcan/CortexDefinitions.h"

#define IME_RESET_THRESHOLD		100
#define POT_RESET_THRESHOLD		200

static Encoder leftEncoder, rightEncoder;

static MasterSlavePIDController Controller;
static TaskHandle LiftControllerTask;

// ---------------- LEFT  SIDE ---------------- //
/**
 * @brief Sets the speed of the left side of the lift
 *
 * @param value
 *		[-100,127] Desired PWM value of the left side of the lift
 *
 * @param immediate
 *		Determines if speed input change is immediate or ramped according to SML
 */
void LiftSetLeft(int value, bool immediate)
{
	if ((digitalRead(DIG_LIFT_BOTLIM) == LOW && value < 0) || (digitalRead(DIG_LIFT_TOPLIM) == LOW && value > 0))
	{
		MotorSet(MOTOR_LIFT_FRONTLEFT, 0, true);
		MotorSet(MOTOR_LIFT_REARLEFT, 0, true);
		MotorSet(MOTOR_LIFT_MIDDLELEFT, 0, true);
	}
	else if (value < -100)
	{
		MotorSet(MOTOR_LIFT_FRONTLEFT,  -100, immediate);
		MotorSet(MOTOR_LIFT_REARLEFT,   -100, immediate);
		MotorSet(MOTOR_LIFT_MIDDLELEFT, -100, immediate);
	}
	else
	{
		MotorSet(MOTOR_LIFT_FRONTLEFT,  value, immediate);
		MotorSet(MOTOR_LIFT_REARLEFT,   value, immediate);
		MotorSet(MOTOR_LIFT_MIDDLELEFT, value, immediate);
	}
}

/**
* @brief Returns the calibrated value of the left lift IME
*/
int LiftGetCalibratedIMELeft()
{
	static int prevValues[10];
	for (int i = 0; i < 9; i++)
		prevValues[i] = prevValues[i+1];
	prevValues[9] = LiftGetRawIMELeft();

	if (digitalRead(DIG_LIFT_BOTLIM) == LOW)
	{
		imeReset(I2C_MOTOR_LIFT_LEFT);
		memset(prevValues, 0, sizeof(prevValues));
		return 0;
	}

	int sum = 0;
	for (int i = 0; i < 10; i++)
		sum += prevValues[i];
	return (int)(sum / 10.0);
}

/**
* @brief Returns the raw value of the left lift IME (corrected for inversion)
*/
int LiftGetRawIMELeft()
{
	int val;
	imeGet(I2C_MOTOR_LIFT_LEFT, &val);
	return val;
}

/**
 * @brief Returns the value of the left lift quadrature encoder (located at the top of the lift for stabilization)
 */
int LiftGetQuadEncLeft()
{
	if (digitalRead(DIG_LIFT_BOTLIM) == LOW && encoderGet(leftEncoder) != 0)
		encoderReset(leftEncoder);
	return encoderGet(leftEncoder);
}

// ---------------- RIGHT SIDE ---------------- //
/**
 * @brief Sets the speed of the right side of the lift
 *
 * @param value
 *		[-127,127] Desired PWM value of the right side of the lift
 *
 * @param immediate
 *		Determines if speed input change is immediate or ramped according to SML
 */
void LiftSetRight(int value, bool immediate)
{
	if ((digitalRead(DIG_LIFT_BOTLIM) == LOW && value < 0) || (digitalRead(DIG_LIFT_TOPLIM) == LOW && value > 0))
	{
		MotorSet(MOTOR_LIFT_FRONTRIGHT,  0, true);
		MotorSet(MOTOR_LIFT_REARRIGHT,   0, true);
		MotorSet(MOTOR_LIFT_MIDDLERIGHT, 0, true);
	}
	else if (value < -100)
	{
		MotorSet(MOTOR_LIFT_FRONTRIGHT,  -100, immediate);
		MotorSet(MOTOR_LIFT_REARRIGHT,   -100, immediate);
		MotorSet(MOTOR_LIFT_MIDDLERIGHT, -100, immediate);
	}
	else
	{
		MotorSet(MOTOR_LIFT_FRONTRIGHT,  value, immediate);
		MotorSet(MOTOR_LIFT_REARRIGHT,   value, immediate);
		MotorSet(MOTOR_LIFT_MIDDLERIGHT, value, immediate);
	}
}

/**
 * @brief Returns the calibrated value of the right lift IME
 */
int LiftGetCalibratedIMERight()
{
	static int prevValues[10];
	for (int i = 0; i < 9; i++)
		prevValues[i] = prevValues[i+1];
	prevValues[9] = LiftGetRawIMERight();

	if (digitalRead(DIG_LIFT_BOTLIM) == LOW)
	{
		imeReset(I2C_MOTOR_LIFT_RIGHT);
		memset(prevValues, 0, sizeof(prevValues));
		return 0;
	}

	int sum = 0;
	for (int i = 0; i < 10; i++)
		sum += prevValues[i];
	return (int)(sum / 10.0);
}

/** 
 * @brief Returns the raw value of the right lift IME (corrected for inversion)
 */
int LiftGetRawIMERight()
{
	int val;
	imeGet(I2C_MOTOR_LIFT_RIGHT, &val);
	return -val;
}

/**
* @brief Returns the value of the left right quadrature encoder (located at the top of the lift for stabilization)
*/
int LiftGetQuadEncRight()
{
	if (digitalRead(DIG_LIFT_BOTLIM) == LOW && encoderGet(rightEncoder) != 0)
		encoderReset(rightEncoder);
	return encoderGet(rightEncoder);
}


// ---------------- MASTER (ALL) ---------------- //
/**
 * @brief Sets the lift to the desired speed using the MasterSlavePIDController for the lift
 *
 * @param value
 *			[-127,127] Speed of the lift
 *
 * @param immediate
 *			Determines if speed input change is immediate or ramped according to SML
 */
void LiftSet(int value, bool immediate)
{
	// For enabled MasterSlavePIDController
	MasterSlavePIDSetOutput(&Controller, value);

	// For disabled MasterSlavePIDControlller
	//LiftSetLeft(value, immediate);
	//LiftSetRight(value, immediate);
}

/**
 * @brief Sets the lift goal height to the supplied parameter.
 *
 * @param value
 *			The new goal height of the lift.
 */
bool LiftSetHeight(int value)
{
	MasterSlavePIDSetGoal(&Controller, value);
	return false;
}

/**
 * @brief Returns the difference between the IMES (right - left)
 *		  Used in the equailizer controller in the MasterSlavePIDController for the lift
 */
static int liftComputeIMEDiff()
{
	return LiftGetCalibratedIMERight() - LiftGetCalibratedIMELeft();
}

/**
 * @brief Returns the difference between the quadrature encoders (right - left)
 *		  Used in the equalizer controller in the MasterSlavePIDController for the lift
 */
static int liftComputeQuadEncDiff()
{
	return LiftGetQuadEncRight() - LiftGetQuadEncLeft();
}

/**
 * @brief Initializes the lift motors and PID controllers
 */
void LiftInitialize()
{
	MotorConfigure(MOTOR_LIFT_FRONTLEFT, true, 0.25);
	MotorConfigure(MOTOR_LIFT_FRONTRIGHT, false, 0.25);
	MotorConfigure(MOTOR_LIFT_MIDDLELEFT, true, 0.25);
	MotorConfigure(MOTOR_LIFT_MIDDLERIGHT, false, 0.25);
	MotorConfigure(MOTOR_LIFT_REARLEFT, false, 0.25);
	MotorConfigure(MOTOR_LIFT_REARRIGHT, false, 0.25);
		
	leftEncoder = encoderInit(DIG_LIFT_ENC_LEFT_TOP, DIG_LIFT_ENC_LEFT_BOT, false);
	rightEncoder = encoderInit(DIG_LIFT_ENC_RIGHT_TOP, DIG_LIFT_ENC_RIGHT_BOT, true);
	
	//                                           Execute           Call			    Kp    Ki   Kd MaI  MiI  Tol
	PIDController master = PIDControllerCreate(&LiftSetLeft, &LiftGetQuadEncLeft,  1.5, 0.01, 0, 300, -200, 2);
	PIDController slave = PIDControllerCreate(&LiftSetRight, &LiftGetQuadEncRight, 1.5, 0.01, 0, 300, -200, 2);
	PIDController equalizer = PIDControllerCreate(NULL, &liftComputeQuadEncDiff,   0.75, 0.05, 0, 800, -600, 2);

	Controller = CreateMasterSlavePIDController(master, slave, equalizer, false);

	LiftControllerTask = InitializeMasterSlaveController(&Controller, 0);
}