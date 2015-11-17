This projects provides a working daemon that takes input from a [PS3 bluetooth remote](http://en.wikipedia.org/wiki/PlayStation_3_accessories#Blu-ray_Remotes) and converts it into something that can be given to [LIRC](http://lirc.org).

bdremote 0.2 was used as a starting point. It was made a bit more portable and easier to read and understand.

Also, more debugging was added, to make it easier to use in situations where the bluetooth stack / user configuration of the [bluetooth](http://bluez.org) stack is to blame.

As of version 0.4, the daemon/driver is feature complete. The following versions will contain bug fixes and patches received from users.

This project is maintained by [Michael Wojciechowski](http://twitter.com/wojci).

Patches are welcome. Please use the mailing list.

Freshmeat.net: http://freshmeat.net/projects/bdremote-ng