<!--
SPDX-FileCopyrightText: 2025 David Jolly <jolly.a.david@gmail.com>
SPDX-License-Identifier: MIT
-->

[![License: MIT](https://shields.io/badge/license-MIT-blue.svg?style=flat)](LICENSES/MIT.txt) [![Build Status](https://builds.sr.ht/~dajolly/cgbl.svg)](https://builds.sr.ht/~dajolly/cgbl?)

![CGBL](docs/cgbl.png)

## Install

```bash
git clone https://git.sr.ht/~dajolly/cgbl
cd cgbl
make
sudo make install
```

```bash
# The default client uses the SDL3 library
# To use a different library, redefine CLIENT
# Supported client types
#  sdl2: SDL2 client
#  sdl3: SDL3 client
make CLIENT=client
# The default installation directory is /usr/local
# To install into a different directory, redefine PREFIX
sudo make install PREFIX=/your/path
```

## Usage

```
Usage: cgbl [options] [file]

Options:
   -d, --debug       Enable debug mode
   -f, --fullscreen  Set window fullscreen
   -h, --help        Show help information
   -s, --scale       Set window scale
   -v, --version     Show version information
```

```bash
# To launch with a rom, run the following command
cgbl rom.gbc
# To launch with debug mode enabled, run the following command
cgbl -d rom.gbc
# To launch with a fullscreen window, run the following command
cgbl -f rom.gbc
# To launch with a scaled window, run the following command
cgbl -s scale rom.gbc
```

## Debugger

```
Options:
   exit                        Exit debug console
   cart                        Display cartridge information
   clkl                        Latch clock
   clkr   clk                  Read data from clock
   clkw   clk data             Write data to clock
   dasm   dasm addr [off]      Disassemble instructions
   help                        Display help information
   itr    int                  Interrupt bus
   memr   addr [off]           Read data from memory
   memw   addr data [off]      Write data to memory
   proc                        Display processor information
   regr   reg                  Read data from register
   regw   reg data             Write data to register
   rst                         Reset bus
   run    [bp]                 Run to breakpoint
   step   [bp]                 Step to next instruction
   ver                         Display version information
```

```bash
# Supported clock types
#  sec: Seconds
#  min: Minutes
#  hr: Hours
#  dayl: Day (LSB)
#  dayh: Day (MSB)
clk=sec|min|hr|dayl|dayh
# Supported interrupt types
#  vblk: Vblank interrupt
#  lcdc: Screen interrupt
#  tmr: Timer interrupt
#  ser: Serial interrupt
#  joy: Input interrupt
int=vblk|lcdc|tmr|ser|joy
# Supported register types
#  a/af/f: AF registers
#  b/bc/c: BC registers
#  d/de/e: DE registers
#  h/hl/l: HL registers
#  pc: Program-counter register
#  sp: Stack-pointer register
reg=a|af|b|bc|c|d|de|e|f|h|hl|l|pc|sp
```

## Keybindings

|Button |Key        |Controller|
|:------|:----------|:---------|
|A      |X          |A         |
|B      |Z          |B         |
|Select |C          |Back      |
|Start  |Space      |Start     |
|Right  |Right-Arrow|Right-Dpad|
|Left   |Left-Arrow |Left-Dpad |
|Up     |Up-Arrow   |Up-Dpad   |
|Down   |Down-Arrow |Down-Dpad |

## Mappers

|Id   |Type                                       |Description         |
|:----|:------------------------------------------|:-------------------|
|0,8-9|[MBC0](https://gbdev.io/pandocs/nombc.html)|32KB ROM/8KB RAM    |
|1-3  |[MBC1](https://gbdev.io/pandocs/MBC1.html) |2MB ROM/32KB RAM    |
|5-6  |[MBC2](https://gbdev.io/pandocs/MBC2.html) |256KB ROM/512B RAM  |
|15-19|[MBC3](https://gbdev.io/pandocs/MBC3.html) |2MB ROM/32KB RAM/RTC|
|25-30|[MBC5](https://gbdev.io/pandocs/MBC5.html) |8MB ROM/128KB RAM   |

## License

Copyright (C) 2025 David Jolly <jolly.a.david@gmail.com>. Released under the [MIT License](LICENSES/MIT.txt).
