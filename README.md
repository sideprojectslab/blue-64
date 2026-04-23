# BT-64

If you would like to get in touch with the developer, please join the SPL [Discord](https://discord.gg/gJsCgebkDw) server.

[FULL README (PDF)](https://raw.githubusercontent.com/sideprojectslab/blue-64/main/doc/README.pdf)

BT-64 is a plug & play bluetooth adapter for the Commodore 64 that plugs onto the keyboard header inside the computer and can control the keyboard lines and emulate keystrokes and joystick inputs. The goal of the project is to support two bluetooth controllers and a bluetooth keyboard simultaneously, in order to be able to controll the C64 completely wirelessly.

<figure align="center" id="blue_64_reg">
	<img src="doc/pictures/longboard_solo.png" width="70%">
	<figcaption>BT-64 "Regular" Board</figcaption>
</figure>

<figure align="center" id="blue_64_dev">
	<img src="doc/pictures/BT-64.png" width="60%">
	<figcaption>BT-64 Developer Board</figcaption>
</figure>

## Build Instructions

* Download ESP-IDF 5.5.4 from the following [link](https://github.com/espressif/esp-idf/releases/tag/v5.5.4) and install it by following the ["get-started"](https://docs.espressif.com/projects/esp-idf/en/v5.5.4/esp32/get-started/index.html) instructions
  * `sudo apt-get install git wget flex bison gperf python3 python3-pip python3-venv cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0`
  * Copy ESP-IDF to the desired location
  * CD inside ESP-IDF folder and run `install.sh`
* As explained in the guide, prior to installing ESP-IDF, set the ESP_IDF_TOOLS environment variable to the location where you want the various Espressif frameworks to be installed. A good way to make this permanent is to add the follwing line (or similar) to your ~/.profile file: `ESP_IDF_TOOLS="/tools/Espressif"` and log-out and log-in again.
*
