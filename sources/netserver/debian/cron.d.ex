#
# Regular cron jobs for the netserver package
#
0 4	* * *	root	[ -x /usr/bin/netserver_maintenance ] && /usr/bin/netserver_maintenance
