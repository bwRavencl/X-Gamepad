/* Copyright 2014 Matteo Hausner */

#include "XPLMDataAccess.h"
#include "XPLMDefs.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMMenus.h"
#include "XPLMPlugin.h"
#include "XPLMProcessing.h"
#include "XPLMUtilities.h"
#include "XPStandardWidgets.h"
#include "XPWidgets.h"

#include <fstream>
#include <string.h>
#include <sstream>

// define name
#define NAME "X-pad"
#define NAME_LOWERCASE "x_pad"

// define version
#define VERSION "0.1"

#define AXIS_ASSIGNMENT_NONE 0
#define AXIS_ASSIGNMENT_YAW 3
#define AXIS_ASSIGNMENT_VIEW_LEFT_RIGHT 41
#define AXIS_ASSIGNMENT_VIEW_UP_DOWN 42

#define JOYSTICK_AXIS_LEFT_X 10
#define JOYSTICK_AXIS_LEFT_Y 11

// global variables
static int lookModifierDown = 0;

// global commandref variables
static XPLMCommandRef lookModifierCommand = NULL;

// global dataref variables
static XPLMDataRef hasJostickDataRef, joystickPitchNullzoneDataRef, joystickAxisAssignmentsDataRef, joystickAxisReverseDataRef, joystickAxisValuesDataRef, leftBrakeRatioDataRef, rightBrakeRatioDataRef, throttleRatioAllDataRef;

// command-handler that handles the look modifier command
int LookModifierCommandHandler(XPLMCommandRef       inCommand,
                               XPLMCommandPhase     inPhase,
                               void *               inRefcon)
{
    int joystickAxisAssignments[100];
    XPLMGetDatavi(joystickAxisAssignmentsDataRef, joystickAxisAssignments, 0, 100);
 
    int joystickAxisReverse[100];
    XPLMGetDatavi(joystickAxisReverseDataRef, joystickAxisReverse, 0, 100);
    
    char out[64];
    sprintf(out, "Axis Assignment: %d\n", joystickAxisAssignments[JOYSTICK_AXIS_LEFT_Y]);
    XPLMDebugString(out);
    
	if (inPhase == xplm_CommandBegin)
    {
        lookModifierDown = 1;
        
        joystickAxisAssignments[JOYSTICK_AXIS_LEFT_X] = AXIS_ASSIGNMENT_VIEW_LEFT_RIGHT;
        joystickAxisAssignments[JOYSTICK_AXIS_LEFT_Y] = AXIS_ASSIGNMENT_VIEW_UP_DOWN;
        
        joystickAxisReverse[JOYSTICK_AXIS_LEFT_Y] = 1;
    }
	else if (inPhase == xplm_CommandEnd)
    {
        lookModifierDown = 0;
        
        joystickAxisAssignments[JOYSTICK_AXIS_LEFT_X] = AXIS_ASSIGNMENT_YAW;
        joystickAxisAssignments[JOYSTICK_AXIS_LEFT_Y] = AXIS_ASSIGNMENT_NONE;
        
        joystickAxisReverse[JOYSTICK_AXIS_LEFT_Y] = 0;
    }
    
    XPLMSetDatavi(joystickAxisAssignmentsDataRef, joystickAxisAssignments, 0, 100);
    XPLMSetDatavi(joystickAxisReverseDataRef, joystickAxisReverse, 0, 100);
    
	return 0;
}

// flightloop-callback that handles the joystick axis
float JoystickAxisFlightCallback(
                            float                inElapsedSinceLastCall,
                            float                inElapsedTimeSinceLastFlightLoop,
                            int                  inCounter,
                            void *               inRefcon)
{
    if (XPLMGetDatai(hasJostickDataRef))
    {

        
        /*for (int i = 0; i < 10; i++)
        {
            char out[64];
            sprintf(out, "Axis Center [%d]: %f\n", i, joystickPitchCenter[i]);
            XPLMDebugString(out);
        }*/
        

        
        if (lookModifierDown == 0)
        {
            float joystickPitchNullzone = XPLMGetDataf(joystickPitchNullzoneDataRef);
            
            float joystickAxisValues[100];
            XPLMGetDatavf(joystickAxisValuesDataRef, joystickAxisValues, 0, 100);
            
            if (joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] < 0.5f - joystickPitchNullzone)
            {
                //XPLMDebugString("Increase throttle!\n");
                char out[64];
                sprintf(out, "Adding: %f\n", 0.05f * (-2.0f * joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] + 1.0f));
                XPLMDebugString(out);
                
                float throttleRatioAll = XPLMGetDataf(throttleRatioAllDataRef);
                float d = 0.05f * (-2.0f * joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] + 1.0f);
                
                XPLMSetDataf(throttleRatioAllDataRef, throttleRatioAll < 1.0f ? throttleRatioAll + d : 1.0f);
            }
            else if (joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] > 0.5f + joystickPitchNullzone)
            {
                //XPLMDebugString("Decrease throttle!\n");
                char out[64];
                sprintf(out, "Subtracting: %f\n", 0.05f * (2 * joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] - 1.0f));
                XPLMDebugString(out);
                
                float throttleRatioAll = XPLMGetDataf(throttleRatioAllDataRef);
                float d = - 0.05f * (2.0f * joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] - 1.0f);
                
                XPLMSetDataf(throttleRatioAllDataRef, throttleRatioAll > 0.0f ? throttleRatioAll + d : 0.0f);
            }
            
            if (joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] == 1.0f)
            {
                //XPLMDebugString("Applying brakes!\n");
                XPLMSetDataf(leftBrakeRatioDataRef, 1.0f);
                XPLMSetDataf(rightBrakeRatioDataRef, 1.0f);
            }
            else
            {
                //XPLMDebugString("Not Applying brakes!\n");
                XPLMSetDataf(leftBrakeRatioDataRef, 0.0f);
                XPLMSetDataf(rightBrakeRatioDataRef, 0.0f);
            }
        }

    }
    
    return -1.0f;
}

PLUGIN_API int XPluginStart(
    char *		outName,
    char *		outSig,
    char *		outDesc)
{
    // set plugin info
    strcpy(outName, NAME);
    strcpy(outSig, "de.bwravencl."NAME_LOWERCASE);
    strcpy(outDesc, NAME" allows flying X-Plane by gamepad!");

    // obtain datarefs
    hasJostickDataRef = XPLMFindDataRef("sim/joystick/has_joystick");
    joystickPitchNullzoneDataRef = XPLMFindDataRef("sim/joystick/joystick_pitch_nullzone");
    joystickAxisAssignmentsDataRef = XPLMFindDataRef("sim/joystick/joystick_axis_assignments");
    joystickAxisValuesDataRef = XPLMFindDataRef("sim/joystick/joystick_axis_values");
    joystickAxisReverseDataRef = XPLMFindDataRef("sim/joystick/joystick_axis_reverse");
    leftBrakeRatioDataRef = XPLMFindDataRef("sim/cockpit2/controls/left_brake_ratio");
    rightBrakeRatioDataRef = XPLMFindDataRef("sim/cockpit2/controls/right_brake_ratio");
    throttleRatioAllDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/throttle_ratio_all");
    
    // create custom commands
	lookModifierCommand = XPLMCreateCommand(NAME"/LookModifier", "Look Modifier");
    
	// register custom commands
	XPLMRegisterCommandHandler(lookModifierCommand, LookModifierCommandHandler, 1, NULL);

    // register flight loop callback
    XPLMRegisterFlightLoopCallback(JoystickAxisFlightCallback, -1, NULL);
    
    return 1;
}

PLUGIN_API void	XPluginStop(void)
{
}

PLUGIN_API void XPluginDisable(void)
{
}

PLUGIN_API int XPluginEnable(void)
{
    return 1;
}

PLUGIN_API void XPluginReceiveMessage(
    XPLMPluginID	inFromWho,
    long			inMessage,
    void *			inParam)
{
}
