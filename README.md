##X-pad

#####License Information:
GNU General Public License v2.0

#####Description:
X-pad adds extended support for gamepads to X-Plane.

For example it allows the user to control functions like throttle, prop pitch or mixture using the relative position of an axis.

In order to facilitate the control of a complex simulation like X-Plane by gamepad, extensive use of modifier keys is made by X-pad.

#####Instructions:
In order to install the plugin, place the 'x_pad' folder in your 'X-Plane 10/Resources/plugins' folder.
After installing the plugin you should start X-Plane and open X-pad's 'Settings' window via the corresponding menu entry in X-Plane's 'Plugins' menu.

In the settings menu select wether you are using a DualShock 3 or Xbox 360 controller.

If you have more than one controller connected you have to adjust the 'Axis Offset' and 'Button Offset' values as well (this is only relevant if you have multiple controllers connected):
- 'Axis Offset' specifies the number of axis that appear in X-Plane's 'Joystick & Equipment' window before the first axis of your DualShock 3 or Xbox 360 controller.
- 'Button Offset' specifies the number of buttons of other portentially connected controllers that have a lower ID than the first button of your DualShock or Xbox 360 controller.

Finally press the 'Set Default Assignments' menu entry to let X-pad set up X-Plane correctly for your controller.
Please refer to the section 'Default Mappings' below in order to see which changes will be made to your joystick configuration.
Also note that X-Plane's nullzone setting will be set to a value of 15% and the sensitivity of the yaw axis to fully linear.

When closing the 'Settings' window all settings will be stored to a configuration file automatically.

#####Disclaimer:
This plugin currently supports both DualShock 3 and Xbox 360 controllers.

When using Linux and a DualShock 3 controller you will need either a patched build of BlueZ (https://iwilcox.me.uk/2012/sixaxis-ubuntu, https://launchpad.net/~nadia-xy/+archive/ubuntu/ppa) or QtSixA (http://qtsixa.sourceforge.net) to pair your controller with your computer.

#####Default Mappings:
Default mappings for a DualShock 3 controller:

| Axis                  | Control               |
| --------------------- | ----------------------|
| Left Joystick X-Axis  | Yaw                   |
| Left Joystick Y-Axis  | Throttle / Collective |
| Right Joystick X-Axis | Roll                  |
| Right Joystick Y-Axis | Pitch                 |

| Button     | Command                             |
| ---------- | ------------------------------------|
| DPAD Left  | Flaps up                            |
| DPAD Right | Flaps down                          |
| DPAD Up    | SpeedBrake / Carb. Heat             |
| DPAD Down  | Gear                                |
| Square     | Reset view / View modifier          |
| Circle     | Mixture modifier                    |
| Triangle   | Prop modifier / Thorttle modifier   |
| Cross      | Cowl flap modifier                  |
| Start      | Autopilot / Disable Flight Director |
| Select     | Beta / Reverser                     |
| L1         | Trim modifier                       |
| R1         | View modifier                       |
| L2         | Brakes                              |
| R2         | Push-to-Talk (mapped to 'O' key)    |
| L3         | Zoom out                            |
| R3         | Zoom in                             |
| PS         | Mouse mode                          |


Default mappings for an Xbox 360 controller:

| Axis                  | Control                                 |
| --------------------- | --------------------------------------- |
| Left Joystick X-Axis  | Yaw                                     |
| Left Joystick Y-Axis  | Throttle / Collective                   |
| Right Joystick X-Axis | Roll                                    |
| Right Joystick Y-Axis | Pitch                                   |
| Left Trigger          | Brake                                   |
| Right Trigger         | None / Push-to-Talk (mapped to 'O' key) |

| Button       | Command                             |
| -------------| ------------------------------------|
| DPAD Left    | Flaps up                            |
| DPAD Right   | Flaps down                          |
| DPAD Up      | SpeedBrake / Carb. Heat             |
| DPAD Down    | Gear                                |
| X            | Cycle view / Reset view             |
| B            | Mixture modifier                    |
| Y            | Prop modifier / Thorttle modifier   |
| A            | Cowl flap modifier                  |
| Start        | Autopilot / Disable Flight Director |
| Back         | Beta / Reverser                     |
| Left Bumper  | Trim modifier                       |
| Right Bumper | View modifier                       |
| Left Stick   | Zoom out                            |
| Right Stick  | Zoom in                             |
| Guide        | Mouse mode                          |

