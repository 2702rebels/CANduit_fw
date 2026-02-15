arduino-cli compile --fqbn esp32:esp32:esp32s3 --build-path ./build
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32s3 --input-dir ./build
