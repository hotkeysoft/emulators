# TODO

## General

- Dynamic connections between components
- Adjust CPU speed
- Improve clock sync between components, use dynamic ratios
- Sync is based on audio only at the moment
- Dynamic hardware selection

### Floppy controller
- Write
- Floppy change
- Write protect

### Serial/RS-232
- 8250 
- Serial mouse

### Joystick
- Everything

## PC

### Keyboard
- BIOS Function KBD_RESET(F9E2)
- Diagnostics

### POST failures:
- KEYBOARD TEST

- Interrupt during REP

## PCjr

### POST failures:
- PC-DOS boot fail: command.com memory error

## Graphics

### General
- Window dynamic resize on mode changes
- Proper aspect ratio for all modes

### CGA
- Mostly done, need tests
- Composite mode
- Graphic glitches (ex: load game SQ1)

### MDA
- Mostly done, need tests

### PCjr
- Mostly done, need tests

### TGA
- Mostly done, need tests

### EGA
- Everything

## Sound

### PC Speaker
- Mostly done, need tests
- Samples work in World Class Leader Board golf and Gauntlet 2

### SN76496 (PCjr/Tandy)
- Switches in the 8255
- Some glitches

### General
- Proper mixing
- Filtering

## Tandy
- Base model mostly done, some issues
- DMA for FDC (comes with RAM extension)
- Need to implement 1000HX to support 720K floppies

## UI

### Main Window
- Floppy indicators
- Change cartridge (PCjr)
- Adjust resolution/scaling
- Frame rate indicator

### Debugger
- Logging when using debugger
- Breakpoints
- Performance
- Improve disassembly

