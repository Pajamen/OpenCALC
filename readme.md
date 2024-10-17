# OpenCALC
Opensource Atmel based "Science-Fiction" calculator. 

![alt text](https://github.com/Pajamen/OpenCALC/blob/testing/foto_beta.jpg?raw=true)

Sorry for my english, it's not my native language and it's rather rusty.

My first "bigger" project with Atmel/Arduino system.

Idea is my own scientific calculator, which removes need for "syntax error", because formula is watched over by system already while typing.
Hardware peripherals are 4x5 matrix keyboard, 16x2 I2C Hitachi LCD display, green and red status LEDs (working/ready).
Testing hardware board is Arduino NANO for simplicity.

Main features:
- irregular keys disabled while typing equation (cannot type multiplication right after division, etc ...)
- bracket watchdog (shows how deep level of brackets is reached)
- live 'slowmotion computing' (equation is computed step by step with ability to print process on display and reduce computing speed)
- user-defined functions (separated Function button with menu and choice of multiple mathematic/esoteric/fun functions)
- upgradable/modifyable/structured computing system based on cycles and separated functions
- having fun with something as simple as calculus

Required libraries:
- LiquidCrystal_I2C V 1.12 by Frank de Brabander
- Keypad V 3.1.1 by Mark Stanley, Alexander Brevig

How does (or should) it work:
- Heart of project are two arrays
 - OPRBAFR - here are saved operators, brackers etc. Numbers are saved as char "N" and empty slots are "X"
 - NUMBAFR - under same index as number "N" in OPRBAFR is saved numerical value.
- example:

OPRBAFR:    N+N-(N*(-N))
NUMBAFR:    203004000500
EQUATION:   2+3-(4*(-5))

- Program automatically rejects input that will cause syntax error (multiple numbers or operators next to each other etc.)
-When "=" is pressed and equation should be solvable (no unclosed brackets, equation doesn't end with operator and so on), program starts rigorous "computing cycle" with solving and cleaning functions until one number is left as "ANS".

DEBUG systems:
- CYCLECHECK - shows how many cycles did it take to compute equation. If >99, system automatically fails to slowmo computing for bugfixing purposes.
- OPRBAFRCHECK - show raw OPRBAFR in top row
- NUMBAFRCHECK - show raw NUMBAFR