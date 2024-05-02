
SET dst=%~dp0src

RMDIR /S /Q %dst%\components\btstack

cd ..\..\resources/bluepad32/external/btstack/port/esp32

SET src=%~dp0

SET IDF_PATH=%dst%

python integrate_btstack.py

pause