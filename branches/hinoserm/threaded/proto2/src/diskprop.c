#include "copyright.h"
#include "config.h"
#include "params.h"

#include "db.h"
#include "tune.h"
#include "props.h"
#include "externs.h"
#include "interface.h"

#ifdef DISKBASE

extern FILE *input_file;
extern FILE *delta_infile;
extern FILE *delta_outfile;

long propcache_hits = 0L;
long propcache_misses = 0L;

struct pload_Q {
    dbref obj;
    int count;
    int Qtype;
};

struct pload_Q propchanged_Q = { NOTHING, 0, PROPS_CHANGED };
struct pload_Q proploaded_Q = { NOTHING, 0, PROPS_LOADED };
struct pload_Q proppri_Q = { NOTHING, 0, PROPS_PRIORITY };

void
removeobj_ringqueue(dbref obj)
{
    struct pload_Q *ref = NULL;

    switch (DBFETCH(obj)->propsmode) {
        case PROPS_UNLOADED:
            return;
            break;              /* never gets here */
        case PROPS_LOADED:
            ref = &proploaded_Q;
            break;
        case PROPS_PRIORITY:
            ref = &proppri_Q;
            break;
        case PROPS_CHANGED:
            ref = &propchanged_Q;
            break;
    }

    if (DBFETCH(obj)->nextold == NOTHING || DBFETCH(obj)->prevold == NOTHING)
        return;
    if (!ref || ref->obj == NOTHING)
        return;

    if (DBFETCH(obj)->nextold == obj || DBFETCH(obj)->prevold == obj) {
        if (ref->obj == obj)
            ref->obj = NOTHING;
    } else {
        DBFETCH(DBFETCH(obj)->prevold)->nextold = DBFETCH(obj)->nextold;
        DBFETCH(DBFETCH(obj)->nextold)->prevold = DBFETCH(obj)->prevold;
        if (ref->obj == obj)
            ref->obj = DBFETCH(obj)->nextold;
    }
    DBFETCH(obj)->prevold = NOTHING;
    DBFETCH(obj)->nextold = NOTHING;
    ref->count--;
}

void
addobject_ringqueue(dbref obj, int mode)
{
    struct pload_Q *ref = NULL;

    removeobj_ringqueue(obj);

    DBFETCH(obj)->propsmode = mode;
    switch (mode) {
        case PROPS_UNLOADED:
            DBFETCH(obj)->nextold = NOTHING;
            DBFETCH(obj)->prevold = NOTHING;
            return;
            break;
        case PROPS_LOADED:
            ref = &proploaded_Q;
            break;
        case PROPS_PRIORITY:
            ref = &proppri_Q;
            break;
        case PROPS_CHANGED:
            ref = &propchanged_Q;
            break;
    }

    if (ref->obj == NOTHING) {
        DBFETCH(obj)->nextold = obj;
        DBFETCH(obj)->prevold = obj;
        ref->obj = obj;
    } else {
        DBFETCH(obj)->nextold = ref->obj;
        DBFETCH(DBFETCH(ref->obj)->prevold)->nextold = obj;
        DBFETCH(obj)->prevold = DBFETCH(ref->obj)->prevold;
        DBFETCH(ref->obj)->prevold = obj;
    }
    ref->count++;
}


dbref
first_ringqueue_obj(struct pload_Q *ref)
{
    if (!ref)
        return NOTHING;
    return (ref->obj);
}


dbref
next_ringqueue_obj(struct pload_Q *ref, dbref obj)
{
    if (DBFETCH(obj)->nextold == ref->obj)
        return NOTHING;
    return (DBFETCH(obj)->nextold);
}


#define FETCHSTATS_INTERVAL1 120
#define FETCHSTATS_INTERVAL2 600
#define FETCHSTATS_INTERVAL3 3600

#define FETCHSTATS_SLOT_TIME 30
#define FETCHSTATS_SLOTS ((FETCHSTATS_INTERVAL3 / FETCHSTATS_SLOT_TIME) + 1)

static int fetchstats[FETCHSTATS_SLOTS];
static int lastfetchslot = -1;

void
update_fetchstats(void)
{
    int slot =
        ((current_systime / FETCHSTATS_SLOT_TIME) % FETCHSTATS_SLOTS);
    int i;

    if (slot != lastfetchslot) {
        if (lastfetchslot == -1) {
            for (i = 0; i < FETCHSTATS_SLOTS; i++) {
                fetchstats[i] = -1;
            }
        }
        while (lastfetchslot != slot) {
            lastfetchslot = ((lastfetchslot + 1) % FETCHSTATS_SLOTS);
            fetchstats[lastfetchslot] = 0;
        }
    }
    fetchstats[slot] += 1;
}


void
report_fetchstats(dbref player)
{
    int i =
        ((current_systime / FETCHSTATS_SLOT_TIME) % FETCHSTATS_SLOTS);
    int count = 0;
    double sum = 0.0, maxv = 0.0, minv;

    while (lastfetchslot != i) {
        lastfetchslot = ((lastfetchslot + 1) % FETCHSTATS_SLOTS);
        fetchstats[lastfetchslot] = 0;
    }

    minv = fetchstats[i];

    for (; count < (FETCHSTATS_INTERVAL1 / FETCHSTATS_SLOT_TIME); count++) {
        if (fetchstats[i] == -1)
            break;
        sum += fetchstats[i];
        if (fetchstats[i] < minv)
            minv = fetchstats[i];
        if (fetchstats[i] > maxv)
            maxv = fetchstats[i];
        i = ((i + FETCHSTATS_SLOTS - 1) % FETCHSTATS_SLOTS);
    }

    notify_fmt(player, "Disk Fetches %2g minute min/ave/max: %.2f/%.2f/%.2f",
               (FETCHSTATS_INTERVAL1 / 60.0),
               (minv * 60.0 / FETCHSTATS_SLOT_TIME),
               (sum * 60.0 / (FETCHSTATS_SLOT_TIME * count)),
               (maxv * 60.0 / FETCHSTATS_SLOT_TIME));

    for (; count < (FETCHSTATS_INTERVAL2 / FETCHSTATS_SLOT_TIME); count++) {
        if (fetchstats[i] == -1)
            break;
        sum += fetchstats[i];
        if (fetchstats[i] < minv)
            minv = fetchstats[i];
        if (fetchstats[i] > maxv)
            maxv = fetchstats[i];
        i = ((i + FETCHSTATS_SLOTS - 1) % FETCHSTATS_SLOTS);
    }

    notify_fmt(player, "Disk Fetches %2g minute min/ave/max: %.2f/%.2f/%.2f",
               (FETCHSTATS_INTERVAL2 / 60.0),
               (minv * 60.0 / FETCHSTATS_SLOT_TIME),
               (sum * 60.0 / (FETCHSTATS_SLOT_TIME * count)),
               (maxv * 60.0 / FETCHSTATS_SLOT_TIME));

    for (; count < (FETCHSTATS_INTERVAL3 / FETCHSTATS_SLOT_TIME); count++) {
        if (fetchstats[i] == -1)
            break;
        sum += fetchstats[i];
        if (fetchstats[i] < minv)
            minv = fetchstats[i];
        if (fetchstats[i] > maxv)
            maxv = fetchstats[i];
        i = ((i + FETCHSTATS_SLOTS - 1) % FETCHSTATS_SLOTS);
    }

    notify_fmt(player, "Disk Fetches %2g minute min/ave/max: %.2f/%.2f/%.2f",
               (FETCHSTATS_INTERVAL3 / 60.0),
               (minv * 60.0 / FETCHSTATS_SLOT_TIME),
               (sum * 60.0 / (FETCHSTATS_SLOT_TIME * count)),
               (maxv * 60.0 / FETCHSTATS_SLOT_TIME));
}

void
report_cachestats(dbref player)
{
    dbref obj;
    int count, checked = 0, ipct;
    bool gap = 0;
    time_t when, now;

    notify(player, "LRU proploaded cache time distribution graph.");

    when = now = time(NULL);
    notify(player, "Mins  Objs (%of db) Graph of #objs vs. age.");

    for (; checked < proploaded_Q.count; when -= 60) {
        count = 0;
        obj = first_ringqueue_obj(&proploaded_Q);
        while (obj != NOTHING) {
            if (DBFETCH(obj)->propstime > (when - 60)
                && DBFETCH(obj)->propstime <= when)
                count++;
            obj = next_ringqueue_obj(&proploaded_Q, obj);
        }
        checked += count;
        ipct = count * 100 / proploaded_Q.count;
        if (ipct > 50)
            ipct = 50;

        if (count) {
            if (gap)
                notify(player, "[gap]");

            notify_fmt(player, "%3ld:%6d (%5.2f%%) %*s", ((now - when) / 60),
                       count, (count * 100.0 / proploaded_Q.count), ipct, "*");
            gap = 0;
        } else {
            gap = 1;
        }
    }
}

void
diskbase_debug(dbref player)
{
    notify_fmt(player, "Propcache hit ratio: %.3f%% (%ld hits / %ld fetches)",
               (100.0 * propcache_hits / (propcache_hits + propcache_misses)),
               propcache_hits, propcache_misses);

    report_fetchstats(player);

    notify_fmt(player, "PropLoaded count: %d", proploaded_Q.count);
    notify_fmt(player, "PropPriority count: %d", proppri_Q.count);
    notify_fmt(player, "PropChanged count: %d", propchanged_Q.count);

    report_cachestats(player);
}


void
unloadprops_with_prejudice(dbref obj)
{
    PropPtr l;

    if ((l = DBFETCH(obj)->properties)) {
        /* if it has props, then dispose */
        delete_proplist(l);
        DBFETCH(obj)->properties = NULL;
    }
    removeobj_ringqueue(obj);
    DBFETCH(obj)->propsmode = PROPS_UNLOADED;
    DBFETCH(obj)->propstime = 0;
}


int
disposeprops_notime(dbref obj)
{
    if (DBFETCH(obj)->propsmode == PROPS_UNLOADED)
        return 0;
    if (DBFETCH(obj)->propsmode == PROPS_CHANGED)
        return 0;

    unloadprops_with_prejudice(obj);
    return 1;
}


int
disposeprops(dbref obj)
{
    if ((current_systime - DBFETCH(obj)->propstime) < tp_clean_interval)
        return 0;               /* don't dispose if less than X minutes old */
    return disposeprops_notime(obj);
}


void
dispose_all_oldprops(void)
{
    dbref i;
    time_t now = current_systime;

    for (i = 0; i < db_top; i++) {
        if ((now - DBFETCH(i)->propstime) >= tp_clean_interval)
            disposeprops_notime(i);
    }
}

void
housecleanprops(void)
{
    int limit, max;
    dbref i, j;

    if ((proploaded_Q.count < 10) ||
        (proploaded_Q.count < (tp_max_loaded_objs * db_top / 100)))
        return;

    limit = 40;
    max = db_top;
    i = first_ringqueue_obj(&proploaded_Q);
    while (limit > 0 && max-- > 0 && i != NOTHING) {
        j = next_ringqueue_obj(&proploaded_Q, i);
        if (disposeprops_notime(i))
            limit--;
        i = j;
    }
}


int
fetchprops_priority(dbref obj, int mode)
{
    /* update fetched timestamp */
    DBFETCH(obj)->propstime = current_systime;

    /* if in memory, don't try to reload. */
    if (DBFETCH(obj)->propsmode != PROPS_UNLOADED) {

        /* but do update the queue position */
        addobject_ringqueue(obj, DBFETCH(obj)->propsmode);
        propcache_hits++;
        return 0;
    }

    propcache_misses++;

    housecleanprops();

    /* actually load in properties from the appropriate file */
    if (FLAGS(obj) & SAVED_DELTA) {
        getproperties(delta_infile, obj);
    } else {
        getproperties(input_file, obj);
    }

    /* update fetch statistics */
    if (!mode)
        update_fetchstats();

    /* add object to appropriate queue */
    addobject_ringqueue(obj, ((mode) ? PROPS_PRIORITY : PROPS_LOADED));

    return 1;
}


void
fetchprops(dbref obj)
{
	DBLOCK(obj);
    fetchprops_priority(obj, 0);
	DBUNLOCK(obj);
}


void
dirtyprops(dbref obj)
{
    if (DBFETCH(obj)->propsmode == PROPS_CHANGED)
        return;
	DBLOCK(obj);
    addobject_ringqueue(obj, PROPS_CHANGED);
	DBUNLOCK(obj);
}

void
undirtyprops(dbref obj)
{
    if (DBFETCH(obj)->propsmode == PROPS_UNLOADED)
        return;

    if (DBFETCH(obj)->propsmode != PROPS_CHANGED) {
        disposeprops(obj);
        return;
    }

    addobject_ringqueue(obj, PROPS_LOADED);
    disposeprops(obj);
}

int
propfetch(dbref obj, PropPtr p)
{
    FILE *f;

    if (!p)
        return 0;
    SetPFlags(p, (PropFlags(p) | PROP_TOUCHED));
    if (PropFlags(p) & PROP_ISUNLOADED) {
        f = (FLAGS(obj) & SAVED_DELTA) ? delta_infile : input_file;
        db_get_single_prop(f, obj, PropDataVal(p));
        return 1;
    }
    return 0;
}

#endif /* DISKBASE */
