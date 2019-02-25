## X-Gamepad

#### License Information:
GNU General Public License v2.0

#### Description:
X-Gamepad adds extended support for gamepads to X-Plane.  
For example it allows the user to control functions like throttle, prop pitch or mixture using the relative position of an axis.  
In order to facilitate the control of a complex simulation like X-Plane by gamepad, extensive use of modifier keys is made by X-Gamepad.

#### Instructions:
- In order to install the plugin, place the 'x_gamepad' folder in your 'X-Plane 11/Resources/plugins' folder.
- After installing the plugin you should start X-Plane and open X-Gamepad's 'Settings' window via the corresponding menu entry in X-Plane's 'Plugins' menu.
- In the settings menu select wether you are using an Xbox 360 or DualShock 4 controller.
- Click the 'Start Configuration' button and follow the instructions on the screen to let X-Gamepad set up X-Plane correctly for your controller.

Please note that X-Plane's nullzone setting will be set to a value of 15% and the sensitivity of the main control axes to 100%.

#### Building:
X-Gamepad uses the CMake build system.  
For example, to build on Windows with Visual Studio 2017 cd to X-Gamepad's project directory and run:
<pre>
mkdir build
cd build
cmake -G "Visual Studio 15 2017 Win64" ..
msbuild /p:Configuration=Release
</pre>

#### Default Mappings:
Default mappings for an Xbox 360 controller:

| Axis                  | Control                |
| --------------------- | ---------------------- |
| Left Joystick X-Axis  | Yaw                    |
| Left Joystick Y-Axis  | Throttle / Collective  |
| Right Joystick X-Axis | Roll                   |
| Right Joystick Y-Axis | Pitch                  |
| Left Trigger          | Control wheel steering |
| Right Trigger         | Brake                  |

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

Default mappings for a DualShock 4 controller:

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
| L2         | Control wheel steering              |
| R2         | Brake                               |
| L3         | Zoom out                            |
| R3         | Zoom in                             |
| PS         | Mouse mode                          |
