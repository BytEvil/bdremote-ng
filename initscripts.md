# Introduction #

The scripts on this page are meant as examples only. _You_ are supposed to adjust them to your own system/distribution.

# Older Ubuntu Systems (pre 10.04) #
Using the following init script for bedremote-ng seems to work on older Ubuntu systems.

```
#!/bin/bash
#
#

BDREMOTED=bdremoteng

BDREMOTE_ARGS="-i 00:11:22:33:44:55 -a 00:66:77:88:99:00 -u user -g group -p 8888 -t 600 -f /tmp/bdremote.log -r10 -e800 -d" 

case "$1" in
    start)
        /etc/init.d/bluetooth stop
        echo -n "Starting bdremote daemon: "
        /bin/sleep 1
        start-stop-daemon --start --quiet --exec "$BDREMOTED" -- $BDREMOTE_ARGS
        /etc/init.d/bluetooth start
        echo
        ;;
    stop)
        echo -n "Stopping bdremote daemon: "
        start-stop-daemon --stop --quiet --exec "$BDREMOTED" 
        echo
        ;;
    restart)
        $0 stop
        $0 start
        ;;
    *)
        echo "Usage: /etc/init.d/bdremote {start|stop|restart}"
        exit 1
esac

exit 0
```

Notice that bluetooth service is stopped before executing `bdremoteng`. The order of services which needs to be started is as follows.

  * Bluetooth (initialize bluetooth devices).
  * bdremote (bind to bluetooth device, it needs to be initialized).
  * LIRC (connect to bdremoteng).

The following from an ubuntu system seems to work.

```
S80bluetooth -> ../init.d/bluetooth
S95bdremote -> ../init.d/bdremote
S99lirc -> ../init.d/lirc
```

# Newer Ubuntu systems (10.04) #

## Bluez ##

Disable hid with:
```
DisablePlugins? = input
```
in `/etc/bluetooth/main.conf`.

## Scripts ##

```
S80bluetooth -> ../init.d/bluetooth
S95bdremote -> ../init.d/bdremote
S99bdremote_lirc -> ../init.d/bdremote_lirc
```

## bt\_ready.sh ##

Helper script used to check if bluetooth is initialized and ready. It will timeout after 30 seconds if no BT device is present.

```
#!/bin/bash

# BT is enabled.
ENABLED=0
# Timeout in seconds.
TIMEOUT=30
# Current try.
COUNT=0
# Device to wait for.
DEV="hci0"

while [[ $ENABLED -eq 0 ]]; do
        echo "Checking if BT device is ready ($COUNT)."

        if [ $COUNT -gt $TIMEOUT ];  then
                echo "Timeout, BT device not ready."
                exit -1
        fi

        for l in `hcitool dev 2> /dev/null` ; do
                if [[ "$l" == "$DEV" ]]; then
                        echo "Found device $DEV."
                        ENABLED=1
                fi
        done

        COUNT=$(($COUNT+1))

        if [ $ENABLED -eq 0 ];  then
                sleep 1
        fi
done
```

### bdremote ###

Notice the use of a script, bt\_ready.sh, to find out if the bluetooth device is ready. This is to avoid a situation where the bluetooth subsystem is not initialized at the time this daemon tries to start.

```
#!/bin/bash
BDREMOTED=/home/user/remote-svn/bdremote-ng/build/linux/bdremoteng

BDREMOTE_ARGS="-i 00:11:22:33:44:55 -a 00:66:77:88:99:00 -u user -g group -p 8888 -t 600 -f /tmp/bdremote.log -r10 -e800 -d -b /home/user/batterychange.sh -n"

case "$1" in
    start)
        /pack/scripts/bin/bt_ready.sh 2>&1 | /usr/bin/logger
        /usr/bin/logger "stopping bluetooth"
        /etc/init.d/bluetooth stop
        /usr/bin/logger "starting bdremote"
        echo -n "Starting bdremote daemon: "
        /bin/sleep 1
        /usr/bin/logger "Attempting to start bdremoteng .. start"
        $BDREMOTED $BDREMOTE_ARGS 2>&1 | /usr/bin/logger &
        /usr/bin/logger "Attempting to start bdremoteng .. end"
        /usr/bin/logger "started bdremote .."
        sleep 1
        /etc/init.d/bluetooth start
        /usr/bin/logger "started bluetooth again .. "
        echo
        ;;
    stop)
        echo -n "Stopping bdremote daemon: "
        killall bdremoteng
        echo
        ;;
    restart)
        $0 stop
        $0 start
        ;;
    *)
        echo "Usage: /etc/init.d/bdremote {start|stop|restart}"
        exit 1
esac

exit 0
```

### bdremote\_lirc ###
```
#!/bin/bash
LIRCD=/usr/sbin/lircd
LIRCD_PID=/var/run/lirc.pid
test -f "$LIRCD" || exit 0

LIRCD_ARGS="-H null --connect 127.0.0.1:8888"

case "$1" in
    start)
        echo -n "Starting lirc daemon: "
        mkdir -p /var/run/lirc 
        start-stop-daemon --start --exec "$LIRCD" -- $LIRCD_ARGS &> /tmp/bdremote-lirc-startup
        echo
        ;;
    stop)
        echo -n "Stopping lirc daemon: "
        start-stop-daemon --stop --quiet --exec "$LIRCD" --pidfile /var/run/lirc/lircd.pid
        echo
        ;;
    restart)
        $0 stop
        $0 start
        ;;
    *)
        echo "Usage: /etc/init.d/bdremote {start|stop|restart}"
        exit 1
esac
exit 0
```