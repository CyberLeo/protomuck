extern void prim_split(PRIM_PROTOTYPE);
extern void prim_rsplit(PRIM_PROTOTYPE);
extern void prim_ctoi(PRIM_PROTOTYPE);
extern void prim_itoc(PRIM_PROTOTYPE);
extern void prim_stod(PRIM_PROTOTYPE);
extern void prim_dtos(PRIM_PROTOTYPE);
extern void prim_midstr(PRIM_PROTOTYPE);
extern void prim_numberp(PRIM_PROTOTYPE);
extern void prim_stringcmp(PRIM_PROTOTYPE);
extern void prim_strcmp(PRIM_PROTOTYPE);
extern void prim_strncmp(PRIM_PROTOTYPE);
extern void prim_strcut (PRIM_PROTOTYPE);
extern void prim_strlen(PRIM_PROTOTYPE);
extern void prim_strcat(PRIM_PROTOTYPE);
extern void prim_atoi(PRIM_PROTOTYPE);
extern void prim_ansi_notify(PRIM_PROTOTYPE);
extern void prim_ansi_notify_exclude(PRIM_PROTOTYPE);
extern void prim_notify(PRIM_PROTOTYPE);
extern void prim_notify_html(PRIM_PROTOTYPE);
extern void prim_notify_html_nocr(PRIM_PROTOTYPE);
extern void prim_notify_descriptor(PRIM_PROTOTYPE);
extern void prim_ansi_notify_descriptor(PRIM_PROTOTYPE);
extern void prim_notify_exclude(PRIM_PROTOTYPE);
extern void prim_notify_html_exclude(PRIM_PROTOTYPE);
extern void prim_notify_html_exclude_nocr(PRIM_PROTOTYPE);
extern void prim_intostr(PRIM_PROTOTYPE);
extern void prim_explode(PRIM_PROTOTYPE);
extern void prim_subst(PRIM_PROTOTYPE);
extern void prim_instr(PRIM_PROTOTYPE);
extern void prim_rinstr(PRIM_PROTOTYPE);
extern void prim_pronoun_sub(PRIM_PROTOTYPE);
extern void prim_toupper(PRIM_PROTOTYPE);
extern void prim_tolower(PRIM_PROTOTYPE);
extern void prim_unparseobj(PRIM_PROTOTYPE);
extern void prim_smatch(PRIM_PROTOTYPE);
extern void prim_striplead(PRIM_PROTOTYPE);
extern void prim_striptail(PRIM_PROTOTYPE);
extern void prim_stringpfx(PRIM_PROTOTYPE);
extern void prim_strencrypt(PRIM_PROTOTYPE);
extern void prim_strdecrypt(PRIM_PROTOTYPE);
extern void prim_tokensplit(PRIM_PROTOTYPE);
extern void prim_fmtstring(PRIM_PROTOTYPE);
extern void prim_parse_ansi(PRIM_PROTOTYPE);
extern void prim_parse_neon(PRIM_PROTOTYPE);
extern void prim_unparse_ansi(PRIM_PROTOTYPE);
extern void prim_escape_ansi(PRIM_PROTOTYPE);
extern void prim_ansi_strlen(PRIM_PROTOTYPE);
extern void prim_ansi_strcut(PRIM_PROTOTYPE);
extern void prim_ansi_strip(PRIM_PROTOTYPE);
extern void prim_ansi_midstr(PRIM_PROTOTYPE);
extern void prim_array_fmtstrings(PRIM_PROTOTYPE);
extern void prim_textattr(PRIM_PROTOTYPE);
extern void prim_flag_2char(PRIM_PROTOTYPE);
extern void prim_power_2char(PRIM_PROTOTYPE);
extern void prim_notify_descriptor_char(PRIM_PROTOTYPE);

#define PRIMS_STRINGS_FUNCS prim_numberp, prim_stringcmp, prim_strcmp,        \
    prim_strncmp, prim_strcut, prim_strlen, prim_strcat, prim_atoi,           \
    prim_ansi_notify, prim_notify, prim_notify_descriptor,                    \
    prim_ansi_notify_descriptor,                                              \
    prim_notify_exclude, prim_intostr,                                        \
    prim_explode, prim_subst, prim_instr, prim_rinstr, prim_pronoun_sub,      \
    prim_toupper, prim_tolower, prim_unparseobj, prim_smatch, prim_striplead, \
    prim_striptail, prim_stringpfx, prim_strencrypt, prim_strdecrypt,         \
    prim_notify_html, prim_notify_html_exclude, prim_notify_html_nocr,        \
    prim_notify_html_exclude_nocr, prim_midstr, prim_ctoi, prim_itoc,         \
    prim_stod, prim_split, prim_rsplit, prim_ansi_notify_exclude, prim_dtos,  \
    prim_tokensplit, prim_fmtstring, prim_parse_ansi, prim_unparse_ansi,      \
    prim_escape_ansi, prim_ansi_strlen, prim_ansi_strcut, prim_ansi_strip,    \
    prim_ansi_midstr, prim_textattr, prim_parse_neon, prim_notify_descriptor, \
    prim_flag_2char, prim_power_2char, prim_notify_descriptor_char,           \
    prim_array_fmtstrings

#define PRIMS_STRINGS_NAMES "NUMBER?", "STRINGCMP", "STRCMP",        \
    "STRNCMP", "STRCUT", "STRLEN", "STRCAT", "ATOI",                 \
    "ANSI_NOTIFY", "NOTIFY", "NOTIFY_DESCRIPTOR",                    \
    "ANSI_NOTIFY_DESCRIPTOR", "NOTIFY_EXCLUDE",                      \
    "INTOSTR", "EXPLODE", "SUBST", "INSTR", "RINSTR", "PRONOUN_SUB", \
    "TOUPPER", "TOLOWER", "UNPARSEOBJ", "SMATCH", "STRIPLEAD",       \
    "STRIPTAIL", "STRINGPFX", "STRENCRYPT", "STRDECRYPT",            \
    "NOTIFY_HTML", "NOTIFY_HTML_EXCLUDE", "HTML_NOCR",               \
    "HTML_EXCLUDE_NOCR", "MIDSTR", "CTOI", "ITOC", "STOD", "SPLIT",  \
    "RSPLIT", "ANSI_NOTIFY_EXCLUDE", "DTOS", "TOKENSPLIT",           \
    "FMTSTRING", "PARSE_ANSI", "UNPARSE_ANSI", "ESCAPE_ANSI",        \
    "ANSI_STRLEN", "ANSI_STRCUT", "ANSI_STRIP", "ANSI_MIDSTR",       \
    "TEXTATTR", "PARSE_NEON", "DESCRNOTIFY", "FLAG_2CHAR",           \
    "POWER_2CHAR", "NOTIFY_DESCRIPTOR_CHAR", "ARRAY_FMTSTRINGS"

#define PRIMS_STRINGS_CNT 56







