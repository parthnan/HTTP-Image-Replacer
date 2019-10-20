.\" $Id: ffproxy.conf.5.s,v 2.3 2005/01/05 15:54:39 niklas Exp niklas $
.\" Copyright (c) 2002-2005 Niklas Olmes <niklas@noxa.de>
.\" See COPYING for license (GNU GPL)
.\" http://faith.eu.org
.Dd Jan 5, 2005
.Dt ffproxy.conf 5
.Sh NAME
.Nm ffproxy.conf
.Nd filtering HTTP/HTTPS proxy server configuration file
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
This manual describes how to use configuration files with the program
and documents the options.
.Sh USING CONFIGURATION FILES
.Ss Default ffproxy.conf
If the command line parameters -f or -F are not used, the proxy
tries to open
.Pa _CFGFILE_ .
If this file does not exist, the program continues execution.
.Ss User Configuration File
Use command line parameter -f to load a non-default configuration
file.  You will notice the warning at the program's startup.  This
is due to the programs implementation that allows to reload
all configuration files.  To disable the warning, use -F instead.
.Ss Deactivating
To use command line options only, use -f "".
.Ss Reloading Configuration
To let the proxy reload its configuration files, that is, besides
the configuration file specified, the contents of db/ and html/,
send the signal HUP to the program's master process.  If
ffproxy runs daemonized, the PID can be found in
.Pa db_files_path/ffproxy.conf .
Otherwise look into your system's syslog log files or process table.
.Pp
Options that can be successfully altered at runtime are 
.Bd -literal -offset indent
child_processes
use_ipv6
use_syslog
log_all_requests
forward_proxy
forward_proxy_port
forward_proxy_ipv6
accel_host
accel_port
accel_user_host
use_keep_alive
unrestricted_connect
timeout_connect
backlog_size
.Ed
.Pp
Set `accel_port 0' or `forward_proxy_port 0' to explicitly disable
acceleration or auxiliary proxy.  Commenting out options is not
sufficient, since configuration options may only overwritten.
.Pp
Changes to other options not mentioned above get silently ignored.
.Sh CONFIGURATION OPTIONS
.Bd -literal
#
# lines starting with '#' are comments
#

# run as daemon?
# (default: no)
#daemonize yes
#daemonize no

# number of child processes,
# that is, the maximum number of concurrent requests
# (default: 10)
#child_processes 10

# ffproxy binds to any IPv4 address
# and any IPv6 address by default
#
# bind to IPv4?  (default: yes)
#bind_ipv4 no
#bind_ipv4 yes
# bind to IPv6?  (default: yes)
#bind_ipv6 no
#bind_ipv6 yes
#
# Hostname or IP to bind to
# (default is any IP)
#
#bind_ipv4_host 192.168.10.1
#bind_ipv4_host martyr.burden.eu.org
#bind_ipv6_host ::1
#bind_ipv6_host oz.burden.eu.org

# listen on port
# (default: 8080)
#port 1111
#port 8080

# use IPv6 when contacting servers?
# (default: yes)
#use_ipv6 no
#use_ipv6 yes

# use syslog?
# (default: yes)
#use_syslog no
#use_syslog yes

# log all requests?
# (default: no)
# to use, set also use_syslog to yes
#log_all_requests yes
#log_all_requests no

# change UID and GID
#
# to use, both uid and gid must be set
# (disabled by default)
#uid proxy
#gid proxy
#uid 37
#gid 38

# change root to (only in connection with uid and gid change)
#   /etc/resolv.conf might need to be copied
#   to chroot_dir/etc/resolv.conf
# (disabled by default)
#chroot_dir _BASE_

# forward to proxy (auxiliary proxy)
# (set `forward_proxy_port 0' to explicitly disable feature
#  (i.e, when reloading configuration file via SIGHUP))
# (disabled by default)
#forward_proxy blackness.burden.eu.org
#forward_proxy 192.168.10.5
#forward_proxy ::1
#forward_proxy_port 8082
#forward_proxy_port 0

# try IPv6 for auxiliary proxy?
# use_ipv6 must be set to yes, too
# (default: yes)
#forward_proxy_ipv6 no
#forward_proxy_ipv6 yes

# path to db/ and html/ directories
# (default: _BASE_)
# (Note: if ffproxy runs chrooted,
#  give a path name relative to new root, or,
#  if db_files_path is the same as root, use db_files_path ./
#  You have to start ffproxy in the new root directory,
#  otherwise it won't find the database files.
#  Please keep in mind that ffproxy's config file has to
#  be within chroot directory, otherwise it will not find
#  its config file on reload)
#db_files_path ./
#db_files_path _BASE_

# http accelerator
# (disabled by default)
#
# if you want to use ffproxy as http accelerator (that is, connecting
# to just one http server and beeing used as front-end to that, e.g.
# in DMZ) uncomments options below (port is optional, defaults to 80)
# (set `accel_port 0' to explicitly disable feature
#  (i.e, when reloading configuration file via SIGHUP))
#accel_host 10.254.1.2
#accel_host revelation.martyr.eu.org
#accel_port 80
#accel_port 0
#
# Omit Host: accel_host:accel_port in Header
# to provide own Host: header via db/filter.header.add?
# (default: yes)
#accel_user_host no
#accel_user_host yes

# keep alive on client to proxy connections
# (enabled by default)
#use_keep_alive no
#use_keep_alive yes

# allow CONNECT request to other than port 443 (HTTPS)
# (CONNECT enables HTTPS proxying)
# (disabled by default for security)
#unrestricted_connect yes
#unrestricted_connect no

# timeout for CONNECT requests in seconds
# (default: 5)
#timeout_connect 20
#timeout_connect 5

# backlog size for accept()
# (default: 4)
#backlog_size 16
#backlog_size 4
.Ed
.Sh VERSION
This manual documents ffproxy 1.6 (2005-01-05).
.Sh FILES
.Pa _CFGFILE_
default configuration file
.Pp
.Pa sample.config
sample configuration file
.Sh SEE ALSO
.Xr ffproxy 8 ,
.Xr ffproxy.quick 7 ,
.Xr regex 7 ,
.Xr re_format 7 ,
.Xr syslogd 8 ,
.Xr chroot 2 ,
.Xr kill 1
