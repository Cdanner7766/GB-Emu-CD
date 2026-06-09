# GB-Emu-CD

A Game Boy (DMG) emulator for the Raspberry Pi Pico, displayed on a 2.2" ILI9225 TFT LCD screen. Based on the [Pico-GB project by YouMakeTech](https://www.youmaketech.com/pico-gb-gameboy-emulator-handheld-for-raspberry-pi-pico/), which uses the [Peanut-GB emulator core](https://github.com/deltabeard/Peanut-GB).

The ROM is compiled directly into the firmware — no SD card required.

---

## Hardware

| Component | Part |
|-----------|------|
| Microcontroller | Raspberry Pi Pico (RP2040) |
| Display | HiLetgo 2.2" ILI9225 176x220 TFT LCD |
| Audio (optional) | MAX98357A I2S amplifier + 2W 8Ω speaker |

### Pin Connections

| Signal | Pico GPIO |
|--------|-----------|
| LCD CS | 17 |
| LCD CLK | 18 |
| LCD SDI | 19 |
| LCD RS | 20 |
| LCD RST | 21 |
| LCD LED | 22 |
| Up | 2 |
| Down | 3 |
| Left | 4 |
| Right | 5 |
| A | 6 |
| B | 7 |
| Select | 8 |
| Start | 9 |
| Audio DIN | 26 |
| Audio BCLK | 27 |
| Audio LRCLK | 28 |

---

## Projects

### Emulator (`SRC/Pico-GB-20250207/`)

The main Game Boy emulator. Key settings at the top of `src/main.c`:

```c
#define ENABLE_LCD    1   // Enable display output
#define ENABLE_SOUND  0   // Enable i2s audio (requires MAX98357A)
#define ENABLE_SDCARD 0   // SD card ROM selector (disabled — ROM is baked in)
```

The CPU is overclocked to 266 MHz for performance.

### Diagnostic (`SRC/diagnostic/`)

A standalone test program that verifies each hardware subsystem in sequence:

1. **Colour flash** — fills the screen red, green, blue to confirm LCD and SPI are working
2. **LCD** — confirms display is alive
3. **SD card** — attempts to mount the SD card filesystem (expected to fail if SD pins are not connected)
4. **Buttons** — live monitor showing PRESSED for each button as you press it

Flash `GB_DIAG.uf2` instead of the emulator firmware to run diagnostics.

---

## Changing the ROM

The ROM is baked into the firmware at build time using `make_rom.py`.

**1. Place your `.gb` ROM file somewhere accessible** (must be an original DMG Game Boy ROM — GBC games are not supported).

**2. Run the ROM converter** from the repo root:

```powershell
python make_rom.py "C:\path\to\your\game.gb"
```

This generates `SRC/Pico-GB-20250207/src/rom.c`.

**3. Rebuild the firmware** (see Recompiling below).

**4. Flash the new `RP2040_GB.uf2`** to the Pico.

---

## Recompiling

### Requirements

- [Raspberry Pi Pico SDK v1.5.1](https://github.com/raspberrypi/pico-setup-windows/releases/latest) (includes compiler, CMake, and Ninja)
- Python 3 (included with the SDK installer)
- Visual Studio (for the Developer Command Prompt) or the Pico Developer Command Prompt

### Build the emulator

Open the **Pico Developer Command Prompt** or **VS Developer Command Prompt** and run:

```cmd
cd SRC\Pico-GB-20250207
mkdir build
cd build
cmake -G "Ninja" ..
ninja
```

Output: `build\RP2040_GB.uf2`

On subsequent builds (after changing source files or the ROM) you only need to run `ninja` from inside the `build` folder — no need to re-run `cmake`.

### Build the diagnostic

```cmd
cd SRC\diagnostic
mkdir build
cd build
cmake -G "Ninja" ..
ninja
```

Output: `build\GB_DIAG.uf2`

### Flashing to the Pico

1. Hold the **BOOTSEL** button on the Pico
2. Plug in the USB cable while holding BOOTSEL
3. A drive called **RPI-RP2** appears in File Explorer
4. Drag and drop the `.uf2` file onto the drive
5. The Pico reboots automatically and runs the new firmware

---

## Hotkeys (in-game)

| Combo | Action |
|-------|--------|
| Select + A | Toggle frame skip (fast-forward) |
| Select + Up / Down | Adjust volume (when audio enabled) |
