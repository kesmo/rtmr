#
# Regular cron jobs for the librtmr package
#
0 4	* * *	root	[ -x /usr/bin/librtmr_maintenance ] && /usr/bin/librtmr_maintenance
