<<<<<<< HEAD
/**
 * @file dummy/opcontrol.c
 * @brief Source file for dummy operator functions
 *
 * Copyright(c) 2014-2015 Olympic Steel Eagles.All rights reserved. <br>
 * Portions of this file may contain elements from the PROS API. <br>
 * See include/API.h for additional notice.
 ********************************************************************************/
=======
/********************************************************************************/
/* @file dummy/init.c		@brief Source file for dummy operator functions.	*/
/*																				*/
/* Copyright (c) 2014-2015 Olympic Steel Eagles. All rights reserved.			*/
/* Portions of this file may contain elements from the PROS API.				*/
/* See include/API.h for additional notice.										*/
/********************************************************************************/
>>>>>>> 7b38228c15fb78daf33c4b4aee5eb4dcd083a15a

#include "main.h"
#include "dummy/MotorDefinitions.h"
#include "sml/SmartMotorLibrary.h"

/**
* @brief Sets motors in motion based on user input (from controls).
*/
void operatorControl() 
{
	for (int i = 1; i <= 12; i++)
		pinMode(i, OUTPUT);

	while (true) 
	{
		for (int i = 1; i < 11; i++)
			motorSet(i, joystickGetAnalog(1, 2));

		for (int i = 1; i <= 12; i++)
			digitalWrite(i, joystickGetDigital(1, 7, JOY_UP));

		delay(20);
	}
}
