.\" $Id: ffproxy.8.s,v 2.3 2005/01/05 15:55:30 niklas Exp niklas $
.\" Copyright (c) 2002-2005 Niklas Olmes <niklas@noxa.de>
.\" See COPYING for license (GNU GPL)
.\" http://faith.eu.org
.Dd Jan 5, 2005
.Dt ffproxy 8
.Sh NAME
.Nm ffproxy
.Nd filtering HTTP/HTTPS proxy server
.Sh SYNOPSIS
.Nm ffproxy
.Op Fl p Ar port
.Op Fl c Ar ip|hostname
.Op Fl C Ar ip|hostname
.Op Fl l Ar childs
.Op Fl u Ar uid|user Fl g Ar gid|group
.Op Fl r Ar dir
.Op Fl D Ar datadir
.Op Fl x Ar proxyip|proxyhost Fl X Ar proxyport
.Op Fl a Ar ip|hostname
.Op Fl A Ar port
.Op Fl f Ar configfile
.Op Fl ds4bBhv 
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
Remind that there is an alternative to command line options
by using configuration files.  See
.Xr ffproxy.conf 5
and
.Pa sample.config
for details.  It allows options that are not available
on command line.
.Pp
The following command line options are recognized.  They specify
general settings like IP to bind to or place of the db/ and html/
directories.  Note that arguments to options must be seperated
from the option by spaces, as are such options from each other.
.Pp
.Bl -tag -width "message"
.It Fl p Ar port
Bind to port.  Default is 8080.
.It Fl c Ar ip|hostname
Bind to IPv4.  Default is any IPv4.
.It Fl C Ar ip|hostname
Bind to IPv6.  Default is any IPv6.
.It Fl l Ar childs
Maximum number of child processes to be forked.  That is, the
maximum number of concurrent requests allowed.  Default is 10.
.It Fl u Ar uid|user Fl g Ar gid|group
Change UID and GID.  Both options must be used.  Default is
not changing UID and GID.
.It Fl r Ar dir
Change root
.Xr chroot 7
to dir.  Used in conjunction with -u and -g.  Because ffproxy
drops its privileges and chroots after reading the configuration files,
-D should be set to . (the current dir).  It might need
.Pa /etc/resolv.conf
copied as etc/resolv.conf in its working directory.  Example:
``# cd /var/ffproxy ; /usr/local/bin/ffproxy -r /var/ffproxy -D . -d -u proxy -g proxy -f ""''
.It Fl x Ar ip|hostname
Specify IP (or hostname) of an auxiliary proxy server that
the program will forward requests to.  Used together with -X.
.It Fl X Ar port
Port number of auxiliary proxy.
.It Fl D Ar dir
Location of the db/ and html/ directories.  For example,
specifying -D /var/ffproxy tells the proxy to search
for db/ files in
.Pa /var/ffproxy/db/
and html/ files in
.Pa /var/ffproxy/html/ .
.It Fl a Ar ip|hostname
Auxiliary forward HTTP server to use (see section HTTP ACCELERATOR).
.It Fl A Ar port
Port to use for above.  Defaults to 80.
.It Fl f Ar configfile
User configuration file to load.  Please note that command
line options get overwritten by set configuration file options.
Default location is
.Pa _CFGFILE_ .
Read
.Xr ffproxy.conf 5
for details.  Use -f "" to disable configuration files.
.It Fl d 
Run as daemon.
.It Fl s
Be silent.  Don't log to syslog.
.It Fl 4
Use IPv4 only.  Do not try contacting servers via IPv6.
.It Fl b
Don't bind to IPv4.  Might be needed under Linux 2.4, due to a ``Feature''
IPv6 binds to IPv4, too.  Try using this option or bind to specific
IPv6 address via -C.
.It Fl B
Don't bind to IPv6.
.It Fl h
Show usage information.
.It Fl v
Display version number.
.El
.Sh THE DB/ DIRECTORY
The db/ directory contains files that control the behaviour
of ffproxy.  The files for filtering are prefixed by `filter'.
Access to the proxy server is controlled by files with prefix
`host'.
.Ss Filtering
Requests or header entries to be filtered are matched by extended
regular expressions or case insensitive by strings.
.Pp
ffproxy is able to filter requests by host, header, remote header, and URL.
The specific files are
.Pp
.Bl -tag -width xxxx -compact -offset indent
.It Ar filter.host.match
.It Ar filter.header.drop
.It Ar filter.header.entry
.It Ar filter.header.match
.It Ar filter.rheader.drop
.It Ar filter.rheader.entry
.It Ar filter.rheader.match
.It Ar filter.url.match
.El
.Pp
Files ending in `drop' specify requests to be completely filtered (dropped).
Files ending in `entry' specify header entries to be removed from the header.
They are matched case insensitive without extended regular expressions.
Files ending in `match' specify extended regular expressions to be
matched against header entries, host, or URL.
.Pp
Adding custom header entries is also supported.  The entries of file
.Pa filter.header.add
will be added to every outgoing request.
.Ss Access Control
Access to the proxy is controlled through the files prefixed `host'.
.Pp
.Pa host.dyndns
contains host names with dynamic
IPv4 addresses.  The host names are resolved to IPv4 addresses and
compared to the client's IP.  If it matches, access is granted.
.Pp
.Pa host.ip
contains static IPv4 and IPv6 address.
.Pp
.Pa host.name
contains official hostnames (reverse lookup).
.Pp
Except for
.Pa host.dyndns ,
the files contain extended regular expressions.
If any of the entries matches, access is granted.
.Ss Layout of db/ Files
Every mentioned file above must exist, although it may be empty.
Every entry is exactly one line.  Empty lines are ignored, as
are lines beginning with a # (comments).
.Pp
The location of the db/ directory may be specified by an
argument to the command line option -D.
If this option and configuration file option db_files_path are not used,
ffproxy will search for db/ and html/ in
.Pa _BASE_ .
.Pp
ffproxy comes with sample db/ files.  They also contain
needed and suggested entries, as described next.
.Ss Suggested db/ file entries
The file
.Pa filter.header.entry
should contain following entries for the program's proper operation
.Bd -literal -offset indent
Accept-Encoding:
Accept:
Connection:
Proxy-Connection:
Host:
.Ed
.Pp
First two lines are needed for browsers that send out Accept*: Headers
but don't understand encoded data coming back from the proxy.
Host:  has to be removed, since proxies require absolute URIs
(Host: is redundant).
.Pp
.Pa filter.header.add
should contain
.Bd -literal -offset indent
Connection: close
Proxy-Connection: close
.Ed
.Pp
We removed the two entries through
.Pa filter.header.entry
and now implant our own to force disconnection after each
request.
.Pp
.Pa filter.rheader.entry
should contain
.Bd -literal -offset indent
Connection:
Proxy-Connection:
.Ed
.Pp
Whatever the server answered, we remove it.
.Sh THE HTML/ DIRECTORY
This directory contains files with HTTP header
and HTML that are sent to
the user's browser if either an error occured or
a request was filtered.  In the files, the variable
.Va $u
will be replaced by the URL,
.Va $h
by the host to connect to, and
.Va $c
by the hostname of the client.
.Pp
Since the files are loaded into memory for faster
execution, the size of each file is limited to
about 8 kB (what is more than enough, the default
files are under 1 kB).
.Pp
The specific files are (every file must exist)
.Pp
.Bl -tag -width xxxxxxxxxxx -compact -offset indent
.It Ar connect
Connection failed (503)
.It Ar filtered
Request filtered (200)
.It Ar invalid
Invalid request (400)
.It Ar post
Unable to post data (400)
.It Ar resolve
Resolve error (503)
.El
.Sh HTTP ACCELERATOR
ffproxy may also be used as a HTTP accelerator, that
is, connecting to just one HTTP server and beeing
a front-end to that.  Use accel_host and accel_port
in configuration file or command line options -a and -A
to use this feature.
.Pp
Default behaviour is *not* sending Host: header to
allow insertion of a custom one via
.Pa filter.header.add
(see section THE DB/ DIRECTORY)
or keeping the original one used by connecting client
(`Host:' hast to be removed from default
.Pa filter.header.entry ,
of course).  To change this, use `accel_user_host no'
in the configuration file.  ``Host: accel_host:accel_port''
will be used then.
.Sh TRANSPARENT OPERATION
It is possible to redirect all HTTP traffic, that is,
traffic to port 80, to the proxy's listening port.  It will
then transparently act as a HTTP proxy, the client not
even knowing it is connecting to a proxy. 
.Pp
On OpenBSD one could enable this by
adding a line like
.Bd -literal -offset indent
rdr on rl0 proto tcp from any to any port 80 -> 127.0.0.1 port 8080
.Ed
.Pp
to
.Pa /etc/pf.conf .
In this example, rl0 is the local interface.  All traffic
coming from rl0 directed to port 80 (HTTP standard port)
is sent to 127.0.0.1:8080 where ffproxy is supposed
to be listening.
.Sh KEEP ALIVE
The program supports keep alive on client to proxy connections.
This is used automatically by default and may be disabled
by setting `use_keep_alive no' in the configuration file.
.Sh HTTPS OPERATION
The proxy allows HTTPS proxying via implementation of the
CONNECT request method.  By default, only port 443 is
allowed for CONNECT.  This may be changed by using
`unrestricted_connect yes' in the configuration file.
Timeout may also be tuned by `timeout_connect seconds'.
.Sh RELOADING CONFIGURATION
Send a SIGHUP to the pid of the ffproxy master process
to let it reload db/ files, html/ files, *and* configuration file.
If no configuration file was specified,
.Pa _CFGFILE_
is tried.  Of course, only some changes to the program can be
done at runtime.  See
.Xr ffproxy.conf 5
for details on options that may be changed at runtime.
.Pp
If daemonized, the master process writes the pid file
.Pa ffproxy.pid
to the working directory, that is, the directory
specified by db_files_path or the command line parameter -D.
It defaults to
.Pa _BASE_ .
The program will terminate if writing fails.
.Sh LOGGING
By default, the proxy logs incorrect and filtered requests.
To log all requests, use the configuration file keyword
`log_all_requests yes'.  Please make sure that you seperate
the programs log output from that of other programs by modifying
.Xr syslog.conf 5 ,
since the output is very noisy.
.Sh FILES
Behaviour of ffproxy is determined by
.Bl -bullet
.It
startup options given either on the command line
or read from configuration files --
.Pa _CFGFILE_
is loaded by default.
.It
the files in db/ which specify filtering options
and who is allowed to connect and use ffproxy
.El
.Pp
If daemonized, ffproxy writes the pid of its master
process to the file named
.Pa ffproxy.pid
in its working directory --
.Pa _BASE_
by default.
.Sh SEE ALSO
.Pa sample.config
for a sample configuration file
.Pp
.Pa _CFGFILE_
for default configuration file
.Pp
.Xr ffproxy.conf 5
for details on config file
.Pp
.Xr ffproxy.quick 7
for a short description of how to set up the proxy
.Pp
.Pa http://faith.eu.org/programs.html
for latest version and patches
.Pp
.Xr regex 7 ,
.Xr re_format 7 ,
.Xr syslogd 8 ,
.Xr chroot 2 ,
.Xr kill 1
.Sh CONTRIBUTORS
Dobrica Pavlinusic <dpavlin@rot13.org>
provided patches for http accelerator feature
.Sh VERSION
This manual documents ffproxy 1.6 (2005-01-05).
.Pp
Send bug reports, comments, suggestions to <niklas@noxa.de>
.Sh AUTHOR
Niklas Olmes <niklas@noxa.de>
