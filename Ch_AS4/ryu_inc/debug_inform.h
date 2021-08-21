#ifndef __DEBUG_INFORM_H__
#define __DEBUG_INFORM_H__

#define DEBUG_LCASACTION_SUBMENU_NO			1
#define DEBUG_MOTORCTRL_SUBMENU_NO			1

#ifndef NDEBUG_DP
#define dp(fmt,args...) iprintf( fmt, ## args )
#define dlp(fmt,args...) iprintf( "[%s %s %d]" fmt, __FILE__,__FUNCTION__,__LINE__, # args )
#define dfp(fmt,args...) iprintf( "[%s %d]-" fmt, __FUNCTION__,__LINE__, ## args ) 
#define dtmp(a)						iprintf(#a" = %d\n",tmp.a)
#define dprintf(fmt,args...) printf( fmt, ## args )

#define diprintf(fmt,args...) iprintf( fmt, ## args )
#define dfpErr(fmt,args...)  iprintf( "[%s %d]-" fmt, __FUNCTION__,__LINE__, ## args ) 

#define errorFunction(X)			{ iprintf( "[%s %d]",__FUNCTION__,__LINE__); errFunction(X); }

#else
#define dp(fmt,args...)
#define dlp(fmt,args...)
#define dfp(fmt,args...) 

#define dtmp(a)
#define dprintf(fmt,args...)

#define diprintf(fmt,args...)
//#define diprintf(fmt,args...) iprintf( fmt, ## args )
#define dfpErr(fmt,args...)  iprintf( "[%s %d]-" fmt, __FUNCTION__,__LINE__, ## args ) 

#define errorFunction(X)			{ errFunction(X); }
//#define errorFunction(X)			{ iprintf( "[%s %d]",__FUNCTION__,__LINE__); errFunction(X); }

#endif



#endif //__DEBUG_INFORM_H__
