#include "copyright.h"
#include "config.h"

/* commands which set parameters */
#include <stdio.h>
#include <ctype.h>
#include "strings.h"

#include "db.h"
#include "params.h"
#include "tune.h"
#include "props.h"
#include "match.h"
#include "interface.h"
#include "externs.h"

#ifdef COMPRESS
#define alloc_compressed(x) alloc_string(compress(x))
#else				/* COMPRESS */
#define alloc_compressed(x) alloc_string(x)
#endif				/* COMPRESS */


static dbref 
match_controlled(int descr, dbref player, const char *name)
{
    dbref   match;
    struct match_data md;

    init_match(descr, player, name, NOTYPE, &md);
    match_absolute(&md);
    match_everything(&md);

    match = noisy_match_result(&md);
    if (match != NOTHING && !controls(player, match)) {
	anotify_fmt(player, CFAIL "%s", tp_noperm_mesg);
	return NOTHING;
    } else {
	return match;
    }
}

void 
do_name(int descr, dbref player, const char *name, char *newname)
{
    dbref   thing;
    char   *password;
    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify_fmt(player, CFAIL "%s", tp_noguest_mesg);
	return;
    }

    if ((thing = match_controlled(descr, player, name)) != NOTHING) {
      if (Protect(thing) && !(MLevel(player) > MLevel(OWNER(thing)))) {
	  anotify_fmt(player, CFAIL "%s", tp_noperm_mesg);
	  return;
      }
	/* check for bad name */
	if (*newname == '\0') {
	    anotify_nolisten2(player, CINFO "Give it what new name?");
	    return;
	}
	/* check for renaming a player */
	if (Typeof(thing) == TYPE_PLAYER) {

	    if(tp_wiz_name && (!Mage(player)) ) {
		    anotify_nolisten2(player, CINFO "Only wizards can change player names.");
		    return;
	    }

	    /* split off password */
	    for (password = newname;
		    *password && !isspace(*password);
		    password++);
	    /* eat whitespace */
	    if (*password) {
		*password++ = '\0';	/* terminate name */
		while (*password && isspace(*password))
		    password++;
	    }
	    /* check for null password */
	    if (!*password) {
		anotify_nolisten2(player, CINFO "You must specify a password to change a player name.");
		anotify_nolisten2(player, CNOTE "E.g.: name player = newname password");
		if( Wiz(OWNER(player)))
		    anotify_nolisten2(player,
			YELLOW "Wizards may use 'yes' for non-wizard players."
		    );
		return;
	    }
	    if(!Wiz(player)||TMage(thing)||strcmp(password,"yes")) {
		if (strcmp(password, DoNull(DBFETCH(thing)->sp.player.password))) {
		    anotify_nolisten2(player, CFAIL "Incorrect password.");
		    return;
		}
	    }
	    if (string_compare(newname, NAME(thing))
		       && !ok_player_name(newname)) {
		anotify_nolisten2(player, CFAIL "That name is either taken or invalid.");
		return;
	    }
	    /* everything ok, notify */
	    log_status("NAME: %s(%d) to %s by %s\n",
		       NAME(thing), thing, newname, NAME(player));
	    delete_player(thing);
	    if (NAME(thing))
		free((void *) NAME(thing));
	    ts_modifyobject(thing);
	    NAME(thing) = alloc_string(newname);
	    add_player(thing);
	    anotify_nolisten2(player, CSUCC "Name set.");
	    return;
	} else {
	    if (!ok_name(newname)) {
		anotify_nolisten2(player, CFAIL "That is not a reasonable name.");
		return;
	    }
	}

	/* everything ok, change the name */
	if (NAME(thing)) {
	    free((void *) NAME(thing));
	}
	ts_modifyobject(thing);
	NAME(thing) = alloc_string(newname);
	anotify_nolisten2(player, CSUCC "Name set.");
	DBDIRTY(thing);
	if (Typeof(thing) == TYPE_EXIT && MLevel(thing)) {
	    SetMLevel(thing, 0);
	    anotify_nolisten2(player, CINFO "Action priority Level reset to zero.");
	}
    }
}

void 
do_describe(int descr, dbref player, const char *name, const char *description)
{
    dbref   thing;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify_fmt(player, CFAIL "%s", tp_noguest_mesg);
	return;
    }

    if ((thing = match_controlled(descr, player, name)) != NOTHING) {
      if (Protect(thing) && !(MLevel(player) > MLevel(OWNER(thing)))) {
	  anotify_fmt(player, CFAIL "%s", tp_noperm_mesg);
	  return;
      }
	ts_modifyobject(thing);
	SETDESC(thing, description);
	if(*description)
	    anotify_nolisten2(player, CSUCC "Description set.");
	else
	    anotify_nolisten2(player, CSUCC "Description cleared.");
    }
}

void 
do_idescribe(int descr, dbref player, const char *name, const char *description)
{
    dbref   thing;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify_fmt(player, CFAIL "%s", tp_noguest_mesg);
	return;
    }

    if ((thing = match_controlled(descr, player, name)) != NOTHING) {
      if (Protect(thing) && !(MLevel(player) > MLevel(OWNER(thing)))) {
	  anotify_fmt(player, CFAIL "%s", tp_noperm_mesg);
	  return;
      }
	ts_modifyobject(thing);
	SETIDESC(thing, description);
	if(*description)
	    anotify_nolisten2(player, CSUCC "IDescription set.");
	else
	    anotify_nolisten2(player, CSUCC "IDescription cleared.");
    }
}


void 
do_ansidescribe(int descr, dbref player, const char *name, const char *description)
{
    dbref   thing;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify_fmt(player, CFAIL "%s", tp_noguest_mesg);
	return;
    }

    if ((thing = match_controlled(descr, player, name)) != NOTHING) {
      if (Protect(thing) && !(MLevel(player) > MLevel(OWNER(thing)))) {
	  anotify_fmt(player, CFAIL "%s", tp_noperm_mesg);
	  return;
      }
	ts_modifyobject(thing);
	SETANSIDESC(thing, description);
	if(*description)
	    anotify_nolisten2(player, CSUCC "ANSIDescription set.");
	else
	    anotify_nolisten2(player, CSUCC "ANSIDescription cleared.");
    }
}

void 
do_iansidescribe(int descr, dbref player, const char *name, const char *description)
{
    dbref   thing;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify_fmt(player, CFAIL "%s", tp_noguest_mesg);
	return;
    }

    if ((thing = match_controlled(descr, player, name)) != NOTHING) {
      if (Protect(thing) && !(MLevel(player) > MLevel(OWNER(thing)))) {
	  anotify_fmt(player, CFAIL "%s", tp_noperm_mesg);
	  return;
      }
	ts_modifyobject(thing);
	SETIANSIDESC(thing, description);
	if(*description)
	    anotify_nolisten2(player, CSUCC "IANSIDescription set.");
	else
	    anotify_nolisten2(player, CSUCC "IANSIDescription cleared.");
    }
}


void 
do_htmldescribe(int descr, dbref player, const char *name, const char *description)
{
    dbref   thing;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify_fmt(player, CFAIL "%s", tp_noguest_mesg);
	return;
    }

    if ((thing = match_controlled(descr, player, name)) != NOTHING) {
      if (Protect(thing) && !(MLevel(player) > MLevel(OWNER(thing)))) {
	  anotify_fmt(player, CFAIL "%s", tp_noperm_mesg);
	  return;
      }
	ts_modifyobject(thing);
	SETHTMLDESC(thing, description);
	if(*description)
	    anotify_nolisten2(player, CSUCC "HTMLDescription set.");
	else
	    anotify_nolisten2(player, CSUCC "HTMLDescription cleared.");
    }
}

void 
do_ihtmldescribe(int descr, dbref player, const char *name, const char *description)
{
    dbref   thing;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify_fmt(player, CFAIL "%s", tp_noguest_mesg);
	return;
    }

    if ((thing = match_controlled(descr, player, name)) != NOTHING) {
      if (Protect(thing) && !(MLevel(player) > MLevel(OWNER(thing)))) {
	  anotify_fmt(player, CFAIL "%s", tp_noperm_mesg);
	  return;
      }
	ts_modifyobject(thing);
	SETIHTMLDESC(thing, description);
	if(*description)
	    anotify_nolisten2(player, CSUCC "IHTMLDescription set.");
	else
	    anotify_nolisten2(player, CSUCC "IHTMLDescription cleared.");
    }
}

void 
do_doing(int descr, dbref player, const char *name, const char *mesg)
{
    dbref   thing;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify_fmt(player, CFAIL "%s", tp_noguest_mesg);
	return;
    }

    if( *mesg ) {
	thing = match_controlled(descr, player, name);
    } else {
	thing = player;
	mesg = name;
    }
    if (thing != NOTHING) {
      if (Protect(thing) && !(MLevel(player) > MLevel(OWNER(thing)))) {
	  anotify_fmt(player, CFAIL "%s", tp_noperm_mesg);
	  return;
      }
	ts_modifyobject(thing);
	SETDOING(thing, mesg);
	if(*mesg)
	    anotify_nolisten2(player, CSUCC "Doing set.");
	else
	    anotify_nolisten2(player, CSUCC "Doing cleared.");
    }
}

void 
do_fail(int descr, dbref player, const char *name, const char *message)
{
    dbref   thing;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify_fmt(player, CFAIL "%s", tp_noguest_mesg);
	return;
    }

    if ((thing = match_controlled(descr, player, name)) != NOTHING) {
	ts_modifyobject(thing);
	SETFAIL(thing, message);
	anotify_nolisten2(player, CSUCC "Message set.");
    }
}

void 
do_success(int descr, dbref player, const char *name, const char *message)
{
    dbref   thing;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify_fmt(player, CFAIL "%s", tp_noguest_mesg);
	return;
    }

    if ((thing = match_controlled(descr, player, name)) != NOTHING) {
	ts_modifyobject(thing);
	SETSUCC(thing, message);
	anotify_nolisten2(player, CSUCC "Message set.");
    }
}

/* sets the drop message for player */
void
do_drop_message(int descr, dbref player, const char *name, const char *message)
{
    dbref   thing;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify_fmt(player, CFAIL "%s", tp_noguest_mesg);
	return;
    }

    if ((thing = match_controlled(descr, player, name)) != NOTHING) {
	ts_modifyobject(thing);
	SETDROP(thing, message);
	anotify_nolisten2(player, CSUCC "Message set.");
    }
}

void 
do_osuccess(int descr, dbref player, const char *name, const char *message)
{
    dbref   thing;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify_fmt(player, CFAIL "%s", tp_noguest_mesg);
	return;
    }

    if ((thing = match_controlled(descr, player, name)) != NOTHING) {
	ts_modifyobject(thing);
	SETOSUCC(thing, message);
	anotify_nolisten2(player, CSUCC "Message set.");
    }
}

void 
do_ofail(int descr, dbref player, const char *name, const char *message)
{
    dbref   thing;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify_fmt(player, CFAIL "%s", tp_noguest_mesg);
	return;
    }

    if ((thing = match_controlled(descr, player, name)) != NOTHING) {
	ts_modifyobject(thing);
	SETOFAIL(thing, message);
	anotify_nolisten2(player, CSUCC "Message set.");
    }
}

void
do_odrop(int descr, dbref player, const char *name, const char *message)
{
    dbref   thing;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify_fmt(player, CFAIL "%s", tp_noguest_mesg);
	return;
    }

    if ((thing = match_controlled(descr, player, name)) != NOTHING) {
	ts_modifyobject(thing);
	SETODROP(thing, message);
	anotify_nolisten2(player, CSUCC "Message set.");
    }
}

void 
do_oecho(int descr, dbref player, const char *name, const char *message)
{
    dbref   thing;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify_fmt(player, CFAIL "%s", tp_noguest_mesg);
	return;
    }

    if ((thing = match_controlled(descr, player, name)) != NOTHING) {
	ts_modifyobject(thing);
	SETOECHO(thing, message);
	anotify_nolisten2(player, CSUCC "Message set.");
    }
}

void 
do_pecho(int descr, dbref player, const char *name, const char *message)
{
    dbref   thing;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify_fmt(player, CFAIL "%s", tp_noguest_mesg);
	return;
    }

    if ((thing = match_controlled(descr, player, name)) != NOTHING) {
	ts_modifyobject(thing);
	SETPECHO(thing, message);
	anotify_nolisten2(player, CSUCC "Message set.");
    }
}

/* sets a lock on an object to the lockstring passed to it.
   If the lockstring is null, then it unlocks the object.
   this returns a 1 or a 0 to represent success. */
int 
setlockstr(int descr, dbref player, dbref thing, const char *keyname)
{
    struct boolexp *key;

    if (*keyname != '\0') {
	key = parse_boolexp(descr, player, keyname, 0);
	if (key == TRUE_BOOLEXP) {
	    return 0;
	} else {
	    /* everything ok, do it */
	    ts_modifyobject(thing);
	    SETLOCK(thing, key);
	    return 1;
	}
    } else {
	ts_modifyobject(thing);
	CLEARLOCK(thing);
	return 1;
    }
}

void 
do_conlock(int descr, dbref player, const char *name, const char *keyname)
{
    dbref   thing;
    struct boolexp *key;
    struct match_data md;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify_fmt(player, CFAIL "%s", tp_noguest_mesg);
	return;
    }

    init_match(descr, player, name, NOTYPE, &md);
    match_absolute(&md);
    match_everything(&md);

    switch (thing = match_result(&md)) {
	case NOTHING:
	    anotify_nolisten2(player, CINFO "I don't see what you want to set the container-lock on!");
	    return;
	case AMBIGUOUS:
	    anotify_nolisten2(player, CINFO "I don't know which one you want to set the container-lock on!");
	    return;
	default:
	    if (!controls(player, thing)) {
		anotify_fmt(player, CFAIL "%s", tp_noperm_mesg);
		return;
	    }
	    break;
    }

    if (force_level) {
        anotify_nolisten2(player, CFAIL "You can't use @flock from an @force or {force}.");
        return;
    }

    if (!*keyname) {
	set_property(thing, "_/clk", PROP_LOKTYP, (PTYPE)TRUE_BOOLEXP);
	ts_modifyobject(thing);
	anotify_nolisten2(player, CSUCC "Container lock cleared.");
    } else {
	key = parse_boolexp(descr, player, keyname, 0);
	if (key == TRUE_BOOLEXP) {
	    anotify_nolisten2(player, CINFO "I don't understand that key.");
	} else {
	    /* everything ok, do it */
	    set_property(thing, "_/clk", PROP_LOKTYP, (PTYPE)key);
	    ts_modifyobject(thing);
	    anotify_nolisten2(player, CSUCC "Container lock set.");
	}
    }
}

void 
do_flock(int descr, dbref player, const char *name, const char *keyname)
{
    dbref   thing;
    struct boolexp *key;
    struct match_data md;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify_fmt(player, CFAIL "%s", tp_noguest_mesg);
	return;
    }

    init_match(descr, player, name, NOTYPE, &md);
    match_absolute(&md);
    match_everything(&md);

    switch (thing = match_result(&md)) {
	case NOTHING:
	    anotify_nolisten2(player, CINFO "I don't see that here.");
	    return;
	case AMBIGUOUS:
	    anotify_nolisten2(player, CINFO "I don't know which one you mean!");
	    return;
	default:
	    if (!controls(player, thing)) {
		anotify_fmt(player, CFAIL "%s", tp_noperm_mesg);
		return;
	    }
	    break;
    }

    if (!*keyname) {
	set_property(thing, "@/flk", PROP_LOKTYP, (PTYPE)TRUE_BOOLEXP);
	ts_modifyobject(thing);
	anotify_nolisten2(player, CSUCC "Force lock cleared.");
    } else {
	key = parse_boolexp(descr, player, keyname, 0);
	if (key == TRUE_BOOLEXP) {
	    anotify_nolisten2(player, CINFO "I don't understand that key.");
	} else {
	    /* everything ok, do it */
	    set_property(thing, "@/flk", PROP_LOKTYP, (PTYPE)key);
	    ts_modifyobject(thing);
	    anotify_nolisten2(player, CSUCC "Force lock set.");
	}
    }
}

void 
do_chlock(int descr, dbref player, const char *name, const char *keyname)
{
    dbref   thing;
    struct boolexp *key;
    struct match_data md;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify_fmt(player, CFAIL "%s", tp_noguest_mesg);
	return;
    }

    init_match(descr, player, name, NOTYPE, &md);
    match_absolute(&md);
    match_everything(&md);

    switch (thing = match_result(&md)) {
	case NOTHING:
	    anotify_nolisten2(player, CINFO "I don't see that here.");
	    return;
	case AMBIGUOUS:
	    anotify_nolisten2(player, CINFO "I don't know which one you mean!");
	    return;
	default:
	    if (!controls(player, thing)) {
		anotify_fmt(player, CFAIL "%s", tp_noperm_mesg);
		return;
	    }
	    break;
    }

    if (!*keyname) {
	set_property(thing, "_/chlk", PROP_LOKTYP, (PTYPE)TRUE_BOOLEXP);
	ts_modifyobject(thing);
	anotify_nolisten2(player, CSUCC "Chown lock cleared.");
    } else {
	key = parse_boolexp(descr, player, keyname, 0);
	if (key == TRUE_BOOLEXP) {
	    anotify_nolisten2(player, CINFO "I don't understand that key.");
	} else {
	    /* everything ok, do it */
	    set_property(thing, "_/chlk", PROP_LOKTYP, (PTYPE)key);
	    ts_modifyobject(thing);
	    anotify_nolisten2(player, CSUCC "Chown lock set.");
	}
    }
}

void 
do_lock(int descr, dbref player, const char *name, const char *keyname)
{
    dbref   thing;
    struct boolexp *key;
    struct match_data md;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify_fmt(player, CFAIL "%s", tp_noguest_mesg);
	return;
    }

    init_match(descr, player, name, NOTYPE, &md);
    match_absolute(&md);
    match_everything(&md);

    switch (thing = match_result(&md)) {
	case NOTHING:
	    anotify_nolisten2(player, CINFO "I don't see that here.");
	    return;
	case AMBIGUOUS:
	    anotify_nolisten2(player, CINFO "I don't know which one you mean!");
	    return;
	default:
	    if (!controls(player, thing)) {
		anotify_fmt(player, CFAIL "%s", tp_noperm_mesg);
		return;
	    }
	    break;
    }

    key = parse_boolexp(descr, player, keyname, 0);
    if (key == TRUE_BOOLEXP) {
	anotify_nolisten2(player, CINFO "I don't understand that key.");
    } else {
	/* everything ok, do it */
	SETLOCK(thing, key);
	ts_modifyobject(thing);
	anotify_nolisten2(player, CSUCC "Locked.");
    }
}

void 
do_unlock(int descr, dbref player, const char *name)
{
    dbref   thing;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify_fmt(player, CFAIL "%s", tp_noguest_mesg);
	return;
    }

    if ((thing = match_controlled(descr, player, name)) != NOTHING) {
	ts_modifyobject(thing);
	CLEARLOCK(thing);
	anotify_nolisten2(player, CSUCC "Unlocked.");
    }
}

int
controls_link(dbref who, dbref what)
{
    switch (Typeof(what)) {
	case TYPE_EXIT:
	    {
		int     i = DBFETCH(what)->sp.exit.ndest;

		while (i > 0) {
		    if (controls(who, DBFETCH(what)->sp.exit.dest[--i]))
			return 1;
		}
		if (who == OWNER(DBFETCH(what)->location))
		    return 1;
                return 0;
            }

	case TYPE_ROOM:
	    {
	        if (controls(who, DBFETCH(what)->sp.room.dropto))
	            return 1;
                return 0;
	    }

	case TYPE_PLAYER:
	    {
	        if (controls(who, DBFETCH(what)->sp.player.home))
	            return 1;
                return 0;
	    }

	case TYPE_THING:
	    {
	        if (controls(who, DBFETCH(what)->sp.thing.home))
	            return 1;
                return 0;
	    }

	case TYPE_PROGRAM:
	default:
	    return 0;
    }
}

void 
do_unlink(int descr, dbref player, const char *name)
{
    dbref   exit;
    struct match_data md;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify_fmt(player, CFAIL "%s", tp_noguest_mesg);
	return;
    }

    init_match(descr, player, name, TYPE_EXIT, &md);
    match_all_exits(&md);
    match_registered(&md);
    match_here(&md);
    match_absolute(&md);
    match_player(&md);
    switch (exit = match_result(&md)) {
	case NOTHING:
	    anotify_nolisten2(player, CINFO "I don't see that here.");
	    break;
	case AMBIGUOUS:
	    anotify_nolisten2(player, CINFO "I don't know which one you mean!");
	    break;
	default:
	    if (!controls(player, exit) && !controls_link(player, exit)) {
		anotify_fmt(player, CFAIL "%s", tp_noperm_mesg);
	    } else {
		switch (Typeof(exit)) {
		    case TYPE_EXIT:
			if (DBFETCH(exit)->sp.exit.ndest != 0) {
			    DBFETCH(OWNER(exit))->sp.player.pennies += tp_link_cost;
			    DBDIRTY(OWNER(exit));
			}
			ts_modifyobject(exit);
			DBSTORE(exit, sp.exit.ndest, 0);
			if (DBFETCH(exit)->sp.exit.dest) {
			    free((void *) DBFETCH(exit)->sp.exit.dest);
			    DBSTORE(exit, sp.exit.dest, NULL);
			}
			anotify_nolisten2(player, CSUCC "Unlinked.");
			if (MLevel(exit)) {
			    SetMLevel(exit, 0);
			    anotify_nolisten2(player, CINFO "Action priority Level reset to 0.");
			}
			break;
		    case TYPE_ROOM:
			ts_modifyobject(exit);
			DBSTORE(exit, sp.room.dropto, NOTHING);
			anotify_nolisten2(player, CSUCC "Dropto removed.");
			break;
		    case TYPE_THING:
			ts_modifyobject(exit);
			DBSTORE(exit, sp.thing.home, OWNER(exit));
			anotify_nolisten2(player, CSUCC "Thing's home reset to owner.");
			break;
		    case TYPE_PLAYER:
			ts_modifyobject(exit);
			DBSTORE(exit, sp.player.home, tp_player_start);
			anotify_nolisten2(player, CSUCC "Player's home reset to default player start room.");
			break;
		    default:
			anotify_nolisten2(player, CFAIL "You can't unlink that!");
			break;
		}
	    }
    }
}

void 
do_chown(int descr, dbref player, const char *name, const char *newowner)
{
    dbref   thing;
    dbref   owner;
    struct match_data md;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify_fmt(player, CFAIL "%s", tp_noguest_mesg);
	return;
    }

    init_match(descr, player, name, NOTYPE, &md);
    match_everything(&md);
    if ((thing = noisy_match_result(&md)) == NOTHING)
	return;

    if (*newowner && string_compare(newowner, "me")) {
	if ((owner = lookup_player(newowner)) == NOTHING) {
	    anotify_nolisten2(player, CINFO "Who?");
	    return;
	}
    } else {
	owner = OWNER(player);
    }

    if ( !controls(OWNER(player), owner) ) {
	anotify_fmt(player, CFAIL "%s", tp_noperm_mesg);
	return;
    }
    if (!controls(OWNER(player),thing) && (
	!(FLAGS(thing) & CHOWN_OK) ||
	Typeof(thing) == TYPE_PROGRAM ||
	!test_lock(descr, player, thing, "_/chlk") )) {
	anotify_nolisten2(player, CFAIL "You can't take possession of that.");
	return;
    }
    if( (Protect(owner) && !(player == owner)) || (Protect(OWNER(thing)) && !(player == OWNER(thing))) ) {
	anotify_fmt(player, CFAIL "%s", tp_noperm_mesg);
	return;
    }
    switch (Typeof(thing)) {
	case TYPE_ROOM:
	    if (!Mage(OWNER(player)) && DBFETCH(player)->location != thing) {
		anotify_nolisten2(player, CINFO "You can only chown \"here\".");
		return;
	    }
	    ts_modifyobject(thing);
	    OWNER(thing) = OWNER(owner);
	    break;
	case TYPE_THING:
	    if (!Mage(OWNER(player)) && DBFETCH(thing)->location != player) {
		anotify_nolisten2(player, CINFO "You aren't carrying that.");
		return;
	    }
	    ts_modifyobject(thing);
	    OWNER(thing) = OWNER(owner);
	    break;
	case TYPE_PLAYER:
	    anotify_nolisten2(player, CFAIL "Players always own themselves.");
	    return;
	case TYPE_EXIT:
	case TYPE_PROGRAM:
	    ts_modifyobject(thing);
	    OWNER(thing) = OWNER(owner);
	    break;
	case TYPE_GARBAGE:
	    anotify_nolisten2(player, CFAIL "Nobody wants garbage.");
	    return;
    }
    if (owner == player) {
	char    buf[BUFFER_LEN], buf1[BUFFER_LEN];
	strcpy( buf1, unparse_object(player, thing));
	sprintf(buf, CSUCC "Owner of %s changed to you.", buf1 );
	anotify_nolisten2(player, buf);
    } else {
	char    buf[BUFFER_LEN], buf1[BUFFER_LEN], buf2[BUFFER_LEN];

	strcpy( buf1, unparse_object(player, thing));
	strcpy( buf2, unparse_object(player, owner));
	sprintf(buf, CSUCC "Owner of %s changed to %s.", buf1, buf2 );
	anotify_nolisten2(player, buf);
    }
    DBDIRTY(thing);
}


/* Note: Gender code taken out.  All gender references are now to be handled
   by property lists...
   Setting of flags and property code done here.  Note that the PROP_DELIMITER
   identifies when you're setting a property.
   A @set <thing>= :clear
   will clear all properties.
   A @set <thing>= type:
   will remove that property.
   A @set <thing>= propname:string
   will add that string property or replace it.
   A @set <thing>= propname:^value
   will add that integer property or replace it.
 */

void
do_sm( dbref player, dbref thing, int mlev ) {
    player = OWNER(player);

    if( (mlev > MLevel(player)) || (MLevel(thing) > MLevel(player)) ||
        ((mlev > LMPI) && !Mage(player) && (Typeof(thing) != TYPE_PROGRAM )) ||
        ( (Typeof(thing) == TYPE_PLAYER) && !Man(player) && !Boy(player) &&
                ( (mlev >= LMAGE) || TMage(thing) )
        ) ||
        ( (Typeof(thing) == TYPE_PLAYER) && (!Wiz(player) ||
                (mlev >= MLevel(player)) || (MLevel(thing) >= MLevel(player))
        ) && !Man(player) )
    ) {
	anotify_fmt(player, CFAIL "%s", tp_noperm_mesg);
	return;
    }
    if (force_level) {
	anotify_nolisten2(player, CFAIL "Can't set this flag from an @force or {force}.");
	return;
    }

    if (mlev >= LMAGE)
	anotify_nolisten2(player, CSUCC "Wizard level set.");	
    else if (mlev)
	anotify_nolisten2(player, CSUCC "Mucker level set.");
    else if(TMage(thing))
	anotify_nolisten2(player, CSUCC "Wizard bit removed.");
    else
	anotify_nolisten2(player, CSUCC "Mucker bit removed.");

    SetMLevel(thing, mlev);
}

void
do_mush_set(int descr, dbref player, char *name, char *setting, char *command)
{
   char *prop;
   char *temp;
   dbref thing;


   if(tp_db_readonly) return;

   if(Guest(player)) {
      anotify_fmt(player, CFAIL "%s", tp_noguest_mesg);
      return;
   }

   if (tp_db_readonly) {
      anotify_nolisten2(player, CFAIL DBRO_MESG);
      return;
   }

   *command++;
   if(!(*command)) {
      anotify_nolisten2(player, CFAIL "That is a bad name for a property!");
      return;
   }
   prop = command;

   if(index(command, PROP_DELIMITER)) {
      anotify_nolisten2(player, CFAIL "That is a bad name for a property!");
      return;
   }

   if(!Arch(OWNER(player)) && Prop_Hidden(prop)){
      anotify_fmt(player, CFAIL "%s", tp_noperm_mesg);
      return;
   }

   if(!Wiz(OWNER(player)) && Prop_SeeOnly(prop)){
      anotify_fmt(player, CFAIL "%s", tp_noperm_mesg);
      return;
   }

   /* find thing */
   if ((thing = match_controlled(descr, player, name)) == NOTHING)
      return;

    if (Protect(thing) && !(MLevel(player) > MLevel(OWNER(thing)))) {
	anotify_fmt(player, CFAIL "%s", tp_noperm_mesg);
	return;
    }

   if(!(*setting)) {
      ts_modifyobject(thing);
      remove_property(thing, prop);
      anotify_nolisten2(player, CSUCC "Property removed.");
   } else {
      ts_modifyobject(thing);
      add_property(thing, prop, setting, 0);
      anotify_nolisten2(player, CSUCC "Property set.");
   }

   return;
}

void 
do_set(int descr, dbref player, const char *name, const char *flag)
{
    dbref   thing;
    const char *p;
    object_flag_type f=0, f2=0;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify_fmt(player, CFAIL "%s", tp_noguest_mesg);
	return;
    }

    if (tp_db_readonly) {
	anotify_nolisten2(player, CFAIL DBRO_MESG);
	return;
    }

    /* find thing */
    if ((thing = match_controlled(descr, player, name)) == NOTHING)
	return;

    /* move p past NOT_TOKEN if present */
    for (p = flag; *p && (*p == NOT_TOKEN || isspace(*p)); p++);

    /* Now we check to see if it's a property reference */
    /* if this gets changed, please also modify boolexp.c */
    if (index(flag, PROP_DELIMITER)) {
	/* copy the string so we can muck with it */
	char   *type = alloc_string(flag);	/* type */
	char   *class = (char *) index(type, PROP_DELIMITER);	/* class */
	char   *x;	/* to preserve string location so we can free it */
	char   *temp;
	int ival = 0;

    if (Protect(thing) && !(MLevel(player) > MLevel(OWNER(thing)))) {
	anotify_fmt(player, CFAIL "%s", tp_noperm_mesg);
	return;
    }

	x = type;
	while (isspace(*type) && (*type != PROP_DELIMITER))
	    type++;
	if (*type == PROP_DELIMITER) {
	    /* clear all properties */
	    for (type++; isspace(*type); type++);
	
	    if (string_compare(type, "clear")) {
		anotify_nolisten2(player, CINFO "Use '@set <obj>=:clear' to clear all props on an object.");
		free((void *) x);
		return;
	    }
	    remove_property_list(thing, Arch(OWNER(player)));
	    ts_modifyobject(thing);
	    anotify_nolisten2(player, CSUCC "All user-owned properties removed.");
	    free((void *) x);
	    return;
	}
	/* get rid of trailing spaces and slashes */
	for (temp = class - 1; temp >= type && isspace(*temp); temp--);
	while (temp >= type && *temp == '/') temp--;
	*(++temp) = '\0';

	class++;		/* move to next character */
	/* while (isspace(*class) && *class) class++; */
	if (*class == '^' && number(class+1))
	    ival = atoi(++class);

	if (!Arch(OWNER(player)) && Prop_Hidden(type)){
     	    anotify_fmt(player, CFAIL "%s", tp_noperm_mesg);
	    free((void *) x);
	    return;
	}
	if (!Wiz(OWNER(player)) && Prop_SeeOnly(type)){
	    anotify_fmt(player, CFAIL "%s", tp_noperm_mesg);
	    free((void *) x);
	    return;
	}
	if (!(*class)) {
	    ts_modifyobject(thing);
	    remove_property(thing, type);
	    anotify_nolisten2(player, CSUCC "Property removed.");
	} else {
	    ts_modifyobject(thing);
	    if (ival) {
		add_property(thing, type, NULL, ival);
	    } else {
		add_property(thing, type, class, 0);
	    }
	    anotify_nolisten2(player, CSUCC "Property set.");
	}
	free((void *) x);
	return;
    }
        /* identify flag */
    if (*p == '\0') {
	anotify_nolisten2(player, CINFO "You must specify a flag to set.");
	return;
    } else if ( string_prefix("ABODE", p) ||
                string_prefix("AUTOSTART", p) ||
                string_prefix("ABATE", p) ) {
	f = ABODE;
    } else if ( !string_compare("0", p) || !string_compare("M0", p) ||
		!string_compare("W0", p) || ( (
		 string_prefix("MEEPER", p) || string_prefix("MPI", p) ||
		 string_prefix("MUCKER", p) || string_prefix("MAGE", p) ||
		 string_prefix("WIZARD", p) || string_prefix("ARCHWIZARD", p) ||
		!string_compare("M1", p) || !string_compare("M2", p) ||
		!string_compare("M3", p) || !string_compare("W1", p) ||
		!string_compare("W2", p) || !string_compare("W3", p) ||
		!string_compare("W4", p) || !string_compare("BOY", p)
	       ) && (*flag == NOT_TOKEN) ) ) {
	do_sm(player, thing, 0);
	return;
    } else if ( string_prefix("MPI", p) || string_prefix("MEEPER", p) ) {
	do_sm(player, thing, LMPI);
	return;
    } else if ( !string_compare("1", p) || !string_compare("M1", p) ||
		string_prefix("MUCKER", p) ) {
	do_sm(player, thing, LM1);
	return;
    } else if ( !string_compare("2", p) || !string_compare("M2", p) ) {
	do_sm(player, thing, LM2);
	return;
    } else if ( !string_compare("3", p) || !string_compare("M3", p) ) {
	do_sm(player, thing, LM3);
	return;
    } else if ( !string_compare("W1", p) || string_prefix("MAGE", p) ) {
	do_sm(player, thing, LMAGE);
	return;
    } else if ( !string_compare("W2", p) || string_prefix("WIZARD", p) ) {
	do_sm(player, thing, LWIZ);
	return;
    } else if ( !string_compare("W3", p) || string_prefix("ARCHWIZARD", p) ) {
	do_sm(player, thing, LARCH);
	return;
    } else if ( !string_compare("W4", p) || !string_compare("BOY", p) ) {
	do_sm(player, thing, LBOY);
	return;
    } else if (string_prefix("PARENT", p) || (string_prefix("PROG_DEBUG", p) && !string_prefix("PRO", p)) || !string_compare("%", p)) {
	f2 = F2PARENT;
    } else if (string_prefix("ZOMBIE", p) || string_prefix("PUPPET", p)) {
	f = ZOMBIE;
    } else if (string_prefix("VEHICLE", p) || string_prefix("VIEWABLE", p)) {
	if (*flag == NOT_TOKEN && Typeof(thing) == TYPE_THING) {
	    dbref obj = DBFETCH(thing)->contents;
	    for (; obj != NOTHING; obj = DBFETCH(obj)->next) {
		if (Typeof(obj) == TYPE_PLAYER) {
		    anotify_nolisten2(player, CINFO "That vehicle still has players in it!");
		    return;
		}
	    }
	}
	f = VEHICLE;
    } else if (string_prefix("LINK_OK", p) || string_prefix("LIGHT", p)) {
	f = LINK_OK;

    } else if (string_prefix("XFORCIBLE", p)) {
	if (force_level) {
	    anotify_nolisten2(player, CFAIL "Can't set this flag from an @force or {force}.");
	    return;
	}
	f = XFORCIBLE;

    } else if ((string_prefix("DARK", p)) || (string_prefix("DEBUG", p))) {
	f = DARK;
    } else if ((string_prefix("STICKY", p)) || (string_prefix("SETUID", p)) ||
	       (string_prefix("SILENT", p))) {
	f = STICKY;
    } else if (string_prefix("QUELL", p)) {
	f = QUELL;
    } else if (string_prefix("BUILDER", p) || string_prefix("BOUND", p)) {
	f = BUILDER;
    } else if (string_prefix("CHOWN_OK", p) || string_prefix("COLOR_ANSI", p) || string_prefix("COLOR_ON", p)) {
	f = CHOWN_OK;
    } else if (string_prefix("JUMP_OK", p)) {
	f = JUMP_OK;
    } else if (string_prefix("HAVEN", p) || string_prefix("HARDUID", p) || string_prefix("HIDE", p)) {
	f = HAVEN;

    } else if (string_prefix("GUEST", p)) {
	f2 = F2GUEST;
    } else if (string_prefix("LOGWALL", p) || !string_compare("!", p)) {
	f2 = F2LOGWALL;
    } else if (string_prefix("MUFCOUNT", p) || !string_compare("+", p)) {
	f2 = F2MUFCOUNT;
    } else if (string_prefix("MOBILE", p) || string_prefix("OFFER", p)) {
	f2 = F2MOBILE;
    } else if (string_prefix("PROTECT", p) || !string_compare("*", p)) {
	f2 = F2PROTECT;
    } else if (string_prefix("ANTIPROTECT", p) || !string_compare("I", p)) {
	f2 = F2ANTIPROTECT;
    } else if (string_prefix("EXAMINE_OK", p) || !string_compare("&", p)) {
	f2 = F2EXAMINE_OK;
    } else if (string_prefix("HIDDEN", p) || !string_compare("#", p)) {
	f2 = F2HIDDEN;
/*    } else if (string_prefix("PUEBLO", p)) {
        anotify_nolisten2(player, CFAIL "This flag can't be user-set."); */ /* Why did we have this here? It is unrequired */
    } else {
	anotify_nolisten2(player, CINFO "I don't recognize that flag.");
	return;
    }
    if ((Protect(thing) && !(MLevel(player) > MLevel(OWNER(thing)))) && !(f2 == F2PROTECT)) {
	anotify_fmt(player, CFAIL "%s", tp_noperm_mesg);
	return;
    }
  if( f2 ) { /* New flags */
    /* check for restricted flag */
    if (restricted2(player, thing, f2)) {
	anotify_fmt(player, CFAIL "%s", tp_noperm_mesg);
	return;
    }
    /* else everything is ok, do the set */
    if (*flag == NOT_TOKEN) {
	/* reset the flag */
	ts_modifyobject(thing);
	FLAG2(thing) &= ~f2;
	DBDIRTY(thing);
	anotify_nolisten2(player, CSUCC "Flag reset.");
    } else {
	/* set the flag */
	ts_modifyobject(thing);
	FLAG2(thing) |= f2;
	DBDIRTY(thing);
	anotify_nolisten2(player, CSUCC "Flag set.");
    }
  } else {
    /* check for restricted flag */
    if (restricted(player, thing, f)) {
	anotify_fmt(player, CFAIL "%s", tp_noperm_mesg);
	return;
    }
    /* else everything is ok, do the set */
    if (*flag == NOT_TOKEN) {
	/* reset the flag */
	ts_modifyobject(thing);
	FLAGS(thing) &= ~f;
	DBDIRTY(thing);
	anotify_nolisten2(player, CSUCC "Flag reset.");
    } else {
	/* set the flag */
	ts_modifyobject(thing);
	FLAGS(thing) |= f;
	DBDIRTY(thing);
	anotify_nolisten2(player, CSUCC "Flag set.");
    }
  }
}

void
do_propset(int descr, dbref player, const char *name, const char *prop)
{
    dbref   thing, ref;
    char *p, *q;
    char buf[BUFFER_LEN];
    char *type, *pname, *value;
    struct match_data md;
    struct boolexp *lok;

    if(tp_db_readonly) return;

    if(Guest(player)) {
	anotify_fmt(player, CFAIL "%s", tp_noguest_mesg);
	return;
    }

    /* find thing */
    if ((thing = match_controlled(descr, player, name)) == NOTHING)
	return;

    if (Protect(thing) && !(MLevel(player) > MLevel(OWNER(thing)))) {
	anotify_fmt(player, CFAIL "%s", tp_noperm_mesg);
	return;
    }

    while (isspace(*prop)) prop++;
    strcpy(buf, prop);

    type = p = buf;
    while(*p && *p != PROP_DELIMITER) p++;
    if (*p) *p++ = '\0';

    if (*type) {
	q = type + strlen(type) - 1;
	while (q >= type && isspace(*q)) *(q--) = '\0';
    }

    pname = p;
    while(*p && *p != PROP_DELIMITER) p++;
    if (*p) *p++ = '\0';
    value = p;

    while (*pname == PROPDIR_DELIMITER || isspace(*pname)) pname++;
    if (*pname) {
	q = pname + strlen(pname) - 1;
	while (q >= pname && isspace(*q)) *(q--) = '\0';
    }

    if (!*pname) {
	anotify_nolisten2(player, CINFO "What property?");
	return;
    }

    if (!Arch(OWNER(player)) && Prop_Hidden(type)){
	anotify_fmt(player, CFAIL "%s", tp_noperm_mesg);
	return;
    }
    if (!Wiz(OWNER(player)) && Prop_SeeOnly(type)){
	anotify_fmt(player, CFAIL "%s", tp_noperm_mesg);
	return;
    }

    if (!*type || string_prefix("string", type)) {
	add_property(thing, pname, value, 0);
    } else if (string_prefix("integer", type)) {
	if (!number(value)) {
	    anotify_nolisten2(player, CINFO "That's not an integer.");
	    return;
	}
	add_property(thing, pname, NULL, atoi(value));
    } else if (string_prefix("float", type)) {
       if (!ifloat(value)) {
          notify(player, "That's not a floating point number!");
          return;
       }
       set_property(thing, pname, PROP_FLTTYP, (PTYPE)value);
    } else if (string_prefix("dbref", type)) {
	init_match(descr, player, value, NOTYPE, &md);
	match_absolute(&md);
	match_everything(&md);
	if ((ref = noisy_match_result(&md)) == NOTHING) return;
	set_property(thing, pname, PROP_REFTYP, (PTYPE)ref);
    } else if (string_prefix("lock", type)) {
	lok = parse_boolexp(descr, player, value, 0);
	if (lok == TRUE_BOOLEXP) {
	    anotify_nolisten2(player, CINFO "I don't understand that lock.");
	    return;
	}
	set_property(thing, pname, PROP_LOKTYP, (PTYPE)lok);
    } else if (string_prefix("erase", type)) {
	if (*value) {
	    anotify_nolisten2(player, CINFO "Don't give a value when erasing a property.");
	    return;
	}
	remove_property(thing, pname);
	anotify_nolisten2(player, CSUCC "Property erased.");
	return;
    } else {
	anotify_nolisten2(player, CINFO "What type of property?");
	anotify_nolisten2(player, CNOTE "Valid types are string, int, float, dbref, lock, and erase.");
	return;
    }
    anotify_nolisten2(player, CSUCC "Property set.");
}



