![http://bdremote-ng.googlecode.com/svn/trunk/doc/remote-small.jpg](http://bdremote-ng.googlecode.com/svn/trunk/doc/remote-small.jpg)

# Introduction #

This is helper application for Sony BD Bluetooth Remote Control, which
allows to use this remote with LIRC.

This is a partial rewrite of bdremote 0.2, so it is easier to understand and maintain. Also, I could not get bdremote 0.2 to work on ubuntu. And it was not the most verbose application, so this one was created instead. Also, porting to a non-bluez bluetooth stack should be possible now. Bluetooth and LIRC/network handing has been separated.

Why use this? Because using a PS3 remote as a HID device makes little sense to me (You have to instruct bluez to connect to it before you can use it. So it means that you need to have a script which continuously tries to connect to the remote. And you have to make the ps3 remote discoverable before the script will succeed.)

# Compilation and installation #

For next instructions I assume that you already have installed
and properly configured Linux bluetooth stack
(at least `bluez-libs`, `libbluetooth-dev` and `bluez-utils` on a Debian system).

Make sure that you have [CMake](http://www.cmake.org) installed.

Execute gen.sh in the build directory. This will run [CMake](http://www.cmake.org), which will generate a build in build/linux. One of the [CMake](http://www.cmake.org) options for in the script is `-DENABLE_REPEAT:BOOL=OFF`. Set this to ON, if you want to use repeating keys.

Type "make" in this directory and you'll get executable named "bdremoteng".
Copy this binary somewhere, for example to /usr/local/sbin.

# Usage #

At first you will have to obtain address of your remote.
Type in console "hciutil scan", then press "Start" and "Enter" buttons on
remote simultaneously and hold for 5 seconds
you will see something like:
```
hostname # hcitool scan
Scanning ...
        00:19:C1:5A:F1:3F       BD Remote Control
hostname #
```
"00:19:C1:5A:F1:3F" is HW address of your remote.

You need to pair your remote with your PC first. This could be done using the bluez tools, like this:
```
hidd --connect 00:19:C1:5A:F1:3F
```

Then start "bdremoteng" as:
```
bdremoteng -a 00:19:C1:5A:F1:3F -n -d
```
(it should write some informative messages about what it is doing and not detach from the terminal it was started from)

you can give extra command-line parameters to "bdremoteng"
```
	-p <port>            Set port number for incoming LIRCD connections.
	-t <timeout>         Set disconnect timeout for BD remote (in seconds).
	-i <address>         BT address of interface to use.
	-a <address>         BT address of remote.
	                     For example: -a 00:19:C1:5A:F1:3F. 
	-b <script>          Execute <script> when battery info changes.
	                     Arguments: <prev charge> <current charge>, both in percent.
	-r <rate>            Key repeat rate. Generate <rate> repeats per second.
	-e <num>             Wait <num> ms before repeating a key.
	-R <suffix>          Auto-generate release events with appended <suffix>.
	-l                   Follow LIRC namespace for the key names.
	-u <username>        Change UID to the UID of this user.
	-g <group>           Change GID to the GID of this group.
	-f <filename>        Write log to <filename>.
	-d                   Enable debug.
	-n                   Don't fork daemon to background.
	-h, --help           Display help.
```

This daemon acts as a remote LIRC server.
In order to use remote with lirc you have to start LIRCD as follows:
```
lircd -H null --connect 127.0.0.1:8888
```

Keep in mind, if you manually compile lircd, you don't
need to compile extra drivers, if you are going to use
lircd only with this remote.

lircd will report remote name as "SonyBDRemote"
( you need this only if you are going to use additional remote
devices and write sophisticated .lircrc files. )

For list of defined keys look in "keys.txt" file.

NOTE: Connection with remote will be established only after you press
some key on remote for first time after start of daemon and it can
take some time, so result of first key press will be slightly delayed.

# Battery change #

This application lets you use the option `-b <script>` to call `<script>` when battery charge changes. `<script>` will be given two arguments: `<prev> <now>`, where `<prev>` is the previous change and `<now>` is the current charge. Both in percent.

Example script used to receive battery change notifications.

```
#!/bin/bash

PREV=$1
NOW=$2

/usr/bin/logger "PS3 remote battery status change: from $PREV% to $NOW%."
```

The above script logs the charge to syslog.

# Reporting Bugs #

When reporting bugs, please provide the following information.

  * SVN version or release information.
  * Output of running `build/gen.sh`.
  * Log files, produced by giving the following arguments to bdremoteng: `-d -f /tmp/bdremote.log`.
  * Core dumps, if applicable.
  * Details about how to reproduce the problem you are experiencing.

# Notes #

  * Disable bluetooth auth and encryption. This application will not work with those enabled. Adding "` auth disable; encrypt disable; `" to `/etc/bluetooth/hcid.conf` might do the trick.

  * Start this application before you start bluez daemons. See [initscripts](initscripts.md).

  * The amount of time, before the connection to the remote is closed is per default set to `60 seconds`. This is far too low, according to Peter Atso. One should use `1800 seconds` instead. The original driver times out after `60 minutes`.