/* Copyright (C) 2015  Matteo Hausner
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "XPLMDataAccess.h"
#include "XPLMMenus.h"
#include "XPLMPlanes.h"
#include "XPLMPlugin.h"
#include "XPLMProcessing.h"
#include "XPLMUtilities.h"

#include <stack>

#if APL
#include "ApplicationServices/ApplicationServices.h"
#elif LIN
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#endif

// define name
#define NAME "X-pad"
#define NAME_LOWERCASE "x_pad"

// define version
#define VERSION "0.5"

// define joystick axis
#define JOYSTICK_AXIS_LEFT_X 0
#define JOYSTICK_AXIS_LEFT_Y 1
#define JOYSTICK_AXIS_RIGHT_X 2
#define JOYSTICK_AXIS_RIGHT_Y 3

// define joystick buttons
#define JOYSTICK_BUTTON_DPAD_LEFT 7
#define JOYSTICK_BUTTON_DPAD_RIGHT 5
#define JOYSTICK_BUTTON_DPAD_UP 4
#define JOYSTICK_BUTTON_DPAD_DOWN 6
#define JOYSTICK_BUTTON_SQUARE 15
#define JOYSTICK_BUTTON_CIRCLE 13
#define JOYSTICK_BUTTON_TRIANGLE 12
#define JOYSTICK_BUTTON_CROSS 14
#define JOYSTICK_BUTTON_START 3
#define JOYSTICK_BUTTON_SELECT 0
#define JOYSTICK_BUTTON_L1 10
#define JOYSTICK_BUTTON_R1 11
#define JOYSTICK_BUTTON_L2 8
#define JOYSTICK_BUTTON_R2 9
#define JOYSTICK_BUTTON_L3 1
#define JOYSTICK_BUTTON_R3 2
#define JOYSTICK_BUTTON_PS 16

// define view types
#define VIEW_TYPE_FORWARDS_WITH_PANEL 1000
#define VIEW_TYPE_CHASE 1017
#define VIEW_TYPE_FORWARDS_WITH_HUD 1023
#define VIEW_TYPE_3D_COCKPIT_COMMAND_LOOK 1026

// define axis assignments
#define AXIS_ASSIGNMENT_NONE 0
#define AXIS_ASSIGNMENT_PITCH 1
#define AXIS_ASSIGNMENT_ROLL 2
#define AXIS_ASSIGNMENT_YAW 3
#define AXIS_ASSIGNMENT_VIEW_LEFT_RIGHT 41
#define AXIS_ASSIGNMENT_VIEW_UP_DOWN 42

// define '.acf' file 'show cockpit object in: 2-d forward panel views' string
#define ACF_STRING_SHOW_COCKPIT_OBJECT_IN_2D_FORWARD_PANEL_VIEWS "P acf/_new_plot_XP3D_cock/0 1"

// define plugin signatures
#define QPAC_A320_PLUGIN_SIGNATURE "QPAC.airbus.fbw"
#define X737_PLUGIN_SIGNATURE "bs.x737.plugin"
#define X_IVAP_PLUGIN_SIGNATURE "ivao.xivap"

// define custom command names
#define CYCLE_RESET_VIEW_COMMAND NAME_LOWERCASE "/cycle_reset_view"
#define SPEED_BRAKE_AND_CARB_HEAT_TOGGLE_ARM_COMMAND NAME_LOWERCASE "/speed_brake_and_carb_heat_toggle_arm"
#define TOOGLE_AUTOPILOT_COMMAND NAME_LOWERCASE "/toggle_autopilot"
#define VIEW_MODIFIER_COMMAND NAME_LOWERCASE "/view_modifier"
#define PROP_PITCH_MODIFIER_COMMAND NAME_LOWERCASE "/prop_pitch_modifier"
#define MIXTURE_CONTROL_MODIFIER_COMMAND NAME_LOWERCASE "/mixture_control_modifier"
#define COWL_FLAP_MODIFIER_COMMAND NAME_LOWERCASE "/cowl_flap_modifier"
#define TRIM_MODIFIER_COMMAND NAME_LOWERCASE "/trim_modifier"
#define TOGGLE_MOUSE_POINTER_CONTROL_COMMAND NAME_LOWERCASE "/toggle_mouse_pointer_control"
#define PUSH_TO_TALK_COMMAND NAME_LOWERCASE "/push_to_talk"

// define long press time
#define BUTTON_LONG_PRESS_TIME 1.0f

// define relative control multiplier
#define JOYSTICK_RELATIVE_CONTROL_MULTIPLIER 1.0f

// define mouse pointer sensitivity
#define JOYSTICK_MOUSE_POINTER_SENSITIVITY 25.0f

// define MAXFLOAT
#if LIN
#define MAXFLOAT FLT_MAX
#endif

// hardcoded '.acf' files that have no 2-d panel
static const char* ACF_WITHOUT2D_PANEL[] = {"727-100.acf", "727-200Adv.acf", "727-200F.acf", "ATR72.acf"};

// global variables
static int viewModifierDown = 0, propPitchModifierDown = 0, mixtureControlModifierDown = 0, cowlFlapModifierDown = 0, trimModifierDown = 0, mousePointerControlEnabled = 0, switchTo3DCommandLook = 0;

static float lastAxisAssignment = 0.0f;

#if LIN
static Display *display = NULL;
#endif

// global assignments stack
static std::stack <int*> buttonAssignmentsStack;

// global commandref variables
static XPLMCommandRef cycleResetViewCommand = NULL, speedBrakeAndCarbHeatToggleArmCommand = NULL, toggleAutopilotCommand = NULL, viewModifierCommand = NULL, propPitchModifierCommand = NULL, mixtureControlModifierCommand = NULL, cowlFlapModifierCommand = NULL, trimModifierCommand = NULL, toggleMousePointerControlCommand = NULL, pushToTalkCommand = NULL;

// global dataref variables
static XPLMDataRef acfRSCMingovPrpDataRef = NULL, acfRSCRedlinePrpDataRef = NULL, acfNumEnginesDataRef = NULL, acfSbrkEQDataRef = NULL, ongroundAnyDataRef = NULL, groundspeedDataRef = NULL, pilotsHeadPsiDataRef = NULL, viewTypeDataRef = NULL, hasJostickDataRef = NULL, joystickPitchNullzoneDataRef = NULL, joystickHeadingNullzoneDataRef = NULL, joystickAxisAssignmentsDataRef = NULL, joystickAxisReverseDataRef = NULL, joystickAxisValuesDataRef = NULL, joystickButtonAssignmentsDataRef = NULL, joystickButtonValuesDataRef = NULL, leftBrakeRatioDataRef = NULL, rightBrakeRatioDataRef = NULL, speedbrakeRatioDataRef = NULL, throttleRatioAllDataRef = NULL, propRotationSpeedRadSecAllDataRef = NULL, mixtureRatioAllDataRef = NULL, carbHeatRatioDataRef = NULL, cowlFlapRatioDataRef = NULL, thrustReverserDeployRatioDataRef = NULL;

// push the current button assignments to the stack
static void PushButtonAssignments(void)
{
    int *joystickButtonAssignments = (int*) malloc(sizeof(int) * 1600);

    XPLMGetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);
    buttonAssignmentsStack.push(joystickButtonAssignments);
}

// pop the topmost button assignments from the stack
static void PopButtonAssignments(void)
{
    if (!buttonAssignmentsStack.empty())
    {
        int *joystickButtonAssignments = buttonAssignmentsStack.top();

        if (joystickButtonAssignments != NULL)
        {
            XPLMSetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);
            free(joystickButtonAssignments);
        }
        buttonAssignmentsStack.pop();
    }
}

// returns the directory seperator - overrides XPLMGetDirectorySeparator() to return the POSIX '/' instead of ':' OS X
static const char* GetDirectorySeparator()
{
    const char *directorySeperator = XPLMGetDirectorySeparator();
    return (strcmp(directorySeperator, ":") == 0 ?  "/" : directorySeperator);
}

// convert an OS X path with ':' directory seperators to a POSIX path with '/' seperators
#if APL
static void CarbonPathToPOSIXPath(char *path)
{
    // replace ':' with '/'
    for(int i = 0; i < strlen(path); i++)
        if(path[i] == ':') path[i] = '/';

    // prepend '/Volumes/' prefix
    const char *prefix = "/Volumes/";
    size_t len = strlen(prefix);

    memmove(path + len, path, strlen(path) + 1);

    for (int i = 0; i < len; i++)
        path[i] = prefix[i];
}
#endif

// returns 1 if the current aircraft does have a 2D panel, otherwise 0 is returned - for non-parseable sub 1004 version '.acf' files 1 is returned
static int Has2DPanel(void)
{
    char fileName[256], path[512];
    XPLMGetNthAircraftModel(0, fileName, path);

    // under OS X replace the ':' seperator with the POSIX '/'
    char *aircraftFilePath = path;
#if APL
    CarbonPathToPOSIXPath(aircraftFilePath);
#endif

    // check if the path to the '.acf' file matches one of the hardcoded aircraft that have no 2D panel and for which the check below fails
    for (int i = 0; i < sizeof(ACF_WITHOUT2D_PANEL) / sizeof(char*); i++)
    {
        if (strstr(aircraftFilePath, ACF_WITHOUT2D_PANEL[i]) != NULL)
            return 0;
    }

    int has2DPanel = 1;

    // search the '.acf' file for a special string which indicates that the aircraft shows the 3D cockpit object in the 2D forward panel view
    FILE *file = fopen(aircraftFilePath, "r");
    if(file != NULL)
    {
        char temp[512];
        while(fgets(temp, 512, file) != NULL)
        {
            if((strstr(temp, ACF_STRING_SHOW_COCKPIT_OBJECT_IN_2D_FORWARD_PANEL_VIEWS)) != NULL)
                has2DPanel = 0;
        }

        fclose(file);
    }

    return has2DPanel;
}

// command-handler that handles the switch / reset view command
static int CycleResetViewCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    static float beginTime = 0.0f;

    if (inPhase == xplm_CommandBegin)
        beginTime = XPLMGetElapsedTime();
    else if (inPhase == xplm_CommandContinue)
    {
        // reset view
        if (XPLMGetElapsedTime() - beginTime >= BUTTON_LONG_PRESS_TIME)
        {
            switch (XPLMGetDatai(viewTypeDataRef))
            {
                case VIEW_TYPE_FORWARDS_WITH_PANEL:
                    XPLMCommandOnce(XPLMFindCommand("sim/view/3d_cockpit_cmnd_look"));
                    XPLMCommandOnce(XPLMFindCommand("sim/view/forward_with_panel"));
                    break;

                case VIEW_TYPE_3D_COCKPIT_COMMAND_LOOK:
                    XPLMCommandOnce(XPLMFindCommand("sim/view/forward_with_panel"));
                    XPLMCommandOnce(XPLMFindCommand("sim/view/3d_cockpit_cmnd_look"));
                    break;

                case VIEW_TYPE_CHASE:
                    XPLMCommandOnce(XPLMFindCommand("sim/view/circle"));
                    XPLMCommandOnce(XPLMFindCommand("sim/view/chase"));
                    break;
            }

            beginTime = MAXFLOAT;
        }
    }
    else if (inPhase == xplm_CommandEnd)
    {
        // cycle view
        if (XPLMGetElapsedTime() - beginTime < BUTTON_LONG_PRESS_TIME && beginTime != MAXFLOAT)
        {
            switch (XPLMGetDatai(viewTypeDataRef))
            {
                case VIEW_TYPE_FORWARDS_WITH_PANEL:
                    XPLMCommandOnce(XPLMFindCommand("sim/view/3d_cockpit_cmnd_look"));
                    break;

                case VIEW_TYPE_3D_COCKPIT_COMMAND_LOOK:
                    XPLMCommandOnce(XPLMFindCommand("sim/view/forward_with_hud"));
                    break;

                case VIEW_TYPE_FORWARDS_WITH_HUD:
                    XPLMCommandOnce(XPLMFindCommand("sim/view/chase"));
                    break;

                case VIEW_TYPE_CHASE:
                default:
                    if (Has2DPanel())
                        XPLMCommandOnce(XPLMFindCommand("sim/view/forward_with_panel"));
                    else
                        XPLMCommandOnce(XPLMFindCommand("sim/view/3d_cockpit_cmnd_look"));
                    break;
            }
        }

        beginTime = 0.0f;
    }

    return 0;
}

// command-handler that handles the speedbrake toggle / arm command or the carb heat, if the plane has no speedbrake
static int SpeedBrakeAndCarbHeatToggleArmCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    // if a speedbrake exists this command controls it
    if (XPLMGetDatai(acfSbrkEQDataRef) != 0)
    {
        static float beginTime = 0.0f;

        float oldSpeedbrakeRatio = XPLMGetDataf(speedbrakeRatioDataRef);

        if (inPhase == xplm_CommandBegin)
            beginTime = XPLMGetElapsedTime();
        else if (inPhase == xplm_CommandContinue)
        {
            // arm / unarm speedbrake
            if (XPLMGetElapsedTime() - beginTime >= BUTTON_LONG_PRESS_TIME)
            {
                float newSpeedbrakeRatio = oldSpeedbrakeRatio == -0.5f ? 0.0f : -0.5f;

                XPLMSetDataf(speedbrakeRatioDataRef, newSpeedbrakeRatio);

                beginTime = MAXFLOAT;
            }
        }
        else if (inPhase == xplm_CommandEnd)
        {
            // toggle speedbrake
            if (XPLMGetElapsedTime() - beginTime < BUTTON_LONG_PRESS_TIME && beginTime != MAXFLOAT)
            {
                float newSpeedbrakeRatio = oldSpeedbrakeRatio <= 0.5f ? 1.0f : 0.0f;

                XPLMSetDataf(speedbrakeRatioDataRef, newSpeedbrakeRatio);
            }

            beginTime = 0.0f;
        }
    }
    // if the aircraft is not equipped with a speedbrake this command toggles the carb heat
    else
    {
        if (inPhase == xplm_CommandBegin)
        {
            int acfNumEngines = XPLMGetDatai(acfNumEnginesDataRef);

            if (acfNumEngines > 0)
            {
                float carbHeatRatio[acfNumEngines];
                XPLMGetDatavf(carbHeatRatioDataRef, carbHeatRatio, 0, acfNumEngines);

                float newCarbHeatRatio = carbHeatRatio[0] <= 0.5f ? 1.0f : 0.0f;

                for (int i = 0; i < acfNumEngines; i++)
                    carbHeatRatio[i] = newCarbHeatRatio;

                XPLMSetDatavf(carbHeatRatioDataRef, carbHeatRatio, 0, acfNumEngines);
            }
        }
    }

    return 0;
}

// check if a plugin with a given signature is enabled
static int isPluginEnabled(const char* pluginSignature)
{
    XPLMPluginID pluginId = XPLMFindPluginBySignature(pluginSignature);

    return XPLMIsPluginEnabled(pluginId);
}

// command-handler that handles the toggle autopilot command depending on the aircraft custom commands are invoked instead of the default ones
static int ToggleAutopilotCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    if (inPhase == xplm_CommandBegin)
    {
        // custom handling of QPAC A320
        if (isPluginEnabled(QPAC_A320_PLUGIN_SIGNATURE) != 0)
        {
            XPLMDataRef ap1EngageDataRef = XPLMFindDataRef("AirbusFBW/AP1Engage");
            XPLMDataRef ap2EngageDataRef = XPLMFindDataRef("AirbusFBW/AP2Engage");

            if (ap1EngageDataRef != NULL && ap2EngageDataRef != NULL)
            {
                if (XPLMGetDatai(ap1EngageDataRef) == 0 && XPLMGetDatai(ap2EngageDataRef) == 0)
                    XPLMCommandOnce(XPLMFindCommand("airbus_qpac/ap1_push"));
                else
                    XPLMCommandOnce(XPLMFindCommand("sim/autopilot/servos_and_flight_dir_off"));
            }
        }
        // custom handling of x737
        else if (isPluginEnabled(X737_PLUGIN_SIGNATURE) != 0)
        {
            XPLMDataRef cmdADataRef = XPLMFindDataRef("x737/systems/afds/CMD_A");
            XPLMDataRef cmdBDataRef = XPLMFindDataRef("x737/systems/afds/CMD_B");

            if (cmdADataRef != NULL && cmdBDataRef != NULL)
            {
                if (XPLMGetDatai(cmdADataRef) == 0 && XPLMGetDatai(cmdBDataRef) == 0)
                    XPLMCommandOnce(XPLMFindCommand("x737/mcp/CMDA_ON"));
                else
                {
                    XPLMCommandOnce(XPLMFindCommand("x737/mcp/CMDA_OFF"));
                    XPLMCommandOnce(XPLMFindCommand("x737/mcp/CMDB_OFF"));
                }
            }
        }
        // default handling
        else
            XPLMCommandOnce(XPLMFindCommand("sim/autopilot/servos_toggle"));
    }
}

// command-handler that handles the view modifier command
static int ViewModifierCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    if (inPhase != xplm_CommandContinue)
    {
        int joystickAxisAssignments[100];
        XPLMGetDatavi(joystickAxisAssignmentsDataRef, joystickAxisAssignments, 0, 100);

        int joystickAxisReverse[100];
        XPLMGetDatavi(joystickAxisReverseDataRef, joystickAxisReverse, 0, 100);

        // only apply the modifier if no other modifier is down which can alter any assignments
        if (inPhase == xplm_CommandBegin && trimModifierDown == 0  && mousePointerControlEnabled == 0)
        {
            viewModifierDown = 1;

            // assign the view controls to the left joystick's axis
            joystickAxisAssignments[JOYSTICK_AXIS_LEFT_X] = AXIS_ASSIGNMENT_VIEW_LEFT_RIGHT;
            joystickAxisAssignments[JOYSTICK_AXIS_LEFT_Y] = AXIS_ASSIGNMENT_VIEW_UP_DOWN;

            // reverse the left joystick's y axis while the view modifier is applied
            joystickAxisReverse[JOYSTICK_AXIS_LEFT_Y] = 1;

            // store the default button assignments
            PushButtonAssignments();

            // assign panel scrolling controls to the dpad
            int joystickButtonAssignments[1600];
            XPLMGetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);

            joystickButtonAssignments[JOYSTICK_BUTTON_DPAD_LEFT] = (std::size_t) XPLMFindCommand("sim/general/left");
            joystickButtonAssignments[JOYSTICK_BUTTON_DPAD_RIGHT] = (std::size_t) XPLMFindCommand("sim/general/right");
            joystickButtonAssignments[JOYSTICK_BUTTON_DPAD_UP] = (std::size_t) XPLMFindCommand("sim/general/up");
            joystickButtonAssignments[JOYSTICK_BUTTON_DPAD_DOWN] = (std::size_t) XPLMFindCommand("sim/general/down");
            joystickButtonAssignments[JOYSTICK_BUTTON_SQUARE] = (std::size_t) XPLMFindCommand("sim/general/rot_left");
            joystickButtonAssignments[JOYSTICK_BUTTON_CIRCLE] = (std::size_t) XPLMFindCommand("sim/general/rot_right");
            joystickButtonAssignments[JOYSTICK_BUTTON_TRIANGLE] = (std::size_t) XPLMFindCommand("sim/general/rot_up");
            joystickButtonAssignments[JOYSTICK_BUTTON_CROSS] = (std::size_t) XPLMFindCommand("sim/general/rot_down");

            XPLMSetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);
        }
        else if (inPhase == xplm_CommandEnd && trimModifierDown == 0 && mousePointerControlEnabled == 0)
        {
            viewModifierDown = 0;

            // assign the default controls to the left joystick's axis
            joystickAxisAssignments[JOYSTICK_AXIS_LEFT_X] = AXIS_ASSIGNMENT_YAW;
            joystickAxisAssignments[JOYSTICK_AXIS_LEFT_Y] = AXIS_ASSIGNMENT_NONE;

            // disable the axis reversing when the view modifier is not applied anymore
            joystickAxisReverse[JOYSTICK_AXIS_LEFT_Y] = 0;

            // restore the default button assignments
            PopButtonAssignments();
        }

        XPLMSetDatavi(joystickAxisAssignmentsDataRef, joystickAxisAssignments, 0, 100);
        XPLMSetDatavi(joystickAxisReverseDataRef, joystickAxisReverse, 0, 100);

        lastAxisAssignment = XPLMGetElapsedTime();
    }

    return 0;
}

// command-handler that handles the prop pitch modifier command
static int PropPitchModifierCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    if (inPhase == xplm_CommandBegin)
        propPitchModifierDown = 1;
    else if (inPhase == xplm_CommandEnd)
        propPitchModifierDown = 0;

    return 0;
}

// command-handler that handles the mixture control modifier command
static int MixtureControlModifierCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    if (inPhase == xplm_CommandBegin)
        mixtureControlModifierDown = 1;
    else if (inPhase == xplm_CommandEnd)
        mixtureControlModifierDown = 0;

    return 0;
}

// command-handler that handles the cowl flap modifier command
static int CowlFlapModifierCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    if (inPhase == xplm_CommandBegin)
        cowlFlapModifierDown = 1;
    else if (inPhase == xplm_CommandEnd)
        cowlFlapModifierDown = 0;

    return 0;
}

// command-handler that handles the trim modifier command
static int TrimModifierCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    // only apply the modifier if no other modifier is down which can alter any assignments
    if (inPhase == xplm_CommandBegin && viewModifierDown == 0 && mousePointerControlEnabled == 0)
    {
        trimModifierDown = 1;

        // store the default button assignments
        PushButtonAssignments();

        // assign trim controls to the buttons and dpad
        int joystickButtonAssignments[1600];
        XPLMGetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);

        joystickButtonAssignments[JOYSTICK_BUTTON_DPAD_LEFT] = (std::size_t) XPLMFindCommand("sim/flight_controls/aileron_trim_left");
        joystickButtonAssignments[JOYSTICK_BUTTON_DPAD_RIGHT] = (std::size_t) XPLMFindCommand("sim/flight_controls/aileron_trim_right");
        joystickButtonAssignments[JOYSTICK_BUTTON_DPAD_UP] = (std::size_t) XPLMFindCommand("sim/flight_controls/pitch_trim_down");
        joystickButtonAssignments[JOYSTICK_BUTTON_DPAD_DOWN] = (std::size_t) XPLMFindCommand("sim/flight_controls/pitch_trim_up");
        joystickButtonAssignments[JOYSTICK_BUTTON_SQUARE] = (std::size_t) XPLMFindCommand("sim/flight_controls/rudder_trim_left");
        joystickButtonAssignments[JOYSTICK_BUTTON_CIRCLE] = (std::size_t) XPLMFindCommand("sim/flight_controls/rudder_trim_right");

        XPLMSetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);
    }
    else if (inPhase == xplm_CommandEnd && viewModifierDown == 0 && mousePointerControlEnabled == 0)
    {
        trimModifierDown = 0;

        // restore the default button assignments
        PopButtonAssignments();
    }

    return 0;
}

// command-handler that handles the toggle mouse pointer control command
static int ToggleMousePointerControlCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    if (inPhase == xplm_CommandBegin)
    {
        int joystickAxisAssignments[100];
        XPLMGetDatavi(joystickAxisAssignmentsDataRef, joystickAxisAssignments, 0, 100);

        if (mousePointerControlEnabled == 0)
        {
            mousePointerControlEnabled = 1;

            // assign no controls to the left joystick's axis since it will control the mouse pointer
            joystickAxisAssignments[JOYSTICK_AXIS_LEFT_X] = AXIS_ASSIGNMENT_NONE;
            joystickAxisAssignments[JOYSTICK_AXIS_LEFT_Y] = AXIS_ASSIGNMENT_NONE;

            // store the default button assignments
            PushButtonAssignments();

            // assign no commands to the l2 and r2 buttons since they will be used as left and right mouse buttons
            int joystickButtonAssignments[1600];
            XPLMGetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);

            joystickButtonAssignments[JOYSTICK_BUTTON_CROSS] = (std::size_t) XPLMFindCommand("sim/none/none");
            joystickButtonAssignments[JOYSTICK_BUTTON_CIRCLE] = (std::size_t) XPLMFindCommand("sim/none/none");

            XPLMSetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);
        }
        else
        {
            mousePointerControlEnabled = 0;

            // release both mouse buttons if they were still pressed while the mouse pointer control mode was turned off
#if APL
            CGEventRef getLocationEvent = CGEventCreate(NULL);
            CGPoint location = CGEventGetLocation(getLocationEvent);
            CFRelease(getLocationEvent);

            if (CGEventSourceButtonState(kCGEventSourceStateCombinedSessionState, kCGMouseButtonLeft) != 0)
            {
                CGEventRef event = CGEventCreateMouseEvent(NULL, kCGEventLeftMouseUp, location, kCGMouseButtonLeft);
                CGEventPost(kCGHIDEventTap, event);
            }
            if (CGEventSourceButtonState(kCGEventSourceStateCombinedSessionState, kCGMouseButtonRight) != 0)
            {
                CGEventRef event = CGEventCreateMouseEvent(NULL, kCGEventRightMouseUp, location, kCGMouseButtonRight);
                CGEventPost(kCGHIDEventTap, event);
            }
#elif LIN
            if (display != NULL)
            {
                XTestFakeButtonEvent(display, 1, False, CurrentTime);
                XTestFakeButtonEvent(display, 2, False, CurrentTime);
                XFlush(display);
            }
#endif

            // assign the default controls to the left joystick's axis
            joystickAxisAssignments[JOYSTICK_AXIS_LEFT_X] = AXIS_ASSIGNMENT_YAW;
            joystickAxisAssignments[JOYSTICK_AXIS_LEFT_Y] = AXIS_ASSIGNMENT_NONE;

            // restore the default button assignments
            PopButtonAssignments();
        }

        XPLMSetDatavi(joystickAxisAssignmentsDataRef, joystickAxisAssignments, 0, 100);

        lastAxisAssignment = XPLMGetElapsedTime();
    }

    return 0;
}

// command-handler that handles the push-to-talk command
static int PushToTalkCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    if (inPhase != xplm_CommandContinue)
    {
        // only do push-to-talk if X-IvAp is enabled
        if (display != NULL && isPluginEnabled(X_IVAP_PLUGIN_SIGNATURE) != 0)
        {
#if APL
            // TODO: OS X implementation
#elif LIN
            KeyCode keycode = XKeysymToKeycode(display, XK_Insert);
            XTestFakeKeyEvent(display, keycode, inPhase == xplm_CommandBegin, CurrentTime);
            XFlush(display);
#endif
        }
    }

    return 0;
}

// normalizes a value of a range [inMin, inMax] to a value of the range [outMin, outMax]
static float Normalize(float value, float inMin, float inMax, float outMin, float outMax)
{
    return (outMax - outMin) / (inMax - inMin) * (value - inMax) + (outMax - outMin);
}

// flightloop-callback that mainly handles the joystick axis among other minor stuff
static float FlightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void *inRefcon)
{
    // handle switch to 3D command look
    if (switchTo3DCommandLook != 0)
    {
        XPLMCommandOnce(XPLMFindCommand("sim/view/3d_cockpit_cmnd_look"));
        switchTo3DCommandLook = 0;
    }

    float elapsedSinceLastAxisAssignment = XPLMGetElapsedTime() - lastAxisAssignment;

    // only handle the left joystick's axis if at least 750 ms have passed since the last axis assignment has occured, so that the user has time to center the joystick after releasing a modifier key
    if (XPLMGetDatai(hasJostickDataRef) &&  elapsedSinceLastAxisAssignment > 0.75f)
    {
        float sensitivityMultiplier = JOYSTICK_RELATIVE_CONTROL_MULTIPLIER * inElapsedSinceLastCall;

        float joystickPitchNullzone = XPLMGetDataf(joystickPitchNullzoneDataRef);

        float joystickAxisValues[100];
        XPLMGetDatavf(joystickAxisValuesDataRef, joystickAxisValues, 0, 100);

        static int joystickAxisLeftXCalibrated = 0;
        if (joystickAxisValues[JOYSTICK_AXIS_LEFT_X] > 0.0f)
            joystickAxisLeftXCalibrated = 1;

        // keep the value of the left joystick's y axis at 0.5 until a value higher/lower than 0.0/1.0 is read because axis can get initialized with a value of 0.0 or 1.0 instead of 0.5 if they haven't been moved yet - this can result in unexpected behaviour especially if the axis is used in relative mode
        static float leftJoystickMinYValue = 1.0f, leftJoystickMaxYValue = 0.0f;

        if (joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] < leftJoystickMinYValue)
            leftJoystickMinYValue = joystickAxisValues[JOYSTICK_AXIS_LEFT_Y];

        if (joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] > leftJoystickMaxYValue)
            leftJoystickMaxYValue = joystickAxisValues[JOYSTICK_AXIS_LEFT_Y];

        if (leftJoystickMinYValue == 1.0f || leftJoystickMaxYValue == 0.0f)
            joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] = 0.5f;

        int acfNumEngines = XPLMGetDatai(acfNumEnginesDataRef);

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
                    float d = sensitivityMultiplier * Normalize(joystickAxisValues[JOYSTICK_AXIS_LEFT_Y], 0.5f, 0.0f, acfRSCMingovPrp, acfRSCRedlinePrp);

                    float newPropRotationSpeedRadSecAll = propRotationSpeedRadSecAll + d;

                    // ensure we don't set values larger than 1.0
                    XPLMSetDataf(propRotationSpeedRadSecAllDataRef, newPropRotationSpeedRadSecAll < acfRSCRedlinePrp ? newPropRotationSpeedRadSecAll : acfRSCRedlinePrp);
                }
                // decrease prop pitch
                else if (joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] > 0.5f + joystickPitchNullzone)
                {
                    // normalize range [0.5, 1.0] to [acfRSCMingovPrp, acfRSCRedlinePrp]
                    float d = sensitivityMultiplier * Normalize(joystickAxisValues[JOYSTICK_AXIS_LEFT_Y], 0.5f, 1.0f, acfRSCMingovPrp, acfRSCRedlinePrp);

                    float newPropRotationSpeedRadSecAll = propRotationSpeedRadSecAll - d;

                    // ensure we don't set values smaller than 0.0
                    XPLMSetDataf(propRotationSpeedRadSecAllDataRef, newPropRotationSpeedRadSecAll > acfRSCMingovPrp ? newPropRotationSpeedRadSecAll : acfRSCMingovPrp);
                }
            }
            else if (mixtureControlModifierDown != 0)
            {
                float mixtureRatioAll = XPLMGetDataf(mixtureRatioAllDataRef);

                // increase mixture setting
                if (joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] < 0.5f - joystickPitchNullzone)
                {
                    // normalize range [0.5, 0.0] to [0.0, 1.0]
                    float d = sensitivityMultiplier * Normalize(joystickAxisValues[JOYSTICK_AXIS_LEFT_Y], 0.5f, 0.0f, 0.0f, 1.0f);

                    float newMixtureRatioAll = mixtureRatioAll + d;

                    // ensure we don't set values larger than 1.0
                    XPLMSetDataf(mixtureRatioAllDataRef, newMixtureRatioAll < 1.0f ? newMixtureRatioAll : 1.0f);
                }
                // decrease mixture setting
                else if (joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] > 0.5f + joystickPitchNullzone)
                {
                    // normalize range [0.5, 1.0] to [0.0, 1.0]
                    float d = sensitivityMultiplier * Normalize(joystickAxisValues[JOYSTICK_AXIS_LEFT_Y], 0.5f, 1.0f, 0.0f, 1.0f);

                    float newMixtureRatioAll = mixtureRatioAll - d;

                    // ensure we don't set values smaller than 0.0
                    XPLMSetDataf(mixtureRatioAllDataRef, newMixtureRatioAll > 0.0f ? newMixtureRatioAll : 0.0f);
                }
            }
            else if (cowlFlapModifierDown != 0)
            {
                float cowlFlapRatio[acfNumEngines];
                XPLMGetDatavf(cowlFlapRatioDataRef, cowlFlapRatio, 0, acfNumEngines);

                // decrease cowl flap setting
                if (joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] < 0.5f - joystickPitchNullzone)
                {
                    // normalize range [0.5, 0.0] to [0.0, 1.0]
                    float d = sensitivityMultiplier * Normalize(joystickAxisValues[JOYSTICK_AXIS_LEFT_Y], 0.5f, 0.0f, 0.0f, 1.0f);

                    // ensure we don't set values smaller than 0.0
                    for (int i = 0; i < acfNumEngines; i++)
                    {
                        float newCowlFlapRatio = cowlFlapRatio[i] - d;
                        cowlFlapRatio[i] = newCowlFlapRatio > 0.0f ? newCowlFlapRatio : 0.0f;
                    }
                }
                // increase cowl flap setting
                else if (joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] > 0.5f + joystickPitchNullzone)
                {
                    // normalize range [0.5, 1.0] to [0.0, 1.0]
                    float d = sensitivityMultiplier * Normalize(joystickAxisValues[JOYSTICK_AXIS_LEFT_Y], 0.5f, 1.0f, 0.0f, 1.0f);

                    // ensure we don't set values larger than 1.0
                    for (int i = 0; i < acfNumEngines; i++)
                    {
                        float newCowlFlapRatio = cowlFlapRatio[i] + d;
                        cowlFlapRatio[i] = newCowlFlapRatio < 1.0f ? newCowlFlapRatio : 1.0f;
                    }
                }

                XPLMSetDatavf(cowlFlapRatioDataRef, cowlFlapRatio, 0, acfNumEngines);
            }
            else if (mousePointerControlEnabled != 0)
            {
                int distX = 0, distY = 0;

                // move mouse pointer left
                if (joystickAxisValues[JOYSTICK_AXIS_LEFT_X] < 0.5f - joystickPitchNullzone)
                {
                    // normalize range [0.5, 0.0] to [0.0, 1.0]
                    float d = Normalize(joystickAxisValues[JOYSTICK_AXIS_LEFT_X], 0.5f, 0.0f, 0.0f, 1.0f);

                    // apply acceleration function (y = x^2)
                    distX -= (int) powf(d * JOYSTICK_MOUSE_POINTER_SENSITIVITY, 2.0f) * inElapsedSinceLastCall;
                }
                // move mouse pointer right
                else if (joystickAxisValues[JOYSTICK_AXIS_LEFT_X] > 0.5f + joystickPitchNullzone)
                {
                    // normalize range [0.5, 1.0] to [0.0, 1.0]
                    float d = Normalize(joystickAxisValues[JOYSTICK_AXIS_LEFT_X], 0.5f, 1.0f, 0.0f, 1.0f);

                    // apply acceleration function (y = x^2)
                    distX += (int) powf(d * JOYSTICK_MOUSE_POINTER_SENSITIVITY, 2.0f) * inElapsedSinceLastCall;
                }

                // move mouse pointer up
                if (joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] < 0.5f - joystickPitchNullzone)
                {
                    // normalize range [0.5, 0.0] to [0.0, 1.0]
                    float d = Normalize(joystickAxisValues[JOYSTICK_AXIS_LEFT_Y], 0.5f, 0.0f, 0.0f, 1.0f);

                    // apply acceleration function (y = x^2)
                    distY -= (int) powf(d * JOYSTICK_MOUSE_POINTER_SENSITIVITY, 2.0f) * inElapsedSinceLastCall;
                }
                // move mouse pointer down
                else if (joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] > 0.5f + joystickPitchNullzone)
                {
                    // normalize range [0.5, 1.0] to [0.0, 1.0]
                    float d = Normalize(joystickAxisValues[JOYSTICK_AXIS_LEFT_Y], 0.5f, 1.0f, 0.0f, 1.0f);

                    // apply acceleration function (y = x^2)
                    distY += (int) powf(d * JOYSTICK_MOUSE_POINTER_SENSITIVITY, 2.0f) * inElapsedSinceLastCall;
                }

#if APL
                // get current mouse pointer location
                CGEventRef getLocationEvent = CGEventCreate(NULL);
                CGPoint oldLocation = CGEventGetLocation(getLocationEvent);
                CFRelease(getLocationEvent);

                CGPoint newLocation;
                newLocation.x = oldLocation.x + distX;
                newLocation.y = oldLocation.y + distY;

                // get active displays
                CGDirectDisplayID activeDisplays[8];
                uint32_t displayCount = 0;
                CGGetActiveDisplayList(8, activeDisplays, &displayCount);

                // get display ids of the display on which the mouse pointer was contained before and will be once moved - values of -1 indicate that the pointer is outside of all displays
                int oldContainingDisplay = -1;
                int newContainingDisplay = -1;
                for (int i = 0; i < displayCount; i++)
                {
                    CGRect screenBounds = CGDisplayBounds(activeDisplays[i]);

                    if (CGRectContainsPoint(screenBounds, oldLocation))
                        oldContainingDisplay = i;

                    if (CGRectContainsPoint(screenBounds, newLocation))
                        newContainingDisplay = i;
                }

                // ensure the pointer is not moved beyond the bounds of the display it was contained in before
                if (newContainingDisplay == -1 && oldContainingDisplay > -1)
                {
                    CGRect screenBounds = CGDisplayBounds(activeDisplays[oldContainingDisplay]);
                    int minX = (int) screenBounds.origin.x;
                    int minY = (int) screenBounds.origin.y;
                    int maxX = (int) minX + screenBounds.size.width - 1;
                    int maxY = (int) minY + screenBounds.size.height - 1;

                    newLocation.x = newLocation.x >= minX ? newLocation.x : minX;
                    newLocation.x = newLocation.x <= maxX ? newLocation.x : maxX;
                    newLocation.y = newLocation.y >= minY ? newLocation.y : minY;
                    newLocation.y = newLocation.y < maxY ? newLocation.y : maxY;
                }

                // move mouse pointer by distX and distY pixels
                CGEventRef moveMouseEvent = CGEventCreateMouseEvent(NULL, kCGEventMouseMoved, newLocation, kCGMouseButtonLeft);
                CGEventPost(kCGHIDEventTap, moveMouseEvent);
                CFRelease(moveMouseEvent);
#elif LIN
                if (display != NULL)
                {
                    XWarpPointer(display, None, None, 0, 0, 0, 0, distX, distY);
                    XFlush(display);
                }
#endif

                int joystickButtonValues[1600];
                XPLMGetDatavi(joystickButtonValuesDataRef, joystickButtonValues, 0, 1600);

                // get mouse button status
                int leftMouseButtonDown = 0, rightMouseButtonDown = 0;
#if APL
                leftMouseButtonDown = CGEventSourceButtonState(kCGEventSourceStateCombinedSessionState, kCGMouseButtonLeft);
                rightMouseButtonDown = CGEventSourceButtonState(kCGEventSourceStateCombinedSessionState, kCGMouseButtonRight);

                // update mouse pointer location since we need the current location to create a mouse up/down event
                getLocationEvent = CGEventCreate(NULL);
                newLocation = CGEventGetLocation(getLocationEvent);
                CFRelease(getLocationEvent);
#elif LIN
                if (display != NULL)
                {
                    Window root, child;
                    int rootX, rootY, winX, winY;
                    unsigned int mask;
                    XQueryPointer(display, DefaultRootWindow(display), &root, &child, &rootX, &rootY, &winX, &winY, &mask);
                    leftMouseButtonDown = (mask & Button1Mask) >> 8;
                    rightMouseButtonDown = (mask & Button2Mask) >> 8;
                }
#endif

                if (joystickButtonValues[JOYSTICK_BUTTON_CROSS] != 0)
                {
                    // press left mouse button down
#if APL
                    if (leftMouseButtonDown == 0)
                    {
                        CGEventRef event = CGEventCreateMouseEvent(NULL, kCGEventLeftMouseDown, newLocation, kCGMouseButtonLeft);
                        CGEventPost(kCGHIDEventTap, event);
                    }
#elif LIN
                    if (display != NULL)
                    {
                        XTestFakeButtonEvent(display, 1, True, CurrentTime);
                        XFlush(display);
                    }
#endif
                }
                else
                {
                    // release left mouse button
#if APL
                    if (leftMouseButtonDown != 0)
                    {
                        CGEventRef event = CGEventCreateMouseEvent(NULL, kCGEventLeftMouseUp, newLocation, kCGMouseButtonLeft);
                        CGEventPost(kCGHIDEventTap, event);
                    }
#elif LIN
                    if (display != NULL)
                    {
                        XTestFakeButtonEvent(display, 1, False, CurrentTime);
                        XFlush(display);
                    }
#endif
                }

                if (joystickButtonValues[JOYSTICK_BUTTON_CIRCLE] != 0)
                {
                    // press right mouse button down
#if APL
                    if (rightMouseButtonDown == 0)
                    {
                        CGEventRef event = CGEventCreateMouseEvent(NULL, kCGEventRightMouseDown, newLocation, kCGMouseButtonRight);
                        CGEventPost(kCGHIDEventTap, event);
                    }
#elif LIN
                    if (display != NULL)
                    {
                        XTestFakeButtonEvent(display, 2, True, CurrentTime);
                        XFlush(display);
                    }
#endif
                }
                else
                {
                    // release right mouse button
#if APL
                    if (rightMouseButtonDown != 0)
                    {
                        CGEventRef event = CGEventCreateMouseEvent(NULL, kCGEventRightMouseUp, newLocation, kCGMouseButtonRight);
                        CGEventPost(kCGHIDEventTap, event);
                    }
#elif LIN
                    if (display != NULL)
                    {
                        XTestFakeButtonEvent(display, 2, False, CurrentTime);
                        XFlush(display);
                    }
#endif
                }
            }
            else
            {
                static float normalizedViewHeading = 0.0f;

                // handle view heading when the joystick has been calibrated, the aircraft is on ground and the current view is 3D cockpit command look
                if (joystickAxisLeftXCalibrated > 0 && XPLMGetDatai(ongroundAnyDataRef) != 0 && XPLMGetDatai(viewTypeDataRef) == VIEW_TYPE_3D_COCKPIT_COMMAND_LOOK)
                {
                    float joystickHeadingNullzone = XPLMGetDataf(joystickHeadingNullzoneDataRef);

                    // handle nullzone
                    if(fabs(joystickAxisValues[JOYSTICK_AXIS_LEFT_X] - 0.5f) > joystickHeadingNullzone)
                    {
                        // normalize axis deflection
                        if (joystickAxisValues[JOYSTICK_AXIS_LEFT_X] > 0.5f)
                            normalizedViewHeading = Normalize(joystickAxisValues[JOYSTICK_AXIS_LEFT_X], 0.5f + joystickHeadingNullzone, 1.0f, 0.0f, 1.0f);
                        else
                            normalizedViewHeading = Normalize(joystickAxisValues[JOYSTICK_AXIS_LEFT_X], 0.5f - joystickHeadingNullzone, 0.0f, 0.0f, -1.0f);

                        float groundspeed = XPLMGetDataf(groundspeedDataRef);

                        // clamp absolute groundspeed to 15 m/s
                        groundspeed = fabs(groundspeed < 15.0f ?  groundspeed : 15.0f);

                        // apply acceleration function (y = x^2) and fade-out with increasing groundspeed
                        float newPilotsHeadPsi = normalizedViewHeading * powf(joystickAxisValues[JOYSTICK_AXIS_LEFT_X] - 0.5f, 2.0f) * 2.0f * Normalize(groundspeed, 15.0f, 0.0f, -1.0f, 0.0f) * 90.0f;

                        XPLMSetDataf(pilotsHeadPsiDataRef, newPilotsHeadPsi);
                    }
                    // reset the view if the axis deflection has jumped from a large value right into the nullzone
                    else if (normalizedViewHeading != 0.0f)
                    {
                        XPLMSetDataf(pilotsHeadPsiDataRef, 0.0f);
                        normalizedViewHeading = 0.0f;
                    }
                }


                float throttleRatioAll = XPLMGetDataf(throttleRatioAllDataRef);

                float thrustReverserDeployRatio[acfNumEngines];
                XPLMGetDatavf(thrustReverserDeployRatioDataRef, thrustReverserDeployRatio, 0, acfNumEngines);

                float averageThrustReverserDeployRatio = 0.0f;
                for (int i = 0; i < acfNumEngines; i++)
                    averageThrustReverserDeployRatio += thrustReverserDeployRatio[i];
                averageThrustReverserDeployRatio /= acfNumEngines;

                // increase throttle setting
                if (joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] < 0.5f - joystickPitchNullzone)
                {
                    // normalize range [0.5, 0.0] to [0.0, 1.0]
                    float d = sensitivityMultiplier * Normalize(joystickAxisValues[JOYSTICK_AXIS_LEFT_Y], 0.5f, 0.0f, 0.0f, 1.0f);

                    // invert d if thrust reversers are engaged
                    if (averageThrustReverserDeployRatio > 0.5f)
                    {
                        float newThrottleRatioAll = throttleRatioAll - d;

                        // ensure we don't set values smaller than 0.0
                        XPLMSetDataf(throttleRatioAllDataRef, newThrottleRatioAll > 0.0f ? newThrottleRatioAll : 0.0f);
                    }
                    else
                    {
                        float newThrottleRatioAll = throttleRatioAll + d;

                        // ensure we don't set values larger than 1.0
                        XPLMSetDataf(throttleRatioAllDataRef, newThrottleRatioAll < 1.0f ? newThrottleRatioAll : 1.0f);
                    }
                }
                // decrease throttle setting
                else if (joystickAxisValues[JOYSTICK_AXIS_LEFT_Y] > 0.5f + joystickPitchNullzone)
                {
                    // normalize range [0.5, 1.0] to [0.0, 1.0]
                    float d = sensitivityMultiplier * Normalize(joystickAxisValues[JOYSTICK_AXIS_LEFT_Y], 0.5f, 1.0f, 0.0f, 1.0f);

                    // invert d if thrust reversers are engaged
                    if (averageThrustReverserDeployRatio > 0.5f)
                    {
                        float newThrottleRatioAll = throttleRatioAll + d;

                        // ensure we don't set values larger than 1.0
                        XPLMSetDataf(throttleRatioAllDataRef, newThrottleRatioAll < 1.0f ? newThrottleRatioAll : 1.0f);
                    }
                    else
                    {
                        float newThrottleRatioAll = throttleRatioAll - d;

                        // ensure we don't set values smaller than 0.0
                        XPLMSetDataf(throttleRatioAllDataRef, newThrottleRatioAll > 0.0f ? newThrottleRatioAll : 0.0f);
                    }
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

// set the default axis and button assignments
static void SetDefaultAssignments(void)
{
    // only set default assignments if a joystick is found and if no modifier is down which can alter any assignments
    if (XPLMGetDatai(hasJostickDataRef) && viewModifierDown == 0 && trimModifierDown == 0  && mousePointerControlEnabled == 0)
    {
        // set default axis assignments
        int joystickAxisAssignments[100];
        XPLMGetDatavi(joystickAxisAssignmentsDataRef, joystickAxisAssignments, 0, 100);

        joystickAxisAssignments[JOYSTICK_AXIS_LEFT_X] = AXIS_ASSIGNMENT_YAW;
        joystickAxisAssignments[JOYSTICK_AXIS_LEFT_Y] = AXIS_ASSIGNMENT_NONE;
        joystickAxisAssignments[JOYSTICK_AXIS_RIGHT_X] = AXIS_ASSIGNMENT_ROLL;
        joystickAxisAssignments[JOYSTICK_AXIS_RIGHT_Y] = AXIS_ASSIGNMENT_PITCH;

        XPLMSetDatavi(joystickAxisAssignmentsDataRef, joystickAxisAssignments, 0, 100);

        // set default button assignments
        int joystickButtonAssignments[1600];
        XPLMGetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);

        joystickButtonAssignments[JOYSTICK_BUTTON_DPAD_LEFT] = (std::size_t) XPLMFindCommand("sim/flight_controls/flaps_up");
        joystickButtonAssignments[JOYSTICK_BUTTON_DPAD_RIGHT] = (std::size_t) XPLMFindCommand("sim/flight_controls/flaps_down");
        joystickButtonAssignments[JOYSTICK_BUTTON_DPAD_UP] = (std::size_t) XPLMFindCommand(SPEED_BRAKE_AND_CARB_HEAT_TOGGLE_ARM_COMMAND);
        joystickButtonAssignments[JOYSTICK_BUTTON_DPAD_DOWN] = (std::size_t) XPLMFindCommand("sim/flight_controls/landing_gear_toggle");
        joystickButtonAssignments[JOYSTICK_BUTTON_SQUARE] = (std::size_t) XPLMFindCommand(CYCLE_RESET_VIEW_COMMAND);
        joystickButtonAssignments[JOYSTICK_BUTTON_CIRCLE] = (std::size_t) XPLMFindCommand(MIXTURE_CONTROL_MODIFIER_COMMAND);
        joystickButtonAssignments[JOYSTICK_BUTTON_TRIANGLE] = (std::size_t) XPLMFindCommand(PROP_PITCH_MODIFIER_COMMAND);
        joystickButtonAssignments[JOYSTICK_BUTTON_CROSS] = (std::size_t) XPLMFindCommand(COWL_FLAP_MODIFIER_COMMAND);
        joystickButtonAssignments[JOYSTICK_BUTTON_START] = (std::size_t) XPLMFindCommand(TOOGLE_AUTOPILOT_COMMAND);
        joystickButtonAssignments[JOYSTICK_BUTTON_SELECT] = (std::size_t) XPLMFindCommand("sim/engines/thrust_reverse_toggle");
        joystickButtonAssignments[JOYSTICK_BUTTON_L1] = (std::size_t) XPLMFindCommand(TRIM_MODIFIER_COMMAND);
        joystickButtonAssignments[JOYSTICK_BUTTON_R1] = (std::size_t) XPLMFindCommand(VIEW_MODIFIER_COMMAND);
        joystickButtonAssignments[JOYSTICK_BUTTON_L2] = (std::size_t) XPLMFindCommand(PUSH_TO_TALK_COMMAND);
        joystickButtonAssignments[JOYSTICK_BUTTON_R2] = (std::size_t) XPLMFindCommand("sim/flight_controls/brakes_toggle_regular");
        joystickButtonAssignments[JOYSTICK_BUTTON_L3] = (std::size_t) XPLMFindCommand("sim/general/zoom_out");
        joystickButtonAssignments[JOYSTICK_BUTTON_R3] = (std::size_t) XPLMFindCommand("sim/general/zoom_in");
        joystickButtonAssignments[JOYSTICK_BUTTON_PS] = (std::size_t) XPLMFindCommand(TOGGLE_MOUSE_POINTER_CONTROL_COMMAND);

        XPLMSetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);
    }
}

// handles the menu-entries
static void MenuHandlerCallback(void *inMenuRef, void *inItemRef)
{
    // set default assignments menu entry
    if ((long) inItemRef == 0)
        SetDefaultAssignments();
}

PLUGIN_API int XPluginStart(char *outName, char *outSig, char *outDesc)
{
    // set plugin info
    strcpy(outName, NAME);
    strcpy(outSig, "de.bwravencl."NAME_LOWERCASE);
    strcpy(outDesc, NAME" allows flying X-Plane by gamepad!");

    // obtain datarefs
    acfRSCMingovPrpDataRef = XPLMFindDataRef("sim/aircraft/controls/acf_RSC_mingov_prp");
    acfRSCRedlinePrpDataRef = XPLMFindDataRef("sim/aircraft/controls/acf_RSC_redline_prp");
    acfNumEnginesDataRef = XPLMFindDataRef("sim/aircraft/engine/acf_num_engines");
    acfSbrkEQDataRef = XPLMFindDataRef("sim/aircraft/parts/acf_sbrkEQ");
    ongroundAnyDataRef = XPLMFindDataRef("sim/flightmodel/failures/onground_any");
    groundspeedDataRef = XPLMFindDataRef("sim/flightmodel/position/groundspeed");
    pilotsHeadPsiDataRef = XPLMFindDataRef("sim/graphics/view/pilots_head_psi");
    viewTypeDataRef = XPLMFindDataRef("sim/graphics/view/view_type");
    hasJostickDataRef = XPLMFindDataRef("sim/joystick/has_joystick");
    joystickPitchNullzoneDataRef = XPLMFindDataRef("sim/joystick/joystick_pitch_nullzone");
    joystickHeadingNullzoneDataRef = XPLMFindDataRef("sim/joystick/joystick_heading_nullzone");
    joystickAxisAssignmentsDataRef = XPLMFindDataRef("sim/joystick/joystick_axis_assignments");
    joystickButtonAssignmentsDataRef = XPLMFindDataRef("sim/joystick/joystick_button_assignments");
    joystickAxisValuesDataRef = XPLMFindDataRef("sim/joystick/joystick_axis_values");
    joystickAxisReverseDataRef = XPLMFindDataRef("sim/joystick/joystick_axis_reverse");
    joystickButtonValuesDataRef = XPLMFindDataRef("sim/joystick/joystick_button_values");
    leftBrakeRatioDataRef = XPLMFindDataRef("sim/cockpit2/controls/left_brake_ratio");
    rightBrakeRatioDataRef = XPLMFindDataRef("sim/cockpit2/controls/right_brake_ratio");
    speedbrakeRatioDataRef = XPLMFindDataRef("sim/cockpit2/controls/speedbrake_ratio");
    throttleRatioAllDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/throttle_ratio_all");
    propRotationSpeedRadSecAllDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/prop_rotation_speed_rad_sec_all");
    mixtureRatioAllDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/mixture_ratio_all");
    carbHeatRatioDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/carb_heat_ratio");
    cowlFlapRatioDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/cowl_flap_ratio");
    thrustReverserDeployRatioDataRef = XPLMFindDataRef("sim/flightmodel2/engines/thrust_reverser_deploy_ratio");

    // create custom commands
    cycleResetViewCommand = XPLMCreateCommand(CYCLE_RESET_VIEW_COMMAND, "Cycle/Reset View");
    speedBrakeAndCarbHeatToggleArmCommand = XPLMCreateCommand(SPEED_BRAKE_AND_CARB_HEAT_TOGGLE_ARM_COMMAND, "Toggle/Arm Speedbrake/Carb Heat");
    toggleAutopilotCommand = XPLMCreateCommand(TOOGLE_AUTOPILOT_COMMAND, "Toggle Autopilot");
    viewModifierCommand = XPLMCreateCommand(VIEW_MODIFIER_COMMAND, "View Modifier");
    propPitchModifierCommand = XPLMCreateCommand(PROP_PITCH_MODIFIER_COMMAND, "Prop Pitch Modifier");
    mixtureControlModifierCommand = XPLMCreateCommand(MIXTURE_CONTROL_MODIFIER_COMMAND, "Mixture Control Modifier");
    cowlFlapModifierCommand = XPLMCreateCommand(COWL_FLAP_MODIFIER_COMMAND, "Cowl Flap Modifier");
    trimModifierCommand = XPLMCreateCommand(TRIM_MODIFIER_COMMAND, "Trim Modifier");
    toggleMousePointerControlCommand = XPLMCreateCommand(TOGGLE_MOUSE_POINTER_CONTROL_COMMAND, "Toggle Mouse Pointer Control");
    pushToTalkCommand = XPLMCreateCommand(PUSH_TO_TALK_COMMAND, "Push-To-Talk");

    // register custom commands
    XPLMRegisterCommandHandler(cycleResetViewCommand, CycleResetViewCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(speedBrakeAndCarbHeatToggleArmCommand, SpeedBrakeAndCarbHeatToggleArmCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(toggleAutopilotCommand, ToggleAutopilotCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(viewModifierCommand, ViewModifierCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(propPitchModifierCommand, PropPitchModifierCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(mixtureControlModifierCommand, MixtureControlModifierCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(cowlFlapModifierCommand, CowlFlapModifierCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(trimModifierCommand, TrimModifierCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(toggleMousePointerControlCommand, ToggleMousePointerControlCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(pushToTalkCommand, PushToTalkCommandHandler, 1, NULL);

    // register flight loop callback
    XPLMRegisterFlightLoopCallback(FlightLoopCallback, -1, NULL);

    // create menu-entries
    int subMenuItem = XPLMAppendMenuItem(XPLMFindPluginsMenu(), NAME, 0, 1);
    XPLMMenuID menu = XPLMCreateMenu(NAME, XPLMFindPluginsMenu(), subMenuItem, MenuHandlerCallback, 0);
    XPLMAppendMenuItem(menu, "Set Default Assignments", (void*) 0, 1);

#if LIN
    display = XOpenDisplay(NULL);
    if(display != NULL)
    {
        int screenNumber = DefaultScreen(display);
        XEvent event;
        XQueryPointer(display, RootWindow(display, DefaultScreen(display)), &event.xbutton.root, &event.xbutton.window, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
    }
#endif

    return 1;
}

PLUGIN_API void	XPluginStop(void)
{
    // revert any remaining button assignments
    while (!buttonAssignmentsStack.empty())
        PopButtonAssignments();

#if LIN
    if(display != NULL)
        XCloseDisplay(display);
#endif
}

PLUGIN_API void XPluginDisable(void)
{
}

PLUGIN_API int XPluginEnable(void)
{
    return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFromWho, long inMessage, void *inParam)
{
    // if another plane or airport is loaded and the plane has no 2D panel, automatically switch the view to the 3D cockpit in the next flightloop
    if (inMessage == XPLM_MSG_PLANE_LOADED || inMessage == XPLM_MSG_AIRPORT_LOADED)
    {
        switchTo3DCommandLook = 0;

        if (Has2DPanel() == 0)
            switchTo3DCommandLook = 1;
    }
}
