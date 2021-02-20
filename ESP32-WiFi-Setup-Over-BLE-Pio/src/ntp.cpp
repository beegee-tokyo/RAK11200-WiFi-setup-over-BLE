#include "main.h"

/** Structure to keep the local time */
struct tm time_info;

/** Timer to frequently update the time */
Ticker get_ntp;

/** Flag if NTP server gave a valid time */
bool ntp_ok = false;

/**
 * Initialize NTP client
 * @return <code>bool</code>
 *		true if time was updated within 10 seconds
 */
bool init_ntp(void)
{
	get_ntp.detach();
	get_ntp.attach(3600, try_get_time);

	configTzTime("UTC-8:00", "0.asia.pool.ntp.org", "1.asia.pool.ntp.org", "2.asia.pool.ntp.org");
	if (getLocalTime(&time_info, 10000))
	{ // wait up to 10sec to sync
		myLog_d("First NTP request successful");
		ntp_ok = true;
		return true;
	}
	return false;
}

/**
 * Try to get time from NTP server
 */
void try_get_time(void)
{
	ntp_ok = false;

	if (getLocalTime(&time_info, 0))
	{
		myLog_d("Updated time from NTP server");
		ntp_ok = true;
	}
}
