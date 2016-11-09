Arduino compatibility example
=============================

This example shows that it is now possible to re-use arduino sketches in
Contiki. This example documents the necessary magic. Arduino specifies
two routines, `setup` and `loop`. Before `setup` is called, the
framework initializes hardware. In original Arduino, all this is done in
a `main` function (in C). For contiki we define a process that does the
same.

See the documentation file in apps/contiki-compat/README.md

We use the Serial1 Port to write the Tag Info out
We use the SPI Bus to connect the RFID reader to the merkurboard
https://github.com/miguelbalboa/rfid
