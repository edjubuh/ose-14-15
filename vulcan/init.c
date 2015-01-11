<<<<<<< HEAD
/**
 * @file vulcan/init.c		
 * @brief Source file for initialize functions.
 *
 * Copyright(c) 2014-2015 Olympic Steel Eagles.All rights reserved. <br>
 * Portions of this file may contain elements from the PROS API. <br>
 * See include/API.h for additional notice.
 ********************************************************************************/

#include "main.h"

#include "sml/SmartMotorLibrary.h"
#include "lcd/LCDFunctions.h"

#include "vulcan/CortexDefinitions.h"
#include "vulcan/Chassis.h"
#include "vulcan/Lift.h"
=======
/********************************************************************************/
/* @file vulcan/init.c		@brief Source file for initialize functions.		*/
/*																				*/
/* Copyright (c) 2014-2015 Olympic Steel Eagles. All rights reserved.			*/
/* Portions of this file may contain elements from the PROS API.				*/	
/* See include/API.h for additional notice.										*/
/********************************************************************************/

#include "main.h"
#include "sml/SmartMotorLibrary.h"
#include "lcd/LCDFunctions.h"

#include "dios/CortexDefinitions.h"
#include "dios/Chassis.h"
#include "dios/Lift.h"
>>>>>>> 7b38228c15fb78daf33c4b4aee5eb4dcd083a15a


/**
 * @brief Runs pre-initialization functions.
 */
void initializeIO() {
<<<<<<< HEAD
	pinMode(DIG_LIFT_BOTLIM_RIGHT, INPUT);
	pinMode(DIG_LIFT_BOTLIM_LEFT, INPUT);
	pinMode(DIG_SCORINGMECH, OUTPUT);
	setTeamName("7701");
=======
	pinMode(3, OUTPUT);
	pinMode(4, OUTPUT);
	setTeamName("7702");
>>>>>>> 7b38228c15fb78daf33c4b4aee5eb4dcd083a15a
}


/**
 * @brief Initializes the robot. Displays graphics depicting process in initialization
 *        sequence.
 */
void initialize()
{
	lcdInitialize();
<<<<<<< HEAD
	printText("Booting Vulcan", Centered, 1);
=======
	printText("Booting Dios", Centered, 1);
>>>>>>> 7b38228c15fb78daf33c4b4aee5eb4dcd083a15a
	printText("IMEs... ", Left, 2); // IMES must be first, followed by MotorManager. Chassis and Lift are not order dependent
	imeInitializeAll();
	delay(100);
	printText("MotorManager... ", Left, 2);
	InitializeMotorManager();
	delay(100);
	printText("Chassis... ", Left, 2);
	ChassisInitialize();
	delay(100);
	printText("Lift... ", Left, 2);
	LiftInitialize();
	delay(200);
	printText("", Left, 2);
	delay(100);
	printText("       pl       ", Left, 2);
	delay(100);
	printText("      mple      ", Left, 2);
	delay(100);
	printText("     omplet     ", Left, 2);
	delay(100);
	printText("    complete    ", Left, 2);
	delay(100);
	printText("   .complete.   ", Left, 2);
	delay(100);
	printText("  ..complete..  ", Left, 2);
	delay(100);
	printText(" ...complete... ", Left, 2);
	delay(100);
	printText("....complete....", Left, 2);
	delay(100);
	printText("", Left, 2);
	delay(100);
	printText("....complete....", Left, 2);
}