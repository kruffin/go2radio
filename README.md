## What is it?
The ODroid Go Advance is a handheld that runs a linux operating system. The image I am running is a version of Ubuntu 20.04. HardKernel makes the device: https://www.hardkernel.com/shop/odroid-go-advance-black-edition/. It's great for playing emulated game systems and I thought it would be even better if I could listen to the radio on it :)

This repo is an attempt to make something a little nicer than the hacked up script I run now that only lets me play one radio station without ssh'ing into the device to alter what plays.

This program should minimally:

- [X] Allow a FM radio station to be played.
- [X] Allow the changing of the station using the controller.
- [X] Allow changing the volume. Shout out to (https://github.com/zfteam/hotkey_for_oga); stole their trick to alter the volume.
- [X] Show what station is playing.
- [X] Lower the backlight while idle to reduce battery use.

![Go 2 Radio](/docs/go2radio_web.jpg)

## External Requirements
This also requires some RTL-SDR USB device to plug into the Go Advance. I'm using one from Adafruit from here: https://www.adafruit.com/product/1497 There are tons of these things around, so that particular one isn't a requirement; but you must have one.

## Configure RTL-SDR
The USB dongle can't be used by the odroid user by default; it can only be accessed with root privileges. The first part of this involves a set of udev rules from the `docs/rtl-sdr.rules` file. Copy these into `/ect/udev/rules.d/` folder. Note that all of the rules allow access if a user is in the `plugdev` group. So let's add the odroid user to that group:

     sudo usermod -a -G plugdev odroid

Now you should be able to access and see the USB device as the odroid user.

**Important:** You'll need to reboot the odroid to have access.

## Build

### Prereqs
This assumes a `sudo apt update` has been run recently.

These packages are required to build this program:

    sudo apt install git build-essential

These packages are required to build the ngsoftfm program:

    sudo apt install cmake pkg-config libusb-1.0-0-dev libasound2-dev libboost-all-dev librtlsdr-dev libhackrf-dev libairspy-dev libbladerf-dev

### Get the code
Pull down the submodules.

    git submodule update --init --recursive

This requires the libgo2 library from: https://github.com/OtherCrashOverride/libgo2 It should already be on the ODroid Go Advance.

The program consists of two main parts, the UI application and the softfm application it controls.

### Build go2radio
To build the UI application run:

    make

### Build NGSoftFM
To build the softfm application detailed instructions are inside the lib/ngsoftfm/ directory including additional packages. The program expects the softfm program to be in lib/ngsoftfm/build/.

Simplified instructions are here:

    cd lib/ngsoftfm
    mkdir build
    cd build
    cmake ..
    make

## Install

Clone the repo and copy over to the go advance (or clone directly on the go advance). Do the build.

Now there are two options to get it into the carousel to run:

1. An additional program within the settings gear menu.
2. Add another system into the carousel just for this and potentially other programs.

### Option 1 - Existing Settings Gear

Put a shell script in /opt/system/ called something like `go2radio.sh` with the path to the built go2radio binary.

    /home/odroid/src/go2radio/go2radio

Make the script executable:

    chmod +x /opt/system/go2radio.sh

You can now run it from the gear menu like the network app and others.

### Option 2 - Add another Gear
This option is really only necessary if you want to alter how the programs run. Some emulators turn the performance to max, run itself and then turn it back to normal. I'm playing with this to see if it fixes the audio blips that randomly occur, but isn't normally necessary.

Make a copy of the /etc/emulationstation/es_systems.cfg

    cp /etc/emulationstation/es_systems.cfg /home/odroid/.emulationstation/.

Then add an additional system at the end still inside the `<systemList> </systemList>` tags.

    <system>
            <name>retropie</name>
            <fullname>Apps</fullname>
            <path>/home/odroid/apps</path>
            <extension>.sh</extension>
            <command>perfmax;%ROM%;perfnorm</command>
            <platform></platform>
Make a copy of the /etc/emulationstation/es_systems.cfg

    cp /etc/emulationstation/es_systems.cfg /home/odroid/.emulationstation/.

Then add an additional system at the end still inside the `<systemList> </systemList>` tags.

    <system>
            <name>retropie</name>
