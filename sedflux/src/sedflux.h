#if !defined( SEDFLUX_H )
#define SEDFLUX_H

#include <glib.h>

typedef struct
{
   gboolean mode_3d;
   gboolean mode_2d;
   gchar*   init_file;
   gchar*   out_file;
   gchar*   working_dir;
   gchar*   run_desc;
   gboolean just_plume;
   gboolean just_rng;
   gboolean summary;
   gboolean warn;
   gint     verbosity;
   gboolean verbose;
   gboolean version;
   char**   active_procs;
}
Sedflux_param_st;

typedef gint32 Sedflux_sig_num;

#define SEDFLUX_SIG_NONE (0)
#define SEDFLUX_SIG_QUIT (1)
#define SEDFLUX_SIG_DUMP (2)
#define SEDFLUX_SIG_CPR  (4)
#define SEDFLUX_SIG_INT  (8)

int        sedflux_set_signal_action( void                );
gboolean   sedflux_signal_is_pending( Sedflux_sig_num sig );
void       sedflux_signal_reset     ( Sedflux_sig_num sig );
void       sedflux_signal_set       ( Sedflux_sig_num sig );

typedef enum
{
   SEDFLUX_ERROR_BAD_DIR ,
   SEDFLUX_ERROR_BAD_INIT_FILE ,
   SEDFLUX_ERROR_MULTIPLE_MODES ,
   SEDFLUX_ERROR_PROCESS_FILE_CHECK
}
Sedflux_error;

#define SEDFLUX_ERROR sedflux_error_quark()

typedef gint32 Sedflux_run_flag;

#define SEDFLUX_RUN_FLAG_SUMMARY (1)
#define SEDFLUX_RUN_FLAG_WARN    (2)

GQuark sedflux_error_quark( void );

gboolean          sedflux                    ( const gchar* init_file , Sedflux_run_flag flag );

Sedflux_param_st* sedflux_parse_command_line ( int argc , char *argv[] , GError** error );
gboolean          sedflux_setup_project_dir  ( gchar** init_file , gchar** working_dir , GError** error );
gint              sedflux_print_info_file    ( const gchar* init_file , const gchar* wd ,
                                               const gchar* cmd_str   , const gchar* desc );

#endif /* SEDFLUX_H */
