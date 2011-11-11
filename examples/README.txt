-*- outline -*-

* INFORMATIONS
Pulsar come from
http://paulbourke.net/miscellaneous/stereographics/stereorender/

I addapted it to support stereo vsync mode.


* CONFIGURATION
Disable window manager effects and/or disable Composite in
/etc/X11/xorg.conf by appending:

Section "Extensions"
    Option         "Composite" "Disable"
EndSection

Compositor prevent to display at 120Hz.


* RUN IN VSYNC STEREO MODE
To force VSYNC (tested whith NVIDIA drivers):
export __GL_SYNC_DISPLAY_DEVICE=DFP-0
export __GL_SYNC_TO_VBLANK=1

Then run pulsar -s 1 to try if your graphic card support opengl
stereo. or pulsar -s 2 to generate vsynced stereo.

If needed press e key to switch eyes.
