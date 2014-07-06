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

// define axis assignments
#define AXIS_ASSIGNMENT_NONE 0
#define AXIS_ASSIGNMENT_YAW 3
#define AXIS_ASSIGNMENT_VIEW_LEFT_RIGHT 41
#define AXIS_ASSIGNMENT_VIEW_UP_DOWN 42

// define button assignments
#define BUTTON_ASSIGNMENT_PITCH_TRIM_UP 259
#define BUTTON_ASSIGNMENT_PITCH_TRIM_DOWN 261
#define BUTTON_ASSIGNMENT_RUDDER_TRIM_LEFT 262
#define BUTTON_ASSIGNMENT_RUDDER_TRIM_RIGHT 264
#define BUTTON_ASSIGNMENT_AILERON_TRIM_LEFT 265
#define BUTTON_ASSIGNMENT_AILERON_TRIM_RIGHT 267
#define BUTTON_ASSIGNMENT_GENERAL_COMMAND_LEFT 940
#define BUTTON_ASSIGNMENT_GENERAL_COMMAND_RIGHT 941
#define BUTTON_ASSIGNMENT_GENERAL_COMMAND_UP 942
#define BUTTON_ASSIGNMENT_GENERAL_COMMAND_DOWN 943

// define joystick axis
#define JOYSTICK_AXIS_LEFT_X 10
#define JOYSTICK_AXIS_LEFT_Y 11

// define joystick buttons
#define JOYSTICK_BUTTON_DPAD_LEFT 167
#define JOYSTICK_BUTTON_DPAD_RIGHT 165
#define JOYSTICK_BUTTON_DPAD_UP 164
#define JOYSTICK_BUTTON_DPAD_DOWN 166
#define JOYSTICK_BUTTON_SQUARE 175
#define JOYSTICK_BUTTON_CIRCLE 173
#define JOYSTICK_BUTTON_TRIANGLE 172
#define JOYSTICK_BUTTON_CROSS 174

// define relative control multiplier
#define JOYSTICK_RELATIVE_CONTROL_MULTIPLIER 0.05f

// global variables
static int viewModifierDown = 0, propPitchModifierDown = 0, mixtureControlModifierDown = 0;

// global commandref variables
static XPLMCommandRef viewModifierCommand = NULL, propPitchModifierCommand = NULL, mixtureControlModifierCommand = NULL, trimModifierCommand = NULL;

// global dataref variables
static XPLMDataRef acfRSCMingovPrpDataRef = NULL, acfRSCRedlinePrpDataRef = NULL, hasJostickDataRef = NULL, joystickPitchNullzoneDataRef = NULL, joystickAxisAssignmentsDataRef = NULL, joystickAxisReverseDataRef = NULL, joystickAxisValuesDataRef = NULL, joystickButtonAssignmentsDataRef = NULL, leftBrakeRatioDataRef = NULL, rightBrakeRatioDataRef = NULL, throttleRatioAllDataRef = NULL, propRotationSpeedRadSecAllDataRef = NULL, mixtureRatioAllDataRef = NULL;

// command-handler that handles the view modifier command
int ViewModifierCommandHandler(XPLMCommandRef       inCommand,
                               XPLMCommandPhase     inPhase,
                               void *               inRefcon)
{
    int joystickAxisAssignments[100];
    XPLMGetDatavi(joystickAxisAssignmentsDataRef, joystickAxisAssignments, 0, 100);
 
    int joystickAxisReverse[100];
    XPLMGetDatavi(joystickAxisReverseDataRef, joystickAxisReverse, 0, 100);
    
    static int defaultJoystickButtonAssignments[1600];
    
	if (inPhase == xplm_CommandBegin)
    {
        viewModifierDown = 1;
        
        // assign the view controls to the left joystick's axis
        joystickAxisAssignments[JOYSTICK_AXIS_LEFT_X] = AXIS_ASSIGNMENT_VIEW_LEFT_RIGHT;
        joystickAxisAssignments[JOYSTICK_AXIS_LEFT_Y] = AXIS_ASSIGNMENT_VIEW_UP_DOWN;
        
        // reverse the left joystick's y axis while the view modifier is applied
        joystickAxisReverse[JOYSTICK_AXIS_LEFT_Y] = 1;
        
        int joystickButtonAssignments[1600];
        XPLMGetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);
        
        // store the default button assignments
        memcpy(defaultJoystickButtonAssignments, joystickButtonAssignments, 1600);
        
        // assign panel scrolling controls to the dpad
        joystickButtonAssignments[JOYSTICK_BUTTON_DPAD_LEFT] = BUTTON_ASSIGNMENT_GENERAL_COMMAND_LEFT;
        joystickButtonAssignments[JOYSTICK_BUTTON_DPAD_RIGHT] = BUTTON_ASSIGNMENT_GENERAL_COMMAND_RIGHT;
        joystickButtonAssignments[JOYSTICK_BUTTON_DPAD_UP] = BUTTON_ASSIGNMENT_GENERAL_COMMAND_UP;
        joystickButtonAssignments[JOYSTICK_BUTTON_DPAD_DOWN] = BUTTON_ASSIGNMENT_GENERAL_COMMAND_DOWN;
        
        XPLMSetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);
    }
	else if (inPhase == xplm_CommandEnd)
    {
        viewModifierDown = 0;
        
        // assign the default controls to the left joystick's axis
        joystickAxisAssignments[JOYSTICK_AXIS_LEFT_X] = AXIS_ASSIGNMENT_YAW;
        joystickAxisAssignments[JOYSTICK_AXIS_LEFT_Y] = AXIS_ASSIGNMENT_NONE;
        
        // disable the axis reversing when the view modifier is not applied anymore
        joystickAxisReverse[JOYSTICK_AXIS_LEFT_Y] = 0;
        
        // restore the default button assignments
        XPLMSetDatavi(joystickButtonAssignmentsDataRef, defaultJoystickButtonAssignments, 0, 1600);
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
int MixtureControlModifierCommandHandler(XPLMCommandRef       inCommand,
                                    XPLMCommandPhase     inPhase,
                                    void *               inRefcon)
{
	if (inPhase == xplm_CommandBegin)
        mixtureControlModifierDown = 1;
	else if (inPhase == xplm_CommandEnd)
        mixtureControlModifierDown = 0;
    
	return 0;
}

// command-handler that handles the trim modifier command
int TrimModifierCommandHandler(XPLMCommandRef       inCommand,
                               XPLMCommandPhase     inPhase,
                               void *               inRefcon)
{
    static int defaultJoystickButtonAssignments[1600];
    
	if (inPhase == xplm_CommandBegin)
    {
        int joystickButtonAssignments[1600];
        XPLMGetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);
        
        // store the default button assignments
        memcpy(defaultJoystickButtonAssignments, joystickButtonAssignments, 1600);
        
        char out[64];
        sprintf(out, "LEFT DPAD: %d\n", joystickButtonAssignments[JOYSTICK_BUTTON_DPAD_LEFT]);
        XPLMDebugString(out);
        
        // assign trim controls to the buttons and dpad
        joystickButtonAssignments[JOYSTICK_BUTTON_SQUARE] = BUTTON_ASSIGNMENT_AILERON_TRIM_LEFT;
        joystickButtonAssignments[JOYSTICK_BUTTON_CIRCLE] = BUTTON_ASSIGNMENT_AILERON_TRIM_RIGHT;
        joystickButtonAssignments[JOYSTICK_BUTTON_TRIANGLE] = BUTTON_ASSIGNMENT_PITCH_TRIM_DOWN;
        joystickButtonAssignments[JOYSTICK_BUTTON_CROSS] = BUTTON_ASSIGNMENT_PITCH_TRIM_UP;
        joystickButtonAssignments[JOYSTICK_BUTTON_DPAD_LEFT] = BUTTON_ASSIGNMENT_RUDDER_TRIM_LEFT;
        joystickButtonAssignments[JOYSTICK_BUTTON_DPAD_RIGHT] = BUTTON_ASSIGNMENT_RUDDER_TRIM_RIGHT;
        
        XPLMSetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);
    }
	else if (inPhase == xplm_CommandEnd)
    {
        // restore the default button assignments
        XPLMSetDatavi(joystickButtonAssignmentsDataRef, defaultJoystickButtonAssignments, 0, 1600);
    }
    
	return 0;
}

// normalizes a value of a range [inMin, inMax] to a value of the range [outMin, outMax]
float Normalize(float value, float inMin, float inMax, float outMin, float outMax)
{
    return (outMax - outMin) / (inMax - inMin) * (value - inMax) + outMax;
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

        if (propPitchModifierDown != 0)
        {
            float acfRSCMingovPrp = XPLMGetDataf(acfRSCMingovPrpDataRef);
            float acfRSCRedlinePrp = XPLMGetDataf(acfRSCRedlinePrpDataRef);
            float propRotationSpeedRadSecAll = XPLMGetDataf(propRotationSpeedRadSecAllDataRef);
            
            // increase prop pitch
            if (joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] < 0.5f - joystickPitchNullzone)
            {
                // normalize range [0.5, 0.0] to [acfRSCMingovPrp, acfRSCRedlinePrp]
                float d = JOYSTICK_RELATIVE_CONTROL_MULTIPLIER * Normalize(joystickAxisValues[JOYSTICK_AXIS_LEFT_Y], 0.5f, 0.0f, acfRSCMingovPrp, acfRSCRedlinePrp);
                
                // ensure we don't set values larger than 1.0
                XPLMSetDataf(propRotationSpeedRadSecAllDataRef, propRotationSpeedRadSecAll < acfRSCRedlinePrp ? propRotationSpeedRadSecAll + d : acfRSCRedlinePrp);
            }
            // decrease prop pitch
            else if (joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] > 0.5f + joystickPitchNullzone)
            {
                // normalize range [0.5, 1.0] to [acfRSCMingovPrp, acfRSCRedlinePrp]
                float d = - JOYSTICK_RELATIVE_CONTROL_MULTIPLIER * Normalize(joystickAxisValues[JOYSTICK_AXIS_LEFT_Y], 0.5f, 1.0f, acfRSCMingovPrp, acfRSCRedlinePrp);
                
                // ensure we don't set values smaller than 0.0
                XPLMSetDataf(propRotationSpeedRadSecAllDataRef, propRotationSpeedRadSecAll > acfRSCMingovPrp ? propRotationSpeedRadSecAll + d : acfRSCMingovPrp);
            }
        }
        else if (mixtureControlModifierDown != 0)
        {
            float mixtureRatioAll = XPLMGetDataf(mixtureRatioAllDataRef);
            
            // increase mixture setting
            if (joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] < 0.5f - joystickPitchNullzone)
            {
                // normalize range [0.5, 0.0] to [0.0, 1.0]
                float d = JOYSTICK_RELATIVE_CONTROL_MULTIPLIER * Normalize(joystickAxisValues[JOYSTICK_AXIS_LEFT_Y], 0.5f, 0.0f, 0.0f, 1.0f);
                
                // ensure we don't set values larger than 1.0
                XPLMSetDataf(mixtureRatioAllDataRef, mixtureRatioAll < 1.0f ? mixtureRatioAll + d : 1.0f);
            }
            // decrease mixture setting
            else if (joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] > 0.5f + joystickPitchNullzone)
            {
                // normalize range [0.5, 1.0] to [0.0, 1.0]
                float d = - JOYSTICK_RELATIVE_CONTROL_MULTIPLIER * Normalize(joystickAxisValues[JOYSTICK_AXIS_LEFT_Y], 0.5f, 1.0f, 0.0f, 1.0f);
                
                // ensure we don't set values smaller than 0.0
                XPLMSetDataf(mixtureRatioAllDataRef, mixtureRatioAll > 0.0f ? mixtureRatioAll + d : 0.0f);
            }
        }
            else
            {
                float throttleRatioAll = XPLMGetDataf(throttleRatioAllDataRef);
            
            // increase throttle setting
            if (joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] < 0.5f - joystickPitchNullzone)
            {
                // normalize range [0.5, 0.0] to [0.0, 1.0]
                float d = JOYSTICK_RELATIVE_CONTROL_MULTIPLIER * Normalize(joystickAxisValues[JOYSTICK_AXIS_LEFT_Y], 0.5f, 0.0f, 0.0f, 1.0f);
                
                // ensure we don't set values larger than 1.0
                XPLMSetDataf(throttleRatioAllDataRef, throttleRatioAll < 1.0f ? throttleRatioAll + d : 1.0f);
            }
            // decrease throttle setting
            else if (joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] > 0.5f + joystickPitchNullzone)
            {
                // normalize range [0.5, 1.0] to [0.0, 1.0]
                float d = - JOYSTICK_RELATIVE_CONTROL_MULTIPLIER * Normalize(joystickAxisValues[JOYSTICK_AXIS_LEFT_Y], 0.5f, 1.0f, 0.0f, 1.0f);
                
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
    acfRSCMingovPrpDataRef = XPLMFindDataRef("sim/aircraft/controls/acf_RSC_mingov_prp");
    acfRSCRedlinePrpDataRef = XPLMFindDataRef("sim/aircraft/controls/acf_RSC_redline_prp");
    hasJostickDataRef = XPLMFindDataRef("sim/joystick/has_joystick");
    joystickPitchNullzoneDataRef = XPLMFindDataRef("sim/joystick/joystick_pitch_nullzone");
    joystickAxisAssignmentsDataRef = XPLMFindDataRef("sim/joystick/joystick_axis_assignments");
    joystickButtonAssignmentsDataRef = XPLMFindDataRef("sim/joystick/joystick_button_assignments");
    joystickAxisValuesDataRef = XPLMFindDataRef("sim/joystick/joystick_axis_values");
    joystickAxisReverseDataRef = XPLMFindDataRef("sim/joystick/joystick_axis_reverse");
    leftBrakeRatioDataRef = XPLMFindDataRef("sim/cockpit2/controls/left_brake_ratio");
    rightBrakeRatioDataRef = XPLMFindDataRef("sim/cockpit2/controls/right_brake_ratio");
    throttleRatioAllDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/throttle_ratio_all");
    propRotationSpeedRadSecAllDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/prop_rotation_speed_rad_sec_all");
    mixtureRatioAllDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/mixture_ratio_all");
    
    // create custom commands
	viewModifierCommand = XPLMCreateCommand(NAME"/view_modifier", "View Modifier");
    propPitchModifierCommand = XPLMCreateCommand(NAME"/prop_pitch_modifier", "Prop Pitch Modifier");
    mixtureControlModifierCommand = XPLMCreateCommand(NAME"/mixture_control_modifier", "Mixture Control Modifier");
    trimModifierCommand = XPLMCreateCommand(NAME"/trim_modifier", "Trim Modifier");
    
	// register custom commands
	XPLMRegisterCommandHandler(viewModifierCommand, ViewModifierCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(propPitchModifierCommand, PropPitchModifierCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(mixtureControlModifierCommand, MixtureControlModifierCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(trimModifierCommand, TrimModifierCommandHandler, 1, NULL);

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
