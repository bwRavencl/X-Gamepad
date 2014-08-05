##X-pad
=====

#####License Information:
GNU General Public License v2.0

#####Description:
X-pad adds extended support for gamepads to X-Plane.
It allows the user to control functions like throttle, prop pitch or mixture using the relative position of an axis.
In order to facilitate the control of a complex simulation like X-Plane by gamepad, extensive use of modifier keys is made by X-pad.
At this point the supported button and axis layout is very much dictated by X-pad based on the author's experience.

#####Instructions:
In order to install the plugin place the 'x_pad.xpl' file in your 'X-Plane 10/Resources/plugins' folder.
After installing the plugin you should start X-Plane and click X-pad's 'Set Default Assignments' menu entry to let X-pad set up X-Plane correctly.
Please refer to the section 'Default Mappings' below in order to see which changes will be made to your joystick configuration.
Furthermore it is recommended that you adjust the nullzone in X-Plane's 'Joystick & Equipment' screen to a value of 15%.

#####Disclaimer:
This plugin currently only supports DualShock 3 controllers connected via Bluetooth under Mac OS X.
Support for other controllers and operating systems may be added in the future.
Since this plugin was designed only for the author's personal usage, it is not expected to be robust and may not work with certain configurations, especially if there are multiple game-controllers connected.

#####Default Mappings:
The default mappings for a DualShock 3 controller:

| Axis                  | Control |
| --------------------- | ------- |
| Left Joystick X-Axis  | yaw     |
| Left Joystick Y-Axis  | none    |
| Right Joystick X-Axis | roll    |
| Right Joystick Y-Axis | pitch   |

| Button     | Command                                                  |
| ---------- | -------------------------------------------------------- |
| DPAD Left  | sim/flight_controls/flap_up                              |
| DPAD Right | sim/flight_controls/flap_down                            |
| DPAD Up    | x_pad/speed_brake_toggle_arm                             |
| DPAD Down  | sim/flight_controls/gear_toggle                          |
| Square     | x_pad/cycle_view                                         |
| Circle     | x_pad/mixture_control_modifier                           |
| Triangle   | x_pad/prop_pitch_modifier                                |
| Cross      | x_pad/cowl_flap_modifier                                 |
| Start      | sim/autopilot/servos_toggle                              | 
| Select     | sim/engines/thrust_reverse_toggle                        |
| L1         | x_pad/trim_modifier                                      |
| R1         | x_pad/view_modifier                                      |
| L2         | sim/none/none (use as Push To Talk Button in XSquawkBox) |
| R2         | sim/flight_controls/brakes_toggle_regular                |
| L3         | sim/general/zoom_out                                     |
| R3         | sim/general/zoom_in                                      |
| PS         | x_pad/toggle_mouse_pointer_control                       |
