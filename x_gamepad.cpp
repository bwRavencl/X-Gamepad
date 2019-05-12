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

#ifndef VERSION
#define VERSION "UNDEFINED"
#endif

#if IBM
#include "glew.h"
#include <process.h>
#include <windows.h>
#elif APL
#include <pthread.h>
#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
#include <OpenGL/gl.h>
#endif

#if LIN
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>
#else
#include "hidapi.h"
#endif

#define NAME "X-Gamepad"
#define NAME_LOWERCASE "x_gamepad"

#if IBM
#define CONFIG_PATH ".\\Resources\\plugins\\" NAME_LOWERCASE "\\" NAME_LOWERCASE ".ini"
#else
#define CONFIG_PATH "./Resources/plugins/" NAME_LOWERCASE "/" NAME_LOWERCASE ".ini"
#endif

#define JOYSTICK_AXIS_ABSTRACT_LEFT_X 0
#define JOYSTICK_AXIS_ABSTRACT_LEFT_Y 1
#define JOYSTICK_AXIS_ABSTRACT_RIGHT_X 2
#define JOYSTICK_AXIS_ABSTRACT_RIGHT_Y 3

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

#define VIEW_TYPE_FORWARDS_WITH_PANEL 1000
#define VIEW_TYPE_CHASE 1017
#define VIEW_TYPE_FORWARDS_WITH_HUD 1023
#define VIEW_TYPE_3D_COCKPIT_COMMAND_LOOK 1026

#define AXIS_ASSIGNMENT_NONE 0
#define AXIS_ASSIGNMENT_PITCH 1
#define AXIS_ASSIGNMENT_ROLL 2
#define AXIS_ASSIGNMENT_YAW 3
#define AXIS_ASSIGNMENT_VIEW_LEFT_RIGHT 41
#define AXIS_ASSIGNMENT_VIEW_UP_DOWN 42

#define DEFAULT_NULLZONE 0.10f

#define DEFAULT_PITCH_SENSITIVITY 1.0f
#define DEFAULT_ROLL_SENSITIVITY 1.0f
#define DEFAULT_HEADING_SENSITIVITY 1.0f

#define ACF_STRING_SHOW_COCKPIT_OBJECT_IN_2D_FORWARD_PANEL_VIEWS "P acf/_new_plot_XP3D_cock/0 1"

#define DREAMFOIL_AS350_PLUGIN_SIGNATURE "DreamFoil.AS350"
#define DREAMFOIL_B407_PLUGIN_SIGNATURE "DreamFoil.B407"
#define HEAD_SHAKE_PLUGIN_SIGNATURE "com.simcoders.headshake"
#define ROTORSIM_EC135_PLUGIN_SIGNATURE "rotorsim.ec135.management"
#define X_IVAP_PLUGIN_SIGNATURE "ivao.xivap"
#define X_XSQUAWKBOX_PLUGIN_SIGNATURE "vatsim.protodev.clients.xsquawkbox"

#define CYCLE_RESET_VIEW_COMMAND NAME_LOWERCASE "/cycle_reset_view"
#define TOGGLE_ARM_SPEED_BRAKE_OR_TOGGLE_CARB_HEAT_COMMAND NAME_LOWERCASE "/toggle_arm_speed_brake_or_toggle_carb_heat"
#define CWS_OR_DISCONNECT_AUTOPILOT NAME_LOWERCASE "/cws_or_disconnect_autopilot"
#define LOOK_MODIFIER_COMMAND NAME_LOWERCASE "/look_modifier"
#define PROP_PITCH_THROTTLE_MODIFIER_COMMAND NAME_LOWERCASE "/prop_pitch_throttle_modifier"
#define MIXTURE_CONTROL_MODIFIER_COMMAND NAME_LOWERCASE "/mixture_control_modifier"
#define COWL_FLAP_MODIFIER_COMMAND NAME_LOWERCASE "/cowl_flap_modifier"
#define TRIM_MODIFIER_COMMAND NAME_LOWERCASE "/trim_modifier"
#define TRIM_RESET_COMMAND NAME_LOWERCASE "/trim_reset"
#define TOGGLE_REVERSE_COMMAND NAME_LOWERCASE "/toggle_reverse"
#define TOGGLE_MOUSE_OR_KEYBOARD_CONTROL_COMMAND NAME_LOWERCASE "/toggle_mouse_or_keyboard_control"
#define PUSH_TO_TALK_COMMAND NAME_LOWERCASE "/push_to_talk"
#define TOGGLE_LEFT_MOUSE_BUTTON_COMMAND NAME_LOWERCASE "/toggle_left_mouse_button"
#define TOGGLE_RIGHT_MOUSE_BUTTON_COMMAND NAME_LOWERCASE "/toggle_right_mouse_button"
#define SCROLL_UP_COMMAND NAME_LOWERCASE "/scroll_up"
#define SCROLL_DOWN_COMMAND NAME_LOWERCASE "/scroll_down"
#define KEYBOARD_SELECTOR_UP_COMMAND NAME_LOWERCASE "/keyboard_selector_up"
#define KEYBOARD_SELECTOR_DOWN_COMMAND NAME_LOWERCASE "/keyboard_selector_down"
#define KEYBOARD_SELECTOR_LEFT_COMMAND NAME_LOWERCASE "/keyboard_selector_left"
#define KEYBOARD_SELECTOR_RIGHT_COMMAND NAME_LOWERCASE "/keyboard_selector_right"
#define PRESS_KEYBOARD_KEY_COMMAND NAME_LOWERCASE "/press_keyboard_key"
#define LOCK_KEYBOARD_KEY_COMMAND NAME_LOWERCASE "/lock_keyboard_key"

#define AUTO_CENTER_VIEW_DISTANCE_LIMIT 0.03f
#define AUTO_CENTER_VIEW_ANGLE_LIMIT 1.5f

#define BUTTON_LONG_PRESS_TIME 1.0f

#define JOYSTICK_RELATIVE_CONTROL_EXPONENT 3.0
#define JOYSTICK_RELATIVE_CONTROL_MULTIPLIER 2.0f

#define JOYSTICK_LOOK_SENSITIVITY 225.0f
#define JOYSTICK_MOUSE_POINTER_SENSITIVITY 30.0f

#define INDICATOR_LEVER_WIDTH 20
#define INDICATOR_LEVER_HEIGHT 144

#define KEY_SELECTOR_MOVEMENT_MIN_ELAPSE_TIME 0.15f
#define KEY_BASE_SIZE 48
#define KEY_BORDER_WIDTH 1
#define KEY_REPEAT_INTERVAL 0.1f

#define THRUST_REVERSER_SETTING_ON_ENGAGEMENT -0.05f

#define TOUCHPAD_MAX_DELTA 150
#define TOUCHPAD_CURSOR_SENSITIVITY 1.0f
#define TOUCHPAD_SCROLL_SENSITIVITY 0.1f

#define INDICATORS_FRAGMENT_SHADER "#version 130\n"                                                                                                                                                                                                                                                                                                                    \
                                   ""                                                                                                                                                                                                                                                                                                                                  \
                                   "uniform ivec2 size;"                                                                                                                                                                                                                                                                                                               \
                                   "uniform float throttle;"                                                                                                                                                                                                                                                                                                           \
                                   "uniform float prop;"                                                                                                                                                                                                                                                                                                               \
                                   "uniform float mixture;"                                                                                                                                                                                                                                                                                                            \
                                   ""                                                                                                                                                                                                                                                                                                                                  \
                                   "void main()"                                                                                                                                                                                                                                                                                                                       \
                                   "{"                                                                                                                                                                                                                                                                                                                                 \
                                   "    gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);"                                                                                                                                                                                                                                                                                      \
                                   ""                                                                                                                                                                                                                                                                                                                                  \
                                   "    float borderWidthX = 1.0 / size.x;"                                                                                                                                                                                                                                                                                            \
                                   "    float borderWidthY = 1.0 / size.y;"                                                                                                                                                                                                                                                                                            \
                                   "    float halfBorderWidthY = borderWidthY / 2.0;"                                                                                                                                                                                                                                                                                  \
                                   ""                                                                                                                                                                                                                                                                                                                                  \
                                   "    if (gl_TexCoord[0].x > borderWidthX && gl_TexCoord[0].x < 1.0 - borderWidthX && gl_TexCoord[0].y > borderWidthY && gl_TexCoord[0].y < 1.0 - borderWidthY && distance(gl_TexCoord[0].y, 0.25) > halfBorderWidthY && distance(gl_TexCoord[0].y, 0.5) > halfBorderWidthY && distance(gl_TexCoord[0].y, 0.75) > halfBorderWidthY)" \
                                   "    {"                                                                                                                                                                                                                                                                                                                             \
                                   "        bool reverser = false;"                                                                                                                                                                                                                                                                                                    \
                                   "        bool beta = false;"                                                                                                                                                                                                                                                                                                        \
                                   "        float throttleNormalized = throttle;"                                                                                                                                                                                                                                                                                      \
                                   "        bool hasProp = true;"                                                                                                                                                                                                                                                                                                      \
                                   "        bool hasMixture = true;"                                                                                                                                                                                                                                                                                                   \
                                   "        int segments = 3;"                                                                                                                                                                                                                                                                                                         \
                                   ""                                                                                                                                                                                                                                                                                                                                  \
                                   "        if (throttleNormalized < -1.0)"                                                                                                                                                                                                                                                                                            \
                                   "        {"                                                                                                                                                                                                                                                                                                                         \
                                   "            reverser = true;"                                                                                                                                                                                                                                                                                                      \
                                   "            throttleNormalized += 2.0;"                                                                                                                                                                                                                                                                                            \
                                   "        }"                                                                                                                                                                                                                                                                                                                         \
                                   "        else if (throttleNormalized < 0.0)"                                                                                                                                                                                                                                                                                        \
                                   "        {"                                                                                                                                                                                                                                                                                                                         \
                                   "            beta = true;"                                                                                                                                                                                                                                                                                                          \
                                   "            throttleNormalized += 1.0;"                                                                                                                                                                                                                                                                                            \
                                   "        }"                                                                                                                                                                                                                                                                                                                         \
                                   "        if (prop < -2.5)"                                                                                                                                                                                                                                                                                                          \
                                   "        {"                                                                                                                                                                                                                                                                                                                         \
                                   "            hasProp = false;"                                                                                                                                                                                                                                                                                                      \
                                   "            segments--;"                                                                                                                                                                                                                                                                                                           \
                                   "        }"                                                                                                                                                                                                                                                                                                                         \
                                   "        if (mixture < -2.5)"                                                                                                                                                                                                                                                                                                       \
                                   "        {"                                                                                                                                                                                                                                                                                                                         \
                                   "            hasMixture = false;"                                                                                                                                                                                                                                                                                                   \
                                   "            segments--;"                                                                                                                                                                                                                                                                                                           \
                                   "        }"                                                                                                                                                                                                                                                                                                                         \
                                   ""                                                                                                                                                                                                                                                                                                                                  \
                                   "        float segmentWidth = 1.0 / segments;"                                                                                                                                                                                                                                                                                      \
                                   "        vec4 backgroundColor = vec4(0.5, 0.5, 0.5, 0.75);"                                                                                                                                                                                                                                                                         \
                                   ""                                                                                                                                                                                                                                                                                                                                  \
                                   "        if (gl_TexCoord[0].x < segmentWidth)"                                                                                                                                                                                                                                                                                      \
                                   "        {"                                                                                                                                                                                                                                                                                                                         \
                                   "            if (reverser)"                                                                                                                                                                                                                                                                                                         \
                                   "                gl_FragColor = gl_TexCoord[0].y > throttleNormalized ? vec4(1.0, 0.5, 0.0, 0.75) : backgroundColor;"                                                                                                                                                                                                               \
                                   "            else if (beta)"                                                                                                                                                                                                                                                                                                        \
                                   "                gl_FragColor = gl_TexCoord[0].y > throttleNormalized ? vec4(1.0, 1.0, 0.0, 0.75) : backgroundColor;"                                                                                                                                                                                                               \
                                   "            else"                                                                                                                                                                                                                                                                                                                  \
                                   "                gl_FragColor = gl_TexCoord[0].y < throttleNormalized ? vec4(0.0, 0.0, 0.0, 0.75) : backgroundColor;"                                                                                                                                                                                                               \
                                   "        }"                                                                                                                                                                                                                                                                                                                         \
                                   "        else if (hasProp && gl_TexCoord[0].x > segmentWidth + borderWidthX && gl_TexCoord[0].x < segmentWidth * 2)"                                                                                                                                                                                                                \
                                   "            gl_FragColor = gl_TexCoord[0].y < prop ? vec4(0.0, 0.0, 1.0, 0.75) : backgroundColor;"                                                                                                                                                                                                                                 \
                                   "        else if (hasMixture && gl_TexCoord[0].x > borderWidthX + segmentWidth * (hasProp ? 2 : 1))"                                                                                                                                                                                                                                \
                                   "            gl_FragColor = gl_TexCoord[0].y < mixture ? vec4(1.0, 0.0, 0.0, 0.75) : backgroundColor;"                                                                                                                                                                                                                              \
                                   "    }"                                                                                                                                                                                                                                                                                                                             \
                                   "}"

#define KEYBOARD_KEY_FRAGMENT_SHADER "#version 130\n"                                                                                                    \
                                     ""                                                                                                                  \
                                     "uniform int keyBaseSize;"                                                                                          \
                                     "uniform float aspect;"                                                                                             \
                                     "uniform bool selected;"                                                                                            \
                                     "uniform bool down;"                                                                                                \
                                     ""                                                                                                                  \
                                     "void main()"                                                                                                       \
                                     "{"                                                                                                                 \
                                     "    float borderWidth = 1.0 / keyBaseSize;"                                                                        \
                                     "    float maxX = 1.0 - borderWidth / aspect;"                                                                      \
                                     "    float minX = borderWidth / aspect;"                                                                            \
                                     "    float maxY = 1.0 - borderWidth;"                                                                               \
                                     "    float minY = borderWidth;"                                                                                     \
                                     ""                                                                                                                  \
                                     "    if (gl_TexCoord[0].x < maxX && gl_TexCoord[0].x > minX && gl_TexCoord[0].y < maxY && gl_TexCoord[0].y > minY)" \
                                     "        gl_FragColor = down ? vec4(0.0, 0.0, 0.0, 0.75) : vec4(0.15, 0.15, 0.15, 0.75);"                           \
                                     "    else"                                                                                                          \
                                     "        gl_FragColor = selected ? vec4(1.0, 0.0, 0.0, 1.0) : vec4(1.0, 1.0, 1.0, 1.0);"                            \
                                     "}"

#if IBM
#define KEY_CODE_ESCAPE 0x1
#define KEY_CODE_F1 0x3B
#define KEY_CODE_F2 0x3C
#define KEY_CODE_F3 0x3D
#define KEY_CODE_F4 0x3E
#define KEY_CODE_F5 0x3F
#define KEY_CODE_F6 0x40
#define KEY_CODE_F7 0x41
#define KEY_CODE_F8 0x42
#define KEY_CODE_F9 0x43
#define KEY_CODE_F10 0x44
#define KEY_CODE_F11 0x57
#define KEY_CODE_F12 0x58
#define KEY_CODE_SYSRQ 0xB7
#define KEY_CODE_SCROLL 0x46
#define KEY_CODE_PAUSE 0xC5
#define KEY_CODE_INSERT 0xD2
#define KEY_CODE_DELETE 0xD3
#define KEY_CODE_HOME 0xC7
#define KEY_CODE_END 0xCF
#define KEY_CODE_GRAVE 0x29
#define KEY_CODE_1 0x2
#define KEY_CODE_2 0x3
#define KEY_CODE_3 0x4
#define KEY_CODE_4 0x5
#define KEY_CODE_5 0x6
#define KEY_CODE_6 0x7
#define KEY_CODE_7 0x8
#define KEY_CODE_8 0x9
#define KEY_CODE_9 0xA
#define KEY_CODE_0 0xB
#define KEY_CODE_MINUS 0xC
#define KEY_CODE_EQUALS 0xD
#define KEY_CODE_BACK 0xE
#define KEY_CODE_NUMLOCK 0x45
#define KEY_CODE_DIVIDE 0xB5
#define KEY_CODE_MULTIPLY 0x37
#define KEY_CODE_SUBTRACT 0x4A
#define KEY_CODE_TAB 0xF
#define KEY_CODE_Q 0x10
#define KEY_CODE_W 0x11
#define KEY_CODE_E 0x12
#define KEY_CODE_R 0x13
#define KEY_CODE_T 0x14
#define KEY_CODE_Y 0x15
#define KEY_CODE_U 0x16
#define KEY_CODE_I 0x17
#define KEY_CODE_O 0x18
#define KEY_CODE_P 0x19
#define KEY_CODE_LBRACKET 0x1A
#define KEY_CODE_RBRACKET 0x1B
#define KEY_CODE_BACKSLASH 0x2B
#define KEY_CODE_NUMPAD7 0x47
#define KEY_CODE_NUMPAD8 0x48
#define KEY_CODE_NUMPAD9 0x49
#define KEY_CODE_ADD 0x4E
#define KEY_CODE_CAPITAL 0x3A
#define KEY_CODE_A 0x1E
#define KEY_CODE_S 0x1F
#define KEY_CODE_D 0x20
#define KEY_CODE_F 0x21
#define KEY_CODE_G 0x22
#define KEY_CODE_H 0x23
#define KEY_CODE_J 0x24
#define KEY_CODE_K 0x25
#define KEY_CODE_L 0x26
#define KEY_CODE_SEMICOLON 0x27
#define KEY_CODE_APOSTROPHE 0x28
#define KEY_CODE_RETURN 0x1C
#define KEY_CODE_NUMPAD4 0x4B
#define KEY_CODE_NUMPAD5 0x4C
#define KEY_CODE_NUMPAD6 0x4D
#define KEY_CODE_PRIOR 0xC9
#define KEY_CODE_LSHIFT 0x2A
#define KEY_CODE_Z 0x2C
#define KEY_CODE_X 0x2D
#define KEY_CODE_C 0x2E
#define KEY_CODE_V 0x2F
#define KEY_CODE_B 0x30
#define KEY_CODE_N 0x31
#define KEY_CODE_M 0x32
#define KEY_CODE_COMMA 0x33
#define KEY_CODE_PERIOD 0x34
#define KEY_CODE_SLASH 0x35
#define KEY_CODE_RSHIFT 0x36
#define KEY_CODE_NUMPAD1 0x4F
#define KEY_CODE_NUMPAD2 0x50
#define KEY_CODE_NUMPAD3 0x51
#define KEY_CODE_NEXT 0xD1
#define KEY_CODE_LCONTROl 0x1D
#define KEY_CODE_LWIN 0xDB
#define KEY_CODE_LMENU 0x38
#define KEY_CODE_SPACE 0x39
#define KEY_CODE_RMENU 0xB8
#define KEY_CODE_RWIN 0xDC
#define KEY_CODE_APPS 0xDD
#define KEY_CODE_RCONTROL 0x9D
#define KEY_CODE_UP 0xC8
#define KEY_CODE_DOWN 0xD0
#define KEY_CODE_LEFT 0xCB
#define KEY_CODE_RIGHT 0xCD
#define KEY_CODE_NUMPAD0 0x52
#define KEY_CODE_NUMPADCOMMA 0xB3
#define KEY_CODE_NUMPADENTER 0x9C
#elif APL
// TODO
#elif LIN
// TODO
#endif

typedef enum
{
    XBOX360,
    DS4
} ControllerType;

typedef enum
{
    DEFAULT,
    LOOK,
    SWITCH_VIEW,
    PROP,
    MIXTURE,
    COWL,
    TRIM,
    MOUSE,
    KEYBOARD
} Mode;

typedef enum
{
    LEFT,
    RIGHT
} MouseButton;

typedef MouseButton Direction;

typedef enum
{
    START,
    AXES,
    BUTTONS,
    ABORT,
    DONE
} ConfigurationStep;

typedef enum
{
    UP,
    DOWN,
    NEW_DOWN,
    NEW_UP
} KeyState;

typedef enum
{
    LEFT_END,
    RIGHT_END,
    BETWEEN
} KeyPosition;

struct KeyboardKey
{
    char *label;
    int keyCode;
    float aspect;
    int width;
    int labelOffsetX;
    KeyPosition position;
    KeyState state;
    float lastInputTime;
    KeyboardKey *above;
    KeyboardKey *below;
    KeyboardKey *left;
    KeyboardKey *right;
};

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

static int AxisIndex(int abstractAxisIndex);
static int ButtonIndex(int abstractButtonIndex);
#if !LIN
static void CleanupDeviceThread(hid_device *handle, hid_device_info *dev);
#endif
static void CleanupShader(GLuint program, GLuint fragmentShader, int deleteProgram);
static int CowlFlapModifierCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
static int CwsOrDisconnectAutopilotCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
#if IBM
static void DeviceThread(void *argument);
#elif APL
static void *DeviceThread(void *argument);
#endif
static void DrawIndicatorsWindow(XPLMWindowID inWindowID, void *inRefcon);
static void DrawKeyboardWindow(XPLMWindowID inWindowID, void *inRefcon);
static void EndKeyboardMode(void);
static float Exponentialize(float value, float inMin, float inMax, float outMin, float outMax);
static void FitGeometryWithinScreenBounds(int *left, int *top, int *right, int *bottom);
static float FlightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void *inRefcon);
inline static int FloatsEqual(float a, float b);
static int inline GetKeyboardWidth(void);
static XPLMDataRef GetThrottleRatioDataRef(void);
static XPLMCursorStatus HandleCursor(XPLMWindowID inWindowID, int x, int y, void *inRefcon);
static void HandleKey(XPLMWindowID inWindowID, char inKey, XPLMKeyFlags inFlags, char inVirtualKey, void *inRefcon, int losingFocus);
static void HandleKeyboardSelectorCommand(XPLMCommandPhase inPhase, KeyboardKey *newSelectedKey);
static int HandleMouseClick(XPLMWindowID inWindowID, int x, int y, XPLMMouseStatus inMouse, void *inRefcon);
static int HandleMouseWheel(XPLMWindowID inWindowID, int x, int y, int wheel, int clicks, void *inRefcon);
static void HandleScrollCommand(XPLMCommandPhase phase, int clicks);
static void HandleToggleMouseButtonCommand(XPLMCommandPhase phase, MouseButton button);
static int Has2DPanel(void);
static KeyboardKey InitKeyboardKey(const char *label, int keyCode, float aspect = 1.0f, KeyPosition position = BETWEEN);
static void InitShader(const char *fragmentShaderString, GLuint *program, GLuint *fragmentShader);
inline static int IsGliderWithSpeedbrakes(void);
static int IsHelicopter(void);
inline static int IsLockKey(KeyboardKey key);
static int IsPluginEnabled(const char *pluginSignature);
static int KeyboardSelectorDownCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
static int KeyboardSelectorLeftCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
static int KeyboardSelectorRightCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
static int KeyboardSelectorUpCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
static void LoadSettings(void);
static int LockKeyboardKeyCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
static int LookModifierCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
static void MakeInput(int keyCode, KeyState state);
static void MenuHandlerCallback(void *inMenuRef, void *inItemRef);
static int MixtureControlModifierCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
static void MoveMousePointer(int distX, int distY, void *display = NULL);
static float Normalize(float value, float inMin, float inMax, float outMin, float outMax);
static void OverrideCameraControls(void);
static void PopButtonAssignments(void);
static int PressKeyboardKeyCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
static int PropPitchOrThrottleModifierCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
static void PushButtonAssignments(void);
static int PushToTalkCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
static void ReleaseAllKeys(void);
static int ResetSwitchViewCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
static void RestoreCameraControls(void);
static void SaveSettings(void);
static void Scroll(int clicks, void *display = NULL);
static int ScrollDownCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
static int ScrollUpCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
static void SetDefaultAssignments(void);
static int SetOverrideHeadShakePlugin(int overrideEnabled);
static int SettingsWidgetHandler(XPWidgetMessage inMessage, XPWidgetID inWidget, long inParam1, long inParam2);
static void StopConfiguration(void);
inline static void SyncLockKeyState(KeyboardKey *key);
static int ToggleArmSpeedBrakeOrToggleCarbHeatCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
static void ToggleKeyboardControl(int vrEnabled = -1);
static void ToggleMode(Mode m, XPLMCommandPhase phase);
static void ToggleMouseButton(MouseButton button, int down, void *display = NULL);
static void ToggleMouseControl(void);
static int ToggleMouseOrKeyboardControlCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
static int ToggleLeftMouseButtonCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
static int ToggleReverseCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
static int ToggleRightMouseButtonCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
static int TrimModifierCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
static int TrimResetCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon);
static void UpdateIndicatorsWindow(int vrEnabled = -1);
static void UpdateSettingsWidgets(void);
inline static void WireKey(KeyboardKey *key, KeyboardKey *left, KeyboardKey *right, KeyboardKey *above, KeyboardKey *below);
static void WireKeys(void);

static KeyboardKey escapeKey = InitKeyboardKey("Esc", KEY_CODE_ESCAPE, 1.0f, LEFT_END);
static KeyboardKey f1Key = InitKeyboardKey("F1", KEY_CODE_F1);
static KeyboardKey f2Key = InitKeyboardKey("F2", KEY_CODE_F2);
static KeyboardKey f3Key = InitKeyboardKey("F3", KEY_CODE_F3);
static KeyboardKey f4Key = InitKeyboardKey("F4", KEY_CODE_F4);
static KeyboardKey f5Key = InitKeyboardKey("F5", KEY_CODE_F5);
static KeyboardKey f6Key = InitKeyboardKey("F6", KEY_CODE_F6);
static KeyboardKey f7Key = InitKeyboardKey("F7", KEY_CODE_F7);
static KeyboardKey f8Key = InitKeyboardKey("F8", KEY_CODE_F8);
static KeyboardKey f9Key = InitKeyboardKey("F9", KEY_CODE_F9);
static KeyboardKey f10Key = InitKeyboardKey("F10", KEY_CODE_F10);
static KeyboardKey f11Key = InitKeyboardKey("F11", KEY_CODE_F11);
static KeyboardKey f12Key = InitKeyboardKey("F12", KEY_CODE_F12);
static KeyboardKey sysRqKey = InitKeyboardKey("SysRq", KEY_CODE_SYSRQ);
static KeyboardKey scrollKey = InitKeyboardKey("Scroll", KEY_CODE_SCROLL);
static KeyboardKey pauseKey = InitKeyboardKey("Pause", KEY_CODE_PAUSE);
static KeyboardKey insertKey = InitKeyboardKey("Ins", KEY_CODE_INSERT, 0.88f);
static KeyboardKey deleteKey = InitKeyboardKey("Del", KEY_CODE_DELETE, 0.88f);
static KeyboardKey homeKey = InitKeyboardKey("Home", KEY_CODE_HOME, 0.88f);
static KeyboardKey endKey = InitKeyboardKey("End", KEY_CODE_END, 0.88f, RIGHT_END);
static KeyboardKey graveKey = InitKeyboardKey("`", KEY_CODE_GRAVE, 1.0f, LEFT_END);
static KeyboardKey d1Key = InitKeyboardKey("1", KEY_CODE_1);
static KeyboardKey d2Key = InitKeyboardKey("2", KEY_CODE_2);
static KeyboardKey d3Key = InitKeyboardKey("3", KEY_CODE_3);
static KeyboardKey d4Key = InitKeyboardKey("4", KEY_CODE_4);
static KeyboardKey d5Key = InitKeyboardKey("5", KEY_CODE_5);
static KeyboardKey d6Key = InitKeyboardKey("6", KEY_CODE_6);
static KeyboardKey d7Key = InitKeyboardKey("7", KEY_CODE_7);
static KeyboardKey d8Key = InitKeyboardKey("8", KEY_CODE_8);
static KeyboardKey d9Key = InitKeyboardKey("9", KEY_CODE_9);
static KeyboardKey d0Key = InitKeyboardKey("0", KEY_CODE_0);
static KeyboardKey minusKey = InitKeyboardKey("-", KEY_CODE_MINUS);
static KeyboardKey equalsKey = InitKeyboardKey("=", KEY_CODE_EQUALS);
static KeyboardKey backKey = InitKeyboardKey("Back", KEY_CODE_BACK, 2.5f);
static KeyboardKey numLockKey = InitKeyboardKey("NumLk", KEY_CODE_NUMLOCK);
static KeyboardKey divideKey = InitKeyboardKey("/", KEY_CODE_DIVIDE);
static KeyboardKey multiplyKey = InitKeyboardKey("*", KEY_CODE_MULTIPLY);
static KeyboardKey subtractKey = InitKeyboardKey("-", KEY_CODE_SUBTRACT, 1.0f, RIGHT_END);
static KeyboardKey tabKey = InitKeyboardKey("Tab", KEY_CODE_TAB, 1.5f, LEFT_END);
static KeyboardKey qKey = InitKeyboardKey("Q", KEY_CODE_Q);
static KeyboardKey wKey = InitKeyboardKey("W", KEY_CODE_W);
static KeyboardKey eKey = InitKeyboardKey("E", KEY_CODE_E);
static KeyboardKey rKey = InitKeyboardKey("R", KEY_CODE_R);
static KeyboardKey tKey = InitKeyboardKey("T", KEY_CODE_T);
static KeyboardKey yKey = InitKeyboardKey("Y", KEY_CODE_Y);
static KeyboardKey uKey = InitKeyboardKey("U", KEY_CODE_U);
static KeyboardKey iKey = InitKeyboardKey("I", KEY_CODE_I);
static KeyboardKey oKey = InitKeyboardKey("O", KEY_CODE_O);
static KeyboardKey pKey = InitKeyboardKey("P", KEY_CODE_P);
static KeyboardKey leftBracketKey = InitKeyboardKey("[", KEY_CODE_LBRACKET);
static KeyboardKey rightBracketKey = InitKeyboardKey("]", KEY_CODE_RBRACKET);
static KeyboardKey backslashKey = InitKeyboardKey("\\", KEY_CODE_BACKSLASH, 2.0f);
static KeyboardKey numpad7Key = InitKeyboardKey("7", KEY_CODE_NUMPAD7);
static KeyboardKey numpad8Key = InitKeyboardKey("8", KEY_CODE_NUMPAD8);
static KeyboardKey numpad9Key = InitKeyboardKey("9", KEY_CODE_NUMPAD9);
static KeyboardKey addKey = InitKeyboardKey("+", KEY_CODE_ADD, 1.0f, RIGHT_END);
static KeyboardKey captialKey = InitKeyboardKey("Caps Lock", KEY_CODE_CAPITAL, 2.0f, LEFT_END);
static KeyboardKey aKey = InitKeyboardKey("A", KEY_CODE_A);
static KeyboardKey sKey = InitKeyboardKey("S", KEY_CODE_S);
static KeyboardKey dKey = InitKeyboardKey("D", KEY_CODE_D);
static KeyboardKey fKey = InitKeyboardKey("F", KEY_CODE_F);
static KeyboardKey gKey = InitKeyboardKey("G", KEY_CODE_G);
static KeyboardKey hKey = InitKeyboardKey("H", KEY_CODE_H);
static KeyboardKey jKey = InitKeyboardKey("J", KEY_CODE_J);
static KeyboardKey kKey = InitKeyboardKey("K", KEY_CODE_K);
static KeyboardKey lKey = InitKeyboardKey("L", KEY_CODE_L);
static KeyboardKey semicolonKey = InitKeyboardKey(";", KEY_CODE_SEMICOLON);
static KeyboardKey apostropheKey = InitKeyboardKey("'", KEY_CODE_APOSTROPHE);
static KeyboardKey returnKey = InitKeyboardKey("Return", KEY_CODE_RETURN, 2.5f);
static KeyboardKey numpad4Key = InitKeyboardKey("4", KEY_CODE_NUMPAD4);
static KeyboardKey numpad5Key = InitKeyboardKey("5", KEY_CODE_NUMPAD5);
static KeyboardKey numpad6Key = InitKeyboardKey("6", KEY_CODE_NUMPAD6);
static KeyboardKey pageUpKey = InitKeyboardKey("PgUp", KEY_CODE_PRIOR, 1.0f, RIGHT_END);
static KeyboardKey leftShiftKey = InitKeyboardKey("Shift", KEY_CODE_LSHIFT, 2.5f, LEFT_END);
static KeyboardKey zKey = InitKeyboardKey("Z", KEY_CODE_Z);
static KeyboardKey xKey = InitKeyboardKey("X", KEY_CODE_X);
static KeyboardKey cKey = InitKeyboardKey("C", KEY_CODE_C);
static KeyboardKey vKey = InitKeyboardKey("V", KEY_CODE_V);
static KeyboardKey bKey = InitKeyboardKey("B", KEY_CODE_B);
static KeyboardKey nKey = InitKeyboardKey("N", KEY_CODE_N);
static KeyboardKey mKey = InitKeyboardKey("M", KEY_CODE_M);
static KeyboardKey commaKey = InitKeyboardKey(",", KEY_CODE_COMMA);
static KeyboardKey periodKey = InitKeyboardKey(".", KEY_CODE_PERIOD);
static KeyboardKey slashKey = InitKeyboardKey("/", KEY_CODE_SLASH);
static KeyboardKey rightShiftKey = InitKeyboardKey("Shift", KEY_CODE_RSHIFT, 3.0f);
static KeyboardKey numpad1Key = InitKeyboardKey("1", KEY_CODE_NUMPAD1);
static KeyboardKey numpad2Key = InitKeyboardKey("2", KEY_CODE_NUMPAD2);
static KeyboardKey numpad3Key = InitKeyboardKey("3", KEY_CODE_NUMPAD3);
static KeyboardKey pageDownKey = InitKeyboardKey("PgDn", KEY_CODE_NEXT, 1.0f, RIGHT_END);
static KeyboardKey leftControlKey = InitKeyboardKey("Ctrl", KEY_CODE_LCONTROl, 1.0f, LEFT_END);
static KeyboardKey leftWindowsKey = InitKeyboardKey("Win", KEY_CODE_LWIN);
static KeyboardKey leftAltKey = InitKeyboardKey("Alt", KEY_CODE_LMENU);
static KeyboardKey spaceKey = InitKeyboardKey("Space", KEY_CODE_SPACE, 4.5f);
static KeyboardKey rightAltKey = InitKeyboardKey("Alt", KEY_CODE_RMENU);
static KeyboardKey rightWindowsKey = InitKeyboardKey("Win", KEY_CODE_RWIN);
static KeyboardKey appsKey = InitKeyboardKey("Menu", KEY_CODE_APPS);
static KeyboardKey rightControlKey = InitKeyboardKey("Ctrl", KEY_CODE_RCONTROL);
static KeyboardKey upKey = InitKeyboardKey("Up", KEY_CODE_UP);
static KeyboardKey downKey = InitKeyboardKey("Down", KEY_CODE_DOWN);
static KeyboardKey leftKey = InitKeyboardKey("Left", KEY_CODE_LEFT);
static KeyboardKey rightKey = InitKeyboardKey("Right", KEY_CODE_RIGHT);
static KeyboardKey numpad0Key = InitKeyboardKey("0", KEY_CODE_NUMPAD0, 2.0f);
static KeyboardKey numpadCommaKey = InitKeyboardKey(",", KEY_CODE_NUMPADCOMMA);
static KeyboardKey numpadEnterKey = InitKeyboardKey("Enter", KEY_CODE_NUMPADENTER, 1.0f, RIGHT_END);

static KeyboardKey *keyboardKeys[] = {&escapeKey, &f1Key, &f2Key, &f3Key, &f4Key, &f5Key, &f6Key, &f7Key, &f8Key, &f9Key, &f10Key, &f11Key, &f12Key, &sysRqKey, &scrollKey, &pauseKey, &insertKey, &deleteKey, &homeKey, &endKey, &graveKey, &d1Key, &d2Key, &d3Key, &d4Key, &d5Key, &d6Key, &d7Key, &d8Key, &d9Key, &d0Key, &minusKey, &equalsKey, &backKey, &numLockKey, &divideKey, &multiplyKey, &subtractKey, &tabKey, &qKey, &wKey, &eKey, &rKey, &tKey, &yKey, &uKey, &iKey, &oKey, &pKey, &leftBracketKey, &rightBracketKey, &backslashKey, &numpad7Key, &numpad8Key, &numpad9Key, &addKey, &captialKey, &aKey, &sKey, &dKey, &fKey, &gKey, &hKey, &jKey, &kKey, &lKey, &semicolonKey, &apostropheKey, &returnKey, &numpad4Key, &numpad5Key, &numpad6Key, &pageUpKey, &leftShiftKey, &zKey, &xKey, &cKey, &vKey, &bKey, &nKey, &mKey, &commaKey, &periodKey, &slashKey, &rightShiftKey, &numpad1Key, &numpad2Key, &numpad3Key, &pageDownKey, &leftControlKey, &leftWindowsKey, &leftAltKey, &spaceKey, &rightAltKey, &rightWindowsKey, &appsKey, &rightControlKey, &upKey, &downKey, &leftKey, &rightKey, &numpad0Key, &numpadCommaKey, &numpadEnterKey};
static KeyboardKey *selectedKey = &kKey;

static int axisOffset = 0, buttonOffset = 0, indicatorsBottom = 0, indicatorsRight = 0, numMixtureLevers = 0, numPropLevers = 0, keyboardBottom = 0, keyboardRight = 0, keyPressActive = 0, lastCinemaVerite = 0, overrideHeadShakePluginFailed = 0, thrustReverserMode = 0, showIndicators = 1, switchTo3DCommandLook = 0;
static float defaultHeadPositionX = FLT_MAX, defaultHeadPositionY = FLT_MAX, defaultHeadPositionZ = FLT_MAX;
static ControllerType controllerType = XBOX360;
static Mode mode = DEFAULT;
static ConfigurationStep configurationStep = START;
static GLuint indicatorsProgram = 0, indicatorsVertexShader = 0, indicatorsFragmentShader = 0, keyboardKeyProgram = 0, keyboardKeyFragmentShader = 0;
static int *pushedJoystickButtonAssignments = NULL;
static XPLMWindowID indicatorsWindow = NULL, keyboardWindow = NULL;

#if IBM
static HINSTANCE hGetProcIDDLL = NULL;
typedef int(__stdcall *pICFUNC)(int, XInputState &);
pICFUNC XInputGetStateEx = NULL;
static HANDLE hidDeviceThread = 0;
#elif APL
static pthread_t hidDeviceThread = 0;
#endif

#if LIN
static Display *display = NULL;
#else
static int hidInitialized = 0;
static volatile int hidDeviceThreadRun = 1;
#endif

static XPLMCommandRef cycleResetViewCommand = NULL, toggleArmSpeedBrakeOrToggleCarbHeatCommand = NULL, cwsOrDisconnectAutopilotCommand = NULL, lookModifierCommand = NULL, propPitchOrThrottleModifierCommand = NULL, mixtureControlModifierCommand = NULL, cowlFlapModifierCommand = NULL, trimModifierCommand = NULL, trimResetCommand = NULL, toggleMousePointerControlCommand = NULL, pushToTalkCommand = NULL, toggleLeftMouseButtonCommand = NULL, toggleReverseCommand = NULL, toggleRightMouseButtonCommand = NULL, scrollUpCommand = NULL, scrollDownCommand = NULL, keyboardSelectorUpCommand = NULL, keyboardSelectorDownCommand = NULL, keyboardSelectorLeftCommand = NULL, keyboardSelectorRightCommand = NULL, pressKeyboardKeyCommand = NULL, lockKeyboardKeyCommand = NULL;
static XPLMDataRef preconfiguredApTypeDataRef = NULL, acfCockpitTypeDataRef = NULL, acfPeXDataRef = NULL, acfPeYDataRef = NULL, acfPeZDataRef = NULL, acfRSCRedlinePrpDataRef = NULL, acfNumEnginesDataRef = NULL, acfThrotmaxREVDataRef = NULL, acfFeatheredPitchDataRef = NULL, acfHasBetaDataRef = NULL, acfSbrkEQDataRef = NULL, acfEnTypeDataRef = NULL, acfPropTypeDataRef = NULL, acfMinPitchDataRef = NULL, acfMaxPitchDataRef = NULL, ongroundAnyDataRef = NULL, groundspeedDataRef = NULL, cinemaVeriteDataRef = NULL, pilotsHeadPsiDataRef = NULL, pilotsHeadTheDataRef = NULL, viewTypeDataRef = NULL, vrEnabledDataRef = NULL, hasJoystickDataRef = NULL, joystickPitchNullzoneDataRef = NULL, joystickRollNullzoneDataRef = NULL, joystickHeadingNullzoneDataRef = NULL, joystickPitchSensitivityDataRef = NULL, joystickRollSensitivityDataRef = NULL, joystickHeadingSensitivityDataRef = NULL, joystickAxisAssignmentsDataRef = NULL, joystickAxisReverseDataRef = NULL, joystickAxisValuesDataRef = NULL, joystickButtonAssignmentsDataRef = NULL, joystickButtonValuesDataRef = NULL, leftBrakeRatioDataRef = NULL, rightBrakeRatioDataRef = NULL, speedbrakeRatioDataRef = NULL, aileronTrimDataRef = NULL, elevatorTrimDataRef = NULL, rudderTrimDataRef = NULL, throttleRatioAllDataRef = NULL, throttleJetRevRatioAllDataRef = NULL, throttleBetaRevRatioAllDataRef = NULL, propPitchDegDataRef = NULL, propRotationSpeedRadSecAllDataRef = NULL, mixtureRatioAllDataRef = NULL, carbHeatRatioDataRef = NULL, cowlFlapRatioDataRef = NULL, overrideToeBrakesDataRef = NULL;
static XPWidgetID settingsWidget = NULL, dualShock4ControllerRadioButton = NULL, xbox360ControllerRadioButton = NULL, configurationStatusCaption = NULL, startConfigurationtButton = NULL, showIndicatorsCheckbox = NULL;

PLUGIN_API int XPluginStart(char *outName, char *outSig, char *outDesc)
{
#if IBM
    hGetProcIDDLL = LoadLibrary("XInput1_3.dll");
    FARPROC lpfnGetProcessID = GetProcAddress(HMODULE(hGetProcIDDLL), (LPCSTR)100);
    XInputGetStateEx = pICFUNC(lpfnGetProcessID);
#endif

    // set plugin info
    strcpy(outName, NAME);
    strcpy(outSig, "de.bwravencl." NAME_LOWERCASE);
    strcpy(outDesc, NAME " allows flying X-Plane by gamepad!");

    // get paths in POSIX format
    XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);

#if IBM
    // init glew
    const GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        XPLMDebugString(NAME ": The following error occured while initializing GLEW:\n");
        XPLMDebugString((const char *)glewGetErrorString(err));
    }
#endif

    // prepare fragment-shaders
    InitShader(INDICATORS_FRAGMENT_SHADER, &indicatorsProgram, &indicatorsFragmentShader);
    InitShader(KEYBOARD_KEY_FRAGMENT_SHADER, &keyboardKeyProgram, &keyboardKeyFragmentShader);

    // obtain datarefs
    preconfiguredApTypeDataRef = XPLMFindDataRef("sim/aircraft/autopilot/preconfigured_ap_type");
    acfCockpitTypeDataRef = XPLMFindDataRef("sim/aircraft/view/acf_cockpit_type");
    acfPeXDataRef = XPLMFindDataRef("sim/aircraft/view/acf_peX");
    acfPeYDataRef = XPLMFindDataRef("sim/aircraft/view/acf_peY");
    acfPeZDataRef = XPLMFindDataRef("sim/aircraft/view/acf_peZ");
    acfRSCRedlinePrpDataRef = XPLMFindDataRef("sim/aircraft/controls/acf_RSC_redline_prp");
    acfNumEnginesDataRef = XPLMFindDataRef("sim/aircraft/engine/acf_num_engines");
    acfThrotmaxREVDataRef = XPLMFindDataRef("sim/aircraft/engine/acf_throtmax_REV");
    acfFeatheredPitchDataRef = XPLMFindDataRef("sim/aircraft/overflow/acf_feathered_pitch");
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
    vrEnabledDataRef = XPLMFindDataRef("sim/graphics/VR/enabled");
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
    throttleJetRevRatioAllDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/throttle_jet_rev_ratio_all");
    throttleBetaRevRatioAllDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/throttle_beta_rev_ratio_all");
    propPitchDegDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/prop_pitch_deg");
    propRotationSpeedRadSecAllDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/prop_rotation_speed_rad_sec_all");
    mixtureRatioAllDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/mixture_ratio_all");
    carbHeatRatioDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/carb_heat_ratio");
    cowlFlapRatioDataRef = XPLMFindDataRef("sim/cockpit2/engine/actuators/cowl_flap_ratio");
    overrideToeBrakesDataRef = XPLMFindDataRef("sim/operation/override/override_toe_brakes");

    // create custom commands
    cycleResetViewCommand = XPLMCreateCommand(CYCLE_RESET_VIEW_COMMAND, "Cycle / Reset View");
    toggleArmSpeedBrakeOrToggleCarbHeatCommand = XPLMCreateCommand(TOGGLE_ARM_SPEED_BRAKE_OR_TOGGLE_CARB_HEAT_COMMAND, "Toggle / Arm Speedbrake / Toggle Carb Heat");
    cwsOrDisconnectAutopilotCommand = XPLMCreateCommand(CWS_OR_DISCONNECT_AUTOPILOT, "CWS / Disconnect Autopilot");
    lookModifierCommand = XPLMCreateCommand(LOOK_MODIFIER_COMMAND, "Look Modifier");
    propPitchOrThrottleModifierCommand = XPLMCreateCommand(PROP_PITCH_THROTTLE_MODIFIER_COMMAND, "Prop Pitch / Throttle Modifier");
    mixtureControlModifierCommand = XPLMCreateCommand(MIXTURE_CONTROL_MODIFIER_COMMAND, "Mixture Control Modifier");
    cowlFlapModifierCommand = XPLMCreateCommand(COWL_FLAP_MODIFIER_COMMAND, "Cowl Flap Modifier");
    trimModifierCommand = XPLMCreateCommand(TRIM_MODIFIER_COMMAND, "Trim Modifier");
    trimResetCommand = XPLMCreateCommand(TRIM_RESET_COMMAND, "Trim Reset");
    toggleReverseCommand = XPLMCreateCommand(TOGGLE_REVERSE_COMMAND, "Toggle Reverse");
    toggleMousePointerControlCommand = XPLMCreateCommand(TOGGLE_MOUSE_OR_KEYBOARD_CONTROL_COMMAND, "Toggle Mouse or Keyboard Control");
    pushToTalkCommand = XPLMCreateCommand(PUSH_TO_TALK_COMMAND, "Push-To-Talk");
    toggleLeftMouseButtonCommand = XPLMCreateCommand(TOGGLE_LEFT_MOUSE_BUTTON_COMMAND, "Toggle Left Mouse Button");
    toggleRightMouseButtonCommand = XPLMCreateCommand(TOGGLE_RIGHT_MOUSE_BUTTON_COMMAND, "Toggle Right Mouse Button");
    scrollUpCommand = XPLMCreateCommand(SCROLL_UP_COMMAND, "Scroll Up");
    scrollDownCommand = XPLMCreateCommand(SCROLL_DOWN_COMMAND, "Scroll Down");
    keyboardSelectorUpCommand = XPLMCreateCommand(KEYBOARD_SELECTOR_UP_COMMAND, "Keyboard Selector Up");
    keyboardSelectorDownCommand = XPLMCreateCommand(KEYBOARD_SELECTOR_DOWN_COMMAND, "Keyboard Selector Down");
    keyboardSelectorLeftCommand = XPLMCreateCommand(KEYBOARD_SELECTOR_LEFT_COMMAND, "Keyboard Selector Left");
    keyboardSelectorRightCommand = XPLMCreateCommand(KEYBOARD_SELECTOR_RIGHT_COMMAND, "Keyboard Selector Right");
    pressKeyboardKeyCommand = XPLMCreateCommand(PRESS_KEYBOARD_KEY_COMMAND, "Press Keyboard Key");
    lockKeyboardKeyCommand = XPLMCreateCommand(LOCK_KEYBOARD_KEY_COMMAND, "Lock Keyboard Key");

    // register custom commands
    XPLMRegisterCommandHandler(cycleResetViewCommand, ResetSwitchViewCommand, 1, NULL);
    XPLMRegisterCommandHandler(toggleArmSpeedBrakeOrToggleCarbHeatCommand, ToggleArmSpeedBrakeOrToggleCarbHeatCommand, 1, NULL);
    XPLMRegisterCommandHandler(cwsOrDisconnectAutopilotCommand, CwsOrDisconnectAutopilotCommand, 1, NULL);
    XPLMRegisterCommandHandler(lookModifierCommand, LookModifierCommand, 1, NULL);
    XPLMRegisterCommandHandler(propPitchOrThrottleModifierCommand, PropPitchOrThrottleModifierCommand, 1, NULL);
    XPLMRegisterCommandHandler(mixtureControlModifierCommand, MixtureControlModifierCommand, 1, NULL);
    XPLMRegisterCommandHandler(cowlFlapModifierCommand, CowlFlapModifierCommand, 1, NULL);
    XPLMRegisterCommandHandler(trimModifierCommand, TrimModifierCommand, 1, NULL);
    XPLMRegisterCommandHandler(trimResetCommand, TrimResetCommand, 1, NULL);
    XPLMRegisterCommandHandler(toggleReverseCommand, ToggleReverseCommand, 1, NULL);
    XPLMRegisterCommandHandler(toggleMousePointerControlCommand, ToggleMouseOrKeyboardControlCommand, 1, NULL);
    XPLMRegisterCommandHandler(pushToTalkCommand, PushToTalkCommand, 1, NULL);
    XPLMRegisterCommandHandler(toggleLeftMouseButtonCommand, ToggleLeftMouseButtonCommand, 1, NULL);
    XPLMRegisterCommandHandler(toggleRightMouseButtonCommand, ToggleRightMouseButtonCommand, 1, NULL);
    XPLMRegisterCommandHandler(scrollUpCommand, ScrollUpCommand, 1, NULL);
    XPLMRegisterCommandHandler(scrollDownCommand, ScrollDownCommand, 1, NULL);
    XPLMRegisterCommandHandler(keyboardSelectorUpCommand, KeyboardSelectorUpCommand, 1, NULL);
    XPLMRegisterCommandHandler(keyboardSelectorDownCommand, KeyboardSelectorDownCommand, 1, NULL);
    XPLMRegisterCommandHandler(keyboardSelectorLeftCommand, KeyboardSelectorLeftCommand, 1, NULL);
    XPLMRegisterCommandHandler(keyboardSelectorRightCommand, KeyboardSelectorRightCommand, 1, NULL);
    XPLMRegisterCommandHandler(pressKeyboardKeyCommand, PressKeyboardKeyCommand, 1, NULL);
    XPLMRegisterCommandHandler(lockKeyboardKeyCommand, LockKeyboardKeyCommand, 1, NULL);

    // initialize indicator default position
    int right = 0, bottom = 0;
    XPLMGetScreenBoundsGlobal(NULL, NULL, &right, &bottom);
    indicatorsRight = right;
    indicatorsBottom = bottom;

    // initialize keyboard default position
    keyboardRight = right / 2 + GetKeyboardWidth() / 2;
    keyboardBottom = bottom;

    WireKeys();

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
    if (display != NULL)
    {
        XEvent event;
        XQueryPointer(display, RootWindow(display, DefaultScreen(display)), &event.xbutton.root, &event.xbutton.window, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
    }
#endif

    return 1;
}

PLUGIN_API void XPluginStop(void)
{
    ReleaseAllKeys();

    CleanupShader(indicatorsProgram, indicatorsFragmentShader, 1);
    CleanupShader(keyboardKeyProgram, keyboardKeyFragmentShader, 1);

    // revert any remaining button assignments
    PopButtonAssignments();

    // unregister custom commands
    XPLMUnregisterCommandHandler(cycleResetViewCommand, ResetSwitchViewCommand, 1, NULL);
    XPLMUnregisterCommandHandler(toggleArmSpeedBrakeOrToggleCarbHeatCommand, ToggleArmSpeedBrakeOrToggleCarbHeatCommand, 1, NULL);
    XPLMUnregisterCommandHandler(cwsOrDisconnectAutopilotCommand, CwsOrDisconnectAutopilotCommand, 1, NULL);
    XPLMUnregisterCommandHandler(lookModifierCommand, LookModifierCommand, 1, NULL);
    XPLMUnregisterCommandHandler(propPitchOrThrottleModifierCommand, PropPitchOrThrottleModifierCommand, 1, NULL);
    XPLMUnregisterCommandHandler(mixtureControlModifierCommand, MixtureControlModifierCommand, 1, NULL);
    XPLMUnregisterCommandHandler(cowlFlapModifierCommand, CowlFlapModifierCommand, 1, NULL);
    XPLMUnregisterCommandHandler(trimModifierCommand, TrimModifierCommand, 1, NULL);
    XPLMUnregisterCommandHandler(trimResetCommand, TrimResetCommand, 1, NULL);
    XPLMUnregisterCommandHandler(toggleReverseCommand, ToggleReverseCommand, 1, NULL);
    XPLMUnregisterCommandHandler(toggleMousePointerControlCommand, ToggleMouseOrKeyboardControlCommand, 1, NULL);
    XPLMUnregisterCommandHandler(pushToTalkCommand, PushToTalkCommand, 1, NULL);
    XPLMUnregisterCommandHandler(toggleLeftMouseButtonCommand, ToggleLeftMouseButtonCommand, 1, NULL);
    XPLMUnregisterCommandHandler(toggleRightMouseButtonCommand, ToggleRightMouseButtonCommand, 1, NULL);
    XPLMUnregisterCommandHandler(scrollUpCommand, ScrollUpCommand, 1, NULL);
    XPLMUnregisterCommandHandler(scrollDownCommand, ScrollDownCommand, 1, NULL);
    XPLMUnregisterCommandHandler(keyboardSelectorUpCommand, KeyboardSelectorUpCommand, 1, NULL);
    XPLMUnregisterCommandHandler(keyboardSelectorDownCommand, KeyboardSelectorDownCommand, 1, NULL);
    XPLMUnregisterCommandHandler(keyboardSelectorLeftCommand, KeyboardSelectorLeftCommand, 1, NULL);
    XPLMUnregisterCommandHandler(keyboardSelectorRightCommand, KeyboardSelectorRightCommand, 1, NULL);
    XPLMUnregisterCommandHandler(pressKeyboardKeyCommand, PressKeyboardKeyCommand, 1, NULL);
    XPLMUnregisterCommandHandler(lockKeyboardKeyCommand, LockKeyboardKeyCommand, 1, NULL);

    // register flight loop callbacks
    XPLMUnregisterFlightLoopCallback(FlightLoopCallback, NULL);

    // release toe brake control
    XPLMSetDatai(overrideToeBrakesDataRef, 0);

#if IBM
    if (hGetProcIDDLL != NULL)
        FreeLibrary(hGetProcIDDLL);
#endif

#if !LIN
    hidDeviceThreadRun = 0;
    if (hidDeviceThread != 0)
#if IBM
        WaitForSingleObject(hidDeviceThread, INFINITE);
#elif APL
        pthread_join(hidDeviceThread, NULL);
#endif
    hid_exit();
#else
    if (display != NULL)
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
    if (inFromWho != XPLM_PLUGIN_XPLANE)
        return;

    switch (inMessage)
    {
    case XPLM_MSG_PLANE_LOADED:
        // reset the default head position if a new plane is loaded so that it is updated during the next flight loop
        defaultHeadPositionX = FLT_MAX;
        defaultHeadPositionY = FLT_MAX;
        defaultHeadPositionZ = FLT_MAX;

        thrustReverserMode = 0;

        // reinitialize indicators window if necessary
        UpdateIndicatorsWindow();

    case XPLM_MSG_AIRPORT_LOADED:
        // schedule a switch to the 3D cockpit view during the next flight loop
        switchTo3DCommandLook = 0;
        if (!Has2DPanel())
            switchTo3DCommandLook = 1;
        break;

    case XPLM_MSG_ENTERED_VR:
    case XPLM_MSG_EXITING_VR:
        const int vrEnabled = inMessage == XPLM_MSG_ENTERED_VR;

        UpdateIndicatorsWindow(vrEnabled);

        // always destroy the keyboard window
        if (keyboardWindow != NULL)
        {
            XPLMDestroyWindow(keyboardWindow);
            keyboardWindow = NULL;
        }

        if (mode == KEYBOARD && !keyPressActive)
        {
            EndKeyboardMode();

            // create a fresh keyboard window immediately and show it again
            ToggleKeyboardControl(vrEnabled);
        }
        break;
    }
}

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

#if !LIN
// hid device thread cleanup function
static void CleanupDeviceThread(hid_device *handle, hid_device_info *dev)
{
    if (handle != NULL)
        hid_close(handle);

    if (dev != NULL)
        free(dev);

    hidDeviceThread = 0;
}
#endif

static void CleanupShader(GLuint program, GLuint fragmentShader, int deleteProgram = 0)
{
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);

    if (deleteProgram)
        glDeleteProgram(program);
}

static int CowlFlapModifierCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    ToggleMode(COWL, inPhase);

    return 0;
}

static int CwsOrDisconnectAutopilotCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    if (inPhase == xplm_CommandContinue)
        return 0;

    switch (XPLMGetDatai(preconfiguredApTypeDataRef))
    {
    case 0:
    case 1:
        if (inPhase == xplm_CommandBegin)
            XPLMCommandOnce(XPLMFindCommand("sim/autopilot/servos_off_any"));
        break;
    default:
        if (inPhase == xplm_CommandBegin)
            XPLMCommandBegin(XPLMFindCommand("sim/autopilot/control_wheel_steer"));
        else
            XPLMCommandEnd(XPLMFindCommand("sim/autopilot/control_wheel_steer"));

        break;
    }

    return 0;
}

#if !LIN
#if IBM
static void DeviceThread(void *argument)
#elif APL
static void *DeviceThread(void *argument)
#endif
{
    struct hid_device_info *dev = (hid_device_info *)argument;
    hid_device *handle = hid_open(dev->vendor_id, dev->product_id, dev->serial_number);
    if (handle == NULL)
    {
        CleanupDeviceThread(handle, dev);
#if IBM
        return;
#elif APL
        return (void *)1;
#endif
    }

    unsigned char data[40];
    static int prevTouchpadButtonDown = 0, prevX1 = 0, prevY1 = 0, prevDown1 = 0, prevDown2 = 0;
    while (hidDeviceThreadRun)
    {
        memset(data, 0, 40);
        if (hid_read(handle, data, 40) == -1)
        {
            CleanupDeviceThread(handle, dev);
#if IBM
            return;
#elif APL
            return (void *)1;
#endif
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
                distX = (int)(dX1 * TOUCHPAD_CURSOR_SENSITIVITY);
            if (prevY1 > 0 && abs(dY1) < TOUCHPAD_MAX_DELTA)
                distY = (int)(dY1 * TOUCHPAD_CURSOR_SENSITIVITY);

            MoveMousePointer(distX, distY);
        }
        else if (prevY1 > 0 && abs(dY1) < TOUCHPAD_MAX_DELTA)
            scrollClicks = (int)(-dY1 * TOUCHPAD_SCROLL_SENSITIVITY);

        Scroll(scrollClicks);

        prevTouchpadButtonDown = touchpadButtonDown;
        prevDown1 = down1;
        prevDown2 = down2;
        prevX1 = x1;
        prevY1 = y1;
    }

    CleanupDeviceThread(handle, dev);

#if IBM
    _endthread();
#elif APL
    return (void *)0;
#endif
}
#endif

static void DrawIndicatorsWindow(XPLMWindowID inWindowID, void *inRefcon)
{
    const int gliderWithSpeedbrakes = IsGliderWithSpeedbrakes();
    const int helicopter = IsHelicopter();

    XPLMSetGraphicsState(0, 0, 0, 0, 1, 0, 0);

    glUseProgram(indicatorsProgram);

    float throttle = 0.0f;
    if (gliderWithSpeedbrakes)
        throttle = 1.0f - XPLMGetDataf(speedbrakeRatioDataRef);
    else
        throttle = XPLMGetDataf(throttleBetaRevRatioAllDataRef);

    const int throttleLocation = glGetUniformLocation(indicatorsProgram, "throttle");
    glUniform1f(throttleLocation, throttle);

    const int propLocation = glGetUniformLocation(indicatorsProgram, "prop");
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
        propRatio = Normalize(XPLMGetDataf(propRotationSpeedRadSecAllDataRef), XPLMGetDataf(acfFeatheredPitchDataRef), XPLMGetDataf(acfRSCRedlinePrpDataRef), 0.0f, 1.0f);

    glUniform1f(propLocation, numPropLevers < 1 ? -3.0f : propRatio);

    const int mixtureLocation = glGetUniformLocation(indicatorsProgram, "mixture");
    glUniform1f(mixtureLocation, numMixtureLevers < 1 ? -3.0f : XPLMGetDataf(mixtureRatioAllDataRef));

    int left, top, right, bottom;
    XPLMGetWindowGeometry(indicatorsWindow, &left, &top, &right, &bottom);

    const int sizeLocation = glGetUniformLocation(indicatorsProgram, "size");
    glUniform2i(sizeLocation, right - left, top - bottom);

    glBegin(GL_QUADS);
    glColor3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f((GLfloat)left, (GLfloat)bottom);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f((GLfloat)left, (GLfloat)top);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f((GLfloat)right, (GLfloat)top);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f((GLfloat)right, (GLfloat)bottom);
    glEnd();

    glUseProgram(0);
}

static void DrawKeyboardWindow(XPLMWindowID inWindowID, void *inRefcon)
{
    XPLMSetGraphicsState(0, 0, 0, 0, 1, 0, 0);

    int windowLeft, windowTop, windowRight, windowBottom;
    XPLMGetWindowGeometry(keyboardWindow, &windowLeft, &windowTop, &windowRight, &windowBottom);

    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f((GLfloat)windowLeft + 1, (GLfloat)(windowBottom));
    glVertex2f((GLfloat)windowLeft + 1, (GLfloat)windowTop);
    glVertex2f((GLfloat)windowRight, (GLfloat)windowTop);
    glVertex2f((GLfloat)windowRight, (GLfloat)windowBottom + 1);
    glEnd();

    KeyboardKey key = escapeKey;
    int left = windowLeft;
    int top = windowTop;
    Direction direction = RIGHT;

    static int labelOffsetY = 0;
    if (labelOffsetY == 0)
    {
        int charHeight;
        XPLMGetFontDimensions(xplmFont_Basic, NULL, &charHeight, NULL);
        labelOffsetY = charHeight / 2;
    }

    glUseProgram(keyboardKeyProgram);
    const int keyBaseSizeLocation = glGetUniformLocation(keyboardKeyProgram, "keyBaseSize");
    const int aspectLocation = glGetUniformLocation(keyboardKeyProgram, "aspect");
    const int selectedLocation = glGetUniformLocation(keyboardKeyProgram, "selected");
    const int downLocation = glGetUniformLocation(keyboardKeyProgram, "down");
    while (1)
    {
        glUseProgram(keyboardKeyProgram);
        glUniform1i(keyBaseSizeLocation, KEY_BASE_SIZE);
        glUniform1f(aspectLocation, key.aspect);
        glUniform1i(selectedLocation, key.keyCode == (*selectedKey).keyCode);
        int lockKey = IsLockKey(key);
        glUniform1i(downLocation, (key.state == NEW_DOWN || key.state == DOWN) && !lockKey);

        const int right = left + key.width;
        const int bottom = top - KEY_BASE_SIZE;

        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f((GLfloat)left, (GLfloat)bottom);
        glTexCoord2f(0.0f, 1.0f);
        glVertex2f((GLfloat)left, (GLfloat)top);
        glTexCoord2f(1.0f, 1.0f);
        glVertex2f((GLfloat)right, (GLfloat)top);
        glTexCoord2f(1.0f, 0.0f);
        glVertex2f((GLfloat)right, (GLfloat)bottom);
        glEnd();

        float labelColor[] = {1.0f, 1.0f, 1.0f};
        if (lockKey && (key.state == DOWN || key.state == NEW_DOWN))
        {
            labelColor[0] = 0.0f;
            labelColor[2] = 0.0f;
        }
        XPLMDrawString(labelColor, left + key.width / 2 - key.labelOffsetX, top - KEY_BASE_SIZE / 2 - labelOffsetY, key.label, NULL, xplmFont_Basic);

        if (key.keyCode == leftControlKey.keyCode)
            break;

        if (key.position == RIGHT_END && direction == RIGHT)
        {
            key = *key.below;
            direction = LEFT;
            left = windowRight - key.width;
            top = bottom;
        }
        else if (key.position == LEFT_END && direction == LEFT)
        {
            key = *key.below;
            direction = RIGHT;
            left = windowLeft;
            top = bottom;
        }
        else if (direction == RIGHT)
        {
            key = *key.right;
            left = right;
        }
        else if (direction == LEFT)
        {
            key = *key.left;
            left -= key.width;
        }
    }

    glUseProgram(0);
}

static void EndKeyboardMode(void)
{
    ReleaseAllKeys();

    // restore the default button assignments
    PopButtonAssignments();

    if (keyboardWindow != NULL)
        XPLMSetWindowIsVisible(keyboardWindow, 0);

    mode = DEFAULT;
}

static float Exponentialize(float value, float inMin, float inMax, float outMin, float outMax)
{
    float n = Normalize(value, inMin, inMax, 0.0f, 1.0f);
    return Normalize(powf(n * 100.0f, JOYSTICK_RELATIVE_CONTROL_EXPONENT), 0.0f, powf(100.0f, JOYSTICK_RELATIVE_CONTROL_EXPONENT), outMin, outMax);
}

static void FitGeometryWithinScreenBounds(int *left, int *top, int *right, int *bottom)
{
    int minLeft, maxTop, maxRight, minBottom;
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

static float FlightLoopCallback(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void *inRefcon)
{
    const float currentTime = XPLMGetElapsedTime();

    KeyboardKey **ptr = keyboardKeys;
    KeyboardKey **endPtr = keyboardKeys + sizeof(keyboardKeys) / sizeof(keyboardKeys[0]);
    while (ptr < endPtr)
    {
        switch ((**ptr).state)
        {
        case NEW_UP:
            if (IsLockKey(**ptr))
                MakeInput((**ptr).keyCode, DOWN);
            MakeInput((**ptr).keyCode, UP);
            (**ptr).state = UP;
            break;
        case NEW_DOWN:
            MakeInput((**ptr).keyCode, DOWN);
            MakeInput((**ptr).keyCode, UP);
            (**ptr).state = DOWN;
            (**ptr).lastInputTime = currentTime;
            break;
        case DOWN:
            if (!IsLockKey(**ptr) && currentTime - (**ptr).lastInputTime > KEY_REPEAT_INTERVAL)
            {
                MakeInput((**ptr).keyCode, DOWN);
                (**ptr).lastInputTime = currentTime;
            }
            break;
        default:
            break;
        }

        ptr++;
    }

    if (showIndicators && !XPLMGetWindowIsVisible(indicatorsWindow))
    {
        // showIndicators is enabled but the indicators window is not visible (this can happen in VR if the user presses the close button)
        showIndicators = 0;
        XPSetWidgetProperty(showIndicatorsCheckbox, xpProperty_ButtonState, showIndicators);
    }

    if (XPLMGetWindowIsVisible(keyboardWindow))
    {
        SyncLockKeyState(&captialKey);
        SyncLockKeyState(&numLockKey);
        SyncLockKeyState(&scrollKey);
    }
    else if (mode == KEYBOARD)
        // we are in keyboard mode but the keyboard window is not visible (this can happen in VR if the user presses the close button)
        EndKeyboardMode();

    // update the default head position when required
    if (FloatsEqual(defaultHeadPositionX, FLT_MAX) || FloatsEqual(defaultHeadPositionY, FLT_MAX) || FloatsEqual(defaultHeadPositionZ, FLT_MAX))
    {
        defaultHeadPositionX = XPLMGetDataf(acfPeXDataRef);
        defaultHeadPositionY = XPLMGetDataf(acfPeYDataRef);
        defaultHeadPositionZ = XPLMGetDataf(acfPeZDataRef);
    }

    const int helicopter = IsHelicopter();

    int currentMouseX, currentMouseY;
    XPLMGetMouseLocation(&currentMouseX, &currentMouseY);

    // handle switch to 3D command look
    if (switchTo3DCommandLook)
    {
        XPLMCommandOnce(XPLMFindCommand("sim/view/3d_cockpit_cmnd_look"));
        switchTo3DCommandLook = 0;
    }

    // disable thrust reverser mode if throttle was moved into positive range
    if (thrustReverserMode && XPLMGetDataf(throttleBetaRevRatioAllDataRef) > 0.0f)
        thrustReverserMode = false;

    if (XPLMGetDatai(hasJoystickDataRef))
    {
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
#endif

#if !LIN
        if (controllerType == DS4)
        {
            static float lastEnumerationTime = 0.0f;
            if (hidDeviceThread == 0 && currentTime - lastEnumerationTime >= 5.0f)
            {
                lastEnumerationTime = currentTime;

                if (!hidInitialized && hid_init() != -1)
                    hidInitialized = 1;

                if (hidInitialized)
                {
                    struct hid_device_info *devs = hid_enumerate(0x0, 0x0);
                    struct hid_device_info *currentDev = devs;

                    while (currentDev != NULL)
                    {
                        if (currentDev->vendor_id == 0x54C && (currentDev->product_id == 0x5C4 || currentDev->product_id == 0x9CC || currentDev->product_id == 0xBA0))
                        {
                            struct hid_device_info *currentDevCopy = (hid_device_info *)calloc(1, sizeof(hid_device_info));
                            currentDevCopy->vendor_id = currentDev->vendor_id;
                            currentDevCopy->product_id = currentDev->product_id;

#if IBM
                            HANDLE threadHandle = (HANDLE)_beginthread(DeviceThread, 0, currentDevCopy);
                            if (threadHandle > 0)
                            {
                                hidDeviceThread = threadHandle;
                                break;
                            }
#elif APL
                            if (pthread_create(&hidDeviceThread, NULL, DeviceThread, currentDevCopy))
                                break;
#endif
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

        static int potentialAxes[100] = {0};
        static int potentialButtons[1600] = {0};

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

        const float sensitivityMultiplier = JOYSTICK_RELATIVE_CONTROL_MULTIPLIER * inElapsedSinceLastCall;

        const float joystickPitchNullzone = XPLMGetDataf(joystickPitchNullzoneDataRef);

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
            const int acfNumEngines = XPLMGetDatai(acfNumEnginesDataRef);

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
            XPLMSetDataf(leftBrakeRatioDataRef, leftBrakeRatio);
            XPLMSetDataf(rightBrakeRatioDataRef, rightBrakeRatio);

            if (controllerType == XBOX360)
            {
#if IBM
                static int leftTriggerDown = 0, rightTriggerDown = 0;

                if (!leftTriggerDown && !rightTriggerDown && joystickAxisValues[JOYSTICK_AXIS_XBOX360_TRIGGERS + axisOffset] >= 0.85f)
                {
                    leftTriggerDown = 1;
                    if (mode == LOOK)
                        XPLMCommandBegin(pushToTalkCommand);
                    else
                        XPLMCommandBegin(cwsOrDisconnectAutopilotCommand);
                }
                else if (!leftTriggerDown && !rightTriggerDown && joystickAxisValues[JOYSTICK_AXIS_XBOX360_TRIGGERS + axisOffset] <= 0.15f)
                    rightTriggerDown = 1;
                else if (leftTriggerDown && !rightTriggerDown && joystickAxisValues[JOYSTICK_AXIS_XBOX360_TRIGGERS + axisOffset] < 0.85f)
                {
                    leftTriggerDown = 0;
                    if (mode == LOOK)
                        XPLMCommandEnd(pushToTalkCommand);
                    else
                        XPLMCommandEnd(cwsOrDisconnectAutopilotCommand);
                }
                else if (!leftTriggerDown && rightTriggerDown && joystickAxisValues[JOYSTICK_AXIS_XBOX360_TRIGGERS + axisOffset] > 0.15f)
                    rightTriggerDown = 0;
#else
                if (joystickAxisValues[JOYSTICK_AXIS_XBOX360_LEFT_TRIGGER + axisOffset] >= 0.5f)
                {
                    if (mode == LOOK)
                        XPLMCommandBegin(pushToTalkCommand);
                    else
                        XPLMCommandBegin(cwsOrDisconnectAutopilotCommand);
                }
                else
                {
                    if (mode == LOOK)
                        XPLMCommandEnd(pushToTalkCommand);
                    else
                        XPLMCommandEnd(cwsOrDisconnectAutopilotCommand);
                }
#endif
            }

            if (mode == LOOK)
            {
                XPLMCommandEnd(XPLMFindCommand("sim/autopilot/servos_off_any"));

                const int viewType = XPLMGetDatai(viewTypeDataRef);

                if (viewType == VIEW_TYPE_3D_COCKPIT_COMMAND_LOOK)
                {
                    float deltaPsi = 0.0f, deltaThe = 0.0f;
                    const float viewSensitivityMultiplier = JOYSTICK_LOOK_SENSITIVITY * inElapsedSinceLastCall;

                    // turn head to the left
                    if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] < 0.5f - joystickPitchNullzone)
                    {
                        // normalize range [0.5, 0.0] to [0.0, 1.0]
                        const float d = Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)], 0.5f, 0.0f, 0.0f, 1.0f);

                        deltaPsi -= d * viewSensitivityMultiplier;
                    }
                    // turn head to the right
                    else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] > 0.5f + joystickPitchNullzone)
                    {
                        // normalize range [0.5, 1.0] to [0.0, 1.0]
                        const float d = Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)], 0.5f, 1.0f, 0.0f, 1.0f);

                        deltaPsi += d * viewSensitivityMultiplier;
                    }

                    // turn head upward
                    if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] < 0.5f - joystickPitchNullzone)
                    {
                        // normalize range [0.5, 0.0] to [0.0, 1.0]
                        const float d = Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 0.0f, 0.0f, 1.0f);

                        deltaThe += d * viewSensitivityMultiplier;
                    }
                    // turn head downward
                    else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] > 0.5f + joystickPitchNullzone)
                    {
                        // normalize range [0.5, 1.0] to [0.0, 1.0]
                        const float d = Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 1.0f, 0.0f, 1.0f);

                        deltaThe -= d * viewSensitivityMultiplier;
                    }

                    const float pilotsHeadPsi = XPLMGetDataf(pilotsHeadPsiDataRef);
                    const float pilotsHeadThe = XPLMGetDataf(pilotsHeadTheDataRef);

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
                        const float d = Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)], 0.5f, 0.0f, 0.0f, 1.0f);

                        // apply acceleration function (y = x^2) and round to integer
                        const int n = (int)(powf(2.0f * d, 2.0f) + 0.5f);

                        // apply the command
                        for (int i = 0; i < n; i++)
                            XPLMCommandOnce(XPLMFindCommand("sim/general/left"));
                    }
                    // move camera to the right
                    else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] > 0.5f + joystickPitchNullzone)
                    {
                        // normalize range [0.5, 1.0] to [0.0, 1.0]
                        const float d = Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)], 0.5f, 1.0f, 0.0f, 1.0f);

                        // apply acceleration function (y = x^2) and round to integer
                        const int n = (int)(powf(2.0f * d, 2.0f) + 0.5f);

                        // apply the command
                        for (int i = 0; i < n; i++)
                            XPLMCommandOnce(XPLMFindCommand("sim/general/right"));
                    }

                    // move camera up
                    if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] < 0.5f - joystickPitchNullzone)
                    {
                        // normalize range [0.5, 0.0] to [0.0, 1.0]
                        const float d = Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 0.0f, 0.0f, 1.0f);

                        // apply acceleration function (y = x^2) and round to integer
                        const int n = (int)(powf(2.0f * d, 2.0f) + 0.5f);

                        // apply the command
                        for (int i = 0; i < n; i++)
                            XPLMCommandOnce(XPLMFindCommand("sim/general/up"));
                    }
                    // move camera down
                    else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] > 0.5f + joystickPitchNullzone)
                    {
                        // normalize range [0.5, 1.0] to [0.0, 1.0]
                        const float d = Normalize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 1.0f, 0.0f, 1.0f);

                        // apply acceleration function (y = x^2) and round to integer
                        const int n = (int)(powf(2.0f * d, 2.0f) + 0.5f);

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
                    const float acfFeatheredPitch = XPLMGetDataf(acfFeatheredPitchDataRef);
                    const float acfRSCRedlinePrp = XPLMGetDataf(acfRSCRedlinePrpDataRef);
                    const float propRotationSpeedRadSecAll = XPLMGetDataf(propRotationSpeedRadSecAllDataRef);

                    // increase prop pitch
                    if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] < 0.5f - joystickPitchNullzone)
                    {
                        // normalize range [0.5, 0.0] to [acfFeatheredPitch, acfRSCRedlinePrp]
                        const float d = sensitivityMultiplier * Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 0.0f, acfFeatheredPitch, acfRSCRedlinePrp);

                        const float newPropRotationSpeedRadSecAll = propRotationSpeedRadSecAll + d;

                        // ensure we don't set values larger than redline
                        XPLMSetDataf(propRotationSpeedRadSecAllDataRef, newPropRotationSpeedRadSecAll < acfRSCRedlinePrp ? newPropRotationSpeedRadSecAll : acfRSCRedlinePrp);
                    }
                    // decrease prop pitch
                    else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] > 0.5f + joystickPitchNullzone)
                    {
                        // normalize range [0.5, 1.0] to [acfFeatheredPitch, acfRSCRedlinePrp]
                        const float d = sensitivityMultiplier * Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 1.0f, acfFeatheredPitch, acfRSCRedlinePrp);

                        const float newPropRotationSpeedRadSecAll = propRotationSpeedRadSecAll - d;

                        // ensure we don't set values smaller than feathered pitch
                        XPLMSetDataf(propRotationSpeedRadSecAllDataRef, newPropRotationSpeedRadSecAll > acfFeatheredPitch ? newPropRotationSpeedRadSecAll : acfFeatheredPitch);
                    }
                }
                else if (mode == MIXTURE)
                {
                    const float mixtureRatioAll = XPLMGetDataf(mixtureRatioAllDataRef);

                    // increase mixture setting
                    if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] < 0.5f - joystickPitchNullzone)
                    {
                        // normalize range [0.5, 0.0] to [0.0, 1.0]
                        const float d = sensitivityMultiplier * Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 0.0f, 0.0f, 1.0f);

                        const float newMixtureRatioAll = mixtureRatioAll + d;

                        // ensure we don't set values larger than 1.0
                        XPLMSetDataf(mixtureRatioAllDataRef, newMixtureRatioAll < 1.0f ? newMixtureRatioAll : 1.0f);
                    }
                    // decrease mixture setting
                    else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] > 0.5f + joystickPitchNullzone)
                    {
                        // normalize range [0.5, 1.0] to [0.0, 1.0]
                        const float d = sensitivityMultiplier * Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 1.0f, 0.0f, 1.0f);

                        const float newMixtureRatioAll = mixtureRatioAll - d;

                        // ensure we don't set values smaller than 0.0
                        XPLMSetDataf(mixtureRatioAllDataRef, newMixtureRatioAll > 0.0f ? newMixtureRatioAll : 0.0f);
                    }
                }
                else if (mode == COWL)
                {
                    float cowlFlapRatio[8];
                    XPLMGetDatavf(cowlFlapRatioDataRef, cowlFlapRatio, 0, acfNumEngines);

                    // decrease cowl flap setting
                    if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] < 0.5f - joystickPitchNullzone)
                    {
                        // normalize range [0.5, 0.0] to [0.0, 1.0]
                        const float d = sensitivityMultiplier * Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 0.0f, 0.0f, 1.0f);

                        // ensure we don't set values smaller than 0.0
                        for (int i = 0; i < acfNumEngines; i++)
                        {
                            const float newCowlFlapRatio = cowlFlapRatio[i] - d;
                            cowlFlapRatio[i] = newCowlFlapRatio > 0.0f ? newCowlFlapRatio : 0.0f;
                        }
                    }
                    // increase cowl flap setting
                    else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] > 0.5f + joystickPitchNullzone)
                    {
                        // normalize range [0.5, 1.0] to [0.0, 1.0]
                        const float d = sensitivityMultiplier * Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 1.0f, 0.0f, 1.0f);

                        // ensure we don't set values larger than 1.0
                        for (int i = 0; i < acfNumEngines; i++)
                        {
                            const float newCowlFlapRatio = cowlFlapRatio[i] + d;
                            cowlFlapRatio[i] = newCowlFlapRatio < 1.0f ? newCowlFlapRatio : 1.0f;
                        }
                    }

                    XPLMSetDatavf(cowlFlapRatioDataRef, cowlFlapRatio, 0, acfNumEngines);
                }
                else if (mode == MOUSE)
                {
                    int distX = 0, distY = 0;

                    // move mouse pointer left
                    if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] < 0.5f - joystickPitchNullzone)
                    {
                        // normalize range [0.5, 0.0] to [0.0, 1.0]
                        const float d = Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)], 0.5f, 0.0f, 0.0f, 1.0f);

                        // apply acceleration function (y = x^2)
                        distX -= (int)(powf(d * JOYSTICK_MOUSE_POINTER_SENSITIVITY, 2.0f) * inElapsedSinceLastCall);
                    }
                    // move mouse pointer right
                    else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] > 0.5f + joystickPitchNullzone)
                    {
                        // normalize range [0.5, 1.0] to [0.0, 1.0]
                        const float d = Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)], 0.5f, 1.0f, 0.0f, 1.0f);

                        // apply acceleration function (y = x^2)
                        distX += (int)(powf(d * JOYSTICK_MOUSE_POINTER_SENSITIVITY, 2.0f) * inElapsedSinceLastCall);
                    }

                    // move mouse pointer up
                    if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] < 0.5f - joystickPitchNullzone)
                    {
                        // normalize range [0.5, 0.0] to [0.0, 1.0]
                        const float d = Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 0.0f, 0.0f, 1.0f);

                        // apply acceleration function (y = x^2)
                        distY -= (int)(powf(d * JOYSTICK_MOUSE_POINTER_SENSITIVITY, 2.0f) * inElapsedSinceLastCall);
                    }
                    // move mouse pointer down
                    else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] > 0.5f + joystickPitchNullzone)
                    {
                        // normalize range [0.5, 1.0] to [0.0, 1.0]
                        const float d = Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 1.0f, 0.0f, 1.0f);

                        // apply acceleration function (y = x^2)
                        distY += (int)(powf(d * JOYSTICK_MOUSE_POINTER_SENSITIVITY, 2.0f) * inElapsedSinceLastCall);
                    }

                    // handle mouse pointer movement
#if LIN
                    MoveMousePointer(distX, distY, display);
#else
                    MoveMousePointer(distX, distY);
#endif
                }
                else
                {
                    if (helicopter && mode == DEFAULT)
                    {
                        float acfMinPitch[8];
                        XPLMGetDatavf(acfMinPitchDataRef, acfMinPitch, 0, 8);
                        float acfMaxPitch[8];
                        XPLMGetDatavf(acfMaxPitchDataRef, acfMaxPitch, 0, 8);
                        float propPitchDeg[8];
                        XPLMGetDatavf(propPitchDegDataRef, propPitchDeg, 0, acfNumEngines);

                        for (int i = 0; i < acfNumEngines; i++)
                        {
                            // increase prop pitch
                            if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] < 0.5f - joystickPitchNullzone)
                            {
                                // normalize range [0.5, 0.0] to [acfMinPitch, acfMaxPitch]
                                const float d = sensitivityMultiplier * Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 0.0f, acfMinPitch[i], acfMaxPitch[i]);

                                const float newPropPitchDeg = propPitchDeg[i] + d;

                                // ensure we don't set values larger than acfMaxPitch
                                propPitchDeg[i] = newPropPitchDeg < acfMaxPitch[i] ? newPropPitchDeg : acfMaxPitch[i];
                            }
                            // decrease prop pitch
                            else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] > 0.5f + joystickPitchNullzone)
                            {
                                // normalize range [0.5, 1.0] to [acfMinPitch, acfMaxPitch]
                                const float d = sensitivityMultiplier * Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 1.0f, acfMinPitch[i], acfMaxPitch[i]);

                                const float newPropPitchDeg = propPitchDeg[i] - d;

                                // ensure we don't set values smaller than acfMinPitch
                                propPitchDeg[i] = newPropPitchDeg > acfMinPitch[i] ? newPropPitchDeg : acfMinPitch[i];
                            }
                        }

                        XPLMSetDatavf(propPitchDegDataRef, propPitchDeg, 0, acfNumEngines);
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
                                if (FloatsEqual(speedbrakeRatio, -0.5f))
                                    speedbrakeRatio = 0.0f;

                                // normalize range [0.5, 0.0] to [0.0, 1.0]
                                const float d = sensitivityMultiplier * Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 0.0f, 0.0f, 1.0f);

                                const float newSpeedbrakeRatio = speedbrakeRatio - d;

                                // ensure we don't set values smaller than 0.0
                                XPLMSetDataf(speedbrakeRatioDataRef, newSpeedbrakeRatio > 0.0f ? newSpeedbrakeRatio : 0.0f);
                            }
                            // increase speedbrake ratio
                            else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] > 0.5f + joystickPitchNullzone)
                            {
                                // de-arm speedbrake if armed
                                if (FloatsEqual(speedbrakeRatio, -0.5f))
                                    speedbrakeRatio = 0.0f;

                                // normalize range [0.5, 1.0] to [0.0, 1.0]
                                const float d = sensitivityMultiplier * Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 1.0f, 0.0f, 1.0f);

                                const float newSpeedbrakeRatio = speedbrakeRatio + d;

                                // ensure we don't set values larger than 1.0
                                XPLMSetDataf(speedbrakeRatioDataRef, newSpeedbrakeRatio < 1.0f ? newSpeedbrakeRatio : 1.0f);
                            }
                        }
                        else
                        {
                            // increase throttle setting
                            if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] < 0.5f - joystickPitchNullzone)
                            {
                                // normalize range [0.5, 0.0] to [0.0, 1.0]
                                const float d = sensitivityMultiplier * Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 0.0f, 0.0f, 1.0f);

                                const XPLMDataRef throttleRatioDataRef = GetThrottleRatioDataRef();

                                const float newThrottleRatioAll = XPLMGetDataf(throttleRatioDataRef) + d;

                                // ensure we don't set values larger than 1.0
                                XPLMSetDataf(throttleRatioDataRef, newThrottleRatioAll < 1.0f ? newThrottleRatioAll : 1.0f);
                            }
                            // decrease throttle setting
                            else if (joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] > 0.5f + joystickPitchNullzone)
                            {
                                // normalize range [0.5, 1.0] to [0.0, 1.0]
                                const float d = sensitivityMultiplier * Exponentialize(joystickAxisValues[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)], 0.5f, 1.0f, 0.0f, 1.0f);

                                const XPLMDataRef throttleRatioDataRef = GetThrottleRatioDataRef();

                                const float newThrottleRatioAll = XPLMGetDataf(throttleRatioDataRef) - d;

                                float lowerThrottleBound;
                                if (thrustReverserMode)
                                {
                                    if (throttleRatioDataRef == throttleBetaRevRatioAllDataRef)
                                        lowerThrottleBound = -2.0f;
                                    else
                                        lowerThrottleBound = -1.0f;
                                }
                                else
                                    lowerThrottleBound = 0.0f;

                                // ensure we don't set values smaller than the lower throttle bound
                                XPLMSetDataf(throttleRatioDataRef, newThrottleRatioAll > lowerThrottleBound ? newThrottleRatioAll : lowerThrottleBound);
                            }
                        }
                    }
                }
            }
        }
    }

    return -1.0f;
}

inline static int FloatsEqual(float a, float b)
{
    return fabs(a - b) < FLT_EPSILON;
}

static XPLMDataRef GetThrottleRatioDataRef(void)
{
    if (XPLMGetDatai(acfHasBetaDataRef))
        return throttleBetaRevRatioAllDataRef;
    else if (XPLMGetDataf(acfThrotmaxREVDataRef) > 0.0f)
        return throttleJetRevRatioAllDataRef;
    else
        return throttleRatioAllDataRef;
}

static int inline GetKeyboardWidth(void)
{
    return KEY_BASE_SIZE * 17 + (int)(KEY_BASE_SIZE * 2.5f);
}

static XPLMCursorStatus HandleCursor(XPLMWindowID inWindowID, int x, int y, void *inRefcon)
{
    return xplm_CursorArrow;
}

static void HandleKey(XPLMWindowID inWindowID, char inKey, XPLMKeyFlags inFlags, char inVirtualKey, void *inRefcon, int losingFocus)
{
}

static void HandleKeyboardSelectorCommand(XPLMCommandPhase inPhase, KeyboardKey *newSelectedKey)
{
    if (keyPressActive)
        return;

    static float lastSelectorMovementTime = 0.0f;
    const float currentTime = XPLMGetElapsedTime();

    if (inPhase == xplm_CommandBegin || (inPhase == xplm_CommandContinue && currentTime - lastSelectorMovementTime > KEY_SELECTOR_MOVEMENT_MIN_ELAPSE_TIME))
    {
        selectedKey = newSelectedKey;
        lastSelectorMovementTime = currentTime;
    }
}

static int HandleMouseClick(XPLMWindowID inWindowID, int x, int y, XPLMMouseStatus inMouse, void *inRefcon)
{
    if (XPLMGetDatai(vrEnabledDataRef))
        return 0;

    static int lastX = -1, lastY = -1;

    switch (inMouse)
    {
    case xplm_MouseDrag:
        if (lastX > 0 && lastY > 0)
        {
            int *right, *bottom;
            if (inWindowID == indicatorsWindow)
            {
                right = &indicatorsRight;
                bottom = &indicatorsBottom;
            }
            else if (inWindowID == keyboardWindow)
            {
                right = &keyboardRight;
                bottom = &keyboardBottom;
            }
            else
                return 0;

            int left = 0, top = 0;
            XPLMGetWindowGeometry(inWindowID, &left, &top, right, bottom);

            int deltaX = x - lastX;
            int deltaY = y - lastY;

            left += deltaX;
            top += deltaY;
            *right += deltaX;
            *bottom += deltaY;

            FitGeometryWithinScreenBounds(&left, &top, right, bottom);
            XPLMSetWindowGeometry(inWindowID, left, top, *right, *bottom);
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

static int HandleMouseWheel(XPLMWindowID inWindowID, int x, int y, int wheel, int clicks, void *inRefcon)
{
    return 0;
}

static void HandleScrollCommand(XPLMCommandPhase phase, int clicks)
{
    static float lastScrollTime = 0.0f;
    float currentTime = XPLMGetElapsedTime();

    if (phase == xplm_CommandBegin || currentTime - lastScrollTime >= 0.1f)
    {
#if LIN
        Scroll(clicks, display);
#else
        Scroll(clicks);
#endif
        lastScrollTime = currentTime;
    }
}

static void HandleToggleMouseButtonCommand(XPLMCommandPhase phase, MouseButton button)
{
    if (phase != xplm_CommandContinue)
#if LIN
        ToggleMouseButton(button, phase == xplm_CommandBegin, display);
#else
        ToggleMouseButton(button, phase == xplm_CommandBegin);
#endif
}

static int Has2DPanel(void)
{
    char fileName[256], path[512];
    XPLMGetNthAircraftModel(0, fileName, path);

    int has2DPanel = 1;

    // search the '.acf' file for a special string which indicates that the aircraft shows the 3D cockpit object in the 2D forward panel view
    FILE *file = fopen(path, "r");
    if (file != NULL)
    {
        char temp[512];
        while (fgets(temp, 512, file) != NULL)
        {
            if ((strstr(temp, ACF_STRING_SHOW_COCKPIT_OBJECT_IN_2D_FORWARD_PANEL_VIEWS)) != NULL)
            {
                has2DPanel = 0;
                break;
            }
        }

        fclose(file);
    }

    return has2DPanel;
}

static KeyboardKey InitKeyboardKey(const char *label, int keyCode, float aspect, KeyPosition position)
{
    const int width = (int)(KEY_BASE_SIZE * aspect);
    const int labelOffsetX = (int)XPLMMeasureString(xplmFont_Basic, label, strlen(label)) / 2;

    KeyboardKey key = {strdup(label), keyCode, aspect, width, labelOffsetX, position, UP, 0.0f, NULL, NULL, NULL, NULL};
    return key;
}

static void InitShader(const char *fragmentShaderString, GLuint *program, GLuint *fragmentShader)
{
    *program = glCreateProgram();

    *fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(*fragmentShader, 1, &fragmentShaderString, 0);
    glCompileShader(*fragmentShader);
    glAttachShader(*program, *fragmentShader);
    GLint isFragmentShaderCompiled = GL_FALSE;
    glGetShaderiv(*fragmentShader, GL_COMPILE_STATUS, &isFragmentShaderCompiled);
    if (isFragmentShaderCompiled == GL_FALSE)
    {
        GLsizei maxLength = 2048;
        GLchar *log = new GLchar[maxLength];
        glGetShaderInfoLog(*fragmentShader, maxLength, &maxLength, log);
        XPLMDebugString(NAME ": The following error occured while compiling a fragment shader:\n");
        XPLMDebugString(log);
        delete[] log;

        CleanupShader(*program, *fragmentShader, 1);

        return;
    }

    glLinkProgram(*program);
    GLint isProgramLinked = GL_FALSE;
    glGetProgramiv(*program, GL_LINK_STATUS, &isProgramLinked);
    if (isProgramLinked == GL_FALSE)
    {
        GLsizei maxLength = 2048;
        GLchar *log = new GLchar[maxLength];
        glGetShaderInfoLog(*program, maxLength, &maxLength, log);
        XPLMDebugString(NAME ": The following error occured while linking a shader program:\n");
        XPLMDebugString(log);
        delete[] log;

        CleanupShader(*program, *fragmentShader, 1);

        return;
    }

    CleanupShader(*program, *fragmentShader, 0);
}

inline static int IsGliderWithSpeedbrakes(void)
{
    return XPLMGetDatai(acfNumEnginesDataRef) < 1 && XPLMGetDatai(acfSbrkEQDataRef);
}

static int IsHelicopter(void)
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

inline static int IsLockKey(KeyboardKey key)
{
    return key.keyCode == KEY_CODE_SCROLL || key.keyCode == KEY_CODE_NUMLOCK || key.keyCode == KEY_CODE_CAPITAL;
}

static int IsPluginEnabled(const char *pluginSignature)
{
    XPLMPluginID pluginId = XPLMFindPluginBySignature(pluginSignature);

    return XPLMIsPluginEnabled(pluginId);
}

static int KeyboardSelectorDownCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    HandleKeyboardSelectorCommand(inPhase, (*selectedKey).below);

    return 0;
}

static int KeyboardSelectorLeftCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    HandleKeyboardSelectorCommand(inPhase, (*selectedKey).left);

    return 0;
}

static int KeyboardSelectorRightCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    HandleKeyboardSelectorCommand(inPhase, (*selectedKey).right);

    return 0;
}

static int KeyboardSelectorUpCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    HandleKeyboardSelectorCommand(inPhase, (*selectedKey).above);

    return 0;
}

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
                controllerType = (ControllerType)v;
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
            else if (line.find("keyboardRight") != std::string::npos)
                iss >> keyboardRight;
            else if (line.find("keyboardBottom") != std::string::npos)
                iss >> keyboardBottom;
        }

        file.close();
    }
}

static int LockKeyboardKeyCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    if (!keyPressActive && inPhase == xplm_CommandBegin)
    {
        if ((*selectedKey).state == UP || (*selectedKey).state == NEW_UP)
            (*selectedKey).state = NEW_DOWN;
        else if ((*selectedKey).state == DOWN || (*selectedKey).state == NEW_DOWN)
            (*selectedKey).state = NEW_UP;
    }

    return 0;
}

static int LookModifierCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    int joystickAxisAssignments[100];
    XPLMGetDatavi(joystickAxisAssignmentsDataRef, joystickAxisAssignments, 0, 100);

    if (inPhase == xplm_CommandEnd)
    {
        if (mode == LOOK)
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
    }
    else if (mode == DEFAULT)
    {
        mode = LOOK;

        // unassign the left joystick's axis
        joystickAxisAssignments[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_X)] = AXIS_ASSIGNMENT_NONE;
        joystickAxisAssignments[AxisIndex(JOYSTICK_AXIS_ABSTRACT_LEFT_Y)] = AXIS_ASSIGNMENT_NONE;

        // store the default button assignments
        PushButtonAssignments();

        // assign panel scrolling controls to the dpad
        int joystickButtonAssignments[1600];
        XPLMGetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);

        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_LEFT)] = (std::size_t)XPLMFindCommand("sim/general/left");
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_RIGHT)] = (std::size_t)XPLMFindCommand("sim/general/right");
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_UP)] = (std::size_t)XPLMFindCommand("sim/general/up");
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_DOWN)] = (std::size_t)XPLMFindCommand("sim/general/down");
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_LEFT)] = (std::size_t)XPLMFindCommand("sim/general/rot_left");
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_RIGHT)] = (std::size_t)XPLMFindCommand("sim/general/rot_right");
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_UP)] = (std::size_t)XPLMFindCommand("sim/general/forward");
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_DOWN)] = (std::size_t)XPLMFindCommand("sim/general/backward");

        // assign push-to-talk controls
        if (controllerType == DS4)
            joystickButtonAssignments[JOYSTICK_BUTTON_DS4_L2 + buttonOffset] = (std::size_t)XPLMFindCommand(PUSH_TO_TALK_COMMAND);

        XPLMSetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);

        // temporarily gain exclusive camera controls
        OverrideCameraControls();
    }

    XPLMSetDatavi(joystickAxisAssignmentsDataRef, joystickAxisAssignments, 0, 100);

    return 0;
}

static void MakeInput(int keyCode, KeyState state)
{
#if IBM
    INPUT input[1];
    input[0].type = INPUT_KEYBOARD;
    input[0].ki.wScan = (WORD)keyCode;
    DWORD flags = KEYEVENTF_SCANCODE;
    if (state == UP)
        flags |= KEYEVENTF_KEYUP;
    input[0].ki.dwFlags = flags;
    SendInput((UINT)1, input, sizeof(INPUT));
#elif APL
    static CGEventSourceRef eventSource = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
    CGEventRef event = CGEventCreateKeyboardEvent(eventSource, (CGKeyCode)keyCode, state == DOWN);
    CGEventPost(kCGHIDEventTap, event);
    if (event != NULL)
        CFRelease(event);
#elif LIN
    if (display != NULL)
    {
        XTestFakeKeyEvent(display, keyCode, state == DOWN, CurrentTime);
        XFlush(display);
    }
#endif
}

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
        settingsWidget = XPCreateWidget(x, y, x2, y2, 1, NAME " Settings", 1, 0, xpWidgetClass_MainWindow);

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
        XPAddWidgetCallback(settingsWidget, (XPWidgetFunc_t)SettingsWidgetHandler);
    }
    else
    {
        // settings widget already created
        if (!XPIsWidgetVisible(settingsWidget))
            XPShowWidget(settingsWidget);
    }
}

static int MixtureControlModifierCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    ToggleMode(MIXTURE, inPhase);

    return 0;
}

static void MoveMousePointer(int distX, int distY, void *display)
{
    if (distX == 0 && distY == 0)
        return;

#if IBM
    INPUT input = {0};
    input.type = INPUT_MOUSE;
    input.mi.dx = (long)distX;
    input.mi.dy = (long)distY;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;

    SendInput((UINT)1, &input, sizeof(INPUT));
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
    for (int i = 0; i < (int)displayCount; i++)
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
        int minX = (int)screenBounds.origin.x;
        int minY = (int)screenBounds.origin.y;
        int maxX = (int)minX + screenBounds.size.width - 1;
        int maxY = (int)minY + screenBounds.size.height - 1;

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
        XWarpPointer((Display *)display, None, None, 0, 0, 0, 0, distX, distY);
        XFlush((Display *)display);
    }
#endif
}

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

static void OverrideCameraControls(void)
{
    // disable cinema verite if it is enabled and store its status
    lastCinemaVerite = XPLMGetDatai(cinemaVeriteDataRef);
    if (lastCinemaVerite)
        XPLMSetDatai(cinemaVeriteDataRef, 0);

    // enalbe the camera override of HeadShake
    overrideHeadShakePluginFailed = SetOverrideHeadShakePlugin(1);
}

static void PopButtonAssignments(void)
{
    if (pushedJoystickButtonAssignments != NULL)
    {
        XPLMSetDatavi(joystickButtonAssignmentsDataRef, pushedJoystickButtonAssignments, 0, 1600);
        free(pushedJoystickButtonAssignments);
        pushedJoystickButtonAssignments = NULL;
    }
}

static int PressKeyboardKeyCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    if (IsLockKey(*selectedKey))
        return LockKeyboardKeyCommand(inCommand, inPhase, inRefcon);

    if (inPhase == xplm_CommandBegin)
    {
        if ((*selectedKey).state == UP || (*selectedKey).state == NEW_UP)
            (*selectedKey).state = NEW_DOWN;
        else if ((*selectedKey).state == DOWN || (*selectedKey).state == NEW_DOWN)
            (*selectedKey).state = NEW_UP;

        keyPressActive = 1;
    }
    else if (inPhase == xplm_CommandEnd && ((*selectedKey).state == DOWN || (*selectedKey).state == NEW_DOWN))
    {
        (*selectedKey).state = NEW_UP;

        keyPressActive = 0;
    }
    return 0;
}

static int PropPitchOrThrottleModifierCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    ToggleMode(PROP, inPhase);

    return 0;
}

static void PushButtonAssignments(void)
{
    if (pushedJoystickButtonAssignments == NULL)
    {
        pushedJoystickButtonAssignments = (int *)malloc(sizeof(int) * 1600);
        XPLMGetDatavi(joystickButtonAssignmentsDataRef, pushedJoystickButtonAssignments, 0, 1600);
    }
}

static int PushToTalkCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    // only do push-to-talk if X-IvAp or XSquawkBox is enabled
    if (inPhase != xplm_CommandContinue && (IsPluginEnabled(X_IVAP_PLUGIN_SIGNATURE) || IsPluginEnabled(X_XSQUAWKBOX_PLUGIN_SIGNATURE)))
    {
        if (inPhase == xplm_CommandBegin)
            oKey.state = NEW_DOWN;
        else if (inPhase == xplm_CommandEnd)
            oKey.state = NEW_UP;
    }

    return 0;
}

static void ReleaseAllKeys(void)
{
    KeyboardKey **ptr = keyboardKeys;
    KeyboardKey **endPtr = keyboardKeys + sizeof(keyboardKeys) / sizeof(keyboardKeys[0]);
    while (ptr < endPtr)
    {
        if ((**ptr).state == NEW_UP || (**ptr).state == DOWN)
        {
            MakeInput((**ptr).keyCode, UP);
            (**ptr).state = UP;
        }
        ptr++;
    }
}

inline static void SyncLockKeyState(KeyboardKey *key)
{
#if IBM
    const int vk = MapVirtualKeyA((*key).keyCode, MAPVK_VSC_TO_VK);
    const int toggled = (GetKeyState(vk) & 1) != 0;
    (*key).state = toggled ? DOWN : UP;
#elif APL
// TODO
#elif LIN
// TODO
#endif
}

static int ResetSwitchViewCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    if (inPhase == xplm_CommandEnd)
    {
        if (mode == SWITCH_VIEW)
        {
            // restore the default button assignments
            PopButtonAssignments();

            mode = DEFAULT;
        }
    }
    else if (mode == DEFAULT)
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

            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_LEFT)] = (std::size_t)XPLMFindCommand("sim/view/chase");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_RIGHT)] = (std::size_t)XPLMFindCommand("sim/view/forward_with_hud");
            int has2DPanel = Has2DPanel();
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_UP)] = (std::size_t)XPLMFindCommand(has2DPanel ? "sim/view/forward_with_2d_panel" : "sim/view/3d_cockpit_cmnd_look");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_DOWN)] = (std::size_t)XPLMFindCommand(has2DPanel ? "sim/view/3d_cockpit_cmnd_look" : "sim/view/forward_with_2d_panel");

            XPLMSetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);
        }
    }

    return 0;
}

static void RestoreCameraControls(void)
{
    // restore cinema verite to its old status
    if (lastCinemaVerite)
        XPLMSetDatai(cinemaVeriteDataRef, 1);

    // disable the camera override of HeadhShake if we enabled it before
    if (!overrideHeadShakePluginFailed)
        SetOverrideHeadShakePlugin(0);
}

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
        file << "keyboardRight=" << keyboardRight << std::endl;
        file << "keyboardBottom=" << keyboardBottom << std::endl;

        file.close();
    }
}

static void Scroll(int clicks, void *display)
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
            XTestFakeButtonEvent((Display *)display, button, True, CurrentTime);
            XTestFakeButtonEvent((Display *)display, button, False, CurrentTime);
        }

        XFlush((Display *)display);
    }
#endif
}

static int ScrollDownCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    HandleScrollCommand(inPhase, -1);
    return 0;
}

static int ScrollUpCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    HandleScrollCommand(inPhase, 1);
    return 0;
}

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

        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_LEFT)] = (std::size_t)XPLMFindCommand("sim/flight_controls/flaps_up");
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_RIGHT)] = (std::size_t)XPLMFindCommand("sim/flight_controls/flaps_down");
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_UP)] = (std::size_t)XPLMFindCommand(TOGGLE_ARM_SPEED_BRAKE_OR_TOGGLE_CARB_HEAT_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_DOWN)] = (std::size_t)XPLMFindCommand("sim/flight_controls/landing_gear_toggle");
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_LEFT_UP)] = (std::size_t)XPLMFindCommand("sim/none/none");
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_LEFT_DOWN)] = (std::size_t)XPLMFindCommand("sim/none/none");
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_RIGHT_UP)] = (std::size_t)XPLMFindCommand("sim/none/none");
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_RIGHT_DOWN)] = (std::size_t)XPLMFindCommand("sim/none/none");
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_LEFT)] = (std::size_t)XPLMFindCommand(CYCLE_RESET_VIEW_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_RIGHT)] = (std::size_t)XPLMFindCommand(MIXTURE_CONTROL_MODIFIER_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_UP)] = (std::size_t)XPLMFindCommand(PROP_PITCH_THROTTLE_MODIFIER_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_DOWN)] = (std::size_t)XPLMFindCommand(COWL_FLAP_MODIFIER_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_CENTER_LEFT)] = (std::size_t)XPLMFindCommand(TOGGLE_REVERSE_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_CENTER_RIGHT)] = (std::size_t)XPLMFindCommand("sim/flight_controls/brakes_toggle_max");
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_BUMPER_LEFT)] = (std::size_t)XPLMFindCommand(TRIM_MODIFIER_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_BUMPER_RIGHT)] = (std::size_t)XPLMFindCommand(LOOK_MODIFIER_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_STICK_LEFT)] = (std::size_t)XPLMFindCommand("sim/general/zoom_out");
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_STICK_RIGHT)] = (std::size_t)XPLMFindCommand("sim/general/zoom_in");
        switch (controllerType)
        {
#if !IBM
        case XBOX360:
            joystickButtonAssignments[JOYSTICK_BUTTON_XBOX360_GUIDE + buttonOffset] = (std::size_t)XPLMFindCommand(TOGGLE_MOUSE_OR_KEYBOARD_CONTROL_COMMAND);
            break;
#endif
        case DS4:
            joystickButtonAssignments[JOYSTICK_BUTTON_DS4_PS + buttonOffset] = (std::size_t)XPLMFindCommand(TOGGLE_MOUSE_OR_KEYBOARD_CONTROL_COMMAND);
            joystickButtonAssignments[JOYSTICK_BUTTON_DS4_L2 + buttonOffset] = (std::size_t)XPLMFindCommand(CWS_OR_DISCONNECT_AUTOPILOT);
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

static int SetOverrideHeadShakePlugin(int overrideEnabled)
{
    if (IsPluginEnabled(HEAD_SHAKE_PLUGIN_SIGNATURE))
    {
        XPLMSetDatai(XPLMFindDataRef("simcoders/headshake/override"), overrideEnabled);

        return 0;
    }

    return 1;
}

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
        if (inParam1 == (long)xbox360ControllerRadioButton)
        {
            if ((int)XPGetWidgetProperty(xbox360ControllerRadioButton, xpProperty_ButtonState, 0))
            {
                StopConfiguration();
                controllerType = XBOX360;
                UpdateSettingsWidgets();
            }
        }
        else if (inParam1 == (long)dualShock4ControllerRadioButton)
        {
            if ((int)XPGetWidgetProperty(dualShock4ControllerRadioButton, xpProperty_ButtonState, 0))
            {
                StopConfiguration();
                controllerType = DS4;
                UpdateSettingsWidgets();
            }
        }
        else if (inParam1 == (long)showIndicatorsCheckbox)
        {
            showIndicators = (int)XPGetWidgetProperty(showIndicatorsCheckbox, xpProperty_ButtonState, 0);
            UpdateIndicatorsWindow();
        }
    }
    else if (inMessage == xpMsg_PushButtonPressed)
    {
        if (inParam1 == (long)startConfigurationtButton)
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

static void StopConfiguration(void)
{
    // if the user closes the widget while he is configuring a controller we need to set the aborted state to perform the cleanup
    if (configurationStep == AXES || configurationStep == BUTTONS)
        configurationStep = ABORT;
    else
        configurationStep = START;
}

inline static void SyncLockKeyState(KeyboardKey *key);
static int ToggleArmSpeedBrakeOrToggleCarbHeatCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
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
                float newSpeedbrakeRatio = FloatsEqual(oldSpeedbrakeRatio, -0.5f) ? 0.0f : -0.5f;
                XPLMSetDataf(speedbrakeRatioDataRef, newSpeedbrakeRatio);
                beginTime = FLT_MAX;
            }
        }
        else if (inPhase == xplm_CommandEnd)
        {
            // toggle speedbrake
            if (XPLMGetElapsedTime() - beginTime < BUTTON_LONG_PRESS_TIME && !FloatsEqual(beginTime, FLT_MAX))
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
            const int acfNumEngines = XPLMGetDatai(acfNumEnginesDataRef);

            if (acfNumEngines > 0)
            {
                float carbHeatRatio[8];
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

static void ToggleKeyboardControl(int vrEnabled)
{
    // if we are in mouse mode we actually want to toggle it off instead of toggling keyboard mode
    if (mode == MOUSE)
    {
        ToggleMouseControl();
        return;
    }

    if (mode == DEFAULT)
    {
        mode = KEYBOARD;

        // store the default button assignments
        PushButtonAssignments();

        // assign the keyboard selector and key commands
        int joystickButtonAssignments[1600];
        XPLMGetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);

        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_UP)] = (std::size_t)XPLMFindCommand(KEYBOARD_SELECTOR_UP_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_DOWN)] = (std::size_t)XPLMFindCommand(KEYBOARD_SELECTOR_DOWN_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_LEFT)] = (std::size_t)XPLMFindCommand(KEYBOARD_SELECTOR_LEFT_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_RIGHT)] = (std::size_t)XPLMFindCommand(KEYBOARD_SELECTOR_RIGHT_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_DOWN)] = (std::size_t)XPLMFindCommand(PRESS_KEYBOARD_KEY_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_RIGHT)] = (std::size_t)XPLMFindCommand(LOCK_KEYBOARD_KEY_COMMAND);

        XPLMSetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);

        if (vrEnabled == -1)
            vrEnabled = XPLMGetDatai(vrEnabledDataRef);

        if (keyboardWindow == NULL)
        {
            XPLMCreateWindow_t keyboardWindowParameters;
            keyboardWindowParameters.structSize = sizeof keyboardWindowParameters;
            keyboardWindowParameters.top = keyboardBottom + KEY_BASE_SIZE * 6;
            keyboardWindowParameters.left = keyboardRight - GetKeyboardWidth();
            keyboardWindowParameters.right = keyboardRight;
            keyboardWindowParameters.bottom = keyboardBottom;
            FitGeometryWithinScreenBounds(&keyboardWindowParameters.left, &keyboardWindowParameters.top, &keyboardWindowParameters.right, &keyboardWindowParameters.bottom);
            keyboardWindowParameters.visible = 1;
            keyboardWindowParameters.drawWindowFunc = DrawKeyboardWindow;
            keyboardWindowParameters.handleKeyFunc = HandleKey;
            keyboardWindowParameters.handleMouseClickFunc = HandleMouseClick;
            keyboardWindowParameters.handleCursorFunc = HandleCursor;
            keyboardWindowParameters.handleMouseWheelFunc = HandleMouseWheel;
            keyboardWindowParameters.decorateAsFloatingWindow = vrEnabled ? xplm_WindowDecorationRoundRectangle : xplm_WindowDecorationSelfDecorated;
            keyboardWindowParameters.layer = xplm_WindowLayerFloatingWindows;
            keyboardWindowParameters.handleRightClickFunc = HandleMouseClick;
            keyboardWindow = XPLMCreateWindowEx(&keyboardWindowParameters);

            XPLMSetWindowPositioningMode(keyboardWindow, vrEnabled ? xplm_WindowVR : xplm_WindowPositionFree, 0);
        }
        else
            XPLMSetWindowIsVisible(keyboardWindow, 1);
    }
    else if (mode == KEYBOARD && !keyPressActive)
        EndKeyboardMode();
}

static void ToggleMode(Mode m, XPLMCommandPhase phase)
{
    if (phase == xplm_CommandEnd)
    {
        if (mode == m)
            mode = DEFAULT;
    }
    else if (mode == DEFAULT)
        mode = m;
}

static void ToggleMouseButton(MouseButton button, int down, void *display)
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
    CGEventType mouseType;
    CGMouseButton mouseButton;
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
        XTestFakeButtonEvent((Display *)display, button == LEFT ? 1 : 3, !down ? False : True, CurrentTime);
        XFlush((Display *)display);
    }
#endif
}

static void ToggleMouseControl(void)
{
    // if we are in keyboard mode we actually want to toggle it off instead of toggling mouse mode
    if (mode == KEYBOARD)
    {
        ToggleKeyboardControl();
        return;
    }

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

        // assign the mouse button and scrolling commands
        int joystickButtonAssignments[1600];
        XPLMGetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);

        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_DOWN)] = (std::size_t)XPLMFindCommand(TOGGLE_LEFT_MOUSE_BUTTON_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_RIGHT)] = (std::size_t)XPLMFindCommand(TOGGLE_RIGHT_MOUSE_BUTTON_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_UP)] = (std::size_t)XPLMFindCommand(SCROLL_UP_COMMAND);
        joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_DOWN)] = (std::size_t)XPLMFindCommand(SCROLL_DOWN_COMMAND);

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

static int ToggleMouseOrKeyboardControlCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
#if LIN
    if (controllerType == Xbox360)
#else
    if (!hidInitialized)
#endif
    {
        static float beginTime = 0.0f;

        if (inPhase == xplm_CommandBegin)
            beginTime = XPLMGetElapsedTime();
        else if (inPhase == xplm_CommandContinue)
        {
            if (XPLMGetElapsedTime() - beginTime >= BUTTON_LONG_PRESS_TIME)
            {
                ToggleKeyboardControl();
                beginTime = FLT_MAX;
            }
        }
        else if (inPhase == xplm_CommandEnd)
        {
            if (XPLMGetElapsedTime() - beginTime < BUTTON_LONG_PRESS_TIME && !FloatsEqual(beginTime, FLT_MAX))
                ToggleMouseControl();

            beginTime = 0.0f;
        }
    }
    else if (inPhase == xplm_CommandBegin)
        ToggleKeyboardControl();

    return 0;
}

static int ToggleLeftMouseButtonCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    HandleToggleMouseButtonCommand(inPhase, LEFT);
    return 0;
}

static int ToggleReverseCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    if (inPhase == xplm_CommandBegin)
    {
        if (thrustReverserMode)
            XPLMSetDataf(throttleBetaRevRatioAllDataRef, 0.0f);
        else
        {
            if (XPLMGetDatai(acfHasBetaDataRef))
                // has beta
                XPLMSetDataf(throttleBetaRevRatioAllDataRef, THRUST_REVERSER_SETTING_ON_ENGAGEMENT);
            else if (XPLMGetDataf(acfThrotmaxREVDataRef) > 0.0f)
                // has thrust reverser
                XPLMSetDataf(throttleJetRevRatioAllDataRef, THRUST_REVERSER_SETTING_ON_ENGAGEMENT);
            else
                return 0;
        }

        thrustReverserMode = !thrustReverserMode;
    }

    return 0;
}

static int ToggleRightMouseButtonCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    HandleToggleMouseButtonCommand(inPhase, RIGHT);
    return 0;
}

static int TrimModifierCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
{
    if (inPhase == xplm_CommandEnd)
    {
        if (mode == TRIM)
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
    }
    else if (mode == DEFAULT)
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
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_LEFT)] = (std::size_t)XPLMFindCommand("sim/flight_controls/rudder_trim_left");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_RIGHT)] = (std::size_t)XPLMFindCommand("sim/flight_controls/rudder_trim_right");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_CENTER_LEFT)] = (std::size_t)XPLMFindCommand(TRIM_RESET_COMMAND);

            XPLMCommandBegin(XPLMFindCommand("AS350/Trim/Force_Trim"));
        }
        // custom handling for DreamFoil B407
        else if (IsPluginEnabled(DREAMFOIL_B407_PLUGIN_SIGNATURE))
        {
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_LEFT)] = (std::size_t)XPLMFindCommand("sim/flight_controls/rudder_trim_left");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_RIGHT)] = (std::size_t)XPLMFindCommand("sim/flight_controls/rudder_trim_right");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_CENTER_LEFT)] = (std::size_t)XPLMFindCommand(TRIM_RESET_COMMAND);

            XPLMCommandBegin(XPLMFindCommand("B407/flight_controls/force_trim"));
        }
        // custom handling for RotorSim EC135
        else if (IsPluginEnabled(ROTORSIM_EC135_PLUGIN_SIGNATURE))
        {
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_LEFT)] = (std::size_t)XPLMFindCommand("ec135/autopilot/beep_left");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_RIGHT)] = (std::size_t)XPLMFindCommand("ec135/autopilot/beep_right");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_UP)] = (std::size_t)XPLMFindCommand("ec135/autopilot/beep_fwd");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_DOWN)] = (std::size_t)XPLMFindCommand("ec135/autopilot/beep_aft");
        }
        // default handling
        else
        {
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_LEFT)] = (std::size_t)XPLMFindCommand("sim/flight_controls/aileron_trim_left");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_RIGHT)] = (std::size_t)XPLMFindCommand("sim/flight_controls/aileron_trim_right");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_UP)] = (std::size_t)XPLMFindCommand("sim/flight_controls/pitch_trim_down");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_DPAD_DOWN)] = (std::size_t)XPLMFindCommand("sim/flight_controls/pitch_trim_up");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_LEFT)] = (std::size_t)XPLMFindCommand("sim/flight_controls/rudder_trim_left");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_FACE_RIGHT)] = (std::size_t)XPLMFindCommand("sim/flight_controls/rudder_trim_right");
            joystickButtonAssignments[ButtonIndex(JOYSTICK_BUTTON_ABSTRACT_CENTER_LEFT)] = (std::size_t)XPLMFindCommand(TRIM_RESET_COMMAND);
        }

        XPLMSetDatavi(joystickButtonAssignmentsDataRef, joystickButtonAssignments, 0, 1600);
    }

    return 0;
}

static int TrimResetCommand(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon)
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

static void UpdateIndicatorsWindow(int vrEnabled)
{
    if (indicatorsWindow)
    {
        XPLMDestroyWindow(indicatorsWindow);
        indicatorsWindow = NULL;
    }

    const int acfNumEngines = XPLMGetDatai(acfNumEnginesDataRef);
    const int gliderWithSpeedbrakes = IsGliderWithSpeedbrakes();

    if (!showIndicators || (acfNumEngines < 1 && !gliderWithSpeedbrakes))
        return;

    numPropLevers = 0;
    numMixtureLevers = 0;
    if (!gliderWithSpeedbrakes)
    {
        const int helicopter = IsHelicopter();

        int acfPropType[8];
        XPLMGetDatavi(acfPropTypeDataRef, acfPropType, 0, 8);
        int acfEnType[8];
        XPLMGetDatavi(acfEnTypeDataRef, acfEnType, 0, 8);
        for (int i = 0; i < acfNumEngines; i++)
        {
            if (acfPropType[i] >= 1 && acfPropType[i] <= 3)
                numPropLevers++;

            if (acfEnType[i] < 2 || (acfEnType[i] == 2 && !helicopter) || acfEnType[i] == 8)
                numMixtureLevers++;
        }
    }

    int width = INDICATOR_LEVER_WIDTH;
    if (numPropLevers > 0)
        width += INDICATOR_LEVER_WIDTH;
    if (numMixtureLevers > 0)
        width += INDICATOR_LEVER_WIDTH;

    if (vrEnabled == -1)
        vrEnabled = XPLMGetDatai(vrEnabledDataRef);

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
    indicatorsWindowParameters.decorateAsFloatingWindow = vrEnabled ? xplm_WindowDecorationRoundRectangle : xplm_WindowDecorationSelfDecorated;
    indicatorsWindowParameters.layer = xplm_WindowLayerFlightOverlay;
    indicatorsWindowParameters.handleRightClickFunc = HandleMouseClick;
    indicatorsWindow = XPLMCreateWindowEx(&indicatorsWindowParameters);

    XPLMSetWindowPositioningMode(indicatorsWindow, vrEnabled ? xplm_WindowVR : xplm_WindowPositionFree, 0);
}

static void UpdateSettingsWidgets(void)
{
    XPSetWidgetProperty(xbox360ControllerRadioButton, xpProperty_ButtonState, (int)(controllerType == XBOX360));
    XPSetWidgetProperty(dualShock4ControllerRadioButton, xpProperty_ButtonState, (int)(controllerType == DS4));

    const char *configurationStatusString;
    switch (configurationStep)
    {
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
    case START:
    default:
        configurationStatusString = "Click 'Start Configuration' to configure X-Plane for the selected controller type.";
        break;
    }
    XPSetWidgetDescriptor(configurationStatusCaption, configurationStatusString);
    XPSetWidgetProperty(configurationStatusCaption, xpProperty_CaptionLit, configurationStep != START);

    XPSetWidgetDescriptor(startConfigurationtButton, configurationStep == AXES || configurationStep == BUTTONS ? "Abort Configuration" : "Start Configuration");

    XPSetWidgetProperty(showIndicatorsCheckbox, xpProperty_ButtonState, showIndicators);
}

inline static void WireKey(KeyboardKey *key, KeyboardKey *left, KeyboardKey *right, KeyboardKey *above, KeyboardKey *below)
{
    (*key).left = left;
    (*key).right = right;
    (*key).above = above;
    (*key).below = below;
}

static void WireKeys(void)
{
    WireKey(&escapeKey, &endKey, &f1Key, &leftControlKey, &graveKey);
    WireKey(&f1Key, &escapeKey, &f2Key, &leftWindowsKey, &d1Key);
    WireKey(&f2Key, &f1Key, &f3Key, &leftAltKey, &d2Key);
    WireKey(&f3Key, &f2Key, &f4Key, &spaceKey, &d3Key);
    WireKey(&f4Key, &f3Key, &f5Key, &spaceKey, &d4Key);
    WireKey(&f5Key, &f4Key, &f6Key, &spaceKey, &d5Key);
    WireKey(&f6Key, &f5Key, &f7Key, &spaceKey, &d6Key);
    WireKey(&f7Key, &f6Key, &f8Key, &rightAltKey, &d7Key);
    WireKey(&f8Key, &f7Key, &f9Key, &rightWindowsKey, &d8Key);
    WireKey(&f9Key, &f8Key, &f10Key, &appsKey, &d9Key);
    WireKey(&f10Key, &f9Key, &f11Key, &rightControlKey, &d0Key);
    WireKey(&f11Key, &f10Key, &f12Key, &upKey, &minusKey);
    WireKey(&f12Key, &f11Key, &sysRqKey, &downKey, &equalsKey);
    WireKey(&sysRqKey, &f12Key, &scrollKey, &downKey, &backKey);
    WireKey(&scrollKey, &sysRqKey, &pauseKey, &leftKey, &backKey);
    WireKey(&pauseKey, &scrollKey, &insertKey, &rightKey, &numLockKey);
    WireKey(&insertKey, &pauseKey, &deleteKey, &numpad0Key, &numLockKey);
    WireKey(&deleteKey, &insertKey, &homeKey, &numpadCommaKey, &divideKey);
    WireKey(&homeKey, &deleteKey, &endKey, &numpadCommaKey, &multiplyKey);
    WireKey(&endKey, &homeKey, &escapeKey, &numpadEnterKey, &subtractKey);
    WireKey(&graveKey, &subtractKey, &d1Key, &escapeKey, &tabKey);
    WireKey(&d1Key, &graveKey, &d2Key, &f1Key, &qKey);
    WireKey(&d2Key, &d1Key, &d3Key, &f2Key, &wKey);
    WireKey(&d3Key, &d2Key, &d4Key, &f3Key, &eKey);
    WireKey(&d4Key, &d3Key, &d5Key, &f4Key, &rKey);
    WireKey(&d5Key, &d4Key, &d6Key, &f5Key, &tKey);
    WireKey(&d6Key, &d5Key, &d7Key, &f6Key, &yKey);
    WireKey(&d7Key, &d6Key, &d8Key, &f7Key, &uKey);
    WireKey(&d8Key, &d7Key, &d9Key, &f8Key, &iKey);
    WireKey(&d9Key, &d8Key, &d0Key, &f9Key, &oKey);
    WireKey(&d0Key, &d9Key, &minusKey, &f10Key, &pKey);
    WireKey(&minusKey, &d0Key, &equalsKey, &f11Key, &leftBracketKey);
    WireKey(&equalsKey, &minusKey, &backKey, &f12Key, &rightBracketKey);
    WireKey(&backKey, &equalsKey, &numLockKey, &scrollKey, &backslashKey);
    WireKey(&numLockKey, &backKey, &divideKey, &insertKey, &numpad7Key);
    WireKey(&divideKey, &numLockKey, &multiplyKey, &deleteKey, &numpad8Key);
    WireKey(&multiplyKey, &divideKey, &subtractKey, &homeKey, &numpad9Key);
    WireKey(&subtractKey, &multiplyKey, &graveKey, &endKey, &addKey);
    WireKey(&tabKey, &addKey, &qKey, &graveKey, &captialKey);
    WireKey(&qKey, &tabKey, &wKey, &d1Key, &aKey);
    WireKey(&wKey, &qKey, &eKey, &d2Key, &aKey);
    WireKey(&eKey, &wKey, &rKey, &d3Key, &sKey);
    WireKey(&rKey, &eKey, &tKey, &d4Key, &dKey);
    WireKey(&tKey, &rKey, &yKey, &d5Key, &fKey);
    WireKey(&yKey, &tKey, &uKey, &d6Key, &gKey);
    WireKey(&uKey, &yKey, &iKey, &d7Key, &hKey);
    WireKey(&iKey, &uKey, &oKey, &d8Key, &jKey);
    WireKey(&oKey, &iKey, &pKey, &d9Key, &kKey);
    WireKey(&pKey, &oKey, &leftBracketKey, &d0Key, &lKey);
    WireKey(&leftBracketKey, &pKey, &rightBracketKey, &minusKey, &semicolonKey);
    WireKey(&rightBracketKey, &leftBracketKey, &backslashKey, &equalsKey, &apostropheKey);
    WireKey(&backslashKey, &rightBracketKey, &numpad7Key, &backKey, &returnKey);
    WireKey(&numpad7Key, &backslashKey, &numpad8Key, &numLockKey, &numpad4Key);
    WireKey(&numpad8Key, &numpad7Key, &numpad9Key, &divideKey, &numpad5Key);
    WireKey(&numpad9Key, &numpad8Key, &addKey, &multiplyKey, &numpad6Key);
    WireKey(&addKey, &numpad9Key, &tabKey, &subtractKey, &pageUpKey);
    WireKey(&captialKey, &pageUpKey, &aKey, &tabKey, &leftShiftKey);
    WireKey(&aKey, &captialKey, &sKey, &wKey, &zKey);
    WireKey(&sKey, &aKey, &dKey, &eKey, &xKey);
    WireKey(&dKey, &sKey, &fKey, &rKey, &cKey);
    WireKey(&fKey, &dKey, &gKey, &tKey, &vKey);
    WireKey(&gKey, &fKey, &hKey, &yKey, &bKey);
    WireKey(&hKey, &gKey, &jKey, &uKey, &nKey);
    WireKey(&jKey, &hKey, &kKey, &iKey, &mKey);
    WireKey(&kKey, &jKey, &lKey, &iKey, &commaKey);
    WireKey(&lKey, &kKey, &semicolonKey, &pKey, &periodKey);
    WireKey(&semicolonKey, &lKey, &apostropheKey, &leftBracketKey, &slashKey);
    WireKey(&apostropheKey, &semicolonKey, &returnKey, &rightBracketKey, &slashKey);
    WireKey(&returnKey, &apostropheKey, &numpad4Key, &backslashKey, &rightShiftKey);
    WireKey(&numpad4Key, &returnKey, &numpad5Key, &numpad7Key, &numpad1Key);
    WireKey(&numpad5Key, &numpad4Key, &numpad6Key, &numpad8Key, &numpad2Key);
    WireKey(&numpad6Key, &numpad5Key, &pageUpKey, &numpad9Key, &numpad3Key);
    WireKey(&pageUpKey, &numpad6Key, &captialKey, &addKey, &pageDownKey);
    WireKey(&leftShiftKey, &pageDownKey, &zKey, &captialKey, &leftWindowsKey);
    WireKey(&zKey, &leftShiftKey, &xKey, &aKey, &leftAltKey);
    WireKey(&xKey, &zKey, &cKey, &sKey, &spaceKey);
    WireKey(&cKey, &xKey, &vKey, &dKey, &spaceKey);
    WireKey(&vKey, &cKey, &bKey, &fKey, &spaceKey);
    WireKey(&bKey, &vKey, &nKey, &gKey, &spaceKey);
    WireKey(&nKey, &bKey, &mKey, &hKey, &rightAltKey);
    WireKey(&mKey, &nKey, &commaKey, &jKey, &rightWindowsKey);
    WireKey(&commaKey, &mKey, &periodKey, &kKey, &appsKey);
    WireKey(&periodKey, &commaKey, &slashKey, &lKey, &rightControlKey);
    WireKey(&slashKey, &periodKey, &rightShiftKey, &semicolonKey, &upKey);
    WireKey(&rightShiftKey, &slashKey, &numpad1Key, &returnKey, &leftKey);
    WireKey(&numpad1Key, &rightShiftKey, &numpad2Key, &numpad4Key, &numpad0Key);
    WireKey(&numpad2Key, &numpad1Key, &numpad3Key, &numpad5Key, &numpad0Key);
    WireKey(&numpad3Key, &numpad2Key, &pageDownKey, &numpad6Key, &numpadCommaKey);
    WireKey(&pageDownKey, &numpad3Key, &leftShiftKey, &pageUpKey, &numpadEnterKey);
    WireKey(&leftControlKey, &numpadEnterKey, &leftWindowsKey, &leftShiftKey, &escapeKey);
    WireKey(&leftWindowsKey, &leftControlKey, &leftAltKey, &leftShiftKey, &f1Key);
    WireKey(&leftAltKey, &leftWindowsKey, &spaceKey, &zKey, &f2Key);
    WireKey(&spaceKey, &leftAltKey, &rightAltKey, &cKey, &f5Key);
    WireKey(&rightAltKey, &spaceKey, &rightWindowsKey, &nKey, &f7Key);
    WireKey(&rightWindowsKey, &rightAltKey, &appsKey, &mKey, &f8Key);
    WireKey(&appsKey, &rightWindowsKey, &rightControlKey, &commaKey, &f9Key);
    WireKey(&rightControlKey, &appsKey, &upKey, &periodKey, &f10Key);
    WireKey(&upKey, &rightControlKey, &downKey, &slashKey, &f11Key);
    WireKey(&downKey, &upKey, &leftKey, &rightShiftKey, &f12Key);
    WireKey(&leftKey, &downKey, &rightKey, &rightShiftKey, &sysRqKey);
    WireKey(&rightKey, &leftKey, &numpad0Key, &rightShiftKey, &scrollKey);
    WireKey(&numpad0Key, &rightKey, &numpadCommaKey, &numpad2Key, &insertKey);
    WireKey(&numpadCommaKey, &numpad0Key, &numpadEnterKey, &numpad3Key, &homeKey);
    WireKey(&numpadEnterKey, &numpadCommaKey, &leftControlKey, &pageDownKey, &endKey);
}