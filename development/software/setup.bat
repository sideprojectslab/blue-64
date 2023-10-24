REM echo off

if not exist "setup_local.bat" (
	REM creating default setup file
	echo echo off>> setup_local.bat
	echo set ESP32_COM_PORT=COM12>> setup_local.bat
	echo set OCD_INTF_CFG=interface\digilent-hs2.cfg>> setup_local.bat
REM	echo set OCD_TARGET_CFG=board\esp-wroom-32.cfg>> setup_local.bat
	echo set OCD_TARGET_CFG=board\esp32-wrover.cfg>> setup_local.bat
REM echo set OCD_TARGET_CFG=board\esp32-wrover-kit-3.3v.cfg>> setup_local.bat
	echo set IDF_PATH=%%IDF_TOOLS_PATH%%\frameworks\esp-idf-v5.1.1>> setup_local.bat
	echo %%IDF_PATH%%\export.bat>> setup_local.bat
)

call setup_local.bat
