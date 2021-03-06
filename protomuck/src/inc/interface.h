
#include "copyright.h"
#include "db.h"
#include "defaults.h"
#include "mcp.h"

/* structures */

struct text_block {
    int     nchars;
    struct text_block *nxt;
    char   *start;
    char   *buf;
};

struct text_queue {
    int     lines;
    struct text_block *head;
    struct text_block **tail;
};

struct descriptor_data {
    int                      descriptor;    /* Descriptor number */
    int                      connected;     /* Connected as a player? */
    int                      did_connect;   /* Was connected to a player? */
    int                      con_number;    /* Connection number */
    int                      booted;        /* 1 = Booted, 2 = Boot with message, 3 = WEB boot */
    int                      fails;         /* Number of fail connection attempts */
    int                      block;         /* Is this descriptor blocked of input? */
    dbref                   *prog;          /* Which programs are blocking the input? -- UNIMPLEMENTED */
    dbref                    player;        /* Player dbref number connected to */
    int                      interactive;   /* 0 = not, 1 = @program/@edit/@mcpedit, 2 = READ primitive */
    char                    *output_prefix; /* Prefix for all output */
    char                    *output_suffix; /* Suffix for all output */
    int                      input_len; 
    int                      output_len;
    int                      output_size;
    struct text_queue        output;
    struct text_queue        input;
    char                    *raw_input;
    char                    *raw_input_at;
    int                      inIAC;         /* Used for telnet negotiation */
    time_t                   last_time;
    time_t                   connected_at;
    int                      hostaddr;      /* HEX host address */
    int	                 port;          /* Port number */
    const char              *hostname;      /* String host name */
    const char              *username;      /* Descriptor user name */
    int                      quota;
    int	                 commands;      /* Number of commands done */
    int                      linelen;
    int                      type;          /* Connection type */
    int                      cport;         /* Connected on this port if inbound, text, pueblo, web, etc. */
    int                      http_login;    /* Logged in to the MUCK through the web port? */
    int                      idletime_set;  /* Time [in minutes] until the IDLE flag must be set */
    object_flag_type         flags;         /* The descriptor flags */
    dbref                    mufprog;       /* If it is one of the MUF-type ports, then this points to the program. -- UNIMPLEMENTED */
    char                    *identify;
    char                    *lastmidi;
#ifdef HTTPDELAY
    const char              *httpdata;
#endif
    struct descriptor_data  *next;          /* Next descriptor information */
    struct descriptor_data **prev;          /* Previous descriptor information */
    McpFrame                 mcpframe;      /* Muck-To-Client protocal information */
};


#define DF_HTML          0x1 /* Connected to the internal WEB server. -- UNIMPLEMENTED */
#define DF_PUEBLO        0x2 /* Allows for HTML/Pueblo extentions on a connected port. -- UNIMPLEMENTED */
#define DF_MUF           0x4 /* Connected onto a MUF or MUF-Listening port. -- UNIMPLEMENTED */
#define DF_IDLE          0x8 /* This is set if the descriptor is idle. */
#define DF_TRUEIDLE     0x10 /* Set if the descriptor goes past the @tune idletime. Also triggers the propqueues if connected. */
#define DF_INTERACTIVE  0x20 /* If the player is in the MUF editor or the READ prim is used, etc. */
#define DF_COLOR        0x40 /* Used in conjunction with ansi_notify_descriptor */

#define DR_FLAGS(x,y)         ((descrdata_by_descr(x))->flags & y)
#define DR_CON_FLAGS(x,y)     ((descrdata_by_index(x))->flags & y)
#define DR_RAW_FLAGS(x,y)     ((x)->flags & y)

#define DR_ADD_FLAGS(x,y)     ((descrdata_by_descr(x))->flags |= y)
#define DR_CON_ADD_FLAGS(x,y) ((descrdata_by_index(x))->flags |= y)
#define DR_RAW_ADD_FLAGS(x,y) ((x)->flags |= y)

#define DR_REM_FLAGS(x,y)     ((descrdata_by_descr(x))->flags &= ~y)
#define DR_CON_REM_FLAGS(x,y) ((descrdata_by_index(x))->flags &= ~y)
#define DR_RAW_REM_FLAGS(x,y) ((x)->flags &= ~y)

#define CT_MUCK		0
#define CT_HTML		1
#define CT_PUEBLO	      2
#define CT_MUF          3
#define CT_OUTBOUND     4
#define CT_LISTEN       5
#define CT_INBOUND      6

/* these symbols must be defined by the interface */

extern char restart_message[BUFFER_LEN];
extern char shutdown_message[BUFFER_LEN];
extern unsigned int bytesIn;
extern unsigned int bytesOut;
extern unsigned int commandTotal;
#ifdef HTTPD
extern void httpd(struct descriptor_data *d, const char *name, const char *http);
extern void httpd_unknown(struct descriptor_data *d, const char *name);
#endif

void shutdownsock(struct descriptor_data *d);
void check_maxd(struct descriptor_data *d);
extern struct descriptor_data * initializesock(int s, const char *hostname, int port, int hostaddr, int ctyp, int cport, int welcome);
extern struct descriptor_data* descrdata_by_index(int index);
extern struct descriptor_data* descrdata_by_descr(int i);
extern int notify(dbref player, const char *msg);
extern int notify_nolisten(dbref player, const char *msg, int ispriv);
extern void notify_descriptor(int c, const char *msg);
extern void notify_descriptor_raw(int descr, const char *msg, int lenght);
extern void notify_descriptor_char(int d, char c);
extern int anotify(dbref player, const char *msg);
extern int notify_html(dbref player, const char *msg);
extern int queue_string(struct descriptor_data *d, const char *s);
extern int notify_nolisten(dbref player, const char *msg, int isprivate);
extern int anotify_nolisten2(dbref player, const char *msg);
extern int notify_html_nolisten(dbref player, const char *msg, int isprivate);
extern int anotify_nolisten(dbref player, const char *msg, int isprivate);
extern int notify_from_echo(dbref from, dbref player, const char *msg, int isprivate);
extern int notify_html_from_echo(dbref from, dbref player, const char *msg, int isprivate);
extern int anotify_from_echo(dbref from, dbref player, const char *msg, int isprivate);
extern int notify_from(dbref from, dbref player, const char *msg);
extern int notify_html_from(dbref from, dbref player, const char *msg);
extern int anotify_from(dbref from, dbref player, const char *msg);
extern void wall_and_flush(const char *msg);
extern void flush_user_output(dbref player);
extern void wall_all(const char *msg);
extern void wall_logwizards(const char *msg);
extern void wall_arches(const char *msg);
extern void wall_wizards(const char *msg);
extern void ansi_wall_wizards(const char *msg);
extern void show_wizards(dbref player);
extern int shutdown_flag; /* if non-zero, interface should shut down */
extern int restart_flag; /* if non-zero, should restart after shut down */
extern void emergency_shutdown(void);
extern int boot_off(dbref player);
extern void boot_player_off(dbref player);
extern int online(dbref player);
extern int *get_player_descrs(dbref player, int *count);
extern struct descriptor_data* get_descr(int, int);
extern int least_idle_player_descr(dbref who);
extern int most_idle_player_descr(dbref who);
extern int pcount(void);
extern int pidle(int c);
extern int pdbref(int c);
extern int pontime(int c);
extern char *phost(int c);
extern char *puser(int c);
extern char *pipnum(int c);
extern const char * addrout( int, int, unsigned short);
extern char *pport(int c);

extern void pboot(int c);
extern void pdboot(int c);
extern void pnotify(int c, char *outstr);
extern int pset_idletime(dbref player, int idle_time);
extern int pdescr(int c);
extern int pdescrcount(void);
extern int pfirstdescr(void);
extern int plastdescr(void);
extern int pdescrcon(int c);
extern McpFrame *descr_mcpframe(int c);
extern void SendText(McpFrame * mfr, const char *text);
extern int mcpframe_to_descr(McpFrame * ptr);
extern int mcpframe_to_user(McpFrame * ptr);
extern int pnextdescr(int c);
extern int pfirstconn(dbref who);
extern int pset_user(struct descriptor_data *d, dbref who);
extern int plogin_user(struct descriptor_data *d, dbref who);
extern int pset_user2(int c, dbref who);
extern int pdescrbufsize(int c);
extern dbref partial_pmatch(const char *name);
extern void do_armageddon( dbref, const char * );
extern void do_dinfo( dbref, const char * );
extern void do_dboot(dbref player, const char *name);
extern void do_dwall( dbref, const char *, const char * );
extern int request( dbref, struct descriptor_data *, const char *msg );
extern int dbref_first_descr(dbref c);
extern int pdescrflush(int c);
extern int pdescrp(int c);
extern int pdescrtype(int c);
extern void pdescr_welcome_user(int c);
extern void pdescr_logout(int c);
extern void pdump_who_users(int c, char *user);
extern const char* host_as_hex( unsigned addr );
extern int index_descr(int index);

/* the following symbols are provided by game.c */

extern void process_command(int descr, dbref player, char *command);

extern dbref create_player(const char *name, const char *password);
extern dbref connect_player(const char *name, const char *password);
extern void do_look_around(int descr, dbref player);

extern int init_game(const char *infile, const char *outfile);
extern void dump_database(void);
extern void panic(const char *);

/* Ansi Colors */
#define ANSINORMAL      "\033[0m"
#define ANSIFLASH       "\033[5m"
#define ANSIINVERT      "\033[7m"
#define ANSIUNDERLINE   "\033[4m"
#define ANSIBOLD        "\033[1m"

#define ANSIBLACK       "\033[0;30m"
#define ANSICRIMSON     "\033[0;31m"
#define ANSIFOREST      "\033[0;32m"
#define ANSIBROWN       "\033[0;33m"
#define ANSINAVY        "\033[0;34m"
#define ANSIVIOLET      "\033[0;35m"
#define ANSIAQUA        "\033[0;36m"
#define ANSIGRAY        "\033[0;37m"

#define ANSIGLOOM       "\033[1;30m"
#define ANSIRED         "\033[1;31m"
#define ANSIGREEN       "\033[1;32m"
#define ANSIYELLOW      "\033[1;33m"
#define ANSIBLUE        "\033[1;34m"
#define ANSIPURPLE      "\033[1;35m"
#define ANSICYAN        "\033[1;36m"
#define ANSIWHITE       "\033[1;37m"

#define ANSICBLACK      "\033[30m"
#define ANSICRED        "\033[31m"
#define ANSICGREEN      "\033[32m"
#define ANSICYELLOW     "\033[33m"
#define ANSICBLUE       "\033[34m"
#define ANSICPURPLE     "\033[35m"
#define ANSICCYAN       "\033[36m"
#define ANSICWHITE      "\033[37m"

#define ANSIBBLACK      "\033[40m"
#define ANSIBRED        "\033[41m"
#define ANSIBGREEN      "\033[42m"
#define ANSIBBROWN      "\033[43m"
#define ANSIBBLUE       "\033[44m"
#define ANSIBPURPLE     "\033[45m"
#define ANSIBCYAN       "\033[46m"
#define ANSIBGRAY       "\033[47m"

/* Colors */
#define NORMAL	  "^NORMAL^"
#define FLASH	  "^FLASH^"
#define INVERT	  "^INVERT^"
#define UNDERLINE "^UNDERLINE^"
#define BOLD      "^BOLD^"

#define BLACK	"^BLACK^"
#define CRIMSON	"^CRIMSON^"
#define FOREST	"^FOREST^"
#define BROWN	"^BROWN^"
#define NAVY	"^NAVY^"
#define VIOLET	"^VIOLET^"
#define AQUA	"^AQUA^"
#define GRAY	"^GRAY^"

#define GLOOM	"^GLOOM^"
#define RED	"^RED^"
#define GREEN	"^GREEN^"
#define YELLOW	"^YELLOW^"
#define BLUE	"^BLUE^"
#define PURPLE	"^PURPLE^"
#define CYAN	"^CYAN^"
#define WHITE	"^WHITE^"

#define CBLACK	"^CBLACK^"
#define CRED	"^CRED^"
#define CGREEN	"^CGREEN^"
#define CYELLOW	"^CYELLOW^"
#define CBLUE	"^CBLUE^"
#define CPURPLE	"^CPURPLE^"
#define CCYAN	"^CCYAN^"
#define CWHITE	"^CWHITE^"

#define BBLACK	"^BBLACK^"
#define BRED	"^BRED^"
#define BGREEN	"^BGREEN^"
#define BBROWN	"^BBROWN^"
#define BBLUE	"^BBLUE^"
#define BPURPLE	"^BPURPLE^"
#define BCYAN	"^BCYAN^"
#define BGRAY	"^BGRAY^"

/* ANSI attributes and color codes for FB6 style ansi routines */

#define ANSI_RESET	"\033[0m"

#define ANSI_BOLD       "\033[1m"
#define ANSI_DIM      	"\033[2m"
#define ANSI_UNDERLINE	"\033[4m"
#define ANSI_FLASH	"\033[5m"
#define ANSI_REVERSE	"\033[7m"

#define ANSI_FG_BLACK	"\033[30m"
#define ANSI_FG_RED	"\033[31m"
#define ANSI_FG_YELLOW	"\033[33m"
#define ANSI_FG_GREEN	"\033[32m"
#define ANSI_FG_CYAN	"\033[36m"
#define ANSI_FG_BLUE	"\033[34m"
#define ANSI_FG_MAGENTA	"\033[35m"
#define ANSI_FG_WHITE	"\033[37m"

#define ANSI_BG_BLACK	"\033[40m"
#define ANSI_BG_RED	"\033[41m"
#define ANSI_BG_YELLOW	"\033[43m"
#define ANSI_BG_GREEN	"\033[42m"
#define ANSI_BG_CYAN	"\033[46m"
#define ANSI_BG_BLUE	"\033[44m"
#define ANSI_BG_MAGENTA	"\033[45m"
#define ANSI_BG_WHITE	"\033[47m"

/* ANSI Colors for in-server commands */
#ifndef NO_SYSCOLOR
#define SYSNORMAL       "\033[0m"
#define SYSBLACK        "\033[0;30m"
#define SYSCRIMSON      "\033[0;31m"
#define SYSFOREST       "\033[0;32m"
#define SYSBROWN        "\033[0;33m"
#define SYSNAVY         "\033[0;34m"
#define SYSVIOLET       "\033[0;35m"
#define SYSAQUA         "\033[0;36m"
#define SYSGRAY         "\033[0;37m"
#define SYSGLOOM        "\033[0;37m"
#define SYSRED          "\033[1;31m"
#define SYSGREEN        "\033[1;32m"
#define SYSYELLOW       "\033[1;33m"
#define SYSBLUE         "\033[1;34m"
#define SYSPURPLE       "\033[1;35m"
#define SYSCYAN         "\033[1;36m"
#define SYSWHITE        "\033[1;37m"
/* These are defined in defaults.h */
#define CFAIL "^FAIL^"
#define CSUCC "^SUCC^"  
#define CINFO "^INFO^"  
#define CNOTE "^NOTE^"
#define CMOVE "^MOVE^"  
#else
#define SYSNORMAL       ""
#define SYSBLACK        ""
#define SYSCRIMSON      ""
#define SYSFOREST       ""
#define SYSBROWN        ""
#define SYSNAVY         ""
#define SYSVIOLET       ""
#define SYSAQUA         ""
#define SYSGRAY         ""
#define SYSGLOOM        ""
#define SYSRED          ""
#define SYSGREEN        ""
#define SYSYELLOW       ""
#define SYSBLUE         ""
#define SYSPURPLE       ""
#define SYSCYAN         ""
#define SYSWHITE        ""
/* These are defined in defaults.h */
#define CFAIL ""
#define CSUCC ""  
#define CINFO ""  
#define CNOTE ""
#define CMOVE ""  
#endif

#define TildeAnsiDigit(x)       (((x) == '-') || (((x) >= '0') && ((x) <= '9')))

