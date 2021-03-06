/* Primitives package */

#include "copyright.h"
#include "config.h"

#include <stdio.h>
#include <ctype.h>

#include "db.h"
#include "inst.h"
#include "externs.h"
#include "match.h"
#include "interface.h"
#include "params.h"
#include "tune.h"
#include "strings.h"
#include "interp.h"

/* We'll put the external temp vars here */

struct inst *oper1, *oper2, *oper3, *oper4;
struct inst temp1, temp2, temp3;
struct inst arrayvar;
int result, tmp;
dbref ref;
char buf[BUFFER_LEN];

void
prim_pop(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    CLEAR(oper1);
}

void
prim_dup(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    CHECKOFLOW(1);
    copyinst(&arg[*top - 1], &arg[*top]);
    (*top)++;
}

void
prim_dupn(PRIM_PROTOTYPE)
{
	int i;

	CHECKOP(1);
	oper1 = POP();
	if (oper1->type != PROG_INTEGER)
		abort_interp("Operand is not an integer.");
	result = oper1->data.number;
	if (result < 0)
		abort_interp("Operand is negative.");
	CLEAR(oper1);
	CHECKOP(result);
	nargs = 0;
	CHECKOFLOW(result);
	for (i = result; i > 0; i--) {
		copyinst(&arg[*top - result], &arg[*top]);
		(*top)++;
	}
}


void
prim_ldup(PRIM_PROTOTYPE)
{
	int i;

	CHECKOP(1);
	oper1 = POP();
	if (oper1->type != PROG_INTEGER)
		abort_interp("Operand is not an integer.");
	result = oper1->data.number;
	if (result < 0)
		abort_interp("Operand is negative.");
	CLEAR(oper1);
	PushInt(result);
	result++;
	CHECKOP(result);
	nargs = 0;
	CHECKOFLOW(result);
	for (i = result; i > 0; i--) {
		copyinst(&arg[*top - result], &arg[*top]);
		(*top)++;
	}
}


void
prim_at(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    temp1 = *(oper1 = POP());
    if ((temp1.type != PROG_VAR) && (temp1.type != PROG_LVAR) && (oper1->type != PROG_SVAR))
        abort_interp("Non-variable argument");
    if (temp1.data.number >= MAX_VAR || temp1.data.number < 0)
        abort_interp("Variable number out of range");
/*    if (temp1.type == PROG_LVAR) {
      copyinst(&(CurrVar[temp1.data.number]), &arg[(*top)++]);
    } else {
      copyinst(&(fr->variables[temp1.data.number]), &arg[(*top)++]);
    } */
    if (temp1.type == PROG_LVAR) {
	/* LOCALVAR */
	copyinst(&(CurrVar[temp1.data.number]), &arg[(*top)++]);
    } else if (temp1.type == PROG_VAR) {
	/* GLOBALVAR */
	copyinst(&(fr->variables[temp1.data.number]), &arg[(*top)++]);
    } else {
	/* SCOPEDVAR */
	struct inst *tmp;

	tmp = scopedvar_get(fr, temp1.data.number);
	if (!tmp)
		abort_interp("Scoped variable number out of range.");
	copyinst(tmp, &arg[(*top)++]);
    }
    CLEAR(&temp1);
}

void
prim_bang(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    if ((oper1->type != PROG_VAR) && (oper1->type != PROG_LVAR) && (oper1->type != PROG_SVAR))
        abort_interp("Non-variable argument (2)");
    if (oper1->data.number >= MAX_VAR || oper1->data.number < 0)
        abort_interp("Variable number out of range. (2)");
/*    if (oper1->type == PROG_LVAR) {
      CLEAR(&(CurrVar[oper1->data.number]));
      copyinst(oper2, &(CurrVar[oper1->data.number]));
    } else {
      CLEAR(&(fr->variables[oper1->data.number]));
      copyinst(oper2, &(fr->variables[oper1->data.number]));
    } */
    if (oper1->type == PROG_LVAR) {
	/* LOCALVAR */
	CLEAR(&(CurrVar[oper1->data.number]));
	copyinst(oper2, &(CurrVar[oper1->data.number]));
    } else if (oper1->type == PROG_VAR) {
	/* GLOBALVAR */
	CLEAR(&(fr->variables[oper1->data.number]));
	copyinst(oper2, &(fr->variables[oper1->data.number]));
    } else {
	/* SCOPEDVAR */
	struct inst *tmp;

	tmp = scopedvar_get(fr, oper1->data.number);
	if (!tmp)
		abort_interp("Scoped variable number out of range.");
	CLEAR(tmp);
	copyinst(oper2, tmp);
    }
    CLEAR(oper1);
    CLEAR(oper2);
}  

void
prim_var(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    if (oper1->type != PROG_INTEGER)
	abort_interp("Non-integer argument");
    result = oper1->data.number;
    CLEAR(oper1);
    push(arg, top, PROG_VAR, MIPSCAST & result);
}

void
prim_localvar(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    if (oper1->type != PROG_INTEGER)
	abort_interp("Non-integer argument");
    result = oper1->data.number;
    CLEAR(oper1);
    push(arg, top, PROG_LVAR, MIPSCAST & result);
}

void
prim_variablep(PRIM_PROTOTYPE)
{
   CHECKOP(1);
   oper1 = POP();
   result = (oper1->type == PROG_LVAR || oper1->type == PROG_VAR || oper1->type == PROG_SVAR);
   CLEAR(oper1);
   PushInt(result);
}

void
prim_swap(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();
    temp2 = *(oper2 = POP());
    arg[(*top)++] = *oper1;
    arg[(*top)++] = temp2;
    /* don't clean! */
}

void
prim_over(PRIM_PROTOTYPE)
{
    CHECKOFLOW(1);
    CHECKOP(2);
    copyinst(&arg[*top - 2], &arg[*top]);
    (*top)++;
}

void
prim_pick(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    temp1 = *(oper1 = POP());
    if (temp1.type != PROG_INTEGER || temp1.data.number <= 0)
	abort_interp("Operand not a positive integer");
    CHECKOP(temp1.data.number);
    copyinst(&arg[*top - temp1.data.number], &arg[*top]);
    (*top)++;
}

void
prim_put(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP();
    oper2 = POP();
    if (oper1->type != PROG_INTEGER || oper1->data.number <= 0)
	abort_interp("Operand not a positive integer");
    tmp = oper1->data.number;
    CHECKOP(tmp);
    CLEAR(&arg[*top - tmp]);
    copyinst(oper2, &arg[*top - tmp]);
    CLEAR(oper1);
    CLEAR(oper2);
}

void
prim_rot(PRIM_PROTOTYPE)
{
    CHECKOP(3);
    oper1 = POP();
    oper2 = POP();
    temp3 = *(oper3 = POP());
    arg[(*top)++] = *oper2;
    arg[(*top)++] = *oper1;
    arg[(*top)++] = temp3;
}

void
prim_popn(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    if (oper1->type != PROG_INTEGER)
	abort_interp("Non-integer argument.");
    tmp = oper1->data.number;
    if( tmp < 0 || tmp >= STACK_SIZE )
	abort_interp("Invalid popn quantity.");
    CHECKOP(tmp);
    for(; tmp > 0; tmp--) {
	CLEAR(oper1);
	oper1 = POP();
    }
    CLEAR(oper1);
}

int
sort0(struct inst *o1, struct inst *o2)
{
	if( o1->type != PROG_STRING || o2->type != PROG_STRING )
	    return 0;

	if(!o1->data.string) return -1;
	if(!o2->data.string) return  1;
	if(!o1->data.string->data) return -1;
	if(!o2->data.string->data) return  1;

	return strcmp(o1->data.string->data,o2->data.string->data);
}

int
sort1(struct inst *o1, struct inst *o2)
{
	if( o1->type != PROG_STRING || o2->type != PROG_STRING )
	    return 0;

	if(!o1->data.string) return  1;
	if(!o2->data.string) return -1;
	if(!o1->data.string->data) return  1;
	if(!o2->data.string->data) return -1;

	return -strcmp(o1->data.string->data,o2->data.string->data);
}

int
sort2(struct inst *o1, struct inst *o2)
{
	if( o1->type != PROG_STRING || o2->type != PROG_STRING )
	    return 0;

	if(!o1->data.string) return -1;
	if(!o2->data.string) return  1;
	if(!o1->data.string->data) return -1;
	if(!o2->data.string->data) return  1;

	return strcasecmp(o1->data.string->data,o2->data.string->data);
}

int
sort3(struct inst *o1, struct inst *o2)
{
	if( o1->type != PROG_STRING || o2->type != PROG_STRING )
	    return 0;

	if(!o1->data.string) return  1;
	if(!o2->data.string) return -1;
	if(!o1->data.string->data) return  1;
	if(!o2->data.string->data) return -1;

	return -strcasecmp(o1->data.string->data,o2->data.string->data);
}

int
sort4(struct inst *o1, struct inst *o2)
{
	if( o1->type != PROG_OBJECT || o2->type != PROG_OBJECT )
	    return 0;

	if( (o1->data.objref < 0) || (o2->data.objref < 0) )
	    return 0;

	if( (o1->data.objref >= db_top) || (o2->data.objref >= db_top) )
	    return 0;

	return strcasecmp(NAME(o1->data.objref),NAME(o2->data.objref));
}

int
sort5(struct inst *o1, struct inst *o2)
{
	if( o1->type != PROG_OBJECT || o2->type != PROG_OBJECT )
	    return 0;

	if( (o1->data.objref < 0) || (o2->data.objref < 0) )
	    return 0;

	if( (o1->data.objref >= db_top) || (o2->data.objref >= db_top) )
	    return 0;

	return -strcasecmp(NAME(o1->data.objref),NAME(o2->data.objref));
}

void
prim_sort(PRIM_PROTOTYPE)
{
    CHECKOP(2);
    oper1 = POP(); /* Sort type */
    oper2 = POP(); /* {s} size */
    
    if (oper1->type != PROG_INTEGER)
	abort_interp("Invalid argument type. (1)");
    if (oper2->type != PROG_INTEGER)
	abort_interp("Invalid argument type. (2)");

    tmp = oper2->data.number;	/* {s} size */
    if ((tmp < 0) || (tmp >= STACK_SIZE - 2))
	abort_interp("Invalid array size.");
    CHECKOP(tmp);

    tmp = oper2->data.number;

    switch(oper1->data.number) {
	case 1:
		qsort(&arg[*top - tmp], tmp, sizeof(arg[0]),(void *)sort1);
		break;
	case 2:
		qsort(&arg[*top - tmp], tmp, sizeof(arg[0]),(void *)sort2);
		break;
	case 3:
		qsort(&arg[*top - tmp], tmp, sizeof(arg[0]),(void *)sort3);
		break;
	case 4:
		qsort(&arg[*top - tmp], tmp, sizeof(arg[0]),(void *)sort4);
		break;
	case 5:
		qsort(&arg[*top - tmp], tmp, sizeof(arg[0]),(void *)sort5);
		break;
	case 0:
	default:
		qsort(&arg[*top - tmp], tmp, sizeof(arg[0]),(void *)sort0);
    }

    result=oper2->data.number;
    CLEAR(oper1);
    CLEAR(oper2);
    CHECKOFLOW(1);
    PushInt(result);
}

void
prim_rotate(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    if (oper1->type != PROG_INTEGER)
	abort_interp("Invalid argument type");
    tmp = oper1->data.number;	/* Depth on stack */
    CHECKOP(abs(tmp));
    if (tmp > 0) {
	temp2 = arg[*top - tmp];
	for (; tmp > 0; tmp--)
	    arg[*top - tmp] = arg[*top - tmp + 1];
	arg[*top - 1] = temp2;
    } else if (tmp < 0) {
	temp2 = arg[*top - 1];
	for (tmp = -1; tmp > oper1->data.number; tmp--)
	    arg[*top + tmp] = arg[*top + tmp - 1];
	arg[*top + tmp] = temp2;
    }
    CLEAR(oper1);
}

void
prim_dbtop(PRIM_PROTOTYPE)
{
    CHECKOP(0);
    ref = (dbref) db_top;
    CHECKOFLOW(1);
    PushObject(ref);
}

void
prim_depth(PRIM_PROTOTYPE)
{
    CHECKOP(0);
    result = *top;
    CHECKOFLOW(1);
    PushInt(result);
}

void
prim_version(PRIM_PROTOTYPE)
{
    char temp[256];
    CHECKOP(0);
    CHECKOFLOW(1);
    sprintf(temp, "%s(ProtoMUCK%s)", VERSION, PROTOBASE);
    PushString(temp);
}

void
prim_prog(PRIM_PROTOTYPE)
{
    CHECKOP(0);
    ref = (dbref) program;
    CHECKOFLOW(1);
    PushObject(ref);
}

void
prim_trig(PRIM_PROTOTYPE)
{
    CHECKOP(0);
    ref = (dbref) fr->trig;
    CHECKOFLOW(1);
    PushObject(ref);
}

void
prim_caller(PRIM_PROTOTYPE)
{
    CHECKOP(0);
    ref = (dbref) fr->caller.st[fr->caller.top - 1];
    CHECKOFLOW(1);
    PushObject(ref);
}

void
prim_intp(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    result = (oper1->type == PROG_INTEGER);
    CLEAR(oper1);
    PushInt(result);
}

void
prim_floatp(PRIM_PROTOTYPE)
{
	CHECKOP(1);
	oper1 = POP();
	result = (oper1->type == PROG_FLOAT);
	CLEAR(oper1);
	PushInt(result);
}

void
prim_arrayp(PRIM_PROTOTYPE)
{
	CHECKOP(1);
	oper1 = POP();
	result = (oper1->type == PROG_ARRAY);
	CLEAR(oper1);
	PushInt(result);
}

void
prim_dictionaryp(PRIM_PROTOTYPE)
{
	CHECKOP(1);
	oper1 = POP();
	result = (oper1->type == PROG_ARRAY && oper1->data.array &&
			  oper1->data.array->type == ARRAY_DICTIONARY);
	CLEAR(oper1);
	PushInt(result);
}

void
prim_stringp(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    result = (oper1->type == PROG_STRING);
    CLEAR(oper1);
    PushInt(result);
}

void
prim_dbrefp(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    result = (oper1->type == PROG_OBJECT);
    CLEAR(oper1);
    PushInt(result);
}

void
prim_addressp(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    result = (oper1->type == PROG_ADD);
    CLEAR(oper1);
    PushInt(result);
}

void
prim_lockp(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    result = (oper1->type == PROG_LOCK);
    CLEAR(oper1);
    PushInt(result);
}

void
prim_socketp(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    result = (oper1->type == PROG_SOCKET);
    CLEAR(oper1);
    PushInt(result);
}

void
prim_markp(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    result = (oper1->type == PROG_MARK);
    CLEAR(oper1);
    PushInt(result);
}

#define ABORT_CHECKARGS(msg) { if (*top == stackpos+1) sprintf(zbuf, "%s (top)", msg); else sprintf(zbuf, "%s (top-%d)", msg, ((*top)-stackpos-1));  abort_interp(zbuf); }

#define MaxComplexity 18     /* A truly ridiculously high number! */

void
prim_checkargs(PRIM_PROTOTYPE)
{
    int     currpos, stackpos;
    int     rngstktop = 0;
    enum {
	itsarange, itsarepeat
    }       rngstktyp[MaxComplexity];
    int     rngstkpos[MaxComplexity];
    int     rngstkcnt[MaxComplexity];
    char zbuf[BUFFER_LEN];

    CHECKOP(1);
    oper1 = POP();		/* string argument */
    if (oper1->type != PROG_STRING)
	abort_interp("Non string argument");
    if (!oper1->data.string) {
	/* if null string, then no args expected. */
	CLEAR(oper1);
	return;
    }
    strcpy(buf, oper1->data.string->data);	/* copy into local buffer */
    currpos = strlen(buf) - 1;
    stackpos = *top - 1;

    while (currpos >= 0) {
	if (isdigit(buf[currpos])) {
	    if (rngstktop >= MaxComplexity)
		abort_interp("Argument expression ridiculously complex");
	    tmp = 1;
	    result = 0;
	    while ((currpos >= 0) && isdigit(buf[currpos])) {
		result = result + (tmp * (buf[currpos] - '0'));
	        tmp = tmp * 10;
		currpos--;
	    }
	    if (result == 0)
		abort_interp("Bad multiplier '0' in argument expression");
	    if (result >= STACK_SIZE)
		abort_interp("Multiplier too large in argument expression");
	    rngstktyp[rngstktop] = itsarepeat;
	    rngstkcnt[rngstktop] = result;
	    rngstkpos[rngstktop] = currpos;
	    rngstktop++;
	} else if (buf[currpos] == '}') {
	    if (rngstktop >= MaxComplexity)
		abort_interp("Argument expression ridiculously complex");
	    if (stackpos < 0)
		ABORT_CHECKARGS("Stack underflow");
	    if (arg[stackpos].type != PROG_INTEGER)
		ABORT_CHECKARGS("Expected an integer range counter");
	    result = arg[stackpos].data.number;
	    if (result < 0)
		ABORT_CHECKARGS("Range counter should be non-negative");
	    rngstkpos[rngstktop] = currpos - 1;
	    rngstkcnt[rngstktop] = result;
	    rngstktyp[rngstktop] = itsarange;
	    rngstktop++;
	    currpos--;
	    if (result == 0) {
		while ((currpos > 0) && (buf[currpos] != '{'))
		    currpos--;
	    }
	    stackpos--;
	} else if (buf[currpos] == '{') {
	    if (rngstktop <= 0)
		abort_interp("Mismatched { in argument expression");
	    if (rngstktyp[rngstktop - 1] != itsarange)
		abort_interp("Misformed argument expression");
	    if (--rngstkcnt[rngstktop - 1] > 0) {
		currpos = rngstkpos[rngstktop - 1];
	    } else {
		rngstktop--;
		currpos--;
		if (rngstktop && (rngstktyp[rngstktop - 1] == itsarepeat)) {
		    if (--rngstkcnt[rngstktop - 1] > 0) {
			currpos = rngstkpos[rngstktop - 1];
		    } else {
			rngstktop--;
		    }
		}
	    }
	} else {
	    switch (buf[currpos]) {
		case 'i':
		    if (stackpos < 0)
			ABORT_CHECKARGS("Stack underflow");
		    if (arg[stackpos].type != PROG_INTEGER)
			ABORT_CHECKARGS("Expected an integer");
		    break;
  		case 'n':
			if (stackpos < 0)
				ABORT_CHECKARGS("Stack underflow.");
			if (arg[stackpos].type != PROG_FLOAT)
				ABORT_CHECKARGS("Expected a float.");
			break;
		case 's':
		case 'S':
		    if (stackpos < 0)
			ABORT_CHECKARGS("Stack underflow");
		    if (arg[stackpos].type != PROG_STRING)
			ABORT_CHECKARGS("Expected a string");
		    if (buf[currpos] == 'S' && !arg[stackpos].data.string)
			ABORT_CHECKARGS("Expected a non-null string");
		    break;
		case 'd':
		case 'p':
		case 'r':
		case 't':
		case 'e':
		case 'f':
		case 'D':
		case 'P':
		case 'R':
		case 'T':
		case 'E':
		case 'F':
		    if (stackpos < 0)
			ABORT_CHECKARGS("Stack underflow");
		    if (arg[stackpos].type != PROG_OBJECT)
			ABORT_CHECKARGS("Expected a dbref");
		    ref = arg[stackpos].data.objref;
		    if ((ref >= db_top) || (ref < HOME))
			ABORT_CHECKARGS("Invalid dbref");
		    switch (buf[currpos]) {
			case 'D':
			    if ((ref < 0) && (ref != HOME))
				ABORT_CHECKARGS("Invalid dbref");
			    if (Typeof(ref) == TYPE_GARBAGE)
				ABORT_CHECKARGS("Invalid dbref");
			case 'd':
			    if (ref < HOME)
				ABORT_CHECKARGS("Invalid dbref");
			    break;

			case 'P':
			    if (ref < 0)
				ABORT_CHECKARGS("Expected player dbref");
			case 'p':
			    if ((ref >= 0) && (Typeof(ref) != TYPE_PLAYER))
				ABORT_CHECKARGS("Expected player dbref");
			    if (ref == HOME)
				ABORT_CHECKARGS("Expected player dbref");
			    break;

			case 'R':
			    if ((ref < 0) && (ref != HOME))
				ABORT_CHECKARGS("Expected room dbref");
			case 'r':
			    if ((ref >= 0) && (Typeof(ref) != TYPE_ROOM))
				ABORT_CHECKARGS("Expected room dbref");
			    break;

			case 'T':
			    if (ref < 0)
				ABORT_CHECKARGS("Expected thing dbref");
			case 't':
			    if ((ref >= 0) && (Typeof(ref) != TYPE_THING))
				ABORT_CHECKARGS("Expected thing dbref");
			    if (ref == HOME)
				ABORT_CHECKARGS("Expected player dbref");
			    break;

			case 'E':
			    if (ref < 0)
				ABORT_CHECKARGS("Expected exit dbref");
			case 'e':
			    if ((ref >= 0) && (Typeof(ref) != TYPE_EXIT))
				ABORT_CHECKARGS("Expected exit dbref");
			    if (ref == HOME)
				ABORT_CHECKARGS("Expected player dbref");
			    break;

			case 'F':
			    if (ref < 0)
				ABORT_CHECKARGS("Expected program dbref");
			case 'f':
			    if ((ref >= 0) && (Typeof(ref) != TYPE_PROGRAM))
				ABORT_CHECKARGS("Expected program dbref");
			    if (ref == HOME)
				ABORT_CHECKARGS("Expected player dbref");
			    break;
		    }
		    break;
		case '?':
		    if (stackpos < 0)
			ABORT_CHECKARGS("Stack underflow");
		    break;
		case 'l':
		    if (stackpos < 0)
			ABORT_CHECKARGS("Stack underflow");
		    if (arg[stackpos].type != PROG_LOCK)
			ABORT_CHECKARGS("Expected a lock boolean expression");
		    break;
		case 'v':
		    if (stackpos < 0)
			ABORT_CHECKARGS("Stack underflow");
		    if ((arg[stackpos].type != PROG_VAR) &&
			    (arg[stackpos].type != PROG_LVAR))
			ABORT_CHECKARGS("Expected a variable");
		    break;
		case 'a':
		    if (stackpos < 0)
			ABORT_CHECKARGS("Stack underflow");
		    if (arg[stackpos].type != PROG_ADD)
			ABORT_CHECKARGS("Expected a function address");
		    break;
		case ' ':
		    /* this is meaningless space.  Ignore it. */
		    stackpos++;
		    break;
		default:
		    abort_interp("Unkown argument type in expression");
		    break;
	    }

	    currpos--;		/* decrement string index */
	    stackpos--;		/* move on to next stack item down */

	    /* are we expecting a repeat of the last argument or range? */
	    if ((rngstktop > 0) && (rngstktyp[rngstktop - 1] == itsarepeat)) {
		/* is the repeat is done yet? */
		if (--rngstkcnt[rngstktop - 1] > 0) {
		    /* no, repeat last argument or range */
		    currpos = rngstkpos[rngstktop - 1];
		} else {
		    /* yes, we're done with this repeat */
		    rngstktop--;
		}
	    }
	}
    }				/* while loop */

    if (rngstktop > 0)
	abort_interp("Badly formed argument expression");
    /* Oops. still haven't finished a range or repeat expression. */

    CLEAR(oper1);		/* clear link to shared string */
}


void
prim_mode(PRIM_PROTOTYPE)
{
    CHECKOP(0);
    result = fr->multitask;
    CHECKOFLOW(1);
    PushInt(result);
}

void
prim_setmode(PRIM_PROTOTYPE)
{
    CHECKOP(1);
    oper1 = POP();
    if (oper1->type != PROG_INTEGER)
	abort_interp("Invalid argument type");
    result = oper1->data.number;
    switch(result) {
	case BACKGROUND:
	    fr->been_background = 1;
	    fr->writeonly = 1;
	    break;
	case FOREGROUND:
	    if (fr->been_background)
		abort_interp("Cannot FOREGROUND a BACKGROUNDed program");
	    break;
	case PREEMPT:
	    break;
	default:
	    abort_interp("Invalid mode");
	    break;
    }
    fr->multitask = result;
    CLEAR(oper1);
}


void
prim_interp(PRIM_PROTOTYPE)
{
    struct inst *oper1, *oper2, *oper3, *rv;
    char buf[BUFFER_LEN];
    struct frame *tmpfr;

    CHECKOP(3);
    oper3 = POP();    /* string -- top stack argument */
    oper2 = POP();    /* dbref  --  trigger */
    oper1 = POP();    /* dbref  --  Program to run */

    if (!valid_object(oper1) || Typeof(oper1->data.objref) != TYPE_PROGRAM)
        abort_interp("Bad program reference. (1)");
    if (!valid_object(oper2))
        abort_interp("Bad object. (2)");
    if (!permissions(mlev, ProgUID, oper2->data.objref))
	abort_interp(tp_noperm_mesg);
    CHECKREMOTE(oper2->data.objref);

    strcpy(buf, match_args);
    strcpy(match_args, oper3->data.string? oper3->data.string->data : "");
/*    rv = interp(fr->descr, player, DBFETCH(player)->location, oper1->data.objref,
			oper2->data.objref, PREEMPT, STD_HARDUID, 1); */
    tmpfr = interp(fr->descr, player, DBFETCH(player)->location, oper1->data.objref,
				   oper2->data.objref, PREEMPT, STD_HARDUID);
    if (tmpfr) {
	rv = interp_loop(player, oper1->data.objref, tmpfr, 1);
    }
    strcpy(match_args, buf);

    CLEAR(oper3);
    CLEAR(oper2);
    CLEAR(oper1);

    if (rv) {
	if (rv->type < PROG_STRING) {
	    push(arg, top, rv->type, MIPSCAST (&rv->data.number));
	} else {
	    push(arg, top, rv->type, MIPSCAST (rv->data.string));
	}
    } else {
        PushNullStr;
    }

}

void
prim_mark(PRIM_PROTOTYPE)
{
	CHECKOP(0);
	CHECKOFLOW(1);
	PushMark();
}

void
prim_findmark(PRIM_PROTOTYPE)
{
	int depth, height, count;

	CHECKOP(0);
	depth = 1;
	height = *top - 1;
	while (height >= 0 && arg[height].type != PROG_MARK) {
		height--;
		depth++;
	}
	count = depth - 1;
	if (height < 0)
		abort_interp("No matching mark on stack!");
	if (depth > 1) {
		temp2 = arg[*top - depth];
		for (; depth > 1; depth--)
			arg[*top - depth] = arg[*top - depth + 1];
		arg[*top - 1] = temp2;
	}
	oper1 = POP();
	CLEAR(oper1);
	PushInt(count);
}

void
prim_reverse(PRIM_PROTOTYPE)
{
	int i;

	CHECKOP(1);
	oper1 = POP();
	if (oper1->type != PROG_INTEGER)
		abort_interp("Invalid argument type.");
	tmp = oper1->data.number;	/* Depth on stack */
	if (tmp < 0)
		abort_interp("Argument must be positive.");
	CHECKOP(tmp);
	if (tmp > 0) {
		for (i = 0; i < (tmp / 2); i++) {
			temp2 = arg[*top - (tmp - i)];
			arg[*top - (tmp - i)] = arg[*top - (i + 1)];
			arg[*top - (i + 1)] = temp2;
		}
	}
	CLEAR(oper1);
}


void
prim_lreverse(PRIM_PROTOTYPE)
{
	int i;

	CHECKOP(1);
	oper1 = POP();
	if (oper1->type != PROG_INTEGER)
		abort_interp("Invalid argument type.");
	tmp = oper1->data.number;	/* Depth on stack */
	if (tmp < 0)
		abort_interp("Argument must be positive.");
	CHECKOP(tmp);
	if (tmp > 0) {
		for (i = 0; i < (tmp / 2); i++) {
			temp2 = arg[*top - (tmp - i)];
			arg[*top - (tmp - i)] = arg[*top - (i + 1)];
			arg[*top - (i + 1)] = temp2;
		}
	}
	CLEAR(oper1);
	PushInt(tmp);
}



/* Internal stack primitives */

struct forvars *push_for(struct forvars *);
struct forvars *pop_for(struct forvars *);

void
prim_for(PRIM_PROTOTYPE)
{
	int fortop;

	CHECKOP(3);
	oper3 = POP();				/* step */
	oper2 = POP();				/* end */
	oper1 = POP();				/* start */

	if (oper1->type != PROG_INTEGER)
		abort_interp("Starting count expected. (1)");
	if (oper2->type != PROG_INTEGER)
		abort_interp("Ending count expected. (2)");
	if (oper3->type != PROG_INTEGER)
		abort_interp("Step count expected. (3)");
	if (fr->fors.top >= STACK_SIZE)
		abort_interp("Too many nested FOR loops.");

	fortop = fr->fors.top++;
	fr->fors.st = push_for(fr->fors.st);
	copyinst(oper1, &fr->fors.st->cur);
	copyinst(oper2, &fr->fors.st->end);
	fr->fors.st->step = oper3->data.number;
	fr->fors.st->didfirst = 0;

	CLEAR(oper1);
	CLEAR(oper2);
	CLEAR(oper3);
}


void
prim_foreach(PRIM_PROTOTYPE)
{
	int fortop;

	CHECKOP(1);
	oper1 = POP();

	if (oper1->type != PROG_ARRAY)
		abort_interp("Array argument expected. (1)");
	if (fr->fors.top >= STACK_SIZE)
		abort_interp("Too many nested FOR loops.");

	fortop = fr->fors.top++;
	fr->fors.st = push_for(fr->fors.st);

	fr->fors.st->cur.type = PROG_INTEGER;
	fr->fors.st->cur.line = 0;
	fr->fors.st->cur.data.number = 0;

	copyinst(oper1, &fr->fors.st->end);
	fr->fors.st->step = 0;
	fr->fors.st->didfirst = 0;

	CLEAR(oper1);
}


void
prim_foriter(PRIM_PROTOTYPE)
{
	CHECKOP(0);

	if (!fr->fors.st)
		abort_interp("Internal error; FOR stack underflow.");

	if (fr->fors.st->end.type == PROG_ARRAY) {
		stk_array *arr = fr->fors.st->end.data.array;

		if (fr->fors.st->didfirst) {
			result = array_next(arr, &fr->fors.st->cur);
		} else {
			result = array_first(arr, &fr->fors.st->cur);
			fr->fors.st->didfirst = 1;
		}
		if (result) {
			array_data *val = array_getitem(arr, &fr->fors.st->cur);

			if (val) {
				PushInst(&fr->fors.st->cur);	/* push key onto stack */
				PushInst(val);	/* push value onto stack */
				tmp = 1;		/* tell following IF to NOT branch out of loop */
			} else {
				tmp = 0;		/* tell following IF to branch out of loop */
			}
		} else {
			tmp = 0;			/* tell following IF to branch out of loop */
		}
	} else {
		int cur = fr->fors.st->cur.data.number;
		int end = fr->fors.st->end.data.number;

		tmp = fr->fors.st->step > 0;
		if (tmp)
			tmp = !(cur > end);
		else
			tmp = !(cur < end);
		if (tmp) {
			CHECKOFLOW(2);
			result = cur;
			fr->fors.st->cur.data.number += fr->fors.st->step;
			PushInt(result);
		} else {
			CHECKOFLOW(1);
		}
	}
	PushInt(tmp);
}


void
prim_forpop(PRIM_PROTOTYPE)
{
	CHECKOP(0);

	if (!(fr->fors.top))
		abort_interp("Internal error; FOR stack underflow.");

	CLEAR(&fr->fors.st->cur);
	CLEAR(&fr->fors.st->end);

	fr->fors.top--;
	fr->fors.st = pop_for(fr->fors.st);
}


