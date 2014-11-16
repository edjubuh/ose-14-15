/************************************************************************/
/* @file opcontrol.c													*/
/* @brief Source file for operator control.								*/
/* Copyright (c) 2014-2015 Olympic Steel Eagles. All rights reserved.	*/
/* Portions of this file may contain elements from the PROS API.		*/
/* See include/API.h for additional notice.								*/
/************************************************************************/

#include "main.h"
#include "dios/Chassis.h"
#include "dios/Lift.h"
#include "dios/ScoringMechanism.h"

void operatorControl()
{
	while (true)
	{
		ChassisSet(joystickGetAnalog(1, 3), joystickGetAnalog(1, 2), false);
		if (joystickGetDigital(1, 6, JOY_UP))
			LiftSet(127, false);
		else if (joystickGetDigital(1, 6, JOY_DOWN))
			LiftSet(-50, false);
		else
			LiftSet(0, false);

		if (joystickGetDigital(1, 5, JOY_DOWN))
			ScoringMechSet(-127, false);
		else if (joystickGetDigital(1, 5, JOY_UP))
			ScoringMechSet(127, false);
		else
			ScoringMechSet(0, false);
	}
}