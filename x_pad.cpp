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
static int viewModifierDown = 0, propPitchModifierDown = 0, mixtureModifierDown = 0;

// global commandref variables
static XPLMCommandRef viewModifierCommand = NULL, propPitchModifierCommand = NULL, mixtureModifierCommand = NULL;

// global dataref variables
static XPLMDataRef hasJostickDataRef = NULL, joystickPitchNullzoneDataRef = NULL, joystickAxisAssignmentsDataRef = NULL, joystickAxisReverseDataRef = NULL, joystickAxisValuesDataRef = NULL, leftBrakeRatioDataRef = NULL, rightBrakeRatioDataRef = NULL, throttleRatioAllDataRef = NULL, propAngleDegreesAllDataRef = NULL, mixtureRatioAllDataRef = NULL;

// command-handler that handles the view modifier command
int ViewModifierCommandHandler(XPLMCommandRef       inCommand,
                               XPLMCommandPhase     inPhase,
                               void *               inRefcon)
{
    int joystickAxisAssignments[100];
    XPLMGetDatavi(joystickAxisAssignmentsDataRef, joystickAxisAssignments, 0, 100);
 
    int joystickAxisReverse[100];
    XPLMGetDatavi(joystickAxisReverseDataRef, joystickAxisReverse, 0, 100);
    
	if (inPhase == xplm_CommandBegin)
    {
        viewModifierDown = 1;
        
        // assign the view controls to the left joystick's axis
        joystickAxisAssignments[JOYSTICK_AXIS_LEFT_X] = AXIS_ASSIGNMENT_VIEW_LEFT_RIGHT;
        joystickAxisAssignments[JOYSTICK_AXIS_LEFT_Y] = AXIS_ASSIGNMENT_VIEW_UP_DOWN;
        
        // reverse the left joystick's y axis while the view modifier is applied
        joystickAxisReverse[JOYSTICK_AXIS_LEFT_Y] = 1;
    }
	else if (inPhase == xplm_CommandEnd)
    {
        viewModifierDown = 0;
        
        // assign the default controls to the left joystick's axis
        joystickAxisAssignments[JOYSTICK_AXIS_LEFT_X] = AXIS_ASSIGNMENT_YAW;
        joystickAxisAssignments[JOYSTICK_AXIS_LEFT_Y] = AXIS_ASSIGNMENT_NONE;
        
        // disable the axis reversing when the view modifier is not applied anymore
        joystickAxisReverse[JOYSTICK_AXIS_LEFT_Y] = 0;
    }
    
    XPLMSetDatavi(joystickAxisAssignmentsDataRef, joystickAxisAssignments, 0, 100);
    XPLMSetDatavi(joystickAxisReverseDataRef, joystickAxisReverse, 0, 100);
    
	return 0;
}

// command-handler that handles the prop pitch modifier command
int PropPitchModifierCommandHandler(XPLMCommandRef       inCommand,
                               XPLMCommandPhase     inPhase,
                               void *               inRefcon)
{
	if (inPhase == xplm_CommandBegin)
        propPitchModifierDown = 1;
	else if (inPhase == xplm_CommandEnd)
        propPitchModifierDown = 0;
    
	return 0;
}

// command-handler that handles the mixture modifier command
int MixtureModifierCommandHandler(XPLMCommandRef       inCommand,
                                    XPLMCommandPhase     inPhase,
                                    void *               inRefcon)
{
	if (inPhase == xplm_CommandBegin)
        mixtureModifierDown = 1;
	else if (inPhase == xplm_CommandEnd)
        mixtureModifierDown = 0;
    
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
        
        float joystickPitchNullzone = XPLMGetDataf(joystickPitchNullzoneDataRef);
        
        float joystickAxisValues[100];
        XPLMGetDatavf(joystickAxisValuesDataRef, joystickAxisValues, 0, 100);
        
        if (viewModifierDown == 0)
        {
            float throttleRatioAll = XPLMGetDataf(throttleRatioAllDataRef);
            
            // increase throttle setting
            if (joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] < 0.5f - joystickPitchNullzone)
            {
                // normalize range (0.5, 0.0) to (0.0, 1.0)
                float d = 0.05f * (-2.0f * joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] + 1.0f);
                
                // ensure we don't set values larger than 1.0
                XPLMSetDataf(throttleRatioAllDataRef, throttleRatioAll < 1.0f ? throttleRatioAll + d : 1.0f);
            }
            // decrease throttle setting
            else if (joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] > 0.5f + joystickPitchNullzone)
            {
                // normalize range (0.5, 1.0) to (0.0, 1.0)
                float d = - 0.05f * (2.0f * joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] - 1.0f);

                // ensure we don't set values smaller than 0.0
                XPLMSetDataf(throttleRatioAllDataRef, throttleRatioAll > 0.0f ? throttleRatioAll + d : 0.0f);
            }
            
            // apply brakes
            if (joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] == 1.0f)
            {
                XPLMSetDataf(leftBrakeRatioDataRef, 1.0f);
                XPLMSetDataf(rightBrakeRatioDataRef, 1.0f);
            }
            // don't apply brakes
            else
            {
                XPLMSetDataf(leftBrakeRatioDataRef, 0.0f);
                XPLMSetDataf(rightBrakeRatioDataRef, 0.0f);
            }
        }
        else if (propPitchModifierDown != 0)
        {
            float propAngleDegreesAll = XPLMGetDataf(propAngleDegreesAllDataRef);
            
            // increase prop pitch
            if (joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] < 0.5f - joystickPitchNullzone)
            {
                // normalize range (0.5, 0.0) to (0.0, 1.0)
                float d = 0.05f * (-2.0f * joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] + 1.0f);
                
                // ensure we don't set values larger than 1.0
                XPLMSetDataf(propAngleDegreesAllDataRef, propAngleDegreesAll < 1.0f ? propAngleDegreesAll + d : 1.0f);
            }
            // decrease prop pitch
            else if (joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] > 0.5f + joystickPitchNullzone)
            {
                // normalize range (0.5, 1.0) to (0.0, 1.0)
                float d = - 0.05f * (2.0f * joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] - 1.0f);
                
                // ensure we don't set values smaller than 0.0
                XPLMSetDataf(propAngleDegreesAllDataRef, propAngleDegreesAll > 0.0f ? propAngleDegreesAll + d : 0.0f);
            }
        }
        else if (mixtureModifierDown != 0)
        {
            float mixtureRatioAll = XPLMGetDataf(mixtureRatioAllDataRef);
            
            // increase mixture setting
            if (joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] < 0.5f - joystickPitchNullzone)
            {
                // normalize range (0.5, 0.0) to (0.0, 1.0)
                float d = 0.05f * (-2.0f * joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] + 1.0f);
                
                // ensure we don't set values larger than 1.0
                XPLMSetDataf(mixtureRatioAllDataRef, mixtureRatioAll < 1.0f ? mixtureRatioAll + d : 1.0f);
            }
            // decrease mixture setting
            else if (joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] > 0.5f + joystickPitchNullzone)
            {
                // normalize range (0.5, 1.0) to (0.0, 1.0)
                float d = - 0.05f * (2.0f * joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] - 1.0f);
                
                // ensure we don't set values smaller than 0.0
                XPLMSetDataf(mixtureRatioAllDataRef, mixtureRatioAll > 0.0f ? mixtureRatioAll + d : 0.0f);
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
    propAngleDegreesAllDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/prop_angle_degrees_all");
    mixtureRatioAllDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/prop_angle_degrees_all");
    
    // create custom commands
	viewModifierCommand = XPLMCreateCommand(NAME"/LookModifier", "Look Modifier");
    
	// register custom commands
	XPLMRegisterCommandHandler(viewModifierCommand, ViewModifierCommandHandler, 1, NULL);

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
