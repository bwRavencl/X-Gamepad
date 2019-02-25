/* Copyright (C) 2019  Matteo Hausner
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
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMMenus.h"
#include "XPLMPlanes.h"
#include "XPLMPlugin.h"
#include "XPLMProcessing.h"
#include "XPLMUtilities.h"
#include "XPStandardWidgets.h"
#include "XPWidgets.h"

#include <fstream>
#include <sstream>
#include <stack>

#ifndef VERSION
#define VERSION "UNDEFINED"
#endif

#if IBM
#include "glew.h"
#include "hidapi.h"
#include <process.h>
#include <windows.h>
#elif APL
#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
#include <OpenGL/gl.h>
#elif LIN
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#endif

// define name
#define NAME "X-Gamepad"
#define NAME_LOWERCASE "x_gamepad"

// define config file path
#if IBM
#define CONFIG_PATH ".\\Resources\\plugins\\" NAME_LOWERCASE "\\" NAME_LOWERCASE ".ini"
#else
#define CONFIG_PATH "./Resources/plugins/" NAME_LOWERCASE "/" NAME_LOWERCASE ".ini"
#endif

// define joystick axis
#define JOYSTICK_AXIS_ABSTRACT_LEFT_X 0
#define JOYSTICK_AXIS_ABSTRACT_LEFT_Y 1
#define JOYSTICK_AXIS_ABSTRACT_RIGHT_X 2
#define JOYSTICK_AXIS_ABSTRACT_RIGHT_Y 3

// define xbox 360 joystick axis
#if IBM
#define JOYSTICK_AXIS_XBOX360_LEFT_X 1
#define JOYSTICK_AXIS_XBOX360_LEFT_Y 0
#define JOYSTICK_AXIS_XBOX360_RIGHT_X 3
#define JOYSTICK_AXIS_XBOX360_RIGHT_Y 2
#define JOYSTICK_AXIS_XBOX360_TRIGGERS 4
#else
#define JOYSTICK_AXIS_XBOX360_LEFT_X 0
#define JOYSTICK_AXIS_XBOX360_LEFT_Y 1
#define JOYSTICK_AXIS_XBOX360_RIGHT_X 3
#define JOYSTICK_AXIS_XBOX360_RIGHT_Y 4
#define JOYSTICK_AXIS_XBOX360_LEFT_TRIGGER 2
#define JOYSTICK_AXIS_XBOX360_RIGHT_TRIGGER 5
#endif

// define dualshock 4 joystick axis
#if IBM
#define JOYSTICK_AXIS_DS4_LEFT_X 3
#define JOYSTICK_AXIS_DS4_LEFT_Y 2
#define JOYSTICK_AXIS_DS4_RIGHT_X 1
#define JOYSTICK_AXIS_DS4_RIGHT_Y 0
#define JOYSTICK_AXIS_DS4_L2 5
#define JOYSTICK_AXIS_DS4_R2 4
#else
#define JOYSTICK_AXIS_DS4_LEFT_X 0
#define JOYSTICK_AXIS_DS4_LEFT_Y 1
#define JOYSTICK_AXIS_DS4_RIGHT_X 3
#define JOYSTICK_AXIS_DS4_RIGHT_Y 4
#define JOYSTICK_AXIS_DS4_L2 2
#define JOYSTICK_AXIS_DS4_R2 5
#endif

// define abstract joystick buttons
#define JOYSTICK_BUTTON_ABSTRACT_DPAD_LEFT 0
#define JOYSTICK_BUTTON_ABSTRACT_DPAD_RIGHT 1
#define JOYSTICK_BUTTON_ABSTRACT_DPAD_UP 2
#define JOYSTICK_BUTTON_ABSTRACT_DPAD_DOWN 3
#define JOYSTICK_BUTTON_ABSTRACT_DPAD_LEFT_UP 4
#define JOYSTICK_BUTTON_ABSTRACT_DPAD_LEFT_DOWN 5
#define JOYSTICK_BUTTON_ABSTRACT_DPAD_RIGHT_UP 6
#define JOYSTICK_BUTTON_ABSTRACT_DPAD_RIGHT_DOWN 7
#define JOYSTICK_BUTTON_ABSTRACT_FACE_LEFT 8
#define JOYSTICK_BUTTON_ABSTRACT_FACE_RIGHT 9
#define JOYSTICK_BUTTON_ABSTRACT_FACE_UP 10
#define JOYSTICK_BUTTON_ABSTRACT_FACE_DOWN 11
#define JOYSTICK_BUTTON_ABSTRACT_CENTER_LEFT 12
#define JOYSTICK_BUTTON_ABSTRACT_CENTER_RIGHT 13
#define JOYSTICK_BUTTON_ABSTRACT_BUMPER_LEFT 14
#define JOYSTICK_BUTTON_ABSTRACT_BUMPER_RIGHT 15
#define JOYSTICK_BUTTON_ABSTRACT_STICK_LEFT 16
#define JOYSTICK_BUTTON_ABSTRACT_STICK_RIGHT 17

// define xbox 360 joystick buttons
#if IBM
#define JOYSTICK_BUTTON_XBOX360_DPAD_LEFT 16
#define JOYSTICK_BUTTON_XBOX360_DPAD_RIGHT 12
#define JOYSTICK_BUTTON_XBOX360_DPAD_UP 10
#define JOYSTICK_BUTTON_XBOX360_DPAD_DOWN 14
#define JOYSTICK_BUTTON_XBOX360_DPAD_LEFT_UP -1
#define JOYSTICK_BUTTON_XBOX360_DPAD_LEFT_DOWN -1
#define JOYSTICK_BUTTON_XBOX360_DPAD_RIGHT_UP -1
#define JOYSTICK_BUTTON_XBOX360_DPAD_RIGHT_DOWN -1
#define JOYSTICK_BUTTON_XBOX360_X 2
#define JOYSTICK_BUTTON_XBOX360_B 1
#define JOYSTICK_BUTTON_XBOX360_Y 3
#define JOYSTICK_BUTTON_XBOX360_A 0
#define JOYSTICK_BUTTON_XBOX360_START 7
#define JOYSTICK_BUTTON_XBOX360_BACK 6
#define JOYSTICK_BUTTON_XBOX360_LEFT_BUMPER 4
#define JOYSTICK_BUTTON_XBOX360_RIGHT_BUMPER 5
#define JOYSTICK_BUTTON_XBOX360_LEFT_STICK 8
#define JOYSTICK_BUTTON_XBOX360_RIGHT_STICK 9
#elif APL
#define JOYSTICK_BUTTON_XBOX360_DPAD_LEFT -1
#define JOYSTICK_BUTTON_XBOX360_DPAD_RIGHT -1
#define JOYSTICK_BUTTON_XBOX360_DPAD_UP -1
#define JOYSTICK_BUTTON_XBOX360_DPAD_DOWN -1
#define JOYSTICK_BUTTON_XBOX360_DPAD_LEFT_UP -1
#define JOYSTICK_BUTTON_XBOX360_DPAD_LEFT_DOWN -1
#define JOYSTICK_BUTTON_XBOX360_DPAD_RIGHT_UP -1
#define JOYSTICK_BUTTON_XBOX360_DPAD_RIGHT_DOWN -1
#define JOYSTICK_BUTTON_XBOX360_X -1
#define JOYSTICK_BUTTON_XBOX360_B -1
#define JOYSTICK_BUTTON_XBOX360_Y -1
#define JOYSTICK_BUTTON_XBOX360_A -1
#define JOYSTICK_BUTTON_XBOX360_START -1
#define JOYSTICK_BUTTON_XBOX360_BACK -1
#define JOYSTICK_BUTTON_XBOX360_LEFT_BUMPER -1
#define JOYSTICK_BUTTON_XBOX360_RIGHT_BUMPER -1
#define JOYSTICK_BUTTON_XBOX360_LEFT_STICK -1
#define JOYSTICK_BUTTON_XBOX360_RIGHT_STICK -1
#define JOYSTICK_BUTTON_XBOX360_GUIDE -1
#elif LIN
#define JOYSTICK_BUTTON_XBOX360_DPAD_LEFT 17
#define JOYSTICK_BUTTON_XBOX360_DPAD_RIGHT 13
#define JOYSTICK_BUTTON_XBOX360_DPAD_UP 11
#define JOYSTICK_BUTTON_XBOX360_DPAD_DOWN 15
#define JOYSTICK_BUTTON_XBOX360_X 2
#define JOYSTICK_BUTTON_XBOX360_B 1
#define JOYSTICK_BUTTON_XBOX360_Y 3
#define JOYSTICK_BUTTON_XBOX360_A 0
#define JOYSTICK_BUTTON_XBOX360_START 7
#define JOYSTICK_BUTTON_XBOX360_BACK 6
#define JOYSTICK_BUTTON_XBOX360_LEFT_BUMPER 4
#define JOYSTICK_BUTTON_XBOX360_RIGHT_BUMPER 5
#define JOYSTICK_BUTTON_XBOX360_LEFT_STICK 9
#define JOYSTICK_BUTTON_XBOX360_RIGHT_STICK 10
#define JOYSTICK_BUTTON_XBOX360_GUIDE 8
#endif

// define dualshock 4 joystick buttons
#if IBM
#define JOYSTICK_BUTTON_DS4_DPAD_LEFT 20
#define JOYSTICK_BUTTON_DS4_DPAD_RIGHT 16
#define JOYSTICK_BUTTON_DS4_DPAD_UP 14
#define JOYSTICK_BUTTON_DS4_DPAD_DOWN 18
#define JOYSTICK_BUTTON_DS4_DPAD_LEFT_UP 21
#define JOYSTICK_BUTTON_DS4_DPAD_LEFT_DOWN 19
#define JOYSTICK_BUTTON_DS4_DPAD_RIGHT_UP 15
#define JOYSTICK_BUTTON_DS4_DPAD_RIGHT_DOWN 17
#define JOYSTICK_BUTTON_DS4_SQUARE 0
#define JOYSTICK_BUTTON_DS4_CIRCLE 2
#define JOYSTICK_BUTTON_DS4_TRIANGLE 3
#define JOYSTICK_BUTTON_DS4_CROSS 1
#define JOYSTICK_BUTTON_DS4_OPTIONS 9
#define JOYSTICK_BUTTON_DS4_PS 12
#define JOYSTICK_BUTTON_DS4_SHARE 8
#define JOYSTICK_BUTTON_DS4_L1 4
#define JOYSTICK_BUTTON_DS4_R1 5
#define JOYSTICK_BUTTON_DS4_L2 6
#define JOYSTICK_BUTTON_DS4_R2 7
#define JOYSTICK_BUTTON_DS4_L3 10
#define JOYSTICK_BUTTON_DS4_R3 11
#elif APL
#define JOYSTICK_BUTTON_DS4_DPAD_LEFT -1
#define JOYSTICK_BUTTON_DS4_DPAD_RIGHT -1
#define JOYSTICK_BUTTON_DS4_DPAD_UP -1
#define JOYSTICK_BUTTON_DS4_DPAD_DOWN -1
#define JOYSTICK_BUTTON_DS4_DPAD_LEFT_UP -1
#define JOYSTICK_BUTTON_DS4_DPAD_LEFT_DOWN -1
#define JOYSTICK_BUTTON_DS4_DPAD_RIGHT_UP -1
#define JOYSTICK_BUTTON_DS4_DPAD_RIGHT_DOWN -1
#define JOYSTICK_BUTTON_DS4_SQUARE -1
#define JOYSTICK_BUTTON_DS4_CIRCLE -1
#define JOYSTICK_BUTTON_DS4_TRIANGLE -1
#define JOYSTICK_BUTTON_DS4_CROSS -1
#define JOYSTICK_BUTTON_DS4_OPTIONS -1
#define JOYSTICK_BUTTON_DS4_PS -1
#define JOYSTICK_BUTTON_DS4_SHARE -1
#define JOYSTICK_BUTTON_DS4_L1 -1
#define JOYSTICK_BUTTON_DS4_R1 -1
#define JOYSTICK_BUTTON_DS4_L2 -1
#define JOYSTICK_BUTTON_DS4_R2 -1
#define JOYSTICK_BUTTON_DS4_L3 -1
#define JOYSTICK_BUTTON_DS4_R3 -1
#elif LIN
#define JOYSTICK_BUTTON_DS4_DPAD_LEFT 19
#define JOYSTICK_BUTTON_DS4_DPAD_RIGHT 15
#define JOYSTICK_BUTTON_DS4_DPAD_UP 13
#define JOYSTICK_BUTTON_DS4_DPAD_DOWN 17
#define JOYSTICK_BUTTON_DS4_DPAD_LEFT_UP 20
#define JOYSTICK_BUTTON_DS4_DPAD_LEFT_DOWN 18
#define JOYSTICK_BUTTON_DS4_DPAD_RIGHT_UP 14
#define JOYSTICK_BUTTON_DS4_DPAD_RIGHT_DOWN 16
#define JOYSTICK_BUTTON_DS4_SQUARE 3
#define JOYSTICK_BUTTON_DS4_CIRCLE 1
#define JOYSTICK_BUTTON_DS4_TRIANGLE 2
#define JOYSTICK_BUTTON_DS4_CROSS 0
#define JOYSTICK_BUTTON_DS4_OPTIONS 9
#define JOYSTICK_BUTTON_DS4_PS 10
#define JOYSTICK_BUTTON_DS4_SHARE 8
#define JOYSTICK_BUTTON_DS4_L1 4
#define JOYSTICK_BUTTON_DS4_R1 5
#define JOYSTICK_BUTTON_DS4_L2 6
#define JOYSTICK_BUTTON_DS4_R2 7
#define JOYSTICK_BUTTON_DS4_L3 11
#define JOYSTICK_BUTTON_DS4_R3 12
#endif

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

// define default nullzone
#define DEFAULT_NULLZONE 0.10f

// define default sensitivities
#define DEFAULT_PITCH_SENSITIVITY 1.0f
#define DEFAULT_ROLL_SENSITIVITY 1.0f
#define DEFAULT_HEADING_SENSITIVITY 1.0f

// define '.acf' file 'show cockpit object in: 2-d forward panel views' string
#define ACF_STRING_SHOW_COCKPIT_OBJECT_IN_2D_FORWARD_PANEL_VIEWS "P acf/_new_plot_XP3D_cock/0 1"

// define default B738 filename
#define DEFAULT_B738_ACF_FILENAME "b738.acf"

// define plugin signatures
#define DREAMFOIL_AS350_PLUGIN_SIGNATURE "DreamFoil.AS350"
#define DREAMFOIL_B407_PLUGIN_SIGNATURE "DreamFoil.B407"
#define HEAD_SHAKE_PLUGIN_SIGNATURE "com.simcoders.headshake"
#define QPAC_A320_PLUGIN_SIGNATURE "QPAC.airbus.fbw"
#define ROTORSIM_EC135_PLUGIN_SIGNATURE "rotorsim.ec135.management"
#define X_IVAP_PLUGIN_SIGNATURE "ivao.xivap"
#define X_XSQUAWKBOX_PLUGIN_SIGNATURE "vatsim.protodev.clients.xsquawkbox"

// define custom command names
#define CYCLE_RESET_VIEW_COMMAND NAME_LOWERCASE "/cycle_reset_view"
#define TOGGLE_ARM_SPEED_BRAKE_OR_TOGGLE_CARB_HEAT_COMMAND NAME_LOWERCASE "/toggle_arm_speed_brake_or_toggle_carb_heat"
#define TOGGLE_AUTOPILOT_OR_DISABLE_FLIGHT_DIRECTOR_COMMAND NAME_LOWERCASE "/toggle_autopilot_or_disable_flight_director"
#define VIEW_MODIFIER_COMMAND NAME_LOWERCASE "/view_modifier"
#define PROP_PITCH_THROTTLE_MODIFIER_COMMAND NAME_LOWERCASE "/prop_pitch_throttle_modifier"
#define MIXTURE_CONTROL_MODIFIER_COMMAND NAME_LOWERCASE "/mixture_control_modifier"
#define COWL_FLAP_MODIFIER_COMMAND NAME_LOWERCASE "/cowl_flap_modifier"
#define TRIM_MODIFIER_COMMAND NAME_LOWERCASE "/trim_modifier"
#define TRIM_RESET_COMMAND_NAME_LOWERCASE "/trim_reset"
#define TOGGLE_BETA_OR_TOGGLE_REVERSE_COMMAND NAME_LOWERCASE "/toggle_beta_or_toggle_reverse"
#define TOGGLE_MOUSE_POINTER_CONTROL_COMMAND NAME_LOWERCASE "/toggle_mouse_pointer_control"
#define PUSH_TO_TALK_COMMAND NAME_LOWERCASE "/push_to_talk"
#define TOGGLE_BRAKES_COMMAND NAME_LOWERCASE "/toggle_brakes"

// define auto-center view limit
#define AUTO_CENTER_VIEW_DISTANCE_LIMIT 0.03f
#define AUTO_CENTER_VIEW_ANGLE_LIMIT 1.5f

// define long press time
#define BUTTON_LONG_PRESS_TIME 1.0f

// define relative control multiplier
#define JOYSTICK_RELATIVE_CONTROL_EXPONENT 3.0

// define relative control multiplier
#define JOYSTICK_RELATIVE_CONTROL_MULTIPLIER 2.0f

// define joystick sensitivity values
#define JOYSTICK_VIEW_SENSITIVITY 225.0f
#define JOYSTICK_MOUSE_POINTER_SENSITIVITY 30.0f

// define indicator size
#define INDICATOR_LEVER_WIDTH 16
#define INDICATOR_LEVER_HEIGHT 120

// define dualshock 4 touchpad stuff
#define TOUCHPAD_MAX_DELTA 150
#define TOUCHPAD_CURSOR_SENSITIVITY 1.0f
#define TOUCHPAD_SCROLL_SENSITIVITY 0.1f

// fragment-shader code
#define FRAGMENT_SHADER "#version 130\n"\
                        "uniform float throttle;"\
                        "uniform float prop;"\
                        "uniform float mixture;"\
                        "uniform vec4 bounds;"\
                        "void main()"\
                        "{"\
                            "vec2 size = vec2(bounds.z - bounds.x, bounds.y - bounds.w);"\
                            "gl_FragColor = vec4(1.0, 1.0, 1.0, 0.1);"\
                            "if (round(gl_FragCoord.x) == round(bounds.x) || round(gl_FragCoord.x) == round(bounds.z) || round(gl_FragCoord.y) == round(bounds.y) || round(gl_FragCoord.y) == round(bounds.w) || round(mod(gl_FragCoord.y - bounds.w, (size.y / 5.0))) == 0)"\
                                "gl_FragColor = vec4(1.0, 1.0, 1.0, 0.5);"\
                            "else"\
                            "{"\
                                "float segments = 3.0;"\
                                "if (prop < -0.5)"\
                                    "segments -= 1.0;"\
                                "if (mixture < -0.5)"\
                                    "segments -= 1.0;"\
                                "float segmentWidth = size.x / segments;"\
                                "if (gl_FragCoord.x < bounds.z - (segments - 1.0) * segmentWidth && gl_FragCoord.y < ((size.y - 2.0) * throttle) + bounds.w + 1.0)"\
                                    "gl_FragColor = vec4(0.0, 0.0, 0.0, 0.5);"\
                                "else if (gl_FragCoord.x >= bounds.z - (segments - 1.0) * segmentWidth && (segments < 2.5 || gl_FragCoord.x < bounds.z - segmentWidth) && gl_FragCoord.y < ((size.y - 2.0) * prop) + bounds.w + 1.0)"\
                                    "gl_FragColor = vec4(0.0, 0.0, 1.0, 0.5);"\
                                "else if (gl_FragCoord.x >= bounds.z - segmentWidth && gl_FragCoord.y < ((size.y - 2.0) * mixture) + bounds.w + 1.0)"\
                                    "gl_FragColor = vec4(1.0, 0.0, 0.0, 0.5);"\
                            "}"\
                        "}"

// define controller types
typedef enum
{
    XBOX360,
    DS4
} ControllerType;

// define mode types
typedef enum
{
    DEFAULT,
    VIEW,
    SWITCH_VIEW,
    PROP,
    MIXTURE,
    COWL,
    TRIM,
    MOUSE
} Mode;

// define mouse buttons
typedef enum
{
    LEFT,
    RIGHT
} MouseButton;

// define configuration steps
typedef enum
{
    START,
    AXES,
    BUTTONS,
    ABORT,
    DONE
} ConfigurationStep;

// define xinput state structure
#if IBM
struct XInputState
{
    unsigned long dwPacketNumber;
    unsigned short up : 1, down : 1, left : 1, right : 1, start : 1, back : 1, l3 : 1, r3 : 1, lButton : 1, rButton : 1, guideButton : 1, unknown : 1, aButton : 1, bButton : 1, xButton : 1, yButton : 1;
    unsigned char lTrigger;
    unsigned char rTrigger;
    short lJoyY;
    short lJoyX;
    short rJoyY;
    short rJoyX;
};
#endif

// global internal variables
static int axisOffset = 0, buttonOffset = 0, switchTo3DCommandLook = 0, overrideControlCinemaVeriteFailed = 0, overrideHeadShakePluginFailed = 0, lastCinemaVerite = 0, showIndicators = 1, indicatorsRight = 0, indicatorsBottom = 0, numPropLevers = 0, numMixtureLevers = 0;
static float defaultHeadPositionX = FLT_MAX, defaultHeadPositionY = FLT_MAX, defaultHeadPositionZ = FLT_MAX;
static ControllerType controllerType = XBOX360;
static Mode mode = DEFAULT;
static ConfigurationStep configurationStep = START;
static GLuint program = 0, fragmentShader = 0;
static std::stack <int*> buttonAssignmentsStack;
static XPLMWindowID indicatorsWindow = NULL;
#if IBM
static HINSTANCE hGetProcIDDLL = NULL;
typedef int(__stdcall * pICFUNC) (int, XInputState&);
pICFUNC XInputGetStateEx = NULL;
static HANDLE hidDeviceThread = 0;
static int hidInitialized = 0;
static volatile int hidDeviceThreadRun = 1;
#elif LIN
static Display *display = NULL;
#endif

// global commandref variables
static XPLMCommandRef cycleResetViewCommand = NULL, toggleArmSpeedBrakeOrToggleCarbHeatCommand = NULL, toggleAutopilotOrDisableFlightDirectorCommand = NULL, viewModifierCommand = NULL, propPitchOrThrottleModifierCommand = NULL, mixtureControlModifierCommand = NULL, cowlFlapModifierCommand = NULL, trimModifierCommand = NULL, trimResetCommand = NULL, toggleBetaOrToggleReverseCommand = NULL, toggleMousePointerControlCommand = NULL, pushToTalkCommand = NULL, toggleBrakesCommand = NULL;

// global dataref variables
static XPLMDataRef acfCockpitTypeDataRef = NULL, acfPeXDataRef = NULL, acfPeYDataRef = NULL, acfPeZDataRef = NULL, acfRSCMingovPrpDataRef = NULL, acfRSCRedlinePrpDataRef = NULL, acfNumEnginesDataRef = NULL, acfHasBetaDataRef = NULL, acfSbrkEQDataRef = NULL, acfEnTypeDataRef = NULL, acfPropTypeDataRef = NULL, acfMinPitchDataRef = NULL, acfMaxPitchDataRef = NULL, ongroundAnyDataRef = NULL, groundspeedDataRef = NULL, cinemaVeriteDataRef = NULL, pilotsHeadPsiDataRef = NULL, pilotsHeadTheDataRef = NULL, viewTypeDataRef = NULL, hasJoystickDataRef = NULL, joystickPitchNullzoneDataRef = NULL, joystickRollNullzoneDataRef = NULL, joystickHeadingNullzoneDataRef = NULL, joystickPitchSensitivityDataRef = NULL, joystickRollSensitivityDataRef = NULL, joystickHeadingSensitivityDataRef = NULL, joystickAxisAssignmentsDataRef = NULL, joystickAxisReverseDataRef = NULL, joystickAxisValuesDataRef = NULL, joystickButtonAssignmentsDataRef = NULL, joystickButtonValuesDataRef = NULL, leftBrakeRatioDataRef = NULL, rightBrakeRatioDataRef = NULL, speedbrakeRatioDataRef = NULL, aileronTrimDataRef = NULL, elevatorTrimDataRef = NULL, rudderTrimDataRef = NULL, throttleRatioAllDataRef = NULL, propPitchDegDataRef = NULL, propRotationSpeedRadSecAllDataRef = NULL, mixtureRatioAllDataRef = NULL, carbHeatRatioDataRef = NULL, cowlFlapRatioDataRef = NULL, thrustReverserDeployRatioDataRef = NULL, overrideToeBrakesDataRef = NULL;

// global widget variables
static XPWidgetID settingsWidget = NULL, dualShock4ControllerRadioButton = NULL, xbox360ControllerRadioButton = NULL, configurationStatusCaption = NULL, startConfigurationtButton = NULL, showIndicatorsCheckbox = NULL;

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

// returns 1 if the current aircraft does have a 2D panel, otherwise 0 is returned - for non-parseable sub 1004 version '.acf' files 1 is returned
static int Has2DPanel(void)
{
    char fileName[256], path[512];
    XPLMGetNthAircraftModel(0, fileName, path);

    int has2DPanel = 1;

    // search the '.acf' file for a special string which indicates that the aircraft shows the 3D cockpit object in the 2D forward panel view
    FILE *file = fopen(path, "r");
    if(file != NULL)
    {
        char temp[512];
        while(fgets(temp, 512, file) != NULL)
        {
            if((strstr(temp, ACF_STRING_SHOW_COCKPIT_OBJECT_IN_2D_FORWARD_PANEL_VIEWS)) != NULL)
            {
                has2DPanel = 0;
                break;
            }
        }

        fclose(file);
    }

    return has2DPanel;
}

// converts an abstract button index to an actual index of the selected controller while respecting the selected button offset
static int ButtonIndex(int abstractButtonIndex)
{
    switch (controllerType)
    {
    case XBOX360:
        switch (abstractButtonIndex)
        {
        case JOYSTICK_BUTTON_ABSTRACT_DPAD_LEFT:
            return JOYSTICK_BUTTON_XBOX360_DPAD_LEFT + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_DPAD_RIGHT:
            return JOYSTICK_BUTTON_XBOX360_DPAD_RIGHT + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_DPAD_UP:
            return JOYSTICK_BUTTON_XBOX360_DPAD_UP + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_DPAD_DOWN:
            return JOYSTICK_BUTTON_XBOX360_DPAD_DOWN + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_FACE_LEFT:
            return JOYSTICK_BUTTON_XBOX360_X + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_FACE_RIGHT:
            return JOYSTICK_BUTTON_XBOX360_B + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_FACE_UP:
            return JOYSTICK_BUTTON_XBOX360_Y + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_FACE_DOWN:
            return JOYSTICK_BUTTON_XBOX360_A + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_CENTER_LEFT:
            return JOYSTICK_BUTTON_XBOX360_BACK + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_CENTER_RIGHT:
            return JOYSTICK_BUTTON_XBOX360_START + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_BUMPER_LEFT:
            return JOYSTICK_BUTTON_XBOX360_LEFT_BUMPER + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_BUMPER_RIGHT:
            return JOYSTICK_BUTTON_XBOX360_RIGHT_BUMPER + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_STICK_LEFT:
            return JOYSTICK_BUTTON_XBOX360_LEFT_STICK + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_STICK_RIGHT:
            return JOYSTICK_BUTTON_XBOX360_RIGHT_STICK + buttonOffset;
        default:
            return -1;
        }
        break;
    case DS4:
        switch (abstractButtonIndex)
        {
        case JOYSTICK_BUTTON_ABSTRACT_DPAD_LEFT:
            return JOYSTICK_BUTTON_DS4_DPAD_LEFT + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_DPAD_RIGHT:
            return JOYSTICK_BUTTON_DS4_DPAD_RIGHT + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_DPAD_UP:
            return JOYSTICK_BUTTON_DS4_DPAD_UP + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_DPAD_DOWN:
            return JOYSTICK_BUTTON_DS4_DPAD_DOWN + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_DPAD_LEFT_UP:
            return JOYSTICK_BUTTON_DS4_DPAD_LEFT_UP + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_DPAD_LEFT_DOWN:
            return JOYSTICK_BUTTON_DS4_DPAD_LEFT_DOWN + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_DPAD_RIGHT_UP:
            return JOYSTICK_BUTTON_DS4_DPAD_RIGHT_UP + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_DPAD_RIGHT_DOWN:
            return JOYSTICK_BUTTON_DS4_DPAD_RIGHT_DOWN + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_FACE_LEFT:
            return JOYSTICK_BUTTON_DS4_SQUARE + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_FACE_RIGHT:
            return JOYSTICK_BUTTON_DS4_CIRCLE + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_FACE_UP:
            return JOYSTICK_BUTTON_DS4_TRIANGLE + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_FACE_DOWN:
            return JOYSTICK_BUTTON_DS4_CROSS + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_CENTER_LEFT:
            return JOYSTICK_BUTTON_DS4_SHARE + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_CENTER_RIGHT:
            return JOYSTICK_BUTTON_DS4_OPTIONS + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_BUMPER_LEFT:
            return JOYSTICK_BUTTON_DS4_L1 + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_BUMPER_RIGHT:
            return JOYSTICK_BUTTON_DS4_R1 + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_STICK_LEFT:
            return JOYSTICK_BUTTON_DS4_L3 + buttonOffset;
        case JOYSTICK_BUTTON_ABSTRACT_STICK_RIGHT:
            return JOYSTICK_BUTTON_DS4_R3 + buttonOffset;
        default:
            return -1;
        }
        break;
    default:
        return -1;
    }
}

// command-handler that hadles the reset view command / view switching modifier command
static int ResetSwitchViewCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    if (inPhase == xplm_CommandBegin)
    {
        // reset view
        switch (XPLMGetDatai(viewTypeDataRef))
        {
        case VIEW_TYPE_FORWARDS_WITH_PANEL:
            XPLMCommandOnce(XPLMFindCommand("sim/view/3d_cockpit_cmnd_look"));
            XPLMCommandOnce(XPLMFindCommand("sim/view/forward_with_2d_panel"));
            break;

        case VIEW_TYPE_3D_COCKPIT_COMMAND_LOOK:
            XPLMCommandOnce(XPLMFindCommand("sim/view/forward_with_2d_panel"));
            XPLMCommandOnce(XPLMFindCommand("sim/view/3d_cockpit_cmnd_look"));
            break;

        case VIEW_TYPE_CHASE:
            XPLMCommandOnce(XPLMFindCommand("sim/view/circle"));
            XPLMCommandOnce(XPLMFindCommand("sim/view/chase"));
            break;
        }

        if (mode == DEFAULT)
        {
            mode = SWITCH_VIEW;

            // store the default button assignments
            PushButtonAssignments();

            // assign view controls to face buttons
            int joystickButtonAssignments[1600];
            XPLMGetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);

            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_LEFT)] = (std::size_t) XPLMFindCommand("sim/view/chase");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_RIGHT)] = (std::size_t) XPLMFindCommand("sim/view/forward_with_hud");
            int has2DPanel = Has2DPanel();
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_UP)] = (std::size_t) XPLMFindCommand(has2DPanel ? "sim/view/forward_with_2d_panel" : "sim/view/3d_cockpit_cmnd_look");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_DOWN)] = (std::size_t) XPLMFindCommand(has2DPanel ? "sim/view/3d_cockpit_cmnd_look" : "sim/view/forward_with_2d_panel");

            XPLMSetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);
        }
    }
    else if (inPhase == xplm_CommandEnd && mode == SWITCH_VIEW)
    {
        // restore the default button assignments
        PopButtonAssignments();

        mode = DEFAULT;
    }

    return 0;
}

// command-handler that handles the speedbrake toggle / arm command or the carb heat, if the plane has no speedbrake
static int ToggleArmSpeedBrakeOrToggleCarbHeatCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    // if a speedbrake exists this command controls it
    if (XPLMGetDatai(acfSbrkEQDataRef))
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

                beginTime = FLT_MAX;
            }
        }
        else if (inPhase == xplm_CommandEnd)
        {
            // toggle speedbrake
            if (XPLMGetElapsedTime() - beginTime < BUTTON_LONG_PRESS_TIME && beginTime != FLT_MAX)
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
                float* carbHeatRatio = (float*) malloc(acfNumEngines * sizeof(float));
                XPLMGetDatavf(carbHeatRatioDataRef, carbHeatRatio, 0, acfNumEngines);

                float newCarbHeatRatio = carbHeatRatio[0] <= 0.5f ? 1.0f : 0.0f;

                for (int i = 0; i < acfNumEngines; i++)
                    carbHeatRatio[i] = newCarbHeatRatio;

                XPLMSetDatavf(carbHeatRatioDataRef, carbHeatRatio, 0, acfNumEngines);
                free(carbHeatRatio);
            }
        }
    }

    return 0;
}

static int IsDefaultB738(void)
{
    char fileName[256], path[512];
    XPLMGetNthAircraftModel(0, fileName, path);

    return strcmp(fileName, DEFAULT_B738_ACF_FILENAME) == 0 ? 1 : 0;
}

// check if a plugin with a given signature is enabled
static int IsPluginEnabled(const char* pluginSignature)
{
    XPLMPluginID pluginId = XPLMFindPluginBySignature(pluginSignature);

    return XPLMIsPluginEnabled(pluginId);
}

// command-handler that handles the toggle autopilot / disable flight director command - depending on the aircraft custom commands are invoked instead of the default ones
static int ToggleAutopilotOrDisableFlightDirectorCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    static float beginTime = 0.0f;

    if (inPhase == xplm_CommandBegin)
        beginTime = XPLMGetElapsedTime();
    else if (inPhase == xplm_CommandContinue)
    {
        // disable flight director
        if (XPLMGetElapsedTime() - beginTime >= BUTTON_LONG_PRESS_TIME)
        {
            // custom handling for default B738
            XPLMCommandRef B738CMDAutopilotFlightDirToggle = XPLMFindCommand("laminar/B738/autopilot/flight_director_toggle");
            XPLMCommandRef B738CMDAutopilotFlightDirFoToggle = XPLMFindCommand("laminar/B738/autopilot/flight_director_fo_toggle");
            if (IsDefaultB738() && B738CMDAutopilotFlightDirToggle != NULL && B738CMDAutopilotFlightDirFoToggle != NULL)
            {
                XPLMCommandOnce(B738CMDAutopilotFlightDirToggle);
                XPLMCommandOnce(B738CMDAutopilotFlightDirFoToggle);
            }
            // custom handling for QPAC A320
            else if (IsPluginEnabled(QPAC_A320_PLUGIN_SIGNATURE))
                XPLMCommandOnce(XPLMFindCommand("sim/autopilot/servos_and_flight_dir_off"));
            // custom handling for RotorSim EC135
            else if (IsPluginEnabled(ROTORSIM_EC135_PLUGIN_SIGNATURE))
                XPLMCommandOnce(XPLMFindCommand("ec135/autopilot/apmd_dcpl"));
            // default handling
            else
                XPLMCommandOnce(XPLMFindCommand("sim/autopilot/servos_fdir_off"));

            beginTime = FLT_MAX;
        }
    }
    else if (inPhase == xplm_CommandEnd)
    {
        // toggle autopilot
        if (XPLMGetElapsedTime() - beginTime < BUTTON_LONG_PRESS_TIME && beginTime != FLT_MAX)
        {
            // custom handling for default B738
            XPLMCommandRef B738CMDAutopilotDisconectButton = XPLMFindCommand("laminar/B738/autopilot/disconnect_button");
            if (IsDefaultB738() && B738CMDAutopilotDisconectButton != NULL)
                XPLMCommandOnce(B738CMDAutopilotDisconectButton);
            // custom handling for QPAC A320
            else if (IsPluginEnabled(QPAC_A320_PLUGIN_SIGNATURE))
            {
                XPLMDataRef ap1EngageDataRef = XPLMFindDataRef("AirbusFBW/AP1Engage");
                XPLMDataRef ap2EngageDataRef = XPLMFindDataRef("AirbusFBW/AP2Engage");

                if (ap1EngageDataRef != NULL && ap2EngageDataRef != NULL)
                {
                    if (!XPLMGetDatai(ap1EngageDataRef) && !XPLMGetDatai(ap2EngageDataRef))
                        XPLMCommandOnce(XPLMFindCommand("airbus_qpac/ap1_push"));
                    else
                        XPLMCommandOnce(XPLMFindCommand("sim/autopilot/servos_and_flight_dir_off"));
                }
            }
            // custom handling for RotorSim EC135
            else if (IsPluginEnabled(ROTORSIM_EC135_PLUGIN_SIGNATURE))
                XPLMCommandOnce(XPLMFindCommand("ec135/autopilot/ap_on"));
            // default handling
            else
                XPLMCommandOnce(XPLMFindCommand("sim/autopilot/servos_toggle"));
        }

        beginTime = 0.0f;
    }

    return 0;
}

static int SetOverrideHeadShakePlugin(int overrideEnabled)
{
    if (IsPluginEnabled(HEAD_SHAKE_PLUGIN_SIGNATURE))
    {
        XPLMSetDatai(XPLMFindDataRef("simcoders/headshake/override"), overrideEnabled);

        return 0;
    }

    return 1;
}

// disables cinema verite and also overrides HeadShake
static void OverrideCameraControls(void)
{
    // disable cinema verite if it is enabled and store its status
    lastCinemaVerite = XPLMGetDatai(cinemaVeriteDataRef);
    if (lastCinemaVerite)
        XPLMSetDatai(cinemaVeriteDataRef, 0);

    // enalbe the camera override of HeadShake
    overrideHeadShakePluginFailed = SetOverrideHeadShakePlugin(1);
}

// restores cinema verite and removes the override of HeadShake
static void RestoreCameraControls(void)
{
    // restore cinema verite to its old status
    if (lastCinemaVerite)
        XPLMSetDatai(cinemaVeriteDataRef, 1);

    // disable the camera override of HeadhShake if we enabled it before
    if (!overrideHeadShakePluginFailed)
        SetOverrideHeadShakePlugin(0);
}

// converts an abstract axis index to an actual index of the selected controller while respecting the selected axis offset
static int AxisIndex(int abstractAxisIndex)
{
    switch (controllerType)
    {
    case XBOX360:
        switch (abstractAxisIndex)
        {
        case JOYSTICK_AXIS_ABSTRACT_LEFT_X:
            return JOYSTICK_AXIS_XBOX360_LEFT_X + axisOffset;
        case JOYSTICK_AXIS_ABSTRACT_LEFT_Y:
            return JOYSTICK_AXIS_XBOX360_LEFT_Y + axisOffset;
        case JOYSTICK_AXIS_ABSTRACT_RIGHT_X:
            return JOYSTICK_AXIS_XBOX360_RIGHT_X + axisOffset;
        case JOYSTICK_AXIS_ABSTRACT_RIGHT_Y:
            return JOYSTICK_AXIS_XBOX360_RIGHT_Y + axisOffset;
        default:
            return -1;
        }
        break;
    case DS4:
        switch (abstractAxisIndex)
        {
        case JOYSTICK_AXIS_ABSTRACT_LEFT_X:
            return JOYSTICK_AXIS_DS4_LEFT_X + axisOffset;
        case JOYSTICK_AXIS_ABSTRACT_LEFT_Y:
            return JOYSTICK_AXIS_DS4_LEFT_Y + axisOffset;
        case JOYSTICK_AXIS_ABSTRACT_RIGHT_X:
            return JOYSTICK_AXIS_DS4_RIGHT_X + axisOffset;
        case JOYSTICK_AXIS_ABSTRACT_RIGHT_Y:
            return JOYSTICK_AXIS_DS4_RIGHT_Y + axisOffset;
        default:
            return -1;
        }
        break;
    default:
        return -1;
    }
}

// command-handler that handles the view modifier command
static int ViewModifierCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    if (inPhase != xplm_CommandContinue)
    {
        int joystickAxisAssignments[100];
        XPLMGetDatavi(joystickAxisAssignmentsDataRef, joystickAxisAssignments, 0, 100);

        if (inPhase == xplm_CommandBegin && mode == DEFAULT)
        {
            mode = VIEW;

            // unassign the left joystick's axis
            joystickAxisAssignments[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] = AXIS_ASSIGNMENT_NONE;
            joystickAxisAssignments[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] = AXIS_ASSIGNMENT_NONE;

            // store the default button assignments
            PushButtonAssignments();

            // assign panel scrolling controls to the dpad
            int joystickButtonAssignments[1600];
            XPLMGetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);

            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_LEFT)] = (std::size_t) XPLMFindCommand("sim/general/left");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_RIGHT)] = (std::size_t) XPLMFindCommand("sim/general/right");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_UP)] = (std::size_t) XPLMFindCommand("sim/general/up");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_DOWN)] = (std::size_t) XPLMFindCommand("sim/general/down");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_LEFT)] = (std::size_t) XPLMFindCommand("sim/general/rot_left");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_RIGHT)] = (std::size_t) XPLMFindCommand("sim/general/rot_right");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_UP)] = (std::size_t) XPLMFindCommand("sim/general/forward");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_DOWN)] = (std::size_t) XPLMFindCommand("sim/general/backward");

            // assign push-to-talk controls
            if (controllerType == DS4)
                joystickButtonAssignments[JOYSTICK_BUTTON_DS4_L2 + buttonOffset] = (std::size_t) XPLMFindCommand(PUSH_TO_TALK_COMMAND);

            XPLMSetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);

            // temporarily gain exclusive camera controls
            OverrideCameraControls();
        }
        else if (inPhase == xplm_CommandEnd && mode == VIEW)
        {
            // auto-center 3D cockpit view if it is only the defined distance or angle off from the center anyways
            if (XPLMGetDatai(viewTypeDataRef) == VIEW_TYPE_3D_COCKPIT_COMMAND_LOOK && fabs(defaultHeadPositionX - XPLMGetDataf(acfPeXDataRef)) <= AUTO_CENTER_VIEW_DISTANCE_LIMIT && fabs(defaultHeadPositionY - XPLMGetDataf(acfPeYDataRef)) <= AUTO_CENTER_VIEW_DISTANCE_LIMIT && fabs(defaultHeadPositionZ - XPLMGetDataf(acfPeZDataRef)) <= AUTO_CENTER_VIEW_DISTANCE_LIMIT)
            {
                XPLMSetDataf(acfPeXDataRef, defaultHeadPositionX);
                XPLMSetDataf(acfPeYDataRef, defaultHeadPositionY);
                XPLMSetDataf(acfPeZDataRef, defaultHeadPositionZ);

                float pilotsHeadPsi = XPLMGetDataf(pilotsHeadPsiDataRef);

                if ((pilotsHeadPsi >= 360.0f - AUTO_CENTER_VIEW_ANGLE_LIMIT || pilotsHeadPsi <= AUTO_CENTER_VIEW_ANGLE_LIMIT) && fabs(XPLMGetDataf(pilotsHeadTheDataRef)) <= AUTO_CENTER_VIEW_ANGLE_LIMIT)
                {
                    XPLMSetDataf(pilotsHeadPsiDataRef, 0.0f);
                    XPLMSetDataf(pilotsHeadTheDataRef, 0.0f);
                }
            }

            // assign the default controls to the left joystick's axis
            joystickAxisAssignments[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] = AXIS_ASSIGNMENT_YAW;
            joystickAxisAssignments[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] = AXIS_ASSIGNMENT_NONE;

            // restore the default button assignments
            PopButtonAssignments();

            // restore camera controls
            RestoreCameraControls();

            mode = DEFAULT;
        }

        XPLMSetDatavi(joystickAxisAssignmentsDataRef, joystickAxisAssignments, 0, 100);
    }

    return 0;
}

// command-handler that handles the prop pitch modifier command for fixed-wing airplanes or the throttle modifier command for helicopters
static int PropPitchOrThrottleModifierCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    if (inPhase == xplm_CommandBegin && mode == DEFAULT)
        mode = PROP;
    else if (inPhase == xplm_CommandEnd && mode == PROP)
        mode = DEFAULT;

    return 0;
}

// command-handler that handles the mixture control modifier command
static int MixtureControlModifierCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    if (inPhase == xplm_CommandBegin && mode == DEFAULT)
        mode = MIXTURE;
    else if (inPhase == xplm_CommandEnd && mode == MIXTURE)
        mode = DEFAULT;

    return 0;
}

// command-handler that handles the cowl flap modifier command
static int CowlFlapModifierCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    if (inPhase == xplm_CommandBegin && mode == DEFAULT)
        mode = COWL;
    else if (inPhase == xplm_CommandEnd && mode == COWL)
        mode = DEFAULT;

    return 0;
}

// command-handler that handles the trim modifier command
static int TrimModifierCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    // only apply the modifier if no other modifier is down which can alter any assignments
    if (inPhase == xplm_CommandBegin && mode == DEFAULT)
    {
        mode = TRIM;

        // store the default button assignments
        PushButtonAssignments();

        // assign trim controls to the buttons and dpad
        int joystickButtonAssignments[1600];
        XPLMGetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);

        // custom handling for DreamFoil AS350
        if (IsPluginEnabled(DREAMFOIL_AS350_PLUGIN_SIGNATURE))
        {
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_LEFT)] = (std::size_t) XPLMFindCommand("sim/flight_controls/rudder_trim_left");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_RIGHT)] = (std::size_t) XPLMFindCommand("sim/flight_controls/rudder_trim_right");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_CENTER_LEFT)] = (std::size_t) XPLMFindCommand(TRIM_RESET_COMMAND_NAME_LOWERCASE);

            XPLMCommandBegin(XPLMFindCommand("AS350/Trim/Force_Trim"));
        }
        // custom handling for DreamFoil B407
        else if (IsPluginEnabled(DREAMFOIL_B407_PLUGIN_SIGNATURE))
        {
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_LEFT)] = (std::size_t) XPLMFindCommand("sim/flight_controls/rudder_trim_left");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_RIGHT)] = (std::size_t) XPLMFindCommand("sim/flight_controls/rudder_trim_right");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_CENTER_LEFT)] = (std::size_t) XPLMFindCommand(TRIM_RESET_COMMAND_NAME_LOWERCASE);

            XPLMCommandBegin(XPLMFindCommand("B407/flight_controls/force_trim"));
        }
        // custom handling for RotorSim EC135
        else if (IsPluginEnabled(ROTORSIM_EC135_PLUGIN_SIGNATURE))
        {
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_LEFT)] = (std::size_t) XPLMFindCommand("ec135/autopilot/beep_left");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_RIGHT)] = (std::size_t) XPLMFindCommand("ec135/autopilot/beep_right");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_UP)] = (std::size_t) XPLMFindCommand("ec135/autopilot/beep_fwd");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_DOWN)] = (std::size_t) XPLMFindCommand("ec135/autopilot/beep_aft");
        }
        // default handling
        else
        {
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_LEFT)] = (std::size_t) XPLMFindCommand("sim/flight_controls/aileron_trim_left");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_RIGHT)] = (std::size_t) XPLMFindCommand("sim/flight_controls/aileron_trim_right");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_UP)] = (std::size_t) XPLMFindCommand("sim/flight_controls/pitch_trim_down");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_DOWN)] = (std::size_t) XPLMFindCommand("sim/flight_controls/pitch_trim_up");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_LEFT)] = (std::size_t) XPLMFindCommand("sim/flight_controls/rudder_trim_left");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_RIGHT)] = (std::size_t) XPLMFindCommand("sim/flight_controls/rudder_trim_right");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_CENTER_LEFT)] = (std::size_t) XPLMFindCommand(TRIM_RESET_COMMAND_NAME_LOWERCASE);
        }

        XPLMSetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);
    }
    else if (inPhase == xplm_CommandEnd && mode == TRIM)
    {
        // restore the default button assignments
        PopButtonAssignments();

        // custom handling for DreamFoil AS350
        if (IsPluginEnabled(DREAMFOIL_AS350_PLUGIN_SIGNATURE))
            XPLMCommandEnd(XPLMFindCommand("AS350/Trim/Force_Trim"));
        // custom handling for DreamFoil B407
        else if (IsPluginEnabled(DREAMFOIL_B407_PLUGIN_SIGNATURE))
            XPLMCommandEnd(XPLMFindCommand("B407/flight_controls/force_trim"));

        mode = DEFAULT;
    }

    return 0;
}

// command-handler that handles the trim reset command
static int TrimResetCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    if (inPhase == xplm_CommandBegin)
    {
        // custom handling for DreamFoil AS350
        if (IsPluginEnabled(DREAMFOIL_AS350_PLUGIN_SIGNATURE))
        {
            XPLMCommandEnd(XPLMFindCommand("AS350/Trim/Force_Trim"));
            XPLMCommandOnce(XPLMFindCommand("AS350/Trim/Trim_Release"));
        }
        // custom handling for DreamFoil B407
        else if (IsPluginEnabled(DREAMFOIL_B407_PLUGIN_SIGNATURE))
        {
            XPLMCommandEnd(XPLMFindCommand("B407/flight_controls/force_trim"));
            XPLMCommandOnce(XPLMFindCommand("B407/flight_controls/trim_release"));
        }
        else
        {
            XPLMSetDataf(aileronTrimDataRef, 0.0f);
            XPLMSetDataf(elevatorTrimDataRef, 0.0f);
        }

        XPLMSetDataf(rudderTrimDataRef, 0.0f);
    }

    return 0;
}

// command-handler that handles the toggle beta or toggle reverse command
static int ToggleBetaOrToggleReverseCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    static float beginTime = 0.0f;

    if (inPhase == xplm_CommandBegin)
        beginTime = XPLMGetElapsedTime();
    else if (inPhase == xplm_CommandContinue)
    {
        // toggle beta
        if (XPLMGetElapsedTime() - beginTime >= BUTTON_LONG_PRESS_TIME)
        {
            if (XPLMGetDatai(acfHasBetaDataRef))
                XPLMCommandOnce(XPLMFindCommand("sim/engines/beta_toggle"));

            beginTime = FLT_MAX;
        }
    }
    else if (inPhase == xplm_CommandEnd)
    {
        // toggle reverse
        if (XPLMGetElapsedTime() - beginTime < BUTTON_LONG_PRESS_TIME && beginTime != FLT_MAX)
            XPLMCommandOnce(XPLMFindCommand("sim/engines/thrust_reverse_toggle"));

        beginTime = 0.0f;
    }

    return 0;
}

// toggle mouse button state
static void ToggleMouseButton(MouseButton button, int down, void *display = NULL)
{
#if IBM
    static int lastLeftDown = 0, lastRightDown = 0;
    if ((button == LEFT && lastLeftDown != down) || (button == RIGHT && lastRightDown != down))
    {
        INPUT input[1];
        input[0].type = INPUT_MOUSE;
        if (button == LEFT)
            input[0].mi.dwFlags = down ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
        else
            input[0].mi.dwFlags = down ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;

        SendInput((UINT)1, input, sizeof(INPUT));

        if (button == LEFT)
            lastLeftDown = down;
        else
            lastRightDown = down;
    }
#elif APL
    CGEventType mouseType = 0;
    CGMouseButton mouseButton = 0;
    if (button == LEFT)
    {
        mouseType = (!down ? kCGEventLeftMouseUp : kCGEventLeftMouseDown);
        mouseButton = kCGMouseButtonLeft;
    }
    else
    {
        mouseType = (!down ? kCGEventRightMouseUp : kCGEventRightMouseDown);
        mouseButton = kCGMouseButtonRight;
    }

    int state = CGEventSourceButtonState(kCGEventSourceStateCombinedSessionState, mouseButton);

    if ((!down && state) || (down && !state))
    {
        CGEventRef getLocationEvent = CGEventCreate(NULL);
        CGPoint location = CGEventGetLocation(getLocationEvent);
        if (getLocationEvent != NULL)
            CFRelease(getLocationEvent);

        CGEventRef event = CGEventCreateMouseEvent(NULL, mouseType, location, mouseButton);
        CGEventPost(kCGHIDEventTap, event);
        if (event != NULL)
            CFRelease(event);
    }
#elif LIN
    if (display != NULL)
    {
        XTestFakeButtonEvent((Display*) display, button == LEFT ? 1 : 3, !down ? False : True, CurrentTime);
        XFlush((Display*) display);
    }
#endif
}

// command-handler that handles the toggle mouse pointer control command
static int ToggleMousePointerControlCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    if (inPhase == xplm_CommandBegin)
    {
        int joystickAxisAssignments[100];
        XPLMGetDatavi(joystickAxisAssignmentsDataRef, joystickAxisAssignments, 0, 100);

        if (mode == DEFAULT)
        {
            mode = MOUSE;

            // assign no controls to the left joystick's axis since it will control the mouse pointer
            joystickAxisAssignments[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] = AXIS_ASSIGNMENT_NONE;
            joystickAxisAssignments[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] = AXIS_ASSIGNMENT_NONE;

            // store the default button assignments
            PushButtonAssignments();

            // assign no commands to the down and right face and up and down dpad buttons since they will be used as mouse buttons and for scrolling
            int joystickButtonAssignments[1600];
            XPLMGetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);

            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_DOWN)] = (std::size_t) XPLMFindCommand("sim/none/none");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_RIGHT)] = (std::size_t) XPLMFindCommand("sim/none/none");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_UP)] = (std::size_t) XPLMFindCommand("sim/none/none");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_DOWN)] = (std::size_t) XPLMFindCommand("sim/none/none");

            XPLMSetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);

            // temporarily gain exclusive camera controls
            OverrideCameraControls();
        }
        else if (mode == MOUSE)
        {
            // release both mouse buttons if they were still pressed while the mouse pointer control mode was turned off
#if LIN
            ToggleMouseButton(LEFT, 0, display);
            ToggleMouseButton(RIGHT, 0, display);
#else
            ToggleMouseButton(LEFT, 0);
            ToggleMouseButton(RIGHT, 0);
#endif

            // assign the default controls to the left joystick's axis
            joystickAxisAssignments[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] = AXIS_ASSIGNMENT_YAW;
            joystickAxisAssignments[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] = AXIS_ASSIGNMENT_NONE;

            // restore the default button assignments
            PopButtonAssignments();

            // restore camera controls
            RestoreCameraControls();

            mode = DEFAULT;
        }

        XPLMSetDatavi(joystickAxisAssignmentsDataRef, joystickAxisAssignments, 0, 100);
    }

    return 0;
}

// scroll up or down
static void Scroll(int clicks, void *display = NULL)
{
    if (clicks == 0)
        return;

#if IBM
    INPUT input[1];
    input[0].type = INPUT_MOUSE;
    input[0].mi.mouseData = clicks * WHEEL_DELTA;
    input[0].mi.dwFlags = MOUSEEVENTF_WHEEL;
    SendInput((UINT)1, input, sizeof(INPUT));
#elif APL
    CGEventRef event = CGEventCreateScrollWheelEvent(NULL, kCGScrollEventUnitLine, 1, clicks);
    CGEventPost(kCGHIDEventTap, event);
    if (event != NULL)
        CFRelease(event);
#elif LIN
    if (display != NULL)
    {
        int button = clicks > 0 ? 4 : 5;

        for (int i = 0; i < abs(clicks); i++)
        {
            XTestFakeButtonEvent((Display*) display, button, True, CurrentTime);
            XTestFakeButtonEvent((Display*) display, button, False, CurrentTime);
        }

        XFlush((Display*) display);
    }
#endif
}

// command-handler that handles the push-to-talk command
static int PushToTalkCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    // only do push-to-talk if X-IvAp or XSquawkBox is enabled
    if (inPhase != xplm_CommandContinue && (IsPluginEnabled(X_IVAP_PLUGIN_SIGNATURE) || IsPluginEnabled(X_XSQUAWKBOX_PLUGIN_SIGNATURE)))
    {
        static int active = 0;
        if (inPhase == xplm_CommandBegin)
            active = 1;

        if (active)
        {
#if IBM
            INPUT input[1];
            input[0].type = INPUT_KEYBOARD;
            input[0].ki.wScan = (WORD) 24;
            DWORD flags;
            if (inPhase == xplm_CommandBegin)
                flags = KEYEVENTF_SCANCODE;
            else
                flags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE;
            input[0].ki.dwFlags = flags;

            SendInput((UINT) 1, input, sizeof(INPUT));
#elif APL
            static CGEventSourceRef eventSource = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
            CGEventRef event = CGEventCreateKeyboardEvent(eventSource, (CGKeyCode) kVK_ANSI_O, inPhase == xplm_CommandBegin);
            CGEventPost(kCGHIDEventTap, event);
            if (event != NULL)
                CFRelease(event);
#elif LIN
            if (display != NULL)
            {
                KeyCode keycode = XKeysymToKeycode(display, XK_O);
                XTestFakeKeyEvent(display, keycode, inPhase == xplm_CommandBegin, CurrentTime);
                XFlush(display);
            }
#endif
            if (inPhase == xplm_CommandEnd)
                active = 0;
        }
    }

    return 0;
}

// command-handler that handles the toggling of brakes
static int ToggleBrakesCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    if (inPhase == xplm_CommandBegin)
    {
        // custom handling for QPAC A320
        if (IsPluginEnabled(QPAC_A320_PLUGIN_SIGNATURE))
            XPLMCommandOnce(XPLMFindCommand("airbus_qpac/park_brake_toggle"));
        // default handling
        else
            XPLMCommandOnce(XPLMFindCommand("sim/flight_controls/brakes_toggle_max"));
    }

    return 0;
}

// check if the player's aircraft is a helicopter
static int IsHelicopter()
{
    if (XPLMGetDatai(acfCockpitTypeDataRef) == 5)
        return 1;

    int acfPropType[8];
    XPLMGetDatavi(acfPropTypeDataRef, acfPropType, 0, 8);
    for (int i = 0; i < XPLMGetDatai(acfNumEnginesDataRef); i++)
    {
        if (acfPropType[i] == 3)
            return 1;
    }

    return 0;
}

// check if the player's aircraft is a glider with speedbrakes
static int IsGliderWithSpeedbrakes()
{
    return XPLMGetDatai(acfNumEnginesDataRef) < 1 && XPLMGetDatai(acfSbrkEQDataRef);
}

// normalizes a value of a range [inMin, inMax] to a value of the range [outMin, outMax]
static float Normalize(float value, float inMin, float inMax, float outMin, float outMax)
{
    float newValue = 0.0f;
    float oldRange = inMax - inMin;

    if (oldRange == 0.0f)
        newValue = outMin;
    else
    {
        float newRange = outMax - outMin;
        newValue = (value - inMin) * newRange / (oldRange + outMin);
    }

    return newValue;
}

static float Exponentialize(float value, float inMin, float inMax, float outMin, float outMax)
{
    float n = Normalize(value, inMin, inMax, 0.0f, 1.0f);
    return Normalize(powf(n * 100.0f, JOYSTICK_RELATIVE_CONTROL_EXPONENT), 0.0f, powf(100.0f, JOYSTICK_RELATIVE_CONTROL_EXPONENT), outMin, outMax);
}

// move the mouse pointer by the specified amounts of pixels - the display parameter is only required for Linux
static void MoveMousePointer(int distX, int distY, void *display = NULL)
{
    if (distX == 0 && distY == 0)
        return;

#if IBM
    INPUT input[1];
    input[0].type = INPUT_MOUSE;
    input[0].mi.dx = (long) distX;
    input[0].mi.dy = (long) distY;
    input[0].mi.dwFlags = MOUSEEVENTF_MOVE;

    SendInput((UINT) 1, input, sizeof(INPUT));
#elif APL
    // get current mouse pointer location
    CGEventRef getLocationEvent = CGEventCreate(NULL);
    CGPoint oldLocation = CGEventGetLocation(getLocationEvent);
    if (getLocationEvent)
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
    if (moveMouseEvent != NULL)
        CFRelease(moveMouseEvent);
#elif LIN
    if (display != NULL)
    {
        XWarpPointer((Display*) display, None, None, 0, 0, 0, 0, distX, distY);
        XFlush((Display*) display);
    }
#endif
}

#if IBM
// hid device thread cleanup function
static void DeviceThreadCleanup(hid_device *handle, hid_device_info *dev)
{
    if (handle != NULL)
        hid_close(handle);

    if (dev != NULL)
        free(dev);

    hidDeviceThread = 0;
}

// hid device thread function
static void DeviceThread(void *argument)
{
    struct hid_device_info *dev = (hid_device_info*) argument;
    hid_device *handle = hid_open(dev->vendor_id, dev->product_id, dev->serial_number);
    if (handle == NULL)
    {
        DeviceThreadCleanup(handle, dev);
        return;
    }

    unsigned char data[40];
    static int prevTouchpadButtonDown = 0, prevX1 = 0, prevY1 = 0, prevDown1 = 0, prevDown2 = 0;
    while (hidDeviceThreadRun)
    {
        memset(data, 0, 40);
        if (hid_read(handle, data, 40) == -1)
        {
            DeviceThreadCleanup(handle, dev);
            return;
        }

        int touchpadButtonDown = (data[7] & 2) != 0;
        int down1 = data[35] >> 7 == 0;
        int down2 = data[39] >> 7 == 0;

        int x1 = data[36] + (data[37] & 0xF) * 255;
        int y1 = ((data[37] & 0xF0) >> 4) + data[38] * 16;
        int dX1 = x1 - prevX1;
        int dY1 = y1 - prevY1;

        if (touchpadButtonDown && !prevTouchpadButtonDown)
        {
            MouseButton button = down2 ? RIGHT : LEFT;
            ToggleMouseButton(button, 1);
        }
        else if (!touchpadButtonDown && prevTouchpadButtonDown)
        {
            ToggleMouseButton(LEFT, 0);
            ToggleMouseButton(RIGHT, 0);
        }

        if (down1 && !prevDown1)
        {
            prevX1 = -1;
            prevY1 = -1;
        }

        int scrollClicks = 0;
        if (!prevDown2 || touchpadButtonDown)
        {
            int distX = 0, distY = 0;

            if (prevX1 > 0 && abs(dX1) < TOUCHPAD_MAX_DELTA)
                distX = (int) (dX1 * TOUCHPAD_CURSOR_SENSITIVITY);
            if (prevY1 > 0 && abs(dY1) < TOUCHPAD_MAX_DELTA)
                distY = (int) (dY1 * TOUCHPAD_CURSOR_SENSITIVITY);

            MoveMousePointer(distX, distY);
        }
        else if (prevY1 > 0 && abs(dY1) < TOUCHPAD_MAX_DELTA)
            scrollClicks = (int) (-dY1 * TOUCHPAD_SCROLL_SENSITIVITY);

        Scroll(scrollClicks);

        prevTouchpadButtonDown = touchpadButtonDown;
        prevDown1 = down1;
        prevDown2 = down2;
        prevX1 = x1;
        prevY1 = y1;
    }

    DeviceThreadCleanup(handle, dev);
    _endthread();
}
#endif

// updates all caption widgets and slider positions associated with settings variables
static void UpdateSettingsWidgets(void)
{
    XPSetWidgetProperty(xbox360ControllerRadioButton, xpProperty_ButtonState, (int) (controllerType == XBOX360));
    XPSetWidgetProperty(dualShock4ControllerRadioButton, xpProperty_ButtonState, (int) (controllerType == DS4));

    const char *configurationStatusString;
    switch (configurationStep)
    {
        case START:
            configurationStatusString = "Click 'Start Configuration' to configure X-Plane for the selected controller type.";
            break;
        case AXES:
            configurationStatusString = "Move the right stick of your controller up and down.";
            break;
        case BUTTONS:
            configurationStatusString = "Now press and release the 'X'-Button on your controller.";
            break;
        case DONE:
            configurationStatusString = "Success! Your controller is now fully configured.";
            break;
        case ABORT:
            configurationStatusString = "Configuration aborted!";
            break;
        default:
            break;
    }
    XPSetWidgetDescriptor(configurationStatusCaption, configurationStatusString);
    XPSetWidgetProperty(configurationStatusCaption, xpProperty_CaptionLit, configurationStep != START);
    
    XPSetWidgetDescriptor(startConfigurationtButton, configurationStep == AXES || configurationStep == BUTTONS ? "Abort Configuration" : "Start Configuration");

    XPSetWidgetProperty(showIndicatorsCheckbox, xpProperty_ButtonState, showIndicators);
}

// saves current settings to the config file
static void SaveSettings(void)
{
    std::fstream file;
    file.open(CONFIG_PATH, std::ios_base::out | std::ios_base::trunc);

    if (file.is_open())
    {
        file << "controllerType=" << controllerType << std::endl;
        file << "axisOffset=" << axisOffset << std::endl;
        file << "buttonOffset=" << buttonOffset << std::endl;
        file << "showIndicators=" << showIndicators << std::endl;
        file << "indicatorsRight=" << indicatorsRight << std::endl;
        file << "indicatorsBottom=" << indicatorsBottom << std::endl;

        file.close();
    }
}

// set the default axis and button assignments
static void SetDefaultAssignments(void)
{
    // only set default assignments if a joystick is found and if no modifier is down which can alter any assignments
    if (XPLMGetDatai(hasJoystickDataRef) && mode == DEFAULT)
    {
        // set default axis assignments
        int joystickAxisAssignments[100];
        XPLMGetDatavi(joystickAxisAssignmentsDataRef, joystickAxisAssignments, 0, 100);

        joystickAxisAssignments[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] = AXIS_ASSIGNMENT_YAW;
        joystickAxisAssignments[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] = AXIS_ASSIGNMENT_NONE;
        joystickAxisAssignments[AxisIndex(JOYSTICK_AXIS_ABSTRACT_RIGHT_X)] = AXIS_ASSIGNMENT_ROLL;
        joystickAxisAssignments[AxisIndex(JOYSTICK_AXIS_ABSTRACT_RIGHT_Y)] = AXIS_ASSIGNMENT_PITCH;
        if (controllerType == XBOX360)
        {
#if IBM
            joystickAxisAssignments[JOYSTICK_AXIS_XBOX360_TRIGGERS + axisOffset] = AXIS_ASSIGNMENT_NONE;
#else
            joystickAxisAssignments[JOYSTICK_AXIS_XBOX360_LEFT_TRIGGER + axisOffset] = AXIS_ASSIGNMENT_NONE;
            joystickAxisAssignments[JOYSTICK_AXIS_XBOX360_RIGHT_TRIGGER + axisOffset] = AXIS_ASSIGNMENT_NONE;
#endif
        }
        else if (controllerType == DS4)
        {
            joystickAxisAssignments[JOYSTICK_AXIS_DS4_L2 + axisOffset] = AXIS_ASSIGNMENT_NONE;
            joystickAxisAssignments[JOYSTICK_AXIS_DS4_R2 + axisOffset] = AXIS_ASSIGNMENT_NONE;
        }

        XPLMSetDatavi(joystickAxisAssignmentsDataRef, joystickAxisAssignments, 0, 100);

        // set default button assignments
        int joystickButtonAssignments[1600];
        XPLMGetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);

        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_LEFT)] = (std::size_t) XPLMFindCommand("sim/flight_controls/flaps_up");
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_RIGHT)] = (std::size_t) XPLMFindCommand("sim/flight_controls/flaps_down");
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_UP)] = (std::size_t) XPLMFindCommand(TOGGLE_ARM_SPEED_BRAKE_OR_TOGGLE_CARB_HEAT_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_DOWN)] = (std::size_t) XPLMFindCommand("sim/flight_controls/landing_gear_toggle");
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_LEFT_UP)] = (std::size_t) XPLMFindCommand("sim/none/none");
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_LEFT_DOWN)] = (std::size_t) XPLMFindCommand("sim/none/none");
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_RIGHT_UP)] = (std::size_t) XPLMFindCommand("sim/none/none");
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_RIGHT_DOWN)] = (std::size_t) XPLMFindCommand("sim/none/none");
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_LEFT)] = (std::size_t) XPLMFindCommand(CYCLE_RESET_VIEW_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_RIGHT)] = (std::size_t) XPLMFindCommand(MIXTURE_CONTROL_MODIFIER_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_UP)] = (std::size_t) XPLMFindCommand(PROP_PITCH_THROTTLE_MODIFIER_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_DOWN)] = (std::size_t) XPLMFindCommand(COWL_FLAP_MODIFIER_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_CENTER_LEFT)] = (std::size_t) XPLMFindCommand(TOGGLE_BETA_OR_TOGGLE_REVERSE_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_CENTER_RIGHT)] = (std::size_t) XPLMFindCommand(TOGGLE_AUTOPILOT_OR_DISABLE_FLIGHT_DIRECTOR_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_BUMPER_LEFT)] = (std::size_t) XPLMFindCommand(TRIM_MODIFIER_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_BUMPER_RIGHT)] = (std::size_t) XPLMFindCommand(VIEW_MODIFIER_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_STICK_LEFT)] = (std::size_t) XPLMFindCommand("sim/general/zoom_out");
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_STICK_RIGHT)] = (std::size_t) XPLMFindCommand("sim/general/zoom_in");
        switch (controllerType)
        {
#if !IBM
        case XBOX360:
            joystickButtonAssignments[JOYSTICK_BUTTON_XBOX360_GUIDE + buttonOffset] = (std::size_t) XPLMFindCommand(TOGGLE_MOUSE_POINTER_CONTROL_COMMAND);
            break;
#endif
        case DS4:
            joystickButtonAssignments[JOYSTICK_BUTTON_DS4_PS + buttonOffset] = (std::size_t) XPLMFindCommand("sim/none/none");
            joystickButtonAssignments[JOYSTICK_BUTTON_DS4_L2 + buttonOffset] = (std::size_t) XPLMFindCommand("sim/autopilot/control_wheel_steer");
            break;
        }

        XPLMSetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);

        // set default nullzone
        XPLMSetDataf(joystickPitchNullzoneDataRef, DEFAULT_NULLZONE);
        XPLMSetDataf(joystickRollNullzoneDataRef, DEFAULT_NULLZONE);
        XPLMSetDataf(joystickHeadingNullzoneDataRef, DEFAULT_NULLZONE);

        // set default sensitivity
        XPLMSetDataf(joystickPitchSensitivityDataRef, DEFAULT_PITCH_SENSITIVITY);
        XPLMSetDataf(joystickRollSensitivityDataRef, DEFAULT_ROLL_SENSITIVITY);
        XPLMSetDataf(joystickHeadingSensitivityDataRef, DEFAULT_HEADING_SENSITIVITY);
    }
}

// flightloop-callback that mainly handles the joystick axis among other minor stuff
static float FlightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void *inRefcon)
{
    // update the default head position when required
    if (defaultHeadPositionX == FLT_MAX || defaultHeadPositionY == FLT_MAX || defaultHeadPositionZ == FLT_MAX)
    {
        defaultHeadPositionX = XPLMGetDataf(acfPeXDataRef);
        defaultHeadPositionY = XPLMGetDataf(acfPeYDataRef);
        defaultHeadPositionZ = XPLMGetDataf(acfPeZDataRef);
    }

    int helicopter = IsHelicopter();

    int currentMouseX, currentMouseY;
    XPLMGetMouseLocation(&currentMouseX, &currentMouseY);

    // handle switch to 3D command look
    if (switchTo3DCommandLook)
    {
        XPLMCommandOnce(XPLMFindCommand("sim/view/3d_cockpit_cmnd_look"));
        switchTo3DCommandLook = 0;
    }

    if (XPLMGetDatai(hasJoystickDataRef))
    {
        float currentTime = XPLMGetElapsedTime();

#if IBM
        static int guideButtonDown = 0;

        if (controllerType == XBOX360 && XInputGetStateEx != NULL)
        {
            XInputState state;
            XInputGetStateEx(0, state);

            if (!guideButtonDown && state.guideButton)
            {
                guideButtonDown = 1;
                XPLMCommandBegin(toggleMousePointerControlCommand);
            }
            else if (guideButtonDown && !state.guideButton)
            {
                guideButtonDown = 0;
                XPLMCommandEnd(toggleMousePointerControlCommand);
            }
        }
        else if (controllerType == DS4)
        {
            static float lastEnumerationTime = 0.0f;
            if (hidDeviceThread == 0 && currentTime - lastEnumerationTime >= 5.0f)
            {
                lastEnumerationTime = currentTime;

                if (!hidInitialized && hid_init() != -1)
                    hidInitialized = 1;

                if (hidInitialized)
                {
                    struct hid_device_info *devs = hid_enumerate(/*0x54C*/0x0, 0x0);
                    struct hid_device_info *currentDev = devs;

                    while (currentDev != NULL)
                    {
                        if (currentDev->vendor_id == 0x54C && (currentDev->product_id == 0x5C4 || currentDev->product_id == 0x9CC || currentDev->product_id == 0xBA0))
                        {
                            struct hid_device_info *currentDevCopy = (hid_device_info*) calloc(1, sizeof hid_device_info);
                            currentDevCopy->vendor_id = currentDev->vendor_id;
                            currentDevCopy->product_id = currentDev->product_id;

                            HANDLE threadHandle = (HANDLE) _beginthread(DeviceThread, 0, currentDevCopy);
                            if (threadHandle > 0)
                            {
                                hidDeviceThread = threadHandle;
                                break;
                            }
                            else
                                hid_free_enumeration(currentDevCopy);
                        }

                        currentDev = currentDev->next;
                    }

                    hid_free_enumeration(devs);
                }
            }
        }
#endif

        float joystickAxisValues[100];
        XPLMGetDatavf(joystickAxisValuesDataRef, joystickAxisValues, 0, 100);

        int joystickButtonValues[1600];
        XPLMGetDatavi(joystickButtonValuesDataRef, joystickButtonValues, 0, 1600);

        static int potentialAxes[100] = { 0 };
        static int potentialButtons[1600] = { 0 };

        switch (configurationStep)
        {
            case AXES:
                // we go through all axes and mark the indices of the axes with values above 0.75, if we see a previously marked axis taking a value below -0.75 we can assume it is the axis the user is moving
                for (int i = 0; i < 100; i++)
                {
                    if (joystickAxisValues[i] > 0.75f)
                        potentialAxes[i] = 1;
                    else if (potentialAxes[i])
                    {
                        axisOffset = i - (controllerType == XBOX360 ? JOYSTICK_AXIS_XBOX360_RIGHT_Y : JOYSTICK_AXIS_DS4_RIGHT_Y);
memset(potentialAxes, 0, sizeof potentialAxes);
                        configurationStep = BUTTONS;
                        UpdateSettingsWidgets();
                        return -1.0f;
                    }
                }
                return -1.0f;
            case BUTTONS:
                // because some joysticks have buttons that are in a depressed state by default, we go through all buttons and mark the indices of the buttons that are not pressed, if we see a previously marked button getting pressed we can assume it is the button the user pressed
                for (int i = 0; i < 1600; i++)
                {
                    if (joystickButtonValues[i])
                        potentialButtons[i] = 1;
                    else if (potentialButtons[i])
                    {
                        buttonOffset = i - (XBOX360 ? JOYSTICK_BUTTON_XBOX360_X : JOYSTICK_BUTTON_DS4_CROSS);
                        SetDefaultAssignments();
                        SaveSettings();
                        memset(potentialButtons, 0, sizeof potentialButtons);
                        configurationStep = DONE;
                        UpdateSettingsWidgets();
                        return -1.0f;
                    }
                }
                return -1.0f;
            case ABORT:
                // we need to cleanup the arrays in case of an abort
                memset(potentialAxes, 0, sizeof potentialAxes);
                memset(potentialButtons, 0, sizeof potentialButtons);
                // we first update the window to display the aborted message
                UpdateSettingsWidgets();
                configurationStep = START;
                return -1.0f;
            default:
                break;
        }

        float sensitivityMultiplier = JOYSTICK_RELATIVE_CONTROL_MULTIPLIER * inElapsedSinceLastCall;

        float joystickPitchNullzone = XPLMGetDataf(joystickPitchNullzoneDataRef);

        static int joystickAxisLeftXCalibrated = 0;
        if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] > 0.0f)
            joystickAxisLeftXCalibrated = 1;

        // keep the value of the left joystick's y axis at 0.5 until a value higher/lower than 0.0/1.0 is read because axis can get initialized with a value of 0.0 or 1.0 instead of 0.5 if they haven't been moved yet - this can result in unexpected behaviour especially if the axis is used in relative mode
        static float leftJoystickMinYValue = 1.0f, leftJoystickMaxYValue = 0.0f;

        if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] < leftJoystickMinYValue)
            leftJoystickMinYValue = joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)];

        if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] > leftJoystickMaxYValue)
            leftJoystickMaxYValue = joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)];

        if (leftJoystickMinYValue == 1.0f || leftJoystickMaxYValue == 0.0f)
            joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] = 0.5f;

        if (joystickAxisLeftXCalibrated)
        {
            int acfNumEngines = XPLMGetDatai(acfNumEnginesDataRef);

            static int brakeMode = 0;

            // handle brakes
            float leftBrakeRatio = 0.0f, rightBrakeRatio = 0.0f;

            if (
#if IBM
                    (controllerType == XBOX360 && joystickAxisValues[JOYSTICK_AXIS_XBOX360_TRIGGERS + axisOffset] <= -0.75f) ||
#else
                    (controllerType == XBOX360 && joystickAxisValues[JOYSTICK_AXIS_XBOX360_RIGHT_TRIGGER + axisOffset] >= 0.5f) ||
#endif
                    (controllerType == DS4 && joystickAxisValues[JOYSTICK_AXIS_DS4_R2 + axisOffset] >= 0.5f))
            {
                if (!brakeMode && mode == DEFAULT)
                {
                    PushButtonAssignments();

                    int joystickButtonAssignments[1600];
                    XPLMGetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);

                    joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_DOWN)] = (std::size_t) XPLMFindCommand(TOGGLE_BRAKES_COMMAND);

                    XPLMSetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);

                    brakeMode = 1;
                }

                switch (controllerType)
                {
                case XBOX360:
#if IBM
                    leftBrakeRatio = rightBrakeRatio = Normalize(joystickAxisValues[JOYSTICK_AXIS_XBOX360_TRIGGERS + axisOffset], -0.75f, -1.0f, 0.0f, 1.0f);
#else
                    leftBrakeRatio = rightBrakeRatio = Normalize(joystickAxisValues[JOYSTICK_AXIS_XBOX360_RIGHT_TRIGGER + axisOffset], 0.5f, 1.0f, 0.0f, 1.0f);
#endif
                    break;
                case DS4:
                    leftBrakeRatio = rightBrakeRatio = Normalize(joystickAxisValues[JOYSTICK_AXIS_DS4_R2 + axisOffset], 0.5f, 1.0f, 0.0f, 1.0f);
                    break;
                }

                if (mode == DEFAULT)
                {
                    // handle only left brake
                    if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] <= 0.3f)
                        rightBrakeRatio = 0.0f;
                    // handle only right brake
                    else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] >= 0.7f)
                        leftBrakeRatio = 0.0f;
                }
            }
            else if (brakeMode)
            {
                PopButtonAssignments();
                brakeMode = 0;
            }
            XPLMSetDataf(leftBrakeRatioDataRef, leftBrakeRatio);
            XPLMSetDataf(rightBrakeRatioDataRef, rightBrakeRatio);

            if (controllerType == XBOX360)
            {
#if IBM
                static int leftTriggerDown = 0, rightTriggerDown = 0;

                if (!leftTriggerDown && !rightTriggerDown && joystickAxisValues[JOYSTICK_AXIS_XBOX360_TRIGGERS + axisOffset] >= 0.85f)
                {
                    leftTriggerDown = 1;
                    if (mode == VIEW)
                        XPLMCommandBegin(pushToTalkCommand);
                    else
                        XPLMCommandBegin(XPLMFindCommand("sim/autopilot/control_wheel_steer"));
                }
                else if (!leftTriggerDown && !rightTriggerDown && joystickAxisValues[JOYSTICK_AXIS_XBOX360_TRIGGERS + axisOffset] <= 0.15f)
                    rightTriggerDown = 1;
                else if (leftTriggerDown && !rightTriggerDown && joystickAxisValues[JOYSTICK_AXIS_XBOX360_TRIGGERS + axisOffset] < 0.85f)
                {
                    leftTriggerDown = 0;
                    if (mode == VIEW)
                        XPLMCommandEnd(pushToTalkCommand);
                    else
                        XPLMCommandEnd(XPLMFindCommand("sim/autopilot/control_wheel_steer"));

                }
                else if (!leftTriggerDown && rightTriggerDown && joystickAxisValues[JOYSTICK_AXIS_XBOX360_TRIGGERS + axisOffset] > 0.15f)
                    rightTriggerDown = 0;
#else
                if (joystickAxisValues[JOYSTICK_AXIS_XBOX360_LEFT_TRIGGER + axisOffset] >= 0.5f)
                {
                    if (mode == VIEW)
                        XPLMCommandBegin(pushToTalkCommand);
                    else
                        XPLMCommandBegin(XPLMFindCommand("sim/autopilot/control_wheel_steer"));
                }
                else
                {
                    if (mode == VIEW)
                        XPLMCommandEnd(pushToTalkCommand);
                    else
                        XPLMCommandEnd(XPLMFindCommand("sim/autopilot/control_wheel_steer"));
                }
#endif
            }

            if (mode == VIEW)
            {
                XPLMCommandEnd(XPLMFindCommand("sim/autopilot/control_wheel_steer"));

                int viewType = XPLMGetDatai(viewTypeDataRef);

                if (viewType == VIEW_TYPE_3D_COCKPIT_COMMAND_LOOK)
                {
                    float deltaPsi = 0.0f, deltaThe = 0.0f;
                    float viewSensitivityMultiplier = JOYSTICK_VIEW_SENSITIVITY * inElapsedSinceLastCall;

                    // turn head to the left
                    if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] < 0.5f - joystickPitchNullzone)
                    {
                        // normalize range [0.5, 0.0] to [0.0, 1.0]
                        float d = Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)], 0.5f, 0.0f, 0.0f, 1.0f);

                        deltaPsi -= d * viewSensitivityMultiplier;
                    }
                    // turn head to the right
                    else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] > 0.5f + joystickPitchNullzone)
                    {
                        // normalize range [0.5, 1.0] to [0.0, 1.0]
                        float d = Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)], 0.5f, 1.0f, 0.0f, 1.0f);

                        deltaPsi += d * viewSensitivityMultiplier;
                    }

                    // turn head upward
                    if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] < 0.5f - joystickPitchNullzone)
                    {
                        // normalize range [0.5, 0.0] to [0.0, 1.0]
                        float d = Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 0.0f, 0.0f, 1.0f);

                        deltaThe += d * viewSensitivityMultiplier;
                    }
                    // turn head downward
                    else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] > 0.5f + joystickPitchNullzone)
                    {
                        // normalize range [0.5, 1.0] to [0.0, 1.0]
                        float d = Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 1.0f, 0.0f, 1.0f);

                        deltaThe -= d * viewSensitivityMultiplier;
                    }

                    float pilotsHeadPsi = XPLMGetDataf(pilotsHeadPsiDataRef);
                    float pilotsHeadThe = XPLMGetDataf(pilotsHeadTheDataRef);

                    float newPilotsHeadPsi = pilotsHeadPsi + deltaPsi;
                    float newPilotsHeadThe = pilotsHeadThe + deltaThe;

                    if (newPilotsHeadThe < -89.9f)
                        newPilotsHeadThe = -89.9f;
                    if (newPilotsHeadThe > 89.9f)
                        newPilotsHeadThe = 89.9f;

                    XPLMSetDataf(pilotsHeadPsiDataRef, newPilotsHeadPsi);
                    XPLMSetDataf(pilotsHeadTheDataRef, newPilotsHeadThe);
                }
                else if (viewType == VIEW_TYPE_FORWARDS_WITH_PANEL || viewType == VIEW_TYPE_CHASE)
                {
                    // move camera to the left
                    if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] < 0.5f - joystickPitchNullzone)
                    {
                        // normalize range [0.5, 0.0] to [0.0, 1.0]
                        float d = Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)], 0.5f, 0.0f, 0.0f, 1.0f);

                        // apply acceleration function (y = x^2) and round to integer
                        int n = (int) (powf(2.0f * d, 2.0f) + 0.5f);

                        // apply the command
                        for (int i = 0; i < n; i++)
                            XPLMCommandOnce(XPLMFindCommand("sim/general/left"));
                    }
                    // move camera to the right
                    else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] > 0.5f + joystickPitchNullzone)
                    {
                        // normalize range [0.5, 1.0] to [0.0, 1.0]
                        float d = Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)], 0.5f, 1.0f, 0.0f, 1.0f);

                        // apply acceleration function (y = x^2) and round to integer
                        int n = (int) (powf(2.0f * d, 2.0f) + 0.5f);

                        // apply the command
                        for (int i = 0; i < n; i++)
                            XPLMCommandOnce(XPLMFindCommand("sim/general/right"));
                    }

                    // move camera up
                    if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] < 0.5f - joystickPitchNullzone)
                    {
                        // normalize range [0.5, 0.0] to [0.0, 1.0]
                        float d = Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 0.0f, 0.0f, 1.0f);

                        // apply acceleration function (y = x^2) and round to integer
                        int n = (int) (powf(2.0f * d, 2.0f) + 0.5f);

                        // apply the command
                        for (int i = 0; i < n; i++)
                            XPLMCommandOnce(XPLMFindCommand("sim/general/up"));
                    }
                    // move camera down
                    else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] > 0.5f + joystickPitchNullzone)
                    {
                        // normalize range [0.5, 1.0] to [0.0, 1.0]
                        float d = Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 1.0f, 0.0f, 1.0f);

                        // apply acceleration function (y = x^2) and round to integer
                        int n = (int) (powf(2.0f * d, 2.0f) + 0.5f);

                        // apply the command
                        for (int i = 0; i < n; i++)
                            XPLMCommandOnce(XPLMFindCommand("sim/general/down"));
                    }
                }
            }
            else
            {
                XPLMCommandEnd(pushToTalkCommand);

                if (!helicopter && mode == PROP)
                {
                    float acfRSCMingovPrp = XPLMGetDataf(acfRSCMingovPrpDataRef);
                    float acfRSCRedlinePrp = XPLMGetDataf(acfRSCRedlinePrpDataRef);
                    float propRotationSpeedRadSecAll = XPLMGetDataf(propRotationSpeedRadSecAllDataRef);

                    // increase prop pitch
                    if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] < 0.5f - joystickPitchNullzone)
                    {
                        // normalize range [0.5, 0.0] to [acfRSCMingovPrp, acfRSCRedlinePrp]
                        float d = sensitivityMultiplier * Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 0.0f, acfRSCMingovPrp, acfRSCRedlinePrp);

                        float newPropRotationSpeedRadSecAll = propRotationSpeedRadSecAll + d;

                        // ensure we don't set values larger than 1.0
                        XPLMSetDataf(propRotationSpeedRadSecAllDataRef, newPropRotationSpeedRadSecAll < acfRSCRedlinePrp ? newPropRotationSpeedRadSecAll : acfRSCRedlinePrp);
                    }
                    // decrease prop pitch
                    else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] > 0.5f + joystickPitchNullzone)
                    {
                        // normalize range [0.5, 1.0] to [acfRSCMingovPrp, acfRSCRedlinePrp]
                        float d = sensitivityMultiplier * Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 1.0f, acfRSCMingovPrp, acfRSCRedlinePrp);

                        float newPropRotationSpeedRadSecAll = propRotationSpeedRadSecAll - d;

                        // ensure we don't set values smaller than 0.0
                        XPLMSetDataf(propRotationSpeedRadSecAllDataRef, newPropRotationSpeedRadSecAll > acfRSCMingovPrp ? newPropRotationSpeedRadSecAll : acfRSCMingovPrp);
                    }
                }
                else if (mode == MIXTURE)
                {
                    float mixtureRatioAll = XPLMGetDataf(mixtureRatioAllDataRef);

                    // increase mixture setting
                    if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] < 0.5f - joystickPitchNullzone)
                    {
                        // normalize range [0.5, 0.0] to [0.0, 1.0]
                        float d = sensitivityMultiplier * Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 0.0f, 0.0f, 1.0f);

                        float newMixtureRatioAll = mixtureRatioAll + d;

                        // ensure we don't set values larger than 1.0
                        XPLMSetDataf(mixtureRatioAllDataRef, newMixtureRatioAll < 1.0f ? newMixtureRatioAll : 1.0f);
                    }
                    // decrease mixture setting
                    else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] > 0.5f + joystickPitchNullzone)
                    {
                        // normalize range [0.5, 1.0] to [0.0, 1.0]
                        float d = sensitivityMultiplier * Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 1.0f, 0.0f, 1.0f);

                        float newMixtureRatioAll = mixtureRatioAll - d;

                        // ensure we don't set values smaller than 0.0
                        XPLMSetDataf(mixtureRatioAllDataRef, newMixtureRatioAll > 0.0f ? newMixtureRatioAll : 0.0f);
                    }
                }
                else if (mode == COWL)
                {
                    float* cowlFlapRatio = (float*) malloc(acfNumEngines * sizeof(float));
                    XPLMGetDatavf(cowlFlapRatioDataRef, cowlFlapRatio, 0, acfNumEngines);

                    // decrease cowl flap setting
                    if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] < 0.5f - joystickPitchNullzone)
                    {
                        // normalize range [0.5, 0.0] to [0.0, 1.0]
                        float d = sensitivityMultiplier * Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 0.0f, 0.0f, 1.0f);

                        // ensure we don't set values smaller than 0.0
                        for (int i = 0; i < acfNumEngines; i++)
                        {
                            float newCowlFlapRatio = cowlFlapRatio[i] - d;
                            cowlFlapRatio[i] = newCowlFlapRatio > 0.0f ? newCowlFlapRatio : 0.0f;
                        }
                    }
                    // increase cowl flap setting
                    else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] > 0.5f + joystickPitchNullzone)
                    {
                        // normalize range [0.5, 1.0] to [0.0, 1.0]
                        float d = sensitivityMultiplier * Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 1.0f, 0.0f, 1.0f);

                        // ensure we don't set values larger than 1.0
                        for (int i = 0; i < acfNumEngines; i++)
                        {
                            float newCowlFlapRatio = cowlFlapRatio[i] + d;
                            cowlFlapRatio[i] = newCowlFlapRatio < 1.0f ? newCowlFlapRatio : 1.0f;
                        }
                    }

                    XPLMSetDatavf(cowlFlapRatioDataRef, cowlFlapRatio, 0, acfNumEngines);
                    free(cowlFlapRatio);
                }
                else if (mode == MOUSE)
                {
                    int distX = 0, distY = 0;

                    // move mouse pointer left
                    if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] < 0.5f - joystickPitchNullzone)
                    {
                        // normalize range [0.5, 0.0] to [0.0, 1.0]
                        float d = Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)], 0.5f, 0.0f, 0.0f, 1.0f);

                        // apply acceleration function (y = x^2)
                        distX -= (int) (powf(d * JOYSTICK_MOUSE_POINTER_SENSITIVITY, 2.0f) * inElapsedSinceLastCall);
                    }
                    // move mouse pointer right
                    else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] > 0.5f + joystickPitchNullzone)
                    {
                        // normalize range [0.5, 1.0] to [0.0, 1.0]
                        float d = Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)], 0.5f, 1.0f, 0.0f, 1.0f);

                        // apply acceleration function (y = x^2)
                        distX += (int) (powf(d * JOYSTICK_MOUSE_POINTER_SENSITIVITY, 2.0f) * inElapsedSinceLastCall);
                    }

                    // move mouse pointer up
                    if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] < 0.5f - joystickPitchNullzone)
                    {
                        // normalize range [0.5, 0.0] to [0.0, 1.0]
                        float d = Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 0.0f, 0.0f, 1.0f);

                        // apply acceleration function (y = x^2)
                        distY -= (int) (powf(d * JOYSTICK_MOUSE_POINTER_SENSITIVITY, 2.0f) * inElapsedSinceLastCall);
                    }
                    // move mouse pointer down
                    else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] > 0.5f + joystickPitchNullzone)
                    {
                        // normalize range [0.5, 1.0] to [0.0, 1.0]
                        float d = Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 1.0f, 0.0f, 1.0f);

                        // apply acceleration function (y = x^2)
                        distY += (int) (powf(d * JOYSTICK_MOUSE_POINTER_SENSITIVITY, 2.0f) * inElapsedSinceLastCall);
                    }

                    // handle mouse pointer movement
#if LIN
                    MoveMousePointer(distX, distY, display);
#else
                    MoveMousePointer(distX, distY);
#endif

                    // handle left and right mouse button presses
#if LIN
                    ToggleMouseButton(LEFT, joystickButtonValues[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_DOWN)], display);
                    ToggleMouseButton(RIGHT, joystickButtonValues[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_RIGHT)], display);
#else
                    ToggleMouseButton(LEFT, joystickButtonValues[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_DOWN)]);
                    ToggleMouseButton(RIGHT, joystickButtonValues[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_RIGHT)]);
#endif
                    // handle scrolling
                    static float lastScrollTime = 0.0f;
                    if (currentTime - lastScrollTime >= 0.1f)
                    {
                        if (joystickButtonValues[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_UP)])
                        {
#if LIN
                            Scroll(1, display);
#else
                            Scroll(1);
#endif
                            lastScrollTime = currentTime;
                        }
                        if (joystickButtonValues[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_DOWN)])
                        {
#if LIN
                            Scroll(-1, display);
#else
                            Scroll(-1);
#endif
                            lastScrollTime = currentTime;
                        }
                    }
                }
                else
                {
                    if (helicopter && mode == DEFAULT)
                    {
                        float acfMinPitch[8];
                        XPLMGetDatavf(acfMinPitchDataRef, acfMinPitch, 0, 8);
                        float acfMaxPitch[8];
                        XPLMGetDatavf(acfMaxPitchDataRef, acfMaxPitch, 0, 8);
                        float* propPitchDeg = (float*) malloc(acfNumEngines * sizeof(float));
                        XPLMGetDatavf(propPitchDegDataRef, propPitchDeg, 0, acfNumEngines);

                        for (int i = 0; i < acfNumEngines; i++)
                        {
                            // increase prop pitch
                            if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] < 0.5f - joystickPitchNullzone)
                            {
                                // normalize range [0.5, 0.0] to [acfMinPitch, acfMaxPitch]
                                float d = sensitivityMultiplier * Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 0.0f, acfMinPitch[i], acfMaxPitch[i]);

                                float newPropPitchDeg = propPitchDeg[i] + d;

                                // ensure we don't set values larger than acfMaxPitch
                                propPitchDeg[i] = newPropPitchDeg < acfMaxPitch[i] ? newPropPitchDeg : acfMaxPitch[i];
                            }
                            // decrease prop pitch
                            else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] > 0.5f + joystickPitchNullzone)
                            {
                                // normalize range [0.5, 1.0] to [acfMinPitch, acfMaxPitch]
                                float d = sensitivityMultiplier * Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 1.0f, acfMinPitch[i], acfMaxPitch[i]);

                                float newPropPitchDeg = propPitchDeg[i] - d;

                                // ensure we don't set values smaller than acfMinPitch
                                propPitchDeg[i] = newPropPitchDeg > acfMinPitch[i] ? newPropPitchDeg : acfMinPitch[i];
                            }
                        }

                        XPLMSetDatavf(propPitchDegDataRef, propPitchDeg, 0, acfNumEngines);
                        free(propPitchDeg);
                    }
                    else
                    {
                        if (IsGliderWithSpeedbrakes())
                        {
                            float speedbrakeRatio = XPLMGetDataf(speedbrakeRatioDataRef);

                            // decrease speedbrake ratio
                            if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] < 0.5f - joystickPitchNullzone)
                            {
                                // de-arm speedbrake if armed
                                if (speedbrakeRatio == -0.5f)
                                    speedbrakeRatio = 0.0f;

                                // normalize range [0.5, 0.0] to [0.0, 1.0]
                                float d = sensitivityMultiplier * Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 0.0f, 0.0f, 1.0f);

                                float newSpeedbrakeRatio = speedbrakeRatio - d;

                                // ensure we don't set values smaller than 0.0
                                XPLMSetDataf(speedbrakeRatioDataRef, newSpeedbrakeRatio > 0.0f ? newSpeedbrakeRatio : 0.0f);
                            }
                            // increase speedbrake ratio
                            else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] > 0.5f + joystickPitchNullzone)
                            {
                                // de-arm speedbrake if armed
                                if (speedbrakeRatio == -0.5f)
                                    speedbrakeRatio = 0.0f;

                                // normalize range [0.5, 1.0] to [0.0, 1.0]
                                float d = sensitivityMultiplier * Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 1.0f, 0.0f, 1.0f);

                                float newSpeedbrakeRatio = speedbrakeRatio + d;

                                // ensure we don't set values larger than 1.0
                                XPLMSetDataf(speedbrakeRatioDataRef, newSpeedbrakeRatio < 1.0f ? newSpeedbrakeRatio : 1.0f);
                            }
                        }
                        else
                        {
                            float throttleRatioAll = XPLMGetDataf(throttleRatioAllDataRef);

                            float* thrustReverserDeployRatio = (float*) malloc(acfNumEngines * sizeof(float));

                            XPLMGetDatavf(thrustReverserDeployRatioDataRef, thrustReverserDeployRatio, 0, acfNumEngines);

                            float averageThrustReverserDeployRatio = 0.0f;
                            for (int i = 0; i < acfNumEngines; i++)
                                averageThrustReverserDeployRatio += thrustReverserDeployRatio[i];
                            averageThrustReverserDeployRatio /= acfNumEngines;

                            // increase throttle setting
                            if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] < 0.5f - joystickPitchNullzone)
                            {
                                // normalize range [0.5, 0.0] to [0.0, 1.0]
                                float d = sensitivityMultiplier * Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 0.0f, 0.0f, 1.0f);

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
                            else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] > 0.5f + joystickPitchNullzone)
                            {
                                // normalize range [0.5, 1.0] to [0.0, 1.0]
                                float d = sensitivityMultiplier * Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 1.0f, 0.0f, 1.0f);

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

                            free(thrustReverserDeployRatio);
                        }
                    }
                }
            }
        }
    }

    return -1.0f;
}

// removes the fragment-shader from video memory, if deleteProgram is set the shader-program is also removed
static void CleanupShader(int deleteProgram = 0)
{
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);

    if (deleteProgram)
        glDeleteProgram(program);
}

// function to load, compile and link the fragment-shader
static void InitShader(const char *fragmentShaderString)
{
    program = glCreateProgram();

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderString, 0);
    glCompileShader(fragmentShader);
    glAttachShader(program, fragmentShader);
    GLint isFragmentShaderCompiled = GL_FALSE;
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isFragmentShaderCompiled);
    if (isFragmentShaderCompiled == GL_FALSE)
    {
        GLsizei maxLength = 2048;
        GLchar *log = new GLchar[maxLength];
        glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, log);
        XPLMDebugString(NAME": The following error occured while compiling the fragment shader:\n");
        XPLMDebugString(log);
        delete[] log;

        CleanupShader(1);

        return;
    }

    glLinkProgram(program);
    GLint isProgramLinked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &isProgramLinked);
    if (isProgramLinked == GL_FALSE)
    {
        GLsizei maxLength = 2048;
        GLchar *log = new GLchar[maxLength];
        glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, log);
        XPLMDebugString(NAME": The following error occured while linking the shader program:\n");
        XPLMDebugString(log);
        delete[] log;

        CleanupShader(1);

        return;
    }

    CleanupShader(0);
}

// draws the content of the indicators window
static void DrawIndicatorsWindow(XPLMWindowID inWindowID, void *inRefcon)
{
    int acfNumEngines = XPLMGetDatai(acfNumEnginesDataRef);
    int gliderWithSpeedbrakes = IsGliderWithSpeedbrakes();
    int helicopter = IsHelicopter();

    XPLMSetGraphicsState(0, 0, 0, 0, 1, 0, 0);

    glUseProgram(program);

    float throttle = 0.0f;
    if (gliderWithSpeedbrakes)
        throttle = 1.0f - XPLMGetDataf(speedbrakeRatioDataRef);
    else
        throttle = XPLMGetDataf(throttleRatioAllDataRef);

    int throttleLocation = glGetUniformLocation(program, "throttle");
    glUniform1f(throttleLocation, throttle);

    int propLocation = glGetUniformLocation(program, "prop");
    float propRatio = 0.0f;
    if (helicopter)
    {
        float acfMinPitch = 0.0f;
        XPLMGetDatavf(acfMinPitchDataRef, &acfMinPitch, 0, 1);
        float acfMaxPitch = 0.0f;
        XPLMGetDatavf(acfMaxPitchDataRef, &acfMaxPitch, 0, 1);
        float propPitchDeg = 0.0f;
        XPLMGetDatavf(propPitchDegDataRef, &propPitchDeg, 0, 1);

        propRatio = Normalize(propPitchDeg, acfMinPitch, acfMaxPitch, 0.0f, 1.0f);
    }
    else
        propRatio = Normalize(XPLMGetDataf(propRotationSpeedRadSecAllDataRef), XPLMGetDataf(acfRSCMingovPrpDataRef), XPLMGetDataf(acfRSCRedlinePrpDataRef), 0.0f, 1.0f);

    glUniform1f(propLocation, numPropLevers < 1 ? -1.0f : propRatio);

    int mixtureLocation = glGetUniformLocation(program, "mixture");
    glUniform1f(mixtureLocation, numMixtureLevers < 1 ? -1.0f : XPLMGetDataf(mixtureRatioAllDataRef));

    int l = 0, t = 0, r = 0, b = 0;
    XPLMGetWindowGeometry(indicatorsWindow, &l, &t, &r, &b);
    int boundsLocation = glGetUniformLocation(program, "bounds");
    glUniform4f(boundsLocation, (GLfloat) l, (GLfloat) t, (GLfloat) r, (GLfloat) b);

    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f((GLfloat) l, (GLfloat) b);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f((GLfloat) l, (GLfloat) t);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f((GLfloat) r, (GLfloat) t);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f((GLfloat) r, (GLfloat) b);
    glEnd();

    glUseProgram(0);
}

static void HandleKey(XPLMWindowID inWindowID, char inKey, XPLMKeyFlags inFlags, char inVirtualKey, void *inRefcon, int losingFocus)
{
}

// modifies the supplied window bounds accordingly to ensure they are within the global screen bounds
static void FitGeometryWithinScreenBounds(int *left, int *top, int *right, int *bottom)
{
    int minLeft = 0, maxTop = 0, maxRight = 0, minBottom = 0;
    XPLMGetScreenBoundsGlobal(&minLeft, &maxTop, &maxRight, &minBottom);

    int leftOverflow = minLeft - *left;
    if (leftOverflow > 0)
    {
        *left = minLeft;
        *right += leftOverflow;
    }

    int topOverflow = maxTop - *top;
    if (topOverflow < 0)
    {
        *top = maxTop;
        *bottom += topOverflow;
    }

    int rightOverflow = maxRight - *right;
    if (rightOverflow < 0)
    {
        *right = maxRight;
        *left += rightOverflow;
    }

    int bottomOverflow = minBottom - *bottom;
    if (bottomOverflow > 0)
    {
        *bottom = minBottom;
        *top += bottomOverflow;
    }
}

// handles the dragging of the indicators window and storing its position
static int HandleMouseClick(XPLMWindowID inWindowID, int x, int y, XPLMMouseStatus inMouse, void *inRefcon)
{
    static int lastX = -1, lastY = -1;

    switch (inMouse) {
       case xplm_MouseDrag:
           if (lastX > 0 && lastY > 0)
           {
               int left = 0, top = 0;
               XPLMGetWindowGeometry(inWindowID, &left, &top, &indicatorsRight, &indicatorsBottom);

               int deltaX = x - lastX;
               int deltaY = y - lastY;

               left += deltaX;
               top += deltaY;
               indicatorsRight += deltaX;
               indicatorsBottom += deltaY;

               FitGeometryWithinScreenBounds(&left, &top, &indicatorsRight, &indicatorsBottom);
               XPLMSetWindowGeometry(inWindowID, left, top, indicatorsRight, indicatorsBottom);
           }
       case xplm_MouseDown:
           lastX = x;
           lastY = y;
           break;
       case xplm_MouseUp:
           lastX = lastY = -1;
           SaveSettings();
           break;
       default:
           break;
    }

    return 1;
}

static XPLMCursorStatus HandleCursor(XPLMWindowID inWindowID, int x, int y, void *inRefcon)
{
    return xplm_CursorArrow;
}

static int HandleMouseWheel(XPLMWindowID inWindowID, int x, int y, int wheel, int clicks, void *inRefcon)
{
    return 0;
}

static void UpdateIndicatorsWindow()
{
    if (indicatorsWindow)
    {
        XPLMDestroyWindow(indicatorsWindow);
        indicatorsWindow = NULL;
    }

    int acfNumEngines = XPLMGetDatai(acfNumEnginesDataRef);
    int gliderWithSpeedbrakes = IsGliderWithSpeedbrakes();

    if (!showIndicators || (acfNumEngines < 1 && !gliderWithSpeedbrakes))
        return;

    numPropLevers = 0;
    numMixtureLevers = 0;
    if (!gliderWithSpeedbrakes)
    {
        int helicopter = IsHelicopter();

        int acfPropType[8];
        XPLMGetDatavi(acfPropTypeDataRef, acfPropType, 0, 8);
        int acfEnType[8];
        XPLMGetDatavi(acfEnTypeDataRef, acfEnType, 0, 8);
        for (int i = 0; i < acfNumEngines; i++)
        {
            if (acfPropType[i] >= 1 && acfPropType[i] <= 3)
                numPropLevers++;

            if (acfEnType[i] <  2 || (acfEnType[i] == 2 && !helicopter) || acfEnType[i] == 8)
                numMixtureLevers++;
        }
    }

    int width = INDICATOR_LEVER_WIDTH;
    if (numPropLevers > 0)
        width += INDICATOR_LEVER_WIDTH;
    if (numMixtureLevers > 0)
        width += INDICATOR_LEVER_WIDTH;

    XPLMCreateWindow_t indicatorsWindowParameters;
    indicatorsWindowParameters.structSize = sizeof indicatorsWindowParameters;
    indicatorsWindowParameters.top = indicatorsBottom + INDICATOR_LEVER_HEIGHT;
    indicatorsWindowParameters.left = indicatorsRight - width;
    indicatorsWindowParameters.right = indicatorsRight;
    indicatorsWindowParameters.bottom = indicatorsBottom;
    FitGeometryWithinScreenBounds(&indicatorsWindowParameters.left, &indicatorsWindowParameters.top, &indicatorsWindowParameters.right, &indicatorsWindowParameters.bottom);
    indicatorsWindowParameters.visible = 1;
    indicatorsWindowParameters.drawWindowFunc = DrawIndicatorsWindow;
    indicatorsWindowParameters.handleKeyFunc = HandleKey;
    indicatorsWindowParameters.handleMouseClickFunc = HandleMouseClick;
    indicatorsWindowParameters.handleCursorFunc = HandleCursor;
    indicatorsWindowParameters.handleMouseWheelFunc = HandleMouseWheel;
    indicatorsWindowParameters.decorateAsFloatingWindow = xplm_WindowDecorationSelfDecorated;
    indicatorsWindowParameters.layer = xplm_WindowLayerFlightOverlay;
    indicatorsWindowParameters.handleRightClickFunc = HandleMouseClick;
    indicatorsWindow = XPLMCreateWindowEx(&indicatorsWindowParameters);
}

// stops the configuration process correctly
static void StopConfiguration(void)
{
    // if the user closes the widget while he is configuring a controller we need to set the aborted state to perform the cleanup
    if (configurationStep == AXES || configurationStep == BUTTONS)
        configurationStep = ABORT;
    else
        configurationStep = START;
}

// handles the settings widget
static int SettingsWidgetHandler(XPWidgetMessage inMessage, XPWidgetID inWidget, long inParam1, long inParam2)
{
    if (inMessage == xpMessage_CloseButtonPushed)
    {
        if (XPIsWidgetVisible(settingsWidget))
        {
            StopConfiguration();
            SaveSettings();
            XPHideWidget(settingsWidget);
        }
    }
    else if (inMessage == xpMsg_ButtonStateChanged)
    {
        if (inParam1 == (long) xbox360ControllerRadioButton)
        {
            if ((int) XPGetWidgetProperty(xbox360ControllerRadioButton, xpProperty_ButtonState, 0))
            {
                StopConfiguration();
                controllerType = XBOX360;
                UpdateSettingsWidgets();
            }
        }
        else if (inParam1 == (long) dualShock4ControllerRadioButton)
        {
            if ((int) XPGetWidgetProperty(dualShock4ControllerRadioButton, xpProperty_ButtonState, 0))
            {
                StopConfiguration();
                controllerType = DS4;
                UpdateSettingsWidgets();
            }
        }
        else if (inParam1 == (long) showIndicatorsCheckbox)
        {
            showIndicators = (int) XPGetWidgetProperty(showIndicatorsCheckbox, xpProperty_ButtonState, 0);
            UpdateIndicatorsWindow();
        }
    }
    else if (inMessage == xpMsg_PushButtonPressed)
    {
        if (inParam1 == (long) startConfigurationtButton)
        {
            if (configurationStep == AXES || configurationStep == BUTTONS)
                configurationStep = ABORT;
            else
                configurationStep = AXES;

            UpdateSettingsWidgets();
        }
    }

    return 0;
}

// handles the menu-entries
static void MenuHandlerCallback(void *inMenuRef, void *inItemRef)
{
    if (settingsWidget == NULL)
    {
        // create settings widget
        int x = 10, y = 0, w = 500, h = 375;
        XPLMGetScreenSize(NULL, &y);
        y -= 100;

        int x2 = x + w;
        int y2 = y - h;

        // widget window
        settingsWidget = XPCreateWidget(x, y, x2, y2, 1, NAME" Settings", 1, 0, xpWidgetClass_MainWindow);

        // add close box
        XPSetWidgetProperty(settingsWidget, xpProperty_MainWindowHasCloseBoxes, 1);

        // add controller type sub window
        XPCreateWidget(x + 10, y - 30, x2 - 10, y - 100 - 10, 1, "", 0, settingsWidget, xpWidgetClass_SubWindow);

        // add controller type caption
        XPCreateWidget(x + 10, y - 30, x2 - 20, y - 45, 1, "Controller Type:", 0, settingsWidget, xpWidgetClass_Caption);

        // add xbox 360 controller radio button
        xbox360ControllerRadioButton = XPCreateWidget(x + 20, y - 60, x + 350 + 20, y - 75, 1, "Xbox 360 Controller", 0, settingsWidget, xpWidgetClass_Button);
        XPSetWidgetProperty(xbox360ControllerRadioButton, xpProperty_ButtonType, xpRadioButton);
        XPSetWidgetProperty(xbox360ControllerRadioButton, xpProperty_ButtonBehavior, xpButtonBehaviorRadioButton);

        // add dualshock 4 controller radio button
        dualShock4ControllerRadioButton = XPCreateWidget(x + 20, y - 85, x + 350 + 20, y - 100, 1, "DualShock 4 Controller", 0, settingsWidget, xpWidgetClass_Button);
        XPSetWidgetProperty(dualShock4ControllerRadioButton, xpProperty_ButtonType, xpRadioButton);
        XPSetWidgetProperty(dualShock4ControllerRadioButton, xpProperty_ButtonBehavior, xpButtonBehaviorRadioButton);

        // add configuration sub window
        XPCreateWidget(x + 10, y - 125, x2 - 10, y - 215 - 10, 1, "", 0, settingsWidget, xpWidgetClass_SubWindow);

        // add assignments caption
        XPCreateWidget(x + 10, y - 125, x2 - 20, y - 140, 1, "Configuration:", 0, settingsWidget, xpWidgetClass_Caption);

        // add important notice caption
        XPCreateWidget(x + 30, y - 155, x2 - 20, y - 170, 1, "Important: First calibrate your controller in X-Plane's 'Joystick' settings screen.", 0, settingsWidget, xpWidgetClass_Caption);

        // add configuration status caption
        configurationStatusCaption = XPCreateWidget(x + 30, y - 175, x2 - 50, y - 190, 1, "", 0, settingsWidget, xpWidgetClass_Caption);

        // start configuration
        startConfigurationtButton = XPCreateWidget(x + 30, y - 200, x + 200 + 30, y - 215, 1, "", 0, settingsWidget, xpWidgetClass_Button);
        XPSetWidgetProperty(startConfigurationtButton, xpProperty_ButtonType, xpPushButton);

        // add indicators sub window
        XPCreateWidget(x + 10, y - 240, x2 - 10, y - 285 - 10, 1, "", 0, settingsWidget, xpWidgetClass_SubWindow);

        // add indicators caption
        XPCreateWidget(x + 10, y - 240, x2 - 20, y - 265, 1, "Indicators:", 0, settingsWidget, xpWidgetClass_Caption);

        // add show indicators radio button
        showIndicatorsCheckbox = XPCreateWidget(x + 20, y - 270, x + 300 + 20, y - 285, 1, "Show Indicators", 0, settingsWidget, xpWidgetClass_Button);
        XPSetWidgetProperty(showIndicatorsCheckbox, xpProperty_ButtonType, xpRadioButton);
        XPSetWidgetProperty(showIndicatorsCheckbox, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox);

        // add about sub window
        XPCreateWidget(x + 10, y - 310, x2 - 10, y - 355 - 10, 1, "", 0, settingsWidget, xpWidgetClass_SubWindow);

        // add about caption
        XPCreateWidget(x + 10, y - 310, x2 - 20, y - 325, 1, NAME " " VERSION, 0, settingsWidget, xpWidgetClass_Caption);
        XPCreateWidget(x + 10, y - 325, x2 - 20, y - 340, 1, "Thank you for using " NAME " by Matteo Hausner", 0, settingsWidget, xpWidgetClass_Caption);
        XPCreateWidget(x + 10, y - 340, x2 - 20, y - 355, 1, "Contact: matteo.hausner@gmail.com or bwravencl.de", 0, settingsWidget, xpWidgetClass_Caption);

        // init checkbox and slider positions
        UpdateSettingsWidgets();

        // register widget handler
        XPAddWidgetCallback(settingsWidget, (XPWidgetFunc_t) SettingsWidgetHandler);
    }
    else
    {
        // settings widget already created
        if (!XPIsWidgetVisible(settingsWidget))
            XPShowWidget(settingsWidget);
    }
}

// loads settings from the config file
static void LoadSettings(void)
{
    std::ifstream file;
    file.open(CONFIG_PATH);

    if (file.is_open())
    {
        std::string line;

        while (getline(file, line))
        {
            std::string val = line.substr(line.find("=") + 1);
            std::istringstream iss(val);

            if (line.find("controllerType") != std::string::npos)
            {
                int v = 0;
                iss >> v;
                controllerType = (ControllerType) v;
            }
            else if (line.find("axisOffset") != std::string::npos)
                iss >> axisOffset;
            else if (line.find("buttonOffset") != std::string::npos)
                iss >> buttonOffset;
            else if (line.find("showIndicators") != std::string::npos)
                iss >> showIndicators;
            else if (line.find("indicatorsRight") != std::string::npos)
                iss >> indicatorsRight;
            else if (line.find("indicatorsBottom") != std::string::npos)
                iss >> indicatorsBottom;
        }

        file.close();
    }
}

PLUGIN_API int XPluginStart(char *outName, char *outSig, char *outDesc)
{
#if IBM
    hGetProcIDDLL = LoadLibrary("XInput1_3.dll");
    FARPROC lpfnGetProcessID = GetProcAddress(HMODULE(hGetProcIDDLL), (LPCSTR) 100);
    XInputGetStateEx = pICFUNC(lpfnGetProcessID);
#endif

    // set plugin info
    strcpy(outName, NAME);
    strcpy(outSig, "de.bwravencl." NAME_LOWERCASE);
    strcpy(outDesc, NAME" allows flying X-Plane by gamepad!");

    // get paths in POSIX format
    XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);

#if IBM
    // init glew
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        XPLMDebugString(NAME": The following error occured while initializing GLEW:\n");
        XPLMDebugString((const char*) glewGetErrorString(err));
    }
#endif

    // prepare fragment-shader
    InitShader(FRAGMENT_SHADER);

    // obtain datarefs
    acfCockpitTypeDataRef = XPLMFindDataRef("sim/aircraft/view/acf_cockpit_type");
    acfPeXDataRef = XPLMFindDataRef("sim/aircraft/view/acf_peX");
    acfPeYDataRef = XPLMFindDataRef("sim/aircraft/view/acf_peY");
    acfPeZDataRef = XPLMFindDataRef("sim/aircraft/view/acf_peZ");
    acfRSCMingovPrpDataRef = XPLMFindDataRef("sim/aircraft/controls/acf_RSC_mingov_prp");
    acfRSCRedlinePrpDataRef = XPLMFindDataRef("sim/aircraft/controls/acf_RSC_redline_prp");
    acfNumEnginesDataRef = XPLMFindDataRef("sim/aircraft/engine/acf_num_engines");
    acfHasBetaDataRef = XPLMFindDataRef("sim/aircraft/overflow/acf_has_beta");
    acfSbrkEQDataRef = XPLMFindDataRef("sim/aircraft/parts/acf_sbrkEQ");
    acfEnTypeDataRef = XPLMFindDataRef("sim/aircraft/prop/acf_en_type");
    acfPropTypeDataRef = XPLMFindDataRef("sim/aircraft/prop/acf_prop_type");
    acfMinPitchDataRef = XPLMFindDataRef("sim/aircraft/prop/acf_min_pitch");
    acfMaxPitchDataRef = XPLMFindDataRef("sim/aircraft/prop/acf_max_pitch");
    ongroundAnyDataRef = XPLMFindDataRef("sim/flightmodel/failures/onground_any");
    groundspeedDataRef = XPLMFindDataRef("sim/flightmodel/position/groundspeed");
    cinemaVeriteDataRef = XPLMFindDataRef("sim/graphics/view/cinema_verite");
    pilotsHeadPsiDataRef = XPLMFindDataRef("sim/graphics/view/pilots_head_psi");
    pilotsHeadTheDataRef = XPLMFindDataRef("sim/graphics/view/pilots_head_the");
    viewTypeDataRef = XPLMFindDataRef("sim/graphics/view/view_type");
    hasJoystickDataRef = XPLMFindDataRef("sim/joystick/has_joystick");
    joystickPitchNullzoneDataRef = XPLMFindDataRef("sim/joystick/joystick_pitch_nullzone");
    joystickRollNullzoneDataRef = XPLMFindDataRef("sim/joystick/joystick_roll_nullzone");
    joystickHeadingNullzoneDataRef = XPLMFindDataRef("sim/joystick/joystick_heading_nullzone");
    joystickPitchSensitivityDataRef = XPLMFindDataRef("sim/joystick/joystick_pitch_sensitivity");
    joystickRollSensitivityDataRef = XPLMFindDataRef("sim/joystick/joystick_roll_sensitivity");
    joystickHeadingSensitivityDataRef = XPLMFindDataRef("sim/joystick/joystick_heading_sensitivity");
    joystickAxisAssignmentsDataRef = XPLMFindDataRef("sim/joystick/joystick_axis_assignments");
    joystickButtonAssignmentsDataRef = XPLMFindDataRef("sim/joystick/joystick_button_assignments");
    joystickAxisValuesDataRef = XPLMFindDataRef("sim/joystick/joystick_axis_values");
    joystickAxisReverseDataRef = XPLMFindDataRef("sim/joystick/joystick_axis_reverse");
    joystickButtonValuesDataRef = XPLMFindDataRef("sim/joystick/joystick_button_values");
    leftBrakeRatioDataRef = XPLMFindDataRef("sim/cockpit2/controls/left_brake_ratio");
    rightBrakeRatioDataRef = XPLMFindDataRef("sim/cockpit2/controls/right_brake_ratio");
    speedbrakeRatioDataRef = XPLMFindDataRef("sim/cockpit2/controls/speedbrake_ratio");
    aileronTrimDataRef = XPLMFindDataRef("sim/cockpit2/controls/aileron_trim");
    elevatorTrimDataRef = XPLMFindDataRef("sim/cockpit2/controls/elevator_trim");
    rudderTrimDataRef = XPLMFindDataRef("sim/cockpit2/controls/rudder_trim");
    throttleRatioAllDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/throttle_ratio_all");
    propPitchDegDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/prop_pitch_deg");
    propRotationSpeedRadSecAllDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/prop_rotation_speed_rad_sec_all");
    mixtureRatioAllDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/mixture_ratio_all");
    carbHeatRatioDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/carb_heat_ratio");
    cowlFlapRatioDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/cowl_flap_ratio");
    thrustReverserDeployRatioDataRef = XPLMFindDataRef("sim/flightmodel2/engines/thrust_reverser_deploy_ratio");
    overrideToeBrakesDataRef = XPLMFindDataRef("sim/operation/override/override_toe_brakes");

    // create custom commands
    cycleResetViewCommand = XPLMCreateCommand(CYCLE_RESET_VIEW_COMMAND, "Cycle / Reset View");
    toggleArmSpeedBrakeOrToggleCarbHeatCommand = XPLMCreateCommand(TOGGLE_ARM_SPEED_BRAKE_OR_TOGGLE_CARB_HEAT_COMMAND, "Toggle / Arm Speedbrake / Toggle Carb Heat");
    toggleAutopilotOrDisableFlightDirectorCommand = XPLMCreateCommand(TOGGLE_AUTOPILOT_OR_DISABLE_FLIGHT_DIRECTOR_COMMAND, "Toggle Autopilot / Disable Flight Director");
    viewModifierCommand = XPLMCreateCommand(VIEW_MODIFIER_COMMAND, "View Modifier");
    propPitchOrThrottleModifierCommand = XPLMCreateCommand(PROP_PITCH_THROTTLE_MODIFIER_COMMAND, "Prop Pitch / Throttle Modifier");
    mixtureControlModifierCommand = XPLMCreateCommand(MIXTURE_CONTROL_MODIFIER_COMMAND, "Mixture Control Modifier");
    cowlFlapModifierCommand = XPLMCreateCommand(COWL_FLAP_MODIFIER_COMMAND, "Cowl Flap Modifier");
    trimModifierCommand = XPLMCreateCommand(TRIM_MODIFIER_COMMAND, "Trim Modifier");
    trimResetCommand = XPLMCreateCommand(TRIM_RESET_COMMAND_NAME_LOWERCASE, "Trim Reset");
    toggleBetaOrToggleReverseCommand = XPLMCreateCommand(TOGGLE_BETA_OR_TOGGLE_REVERSE_COMMAND, "Toggle Beta / Toggle Reverse");
    toggleMousePointerControlCommand = XPLMCreateCommand(TOGGLE_MOUSE_POINTER_CONTROL_COMMAND, "Toggle Mouse Pointer Control");
    pushToTalkCommand = XPLMCreateCommand(PUSH_TO_TALK_COMMAND, "Push-To-Talk");
    toggleBrakesCommand = XPLMCreateCommand(TOGGLE_BRAKES_COMMAND, "Toggle Brakes");

    // register custom commands
    XPLMRegisterCommandHandler(cycleResetViewCommand, ResetSwitchViewCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(toggleArmSpeedBrakeOrToggleCarbHeatCommand, ToggleArmSpeedBrakeOrToggleCarbHeatCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(toggleAutopilotOrDisableFlightDirectorCommand, ToggleAutopilotOrDisableFlightDirectorCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(viewModifierCommand, ViewModifierCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(propPitchOrThrottleModifierCommand, PropPitchOrThrottleModifierCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(mixtureControlModifierCommand, MixtureControlModifierCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(cowlFlapModifierCommand, CowlFlapModifierCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(trimModifierCommand, TrimModifierCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(trimResetCommand, TrimResetCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(toggleBetaOrToggleReverseCommand, ToggleBetaOrToggleReverseCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(toggleMousePointerControlCommand, ToggleMousePointerControlCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(pushToTalkCommand, PushToTalkCommandHandler, 1, NULL);
    XPLMRegisterCommandHandler(toggleBrakesCommand, ToggleBrakesCommandHandler, 1, NULL);

    // initialize indicator default position
    int right = 0, bottom = 0;
    XPLMGetScreenBoundsGlobal(NULL, NULL, &right, &bottom);
    indicatorsRight = right;
    indicatorsBottom = bottom;

    // read and apply config file
    LoadSettings();

    // acquire toe brake control
    XPLMSetDatai(overrideToeBrakesDataRef, 1);

    // register flight loop callbacks
    XPLMRegisterFlightLoopCallback(FlightLoopCallback, -1, NULL);

    // initialize indicators window if necessary
    UpdateIndicatorsWindow();

    // create menu-entries
    int menuContainerIndex = XPLMAppendMenuItem(XPLMFindPluginsMenu(), NAME, 0, 0);
    XPLMMenuID menuID = XPLMCreateMenu(NAME, XPLMFindPluginsMenu(), menuContainerIndex, MenuHandlerCallback, NULL);
    XPLMAppendMenuItem(menuID, "Settings", NULL, 1);

#if LIN
    display = XOpenDisplay(NULL);
    if(display != NULL)
    {
        XEvent event;
        XQueryPointer(display, RootWindow(display, DefaultScreen(display)), &event.xbutton.root, &event.xbutton.window, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
    }
#endif

    return 1;
}

PLUGIN_API void XPluginStop(void)
{
    CleanupShader(1);

    // revert any remaining button assignments
    while (!buttonAssignmentsStack.empty())
        PopButtonAssignments();

    // unregister custom commands
    XPLMUnregisterCommandHandler(cycleResetViewCommand, ResetSwitchViewCommandHandler, 1, NULL);
    XPLMUnregisterCommandHandler(toggleArmSpeedBrakeOrToggleCarbHeatCommand, ToggleArmSpeedBrakeOrToggleCarbHeatCommandHandler, 1, NULL);
    XPLMUnregisterCommandHandler(toggleAutopilotOrDisableFlightDirectorCommand, ToggleAutopilotOrDisableFlightDirectorCommandHandler, 1, NULL);
    XPLMUnregisterCommandHandler(viewModifierCommand, ViewModifierCommandHandler, 1, NULL);
    XPLMUnregisterCommandHandler(propPitchOrThrottleModifierCommand, PropPitchOrThrottleModifierCommandHandler, 1, NULL);
    XPLMUnregisterCommandHandler(mixtureControlModifierCommand, MixtureControlModifierCommandHandler, 1, NULL);
    XPLMUnregisterCommandHandler(cowlFlapModifierCommand, CowlFlapModifierCommandHandler, 1, NULL);
    XPLMUnregisterCommandHandler(trimModifierCommand, TrimModifierCommandHandler, 1, NULL);
    XPLMUnregisterCommandHandler(toggleBetaOrToggleReverseCommand, ToggleBetaOrToggleReverseCommandHandler, 1, NULL);
    XPLMUnregisterCommandHandler(toggleMousePointerControlCommand, ToggleMousePointerControlCommandHandler, 1, NULL);
    XPLMUnregisterCommandHandler(pushToTalkCommand, PushToTalkCommandHandler, 1, NULL);
    XPLMUnregisterCommandHandler(toggleBrakesCommand, ToggleBrakesCommandHandler, 1, NULL);

    // register flight loop callbacks
    XPLMUnregisterFlightLoopCallback(FlightLoopCallback, NULL);

    // release toe brake control
    XPLMSetDatai(overrideToeBrakesDataRef, 0);

#if IBM
    if (hGetProcIDDLL != NULL)
        FreeLibrary(hGetProcIDDLL);


    hidDeviceThreadRun = 0;
    if (hidDeviceThread != 0)
        WaitForSingleObject(hidDeviceThread, INFINITE);

    hid_exit();
#elif LIN
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
    switch (inMessage)
    {
    case XPLM_MSG_PLANE_LOADED:
        // reset the default head position if a new plane is loaded so that it is updated during the next flight loop
        defaultHeadPositionX = FLT_MAX;
        defaultHeadPositionY = FLT_MAX;
        defaultHeadPositionZ = FLT_MAX;

        // reinitialize indicators window if necessary
        UpdateIndicatorsWindow();

    case XPLM_MSG_AIRPORT_LOADED:
        // schedule a switch to the 3D cockpit view during the next flight loop
        switchTo3DCommandLook = 0;
        if (!Has2DPanel())
            switchTo3DCommandLook = 1;
        break;
    }
}
