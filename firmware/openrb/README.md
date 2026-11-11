# firmware/openrb

Firmware for the OpenRB-150 used with `kingspider-ros`.

- `openRB.bin`: firmware image to write
- `openRB.ino`: its source (Arduino IDE 1.8.19, board `OpenRB-150` samd 0.2.1, library `Dynamixel2Arduino` 0.6.3)

## Write

```bash
BOSSAC="$HOME/.arduino15/packages/arduino/tools/bossac/1.7.0-arduino3/bossac"
PORT=/dev/ttyACM0
```

Press the RESET button twice quickly to enter the bootloader mode, then:

```bash
"$BOSSAC" --port="$PORT" -U true -i -e -w -v openRB.bin -R
```
