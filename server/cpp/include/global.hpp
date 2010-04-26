
#ifndef __GLOBAL_HPP
#define __GLOBAL_HPP

//	symbol		bit	   value	cumul
#define DBG_INIT	0	// 1		1
#define DBG_GUI		1	// 2		3
#define DBG_WO		2	// 4		7
#define DBG_HTTP	3	// 8		15
#define DBG_IPMC	4	// 16		31
#define DBG_VGL		5	// 32		63
#define DBG_TOOL	6	// 64		127
#define DBG_NET		7	// 128		255
#define DBG_RTP		8	// 256		511
#define DBG_SQL		9	// 512		1023
#define DBG_COL		10	// 1024		2047
#define DBG_IFC		11	// 2048		4095
#define DBG_MAN		12	// 4096		8191
#define DBG_VNC		13	// 8192		16383
#define DBG_14		14	// 16384	32767
#define DBG_15		15	// 32768	65535

#define DBG_FORCE	-1	// always true

void trace(int dbgmask, const char *s, ...);
void error(const char *s, ...);
void notice(const char *s, ...);

#endif
