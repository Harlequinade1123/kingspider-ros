# firmware/openrb

Firmware for the OpenRB-150 used with `kingspider-ros`.

| File | Description |
|---|---|
| `kingspider.bin` | Flash image read back from the robot with `bossac -r` (the reference) |
| `openRB.ino` | Source reconstructed by decompiling `kingspider.bin` (the original source was lost) |

`openRB.ino` is **not** bit-identical to `kingspider.bin`: building it gives a different binary
(33,052 bytes vs. 32,704 bytes of code). Use `kingspider.bin` for the known-good behaviour.

To build `openRB.ino`: Arduino IDE 1.8.19, board `OpenRB-150` (samd 0.2.1), library `Dynamixel2Arduino` 0.7.0.

SHA-256 of `kingspider.bin`: `97c9ec84dd0ef452faa6fb2da82e92139d8e43ce2affe0fdb3429cfcdc916dde`

## Copy the firmware to another OpenRB-150 (Ubuntu)

```bash
BOSSAC="$HOME/.arduino15/packages/arduino/tools/bossac/1.7.0-arduino3/bossac"
PORT=/dev/ttyACM0
```

Press the RESET button twice quickly to enter the bootloader mode, then:

```bash
# write kingspider.bin to the target board (connect only the target board)
"$BOSSAC" --port="$PORT" -U true -i -e -w -v kingspider.bin -R

# read back the program of an existing board (does not erase it)
"$BOSSAC" --port="$PORT" -i
"$BOSSAC" --port="$PORT" -r backup.bin
```
