Potentiometer Driver
====================

This App allows sending potentiometer values to a remote node. This is
currently used to change colors of the led-strip app but the resource
used and the IP address are configurable -- so we can use it for any
other destination.

The app sends its value to the remote only if the value has changed. In
addition it has a retransmit interval (in seconds) that can retransmit
the value after a timeout if the value has not changed. Setting this
retransmit interval to 0 will turn off the retransmit feature. Note that
we sample the value only every second: We don't want to use up the whole
bandwidth for this app alone.
