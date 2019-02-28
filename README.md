SX1231 Output Data Serializer Library
=====================================
This library provides functionality to use a Semtech SX1231/RFM69 Radio module
to transmit a string of bits at a given rate. The library uses SPI to
communicate with the module. The modules buffering is used to make sure the
transmitted signal has accurate timing.

NOTE: This code is a mashup between parts of ser4010 and rf_pkt_drv. I mostly
wrote it because I already had most of the code, and to see if this idea would
work. I haven't tested it much in practice.

Usage
-----
The tools/ directory provides some example programs on how to use the library
from C.

To use the library from a higher level language, ie. Python, the sx1231_raw can
be used. sx1231_raw takes the configuration as arguments and the data should be
provided on stdin as a hex encoded string followed by a newline or end-of-file.
Multiple frames can be transmitted by providing multiple lines.

Compiling the Software
----------------------
Run the following to compile the libsx1231_ods library and tools:

    # cd build
    # cmake ../
    # make

The software uses /dev/spidev0.0 by default as SPI interface. If your module is
connected to a different interface you can change the default by using the
following cmake command instead of the above one:

    # cmake ../ -DDEFAULT_DEV_PATH=<your spi interface>

By default the PA connected to the PA_BOOST pin is used to transmit. This
works for the commonly used RFM69H(W/CW) high power modules. If you use a low
power RFM69(W/CW) module, then use the following cmake command instead:

    # cmake ../ -DWITH_PA1_DEFAULT=OFF
