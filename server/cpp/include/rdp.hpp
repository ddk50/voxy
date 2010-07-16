
#include <iconv.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>

#include "parse.h"
#include "types.h"

//typedef bool RD_BOOL;

class rdp {    
private:
    uint16 g_mcs_userid;
    char g_username[64];
    char g_codepage[16];
    RD_BOOL g_bitmap_compression;
    RD_BOOL g_orders;
    RD_BOOL g_encryption;
    RD_BOOL g_desktop_save;
    RD_BOOL g_polygon_ellipse_orders;
    RD_BOOL g_use_rdp5;
    uint16 g_server_rdp_version;
    uint32 g_rdp5_performanceflags;
    int g_server_depth;
    int g_width;
    int g_height;
    RD_BOOL g_bitmap_cache;
    RD_BOOL g_bitmap_cache_persist_enable;
    RD_BOOL g_numlock_sync;

    uint8 *g_next_packet;
    uint32 g_rdp_shareid;

    RDPCOMP g_mppc_dict;

    /* Session Directory support */
    RD_BOOL g_redirect;
    char g_redirect_server[64];
    char g_redirect_domain[16];
    char g_redirect_password[64];
    char g_redirect_username[64];
    char g_redirect_cookie[128];
    uint32 g_redirect_flags;
    /* END Session Directory support */

    STREAM rdp_s;
    iconv_t iconv_h;
	int current_status;    

private:
    
    STREAM rdp_recv(uint8 * type);  
    STREAM rdp_init_data(int maxlen);
    void rdp_send_data(STREAM s, uint8 data_pdu_type);
    void rdp_send_logon_info(uint32 flags, char *domain, char *user,
                             char *password, char *program, char *directory);
    void rdp_send_control(uint16 action);
    void rdp_send_synchronise(void);
    void rdp_send_input(uint32 time, uint16 message_type,
                        uint16 device_flags, uint16 param1, uint16 param2);
    void rdp_send_client_window_status(int status);
    void rdp_enum_bmpcache2(void);
    void rdp_send_fonts(uint16 seq);
    void rdp_out_general_caps(STREAM s);
    void rdp_out_bitmap_caps(STREAM s);
    void rdp_out_order_caps(STREAM s);
    void rdp_out_bmpcache_caps(STREAM s);
    void rdp_out_bmpcache2_caps(STREAM s);
    void rdp_out_control_caps(STREAM s);
    void rdp_out_activate_caps(STREAM s);
    void rdp_out_pointer_caps(STREAM s);
    void rdp_out_share_caps(STREAM s);
    void rdp_out_colcache_caps(STREAM s);
    void rdp_out_unknown_caps(STREAM s, uint16 id, uint16 length, uint8 * caps);
    void rdp_send_confirm_active(void);
    void rdp_process_general_caps(STREAM s);
    void rdp_process_bitmap_caps(STREAM s);
    void rdp_process_server_caps(STREAM s, uint16 length);
    void process_demand_active(STREAM s);
    void process_colour_pointer_pdu(STREAM s);
    void process_cached_pointer_pdu(STREAM s);
    void process_system_pointer_pdu(STREAM s);
    void process_pointer_pdu(STREAM s);
    void process_bitmap_updates(STREAM s);
    void process_palette(STREAM s);
    void process_update_pdu(STREAM s);
    void process_disconnect_pdu(STREAM s, uint32 * ext_disc_reason);
    RD_BOOL process_data_pdu(STREAM s, uint32 * ext_disc_reason);
    RD_BOOL process_redirect_pdu(STREAM s /*, uint32 * ext_disc_reason */ );    

public:
    rdp();
    ~rdp();    
    
    void rdp_out_unistr(STREAM s, char *string, int len);
    int rdp_in_unistr(STREAM s, char *string, int str_size, int in_len);    
    /* Process incoming packets */
    /* nevers gets out of here till app is done */    
    void rdp_main_loop(RD_BOOL * deactivated, uint32 * ext_disc_reason);

    /* used in uiports and rdp_main_loop, processes the rdp packets waiting */
    RD_BOOL rdp_loop(RD_BOOL * deactivated, uint32 * ext_disc_reason);
    
    /* Establish a connection up to the RDP layer */
    RD_BOOL rdp_connect(char *server, uint32 flags, char *domain, char *password,   
                        char *command, char *directory);

    /* Establish a reconnection up to the RDP layer */
    RD_BOOL rdp_reconnect(char *server, uint32 flags, char *domain, char *password, 
                          char *command, char *directory, char *cookie);    

    /* Called during redirection to reset the state to support redirection */
    void rdp_reset_state(void);

    /* Disconnect from the RDP layer */
    void rdp_disconnect(void);    

};
