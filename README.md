# ci40-motion-sensor-upnp-cp
A multi sensor CI40 project for triggering and streaming music to multiple wireless speakers around the home.

- The CI40 detects an interacts with wireless (Caskeid supported) speakers using a UPNP control point.
- 6LoWPAN remote clickers running Contiki attached with motion sensors are associated with the speakers.
- When a clicker detects motion the CI40 is alerted and the associated speaker is triggered to play.
- Upon a set timer elapsing the speaker is muted unless further motion is detected whereby the timer is reset.

# Requirements
- CI40
- At least one 6LoWPAN Clicker
- Wifi network
- At least one Caskeid UPNP supported speaker such as a Jongo: http://www.pure.com/wireless-speakers/jongo-multiroom-speakers
- PIC32

Project contains CI40 source code and dependancies as well as contiki project source for the remote clicker applications.
