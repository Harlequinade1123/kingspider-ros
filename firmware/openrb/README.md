# firmware/openrb

Firmware for the OpenRB-150 used with `kingspider-ros`.

| File | Description |
|---|---|
| `openRB.ino` | Source, reconstructed from `kingspider.bin` (the original source was lost) |
| `kingspider.bin` | Flash image read back from the robot with `bossac -r` |

## Build

`openRB.ino` reproduces `kingspider.bin` exactly with:

- Arduino IDE 1.8.19
- board `OpenRB-150` (samd 0.2.1)
- library `Dynamixel2Arduino` **0.6.3** (other versions, including 0.7.0, give a different binary)

The build is 32,684 bytes. `bossac` pads the last 64-byte flash page with zeros and the rest of the flash is 0xFF,
so the read-back image equals the build plus this padding:

```bash
python3 - <<'PY'
n = open('openRB.ino.bin', 'rb').read()   # exported/compiled binary
o = open('kingspider.bin', 'rb').read()
img = n + b'\0' * (-len(n) % 64)
print(img + b'\xff' * (len(o) - len(img)) == o)   # True
PY
```

The generated code depends on the details of the source (statement order, types and names of global variables).
Do not change them unless you intend to change the binary.

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
