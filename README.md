# Blue-64
Blue-64 is a plug & play bluetooth adapter for the Commodore 64 that plugs onto the keyboard header inside the computer and can control the keyboard lines and emulate keystrokes and joystick inputs. The goal of the project is to support two bluetooth controllers and a bluetooth keyboard simultaneously, in order to be able to controll the C64 completely wirelessly.

If you would like to get in touch with the developer, please join the SPL discord server at this link:
https://discord.gg/gJsCgebkDw

![](https://github.com/sideprojectslab/blue-64/blob/main/doc/pictures/Blue-64.png)

### Additional functions
Blue-64 has an internal menu that is visualized by printing text on screen. The menu allows typing and executing frequently used macros like loading the tape, loading and/or running programs from disk drive etc. as well as selecting keyboard mapping and remapping unused controller buttons.

| Function                       | Button(s)   |
|:---:                           |:---:        |
| Swap Player 1-2                | View + Y    |
| Cycle Menu (controller)        | View + A/B  |
| Select/Run Menu (controller)   | View + Menu |
| Cycle Menu (keyboard)          | AltGr + Up/Down/Left/Right Arrow  |
| Select/Run Menu (keyboard)     | AltGr + Enter |

## Button Mapping (xInput)
| Joystick     | Controller  | Controller Alt. |
|:---:         |:---:        |:---:            |
| UP           | D-Pad UP    | Button B        |
| DOWN         | D-Pad DOWN  | Button X        |
| LEFT         | D-Pad LEFT  | Left Analog     |
| RIGHT        | D-Pad RIGHT | Left Analog     |
| FIRE         | Button A    | -               |

Controller buttons not mentioned in the table above can be mapped to emulate any keyboard key through the on-screen menu. Default Controller-Keyboard mappings are reported below:

| Keyboard     | Controller  |
|:---:         |:---:        |
| SPACE        | Button Menu |
| F1           | Button Y    |

## C64 Compatibility
Blue-64 has been verified to be compatible with the following motherboard revisions:

| Motherboard      | Notes   |
|:---:             |:---:    |
| 250407  | - |
| 250425  | - |
| 250466  | - |
| 326298  | - |
| ku14194 | - |
| 250469  | Needs a special adapter and the female<br>header is mounted upside down |

![](https://github.com/sideprojectslab/blue-64/blob/main/doc/pictures/Blue-64-adpt.JPG)

## Device Support
Blue-64 is based on the bluepad32 library from Ricardo Quesada. As such it supports the exact same devices (bluetooth controllers & keyboards) as bluepad32. A list of supported devices can be found in the "Supported Controllers" section at the following link:
https://github.com/ricardoquesada/bluepad32?tab=readme-ov-file

## Keyboard Support
At the moment Blue-64 only supports Bluetooth-Low-Energy keyboards (BLE) and only supports the English US layout. The key mapping can be switched between "symbolic" and "positional (vice)" through the on-screen menu. With "symbolic" mapping the keys on the bluetooth keyboard do exactly what they say, including their "shift" function (if applicable) with a few exceptions:

| En-US Key (symbolic) | C64 Key     |
|:---:                 |:---:        |
| ~                    | Arrow Left  |
| \                    | Arrow Up    |
| 6+shift              | £           |
| tab / shift+tab      | Stop / Run  |
| esc                  | Restore     |
| delete, f12          | Clear       |
| home, f9             | home        |
| insert, f10          | insert      |
| start (windows)      | Commodore   |


## Limitations
Blue-64 can only interact with the lines present on the keyboard header, thus it has no access to the "paddle" control lines. Therefore it cannot emulate the Commodore mouse, paddle controls, and does not support additional joystick fire buttons (other than the primary one) that are based on paddle control.

## Installation & Precautions
Most notably, particular care shall be used when plugging the Blue-64 onto the motherboard, as the female connector on the Blue-64 has no alignment key and thus won't prevent incorrect installation. Always install the Blue-64 with the computer turned off and verify carefully that the connection to the motherboard header is properly aligned.

A step-by-steb video guide on how to assemble and install the Blue-64 board is available at the following link:
https://youtu.be/yE2yL8-_jRQ?si=YmAZqs-F023yzRW5
Users shall follow these instructions carefully and fully understand the circuit's limitations before installing and/or using it.

Incorrect installation of the board supply or failure to comply with the recommended operating conditions may result in damage to the board and/or to the computer, with risk of overheating, fire and/or explosion.

## Firmware Update
Firmware binaries can be found in the "Releases" section of the GitHub page. The three files in the "binaries.zip" folder are necessary to perform a firmware update:
- bootloader.bin
- partition-table.bin
- blue-64-app.bin

Download and install the CP210X Universal Windows Drivers for the on-board programmer at this website:
https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers?tab=downloads

Download the Espressif Flash Download Tool at this website:
https://www.espressif.com/en/support/download/other-tools

Run the .exe application and select "ESP32" as target and "Develop" as work mode when prompted. In the following screen upload the three binary files in the order at the addresses shown in the screenshot below:

![](https://github.com/sideprojectslab/blue-64/blob/main/doc/pictures/flash_tool.png)

Connect the PC to the USB port on the Blue-64 and select the appropriate COM port in the Flash Download Tool. Press the "erase" button to erase the FLASH and finally press "start" and the new firmware will be downloaded to the board (should take less than a minute).

In case the method above does not work, please try the online tool at this website:
https://espressif.github.io/esptool-js/

Don't worry you can't brick it (as far as I know), if something fails you will always be able to re-try flashing the new firmware.

## TO-DOs
[] BT Controllers show the correct port/player indicator
[] Configurable autofire (controller only)

## License
License information is included on top of all software source files as well as in all schematics. Files that do not contain explicit licensing information are subject to the licensing terms stated in the LICENSE.txt provided in the main project folder:

Unless stated otherwise in individual files, all hardware design Schematics, Bill of Materials, Gerber files and manuals are licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International. To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-sa/4.0/

Unless otherwise stated in individual files, all software source files are Licensed under the Apache License, Version 2.0. You may obtain a copy of this license at http://www.apache.org/licenses/LICENSE-2.0

## Disclaimer
All material is provided on an 'AS IS' BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND in accordance to the license deed applicable to
each individual file.
