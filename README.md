## What is it?
The ODroid Go Advance is a handheld that runs a linux operating system. The image I am running is a version of Ubuntu 20.04. HardKernel makes the device: https://www.hardkernel.com/shop/odroid-go-advance-black-edition/. It's great for playing emulated game systems and I thought it would be even better if I could listen to the radio on it :)

This repo is an attempt to make something a little nicer than the hacked up script I run now that only lets me play one radio station without ssh'ing into the device to alter what plays.

This program should minimally:

[ ] Allow a FM radio station to be played.
[ ] Allow the changing of the station using the controller.
[ ] Allow changing the volume.
[ ] Show what station is playing.

## External Requirements
This also requires some RTL-SDR USB device to plug into the Go Advance. I'm using one from Adafruit from here: https://www.adafruit.com/product/1497 There are tons of these things around, so that particular one isn't a requirement; but you must have one.

## Configure RTL-SDR
The USB dongle can't be used by the odroid user by default; it can only be accessed with root privileges. The first part of this involves a set of udev rules from the `docs/rtl-sdr.rules` file. Copy these into `/ect/udev/rules.d/` folder. Note that all of the rules allow access if a user is in the `plugdev` group. So let's add the odroid user to that group:

     sudo usermod -a -G plugdev odroid

Now you should be able to access and see the USB device as the odroid user.

## Build

Pull down the submodules.

    git submodule update --init --recursive

This requires the libgo2 library from: https://github.com/OtherCrashOverride/libgo2 It should already be on the ODroid Go Advance.

Run:

    make

## Install

Put a shell script in /opt/system/ called something like `go2radio.sh` with the path to the built go2radio binary.

    /home/odroid/src/go2radio/go2radio

Make the script executable:

    chmod +x /opt/system/go2radio.sh


