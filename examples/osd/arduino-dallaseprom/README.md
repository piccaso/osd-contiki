Arduino compatibility example
=============================

This example shows that it is now possible to re-use arduino sketches in
Contiki. This example documents the necessary magic. Arduino specifies
two routines, `setup` and `loop`. Before `setup` is called, the
framework initializes hardware. In original Arduino, all this is done in
a `main` function (in C). For contiki we define a process that does the
same.

DallasEPROM
===========

Arduino library for Dallas 1-Wire (E)EPROMs
https://github.com/pceric/DallasEPROM


See the documentation file in apps/contiki-compat/README.md

Build and Flash Merkurboard 256
===============================

make clean TARGET=osd-merkur-256 flash

