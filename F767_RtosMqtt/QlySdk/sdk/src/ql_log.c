#include "ql_log.h"
#include "ql_util.h"

#define GET_ALIGN_STRING_LEN(str)    ((ql_strlen(str) + 3) & ~3)

char g_log_str[G_LOG_STR_LEN] = {0};
char g_cld_log_str[26] = {0};
int dm[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
void day_date(type_u32 day, struct s_time* st)
{
	int y, m = 0, d = day + 1; /* d days has passed, now are d + 1 */
	int i;

	y = d / 365;
	d -= 365 * y;
	d -= y / 4 - y / 100 + y / 400; // num of leap years
	while (d <= 0) 
    {
		if (!(y % 400) || (y % 4 == 0 && y % 100))
        {
		    d += 1;
        }
		d += 365;
		y -= 1;
	}
	y += 1970;

    if (!(y % 400) || (y % 4 == 0 && y % 100))
    {
        dm[1] = 29; /* modify days of February */
    }

	for (i = 0; i < 12; i++){
		if (d - dm[i] > 0) {
			d -= dm[i];
		} else {
			m = i;
			break;
		}
    }
    m += 1;
    
    st->year = y;
    st->mon = m;
    st->day = d;
}

void iot_parse_timestamp(type_u32 tick, struct s_time* st)
{
	struct s_time date;
	tick += 8 * 60 * 60; // peking time
	st->sec = tick % 60; tick /= 60;
	st->min = tick % 60; tick /= 60;
	st->hour = tick % 24; tick /= 24;
	/* tick are now days from 1970-01-01 */

	st->week = ((tick + 4) -1) % 7 + 1; /* 1970-01-01 is Thursday */
	day_date(tick, &date);
	st->year = date.year;
	st->mon = date.mon;
	st->day = date.day;
}

char* get_time_str(void)
{
    static char __time_str[12];
    static struct s_time st;
    iot_parse_timestamp(get_current_time(), &st);
    ql_snprintf(__time_str, 12, "%02d#%02d:%02d:%02d", st.day, st.hour, st.min, st.sec);
    __time_str[11] = '\0';
    return __time_str;
}

#if 0/*log online switch*/ 
extern local_log_t g_local_log;
type_u8 set_log_online_switch(type_u8 type)
{
    if(type < 0 || type > 3)
    {
        ql_log_err(ERR_EVENT_RX_DATA, "log_t: %d err\r\n",type);
        return -1;
    }
    else
    {       
        if(ql_local_log_load() != 0)
        {
            ql_log_err(ERR_EVENT_LOCAL_DATA, "set log err");
            return -1;
        }
        g_local_log.cfg_log_control = type ;

        if(0 != ql_local_log_save())
        {          
            ql_log_err( ERR_EVENT_LOCAL_DATA, "log save failed.");
            return -1;
        }
        g_client_handle.log_online = g_local_log.cfg_log_control;
        printf("debug:%s,cfg_log_control = %d,log_online = %d \r\n",__func__,g_local_log.cfg_log_control,g_client_handle.log_online);
    }    
    return 0;
}
type_u8 get_log_online_switch()
{
   return g_client_handle.log_online;
}
#endif