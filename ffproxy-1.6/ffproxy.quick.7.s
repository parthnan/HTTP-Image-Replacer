.\" $Id: ffproxy.quick.7.s,v 2.2 2005/01/05 15:12:34 niklas Exp niklas $
.\" Copyright (c) 2002-2005 Niklas Olmes <niklas@noxa.de>
.\" See COPYING for license (GNU GPL)
.\" http://faith.eu.org
.Dd Jan 5, 2005
.Dt ffproxy.quick 7
.Sh NAME
.Nm ffproxy.quick
.Nd filtering HTTP/HTTPS proxy server quick introduction
.Sh DESCRIPTION
.Nm ffproxy
is a filtering HTTP/HTTPS proxy server.  It is able to filter
by host, URL, and header.  Custom header entries can be filtered
and added.  It can even drop its privileges and optionally
.Xr chroot 2
to some directory.  Logging to
.Xr syslog 3
is supported, as is using another auxiliary proxy server.
An HTTP accelerator feature (acting as a front-end to an HTTP server)
is included.  Contacting IPv6 servers as well as binding to IPv6 is
supported and allows transparent IPv6 over IPv4 browsing (and vice versa).
.Pp
This manual describes how to set up a basic HTTP proxy installation.
It is assumed that you already have compiled the program or
installed it via port or package.
.Sh COPYING FILES
The program comes with default configuration files that contain
both examples and suggested entries.  You can simply copy them to
a directory of your choice.  This directory will become the program's
working directory.
.Bd -literal -offset indent
mkdir /var/ffproxy
tar cf - db/ html/ | ( cd /var/ffproxy ; tar xf - )
cp sample.config /var/ffproxy/ffproxy.conf
.Ed
.Pp
Above example would install all needed files to
.Pa /var/ffproxy ,
which is ffproxy's default working directory.
.Sh SECURING
The proxy now has its own working directory.  By default,
ffproxy does not change UID/GID after start.  For security
reasons we want to enable it.  You have two choices know:
Either use existing UID/GID or add custom UID/GID for ffproxy.
See
.Xr adduser 8
or
.Xr useradd 8 ,
depending on your system, on how to create new IDs.
.Pp
Edit
.Pa ffproxy.conf
and change the lines containing uid and gid
.Bd -literal -offset indent
# change UID and GID
#
# to use, both uid and gid must be set
# (disabled by default)
#uid proxy
#gid proxy
uid _ffproxy
gid _ffproxy
.Ed
.Pp
In addition to changing UID and GID, ffproxy should be
executed change-rooted to its working directory.  So we
change chroot_dir and db_files_path in the configuration file
.Bd -literal -offset indent
# change root to (only in connection with uid and gid change)
# (disabled by default)
chroot_dir /var/ffproxy

# path to db/ and html/ directories
# (default: /var/ffproxy)
db_files_path .
.Ed
.Pp
db_files_path must be changed, too, since that is relative
to new root.  Finally, we copy /etc/resolv.conf to ffproxy's
home to enable DNS in chroot and chown /var/ffproxy so
the proxy's master process can write its PID file
.Bd -literal -offset indent
mkdir /var/ffproxy/etc
cp /etc/resolv.conf /var/ffproxy/etc/
chmod 750 /var/ffproxy
chown _ffproxy._ffproxy /var/ffproxy
.Ed
.Sh ACCESS TO THE PROXY
By default, nobody is allowed to connect to ffproxy.
Let's say, we want to provide LAN users a filtering proxy
to shut down malicous content coming from the Internet.
So the proxy has to be listening on the local network
interface only.  We change bind_ipv4 and bind_ipv6
appropiately in
.Pa ffproxy.conf
.Bd -literal -offset indent
bind_ipv4 martyr.burden.eu.org
bind_ipv6 martyr.burden.eu.org
.Ed
.Pp
Additionally, we have to change
.Pa db/access.ip .
By, for example,
.Bd -literal -offset indent
^192\\.168\\.10\\.
.Ed
.Pp
we allow 192.168.10.0/24 to use our proxy.
.Sh STARTING THE PROXY
Last step is starting ffproxy.  Keep in mind that
we run the program change-rooted to /var/ffproxy,
so files are relative to new root.
.Bd -literal -offset indent
cd /var/ffproxy ; /usr/local/bin/ffproxy -f ffproxy.conf
.Ed
.Pp
starts ffproxy.  Now test if it works correctly.
If not, change ffproxy.conf and/or read
.Xr ffproxy 8
.Xr ffproxy.conf 5
.Pp
ffproxy is not running as daemon right know.  If everything
seems to work, simply shut down the proxy by pressing
CTRL-C, set `daemonize yes' in the configuration file and
start ffproxy again.
.Sh TRANSPARENT OPERATION
The proxy allows transparent operation, that is, HTTP
traffic is redirect to the proxy which simulates a HTTP
server so that the users don't have to specify a
proxy server.  Consider forced usage of a proxy server as well.
To do that, you will have to configure your NAT accordingly.
On OpenBSD you'll want a line like
.Bd -literal -offset indent
rdr on rl0 proto tcp from any to any port 80 -> 127.0.0.1 port 8080
.Ed
.Pp
in
.Pa /etc/pf.conf .
See your NAT's documentation for details on how to do this.
.Sh VERSION
This manual documents ffproxy 1.6 (2005-01-05).
.Sh SEE ALSO
.Xr ffproxy 8 ,
.Xr ffproxy.conf 5 ,
.Xr pf.conf 5
