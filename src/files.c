/*	SCCS Id: @(#)files.c	3.4	2003/11/14	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "dlb.h"

#ifdef TTY_GRAPHICS
#include "wintty.h" /* more() */
#endif

#include <ctype.h>

#if !defined(MAC) && !defined(O_WRONLY) && !defined(AZTEC_C)
#include <fcntl.h>
#endif

#include <errno.h>
#ifdef _MSC_VER	/* MSC 6.0 defines errno quite differently */
# if (_MSC_VER >= 600)
#  define SKIP_ERRNO
# endif
#else
# ifdef NHSTDC
#  define SKIP_ERRNO
# endif
#endif
#ifndef SKIP_ERRNO
# ifdef _DCC
const
# endif
extern int errno;
#endif

#if defined(UNIX) && defined(QT_GRAPHICS)
#include <dirent.h>
#endif

#if defined(MSDOS) || defined(OS2) || defined(TOS) || defined(WIN32)
# ifndef GNUDOS
#include <sys\stat.h>
# else
#include <sys/stat.h>
# endif
#endif
#ifndef O_BINARY	/* used for micros, no-op for others */
# define O_BINARY 0
#endif

#ifdef PREFIXES_IN_USE
#define FQN_NUMBUF 4
static char fqn_filename_buffer[FQN_NUMBUF][FQN_MAX_FILENAME];
#endif

#if !defined(MFLOPPY) && !defined(VMS) && !defined(WIN32)
char bones[] = "bonesnn.xxx";
char lock[PL_NSIZ+14] = "1lock"; /* long enough for uid+name+.99 */
#else
# if defined(MFLOPPY)
char bones[FILENAME];		/* pathname of bones files */
char lock[FILENAME];		/* pathname of level files */
# endif
# if defined(VMS)
char bones[] = "bonesnn.xxx;1";
char lock[PL_NSIZ+17] = "1lock"; /* long enough for _uid+name+.99;1 */
# endif
# if defined(WIN32)
char bones[] = "bonesnn.xxx";
char lock[PL_NSIZ+25];		/* long enough for username+-+name+.99 */
# endif
#endif

#if defined(UNIX) || defined(__BEOS__)
#define SAVESIZE	(PL_NSIZ + 13)	/* save/99999player.e */
#else
# ifdef VMS
#define SAVESIZE	(PL_NSIZ + 22)	/* [.save]<uid>player.e;1 */
# else
#  if defined(WIN32)
#define SAVESIZE	(PL_NSIZ + 40)	/* username-player.NetHack-saved-game */
#  else
#define SAVESIZE	FILENAME	/* from macconf.h or pcconf.h */
#  endif
# endif
#endif

char SAVEF[SAVESIZE];	/* holds relative path of save file from playground */
#ifdef MICRO
char SAVEP[SAVESIZE];	/* holds path of directory for save file */
#endif

#ifdef WIZARD
#define WIZKIT_MAX 128
static char wizkit[WIZKIT_MAX];
STATIC_DCL FILE *NDECL(fopen_wizkit_file);
#endif

#ifdef AMIGA
extern char PATH[];	/* see sys/amiga/amidos.c */
extern char bbs_id[];
static int lockptr;
# ifdef __SASC_60
#include <proto/dos.h>
# endif

#include <libraries/dos.h>
extern void FDECL(amii_set_text_font, ( char *, int ));
#endif

#if defined(WIN32) || defined(MSDOS)
static int lockptr;
# ifdef MSDOS
#define Delay(a) msleep(a)
# endif
#define Close close
#ifndef WIN_CE
#define DeleteFile unlink
#endif
#endif

#define unlink remove

#ifdef USER_SOUNDS
extern char *sounddir;
#endif

extern int n_dgns;		/* from dungeon.c */

STATIC_DCL char *FDECL(set_bonesfile_name, (char *,d_level*));
STATIC_DCL char *NDECL(set_bonestemp_name);
STATIC_DCL char *FDECL(make_lockname, (const char *,char *));
STATIC_DCL FILE *FDECL(fopen_config_file, (const char *));
STATIC_DCL int FDECL(get_uchars, (FILE *,char *,char *,uchar *,BOOLEAN_P,int,const char *));
int FDECL(parse_config_line, (FILE *,char *,char *,char *));
#ifdef NOCWD_ASSUMPTIONS
STATIC_DCL void FDECL(adjust_prefix, (char *, int));
#endif
#ifdef SELF_RECOVER
STATIC_DCL boolean FDECL(copy_bytes, (int, int));
#endif
#ifdef HOLD_LOCKFILE_OPEN
STATIC_DCL int FDECL(open_levelfile_exclusively, (const char *, int, int));
#endif

/*
 * fname_encode()
 *
 *   Args:
 *	legal		zero-terminated list of acceptable file name characters
 *	quotechar	lead-in character used to quote illegal characters as hex digits
 *	s		string to encode
 *	callerbuf	buffer to house result
 *	bufsz		size of callerbuf
 *
 *   Notes:
 *	The hex digits 0-9 and A-F are always part of the legal set due to
 *	their use in the encoding scheme, even if not explicitly included in 'legal'.
 *
 *   Sample:
 *	The following call:
 *	    (void)fname_encode("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
 *				'%', "This is a % test!", buf, 512);
 *	results in this encoding:
 *	    "This%20is%20a%20%25%20test%21"
 */
char *
fname_encode(legal, quotechar, s, callerbuf, bufsz)
const char *legal;
char quotechar;
char *s, *callerbuf;
int bufsz;
{
	char *sp, *op;
	int cnt = 0;
	static char hexdigits[] = "0123456789ABCDEF";

	sp = s;
	op = callerbuf;
	*op = '\0';
	
	while (*sp) {
		/* Do we have room for one more character or encoding? */
		if ((bufsz - cnt) <= 4) return callerbuf;

		if (*sp == quotechar) {
			(void)sprintf(op, "%c%02X", quotechar, *sp);
			 op += 3;
			 cnt += 3;
		} else if ((index(legal, *sp) != 0) || (index(hexdigits, *sp) != 0)) {
			*op++ = *sp;
			*op = '\0';
			cnt++;
		} else {
			(void)sprintf(op,"%c%02X", quotechar, *sp);
			op += 3;
			cnt += 3;
		}
		sp++;
	}
	return callerbuf;
}

/*
 * fname_decode()
 *
 *   Args:
 *	quotechar	lead-in character used to quote illegal characters as hex digits
 *	s		string to decode
 *	callerbuf	buffer to house result
 *	bufsz		size of callerbuf
 */
char *
fname_decode(quotechar, s, callerbuf, bufsz)
char quotechar;
char *s, *callerbuf;
int bufsz;
{
	char *sp, *op;
	int k,calc,cnt = 0;
	static char hexdigits[] = "0123456789ABCDEF";

	sp = s;
	op = callerbuf;
	*op = '\0';
	calc = 0;

	while (*sp) {
		/* Do we have room for one more character? */
		if ((bufsz - cnt) <= 2) return callerbuf;
		if (*sp == quotechar) {
			sp++;
			for (k=0; k < 16; ++k) if (*sp == hexdigits[k]) break;
			if (k >= 16) return callerbuf;	/* impossible, so bail */
			calc = k << 4; 
			sp++;
			for (k=0; k < 16; ++k) if (*sp == hexdigits[k]) break;
			if (k >= 16) return callerbuf;	/* impossible, so bail */
			calc += k; 
			sp++;
			*op++ = calc;
			*op = '\0';
		} else {
			*op++ = *sp++;
			*op = '\0';
		}
		cnt++;
	}
	return callerbuf;
}

#ifndef PREFIXES_IN_USE
/*ARGSUSED*/
#endif
const char *
fqname(basename, whichprefix, buffnum)
const char *basename;
int whichprefix, buffnum;
{
#ifndef PREFIXES_IN_USE
	return basename;
#else
	if (!basename || whichprefix < 0 || whichprefix >= PREFIX_COUNT)
		return basename;
	if (!fqn_prefix[whichprefix])
		return basename;
	if (buffnum < 0 || buffnum >= FQN_NUMBUF) {
		impossible("Invalid fqn_filename_buffer specified: %d",
								buffnum);
		buffnum = 0;
	}
	if (strlen(fqn_prefix[whichprefix]) + strlen(basename) >=
						    FQN_MAX_FILENAME) {
		impossible("fqname too long: %s + %s", fqn_prefix[whichprefix],
						basename);
		return basename;	/* XXX */
	}
	Strcpy(fqn_filename_buffer[buffnum], fqn_prefix[whichprefix]);
	return strcat(fqn_filename_buffer[buffnum], basename);
#endif
}

/* reasonbuf must be at least BUFSZ, supplied by caller */
/*ARGSUSED*/
int
validate_prefix_locations(reasonbuf)
char *reasonbuf;
{
#if defined(NOCWD_ASSUMPTIONS)
	FILE *fp;
	const char *filename;
	int prefcnt, failcount = 0;
	char panicbuf1[BUFSZ], panicbuf2[BUFSZ], *details;

	if (reasonbuf) reasonbuf[0] = '\0';
	for (prefcnt = 1; prefcnt < PREFIX_COUNT; prefcnt++) {
		/* don't test writing to configdir or datadir; they're readonly */
		if (prefcnt == CONFIGPREFIX || prefcnt == DATAPREFIX) continue;
		filename = fqname("validate", prefcnt, 3);
		if ((fp = fopen(filename, "w"))) {
			fclose(fp);
			(void) unlink(filename);
		} else {
			if (reasonbuf) {
				if (failcount) Strcat(reasonbuf,", ");
				Strcat(reasonbuf, fqn_prefix_names[prefcnt]);
			}
			/* the paniclog entry gets the value of errno as well */
			Sprintf(panicbuf1,"Invalid %s", fqn_prefix_names[prefcnt]);
#if defined (NHSTDC) && !defined(NOTSTDC)
			if (!(details = strerror(errno)))
#endif
			details = "";
			Sprintf(panicbuf2,"\"%s\", (%d) %s",
				fqn_prefix[prefcnt], errno, details);
			paniclog(panicbuf1, panicbuf2);
			failcount++;
		}	
	}
	if (failcount)
		return 0;
	else
#endif
	return 1;
}

/* fopen a file, with OS-dependent bells and whistles */
/* NOTE: a simpler version of this routine also exists in util/dlb_main.c */
FILE *
fopen_datafile(filename, mode, prefix)
const char *filename, *mode;
int prefix;
{
	FILE *fp;

	filename = fqname(filename, prefix, prefix == TROUBLEPREFIX ? 3 : 0);
#ifdef VMS	/* essential to have punctuation, to avoid logical names */
    {
	char tmp[BUFSIZ];

	if (!index(filename, '.') && !index(filename, ';'))
		filename = strcat(strcpy(tmp, filename), ";0");
	fp = fopen(filename, mode, "mbc=16");
    }
#else
	fp = fopen(filename, mode);
#endif
	return fp;
}

/* ----------  BEGIN LEVEL FILE HANDLING ----------- */

#ifdef MFLOPPY
/* Set names for bones[] and lock[] */
void
set_lock_and_bones()
{
	if (!ramdisk) {
		Strcpy(levels, permbones);
		Strcpy(bones, permbones);
	}
	append_slash(permbones);
	append_slash(levels);
#ifdef AMIGA
	strncat(levels, bbs_id, PATHLEN);
#endif
	append_slash(bones);
	Strcat(bones, "bonesnn.*");
	Strcpy(lock, levels);
#ifndef AMIGA
	Strcat(lock, alllevels);
#endif
	return;
}
#endif /* MFLOPPY */


/* Construct a file name for a level-type file, which is of the form
 * something.level (with any old level stripped off).
 * This assumes there is space on the end of 'file' to append
 * a two digit number.  This is true for 'level'
 * but be careful if you use it for other things -dgk
 */
void
set_levelfile_name(file, lev)
char *file;
int lev;
{
	char *tf;

	tf = rindex(file, '.');
	if (!tf) tf = eos(file);
	Sprintf(tf, ".%d", lev);
#ifdef VMS
	Strcat(tf, ";1");
#endif
	return;
}

FILE*
create_levelfile(lev, errbuf)
int lev;
char errbuf[];
{
	FILE* fd;
	const char *fq_lock;

	if (errbuf) *errbuf = '\0';
	set_levelfile_name(lock, lev);
	fq_lock = fqname(lock, LEVELPREFIX, 0);

        fd = fopen(fq_lock, "wb");

	if (fd != 0)
	    level_info[lev].flags |= LFILE_EXISTS;
	else if (errbuf)	/* failure explanation */
	    Sprintf(errbuf,
		    "Cannot create file \"%s\" for level %d (errno %d).",
		    lock, lev, errno);

	return fd;
}


FILE*
open_levelfile(lev, errbuf)
int lev;
char errbuf[];
{
	FILE* fd;
	const char *fq_lock;

	if (errbuf) *errbuf = '\0';
	set_levelfile_name(lock, lev);
	fq_lock = fqname(lock, LEVELPREFIX, 0);
	fd = fopen(fq_lock, "rb");

	/* for failure, return an explanation that our caller can use;
	   settle for `lock' instead of `fq_lock' because the latter
	   might end up being too big for nethack's BUFSZ */
	if (fd == 0 && errbuf)
	    Sprintf(errbuf,
		    "Cannot open file \"%s\" for level %d (errno %d).",
		    lock, lev, errno);

	return fd;
}


void
delete_levelfile(lev)
int lev;
{
	/*
	 * Level 0 might be created by port specific code that doesn't
	 * call create_levfile(), so always assume that it exists.
	 */
	if (lev == 0 || (level_info[lev].flags & LFILE_EXISTS)) {
		set_levelfile_name(lock, lev);
#ifdef HOLD_LOCKFILE_OPEN
		if (lev == 0) really_close();
#endif
		(void) unlink(fqname(lock, LEVELPREFIX, 0));
		level_info[lev].flags &= ~LFILE_EXISTS;
	}
}


void
clearlocks()
{
#if !defined(PC_LOCKING) && defined(MFLOPPY) && !defined(AMIGA)
	eraseall(levels, alllevels);
	if (ramdisk)
		eraseall(permbones, alllevels);
#else
	register int x;

	/* can't access maxledgerno() before dungeons are created -dlc */
	for (x = (n_dgns ? maxledgerno() : 0); x >= 0; x--)
		delete_levelfile(x);	/* not all levels need be present */
#endif
}

/* ----------  END LEVEL FILE HANDLING ----------- */


/* ----------  BEGIN BONES FILE HANDLING ----------- */

/* set up "file" to be file name for retrieving bones, and return a
 * bonesid to be read/written in the bones file.
 */
STATIC_OVL char *
set_bonesfile_name(file, lev)
char *file;
d_level *lev;
{
	s_level *sptr;
	char *dptr;

	Sprintf(file, "bon%c%s", dungeons[lev->dnum].boneid,
			In_quest(lev) ? urole.filecode : "0");
	dptr = eos(file);
	if ((sptr = Is_special(lev)) != 0)
	    Sprintf(dptr, ".%c", sptr->boneid);
	else
	    Sprintf(dptr, ".%d", lev->dlevel);
#ifdef VMS
	Strcat(dptr, ";1");
#endif
	return(dptr-2);
}

/* set up temporary file name for writing bones, to avoid another game's
 * trying to read from an uncompleted bones file.  we want an uncontentious
 * name, so use one in the namespace reserved for this game's level files.
 * (we are not reading or writing level files while writing bones files, so
 * the same array may be used instead of copying.)
 */
STATIC_OVL char *
set_bonestemp_name()
{
	char *tf;

	tf = rindex(lock, '.');
	if (!tf) tf = eos(lock);
	Sprintf(tf, ".bn");
#ifdef VMS
	Strcat(tf, ";1");
#endif
	return lock;
}

FILE*
create_bonesfile(lev, bonesid, errbuf)
d_level *lev;
char **bonesid;
char errbuf[];
{
	const char *file;
	FILE* fd;

	if (errbuf) *errbuf = '\0';
	*bonesid = set_bonesfile_name(bones, lev);
	file = set_bonestemp_name();
	file = fqname(file, BONESPREFIX, 0);

	fd = fopen(file, "wb");
	if (fd == 0 && errbuf) /* failure explanation */
	    Sprintf(errbuf,
		    "Cannot create bones \"%s\", id %s (errno %d).",
		    lock, *bonesid, errno);


	return fd;
}

#ifdef MFLOPPY
/* remove partial bonesfile in process of creation */
void
cancel_bonesfile()
{
	const char *tempname;

	tempname = set_bonestemp_name();
	tempname = fqname(tempname, BONESPREFIX, 0);
	(void) unlink(tempname);
}
#endif /* MFLOPPY */

/* move completed bones file to proper name */
void
commit_bonesfile(lev)
d_level *lev;
{
	const char *fq_bones, *tempname;
	int ret;

	(void) set_bonesfile_name(bones, lev);
	fq_bones = fqname(bones, BONESPREFIX, 0);
	tempname = set_bonestemp_name();
	tempname = fqname(tempname, BONESPREFIX, 1);

	ret = rename(tempname, fq_bones);
#ifdef WIZARD
	if (wizard && ret != 0)
		pline("couldn't rename %s to %s.", tempname, fq_bones);
#endif
}


FILE*
open_bonesfile(lev, bonesid)
d_level *lev;
char **bonesid;
{
	const char *fq_bones;
	FILE* fd;

	*bonesid = set_bonesfile_name(bones, lev);
	fq_bones = fqname(bones, BONESPREFIX, 0);
	uncompress(fq_bones);	/* no effect if nonexistent */
	fd = fopen(fq_bones, "rb");
	return fd;
}


int
delete_bonesfile(lev)
d_level *lev;
{
	(void) set_bonesfile_name(bones, lev);
	return !(unlink(fqname(bones, BONESPREFIX, 0)) < 0);
}


/* assume we're compressing the recently read or created bonesfile, so the
 * file name is already set properly */
void
compress_bonesfile()
{
	compress(fqname(bones, BONESPREFIX, 0));
}

/* ----------  END BONES FILE HANDLING ----------- */


/* ----------  BEGIN SAVE FILE HANDLING ----------- */

/* set savefile name in OS-dependent manner from pre-existing plname,
 * avoiding troublesome characters */
void
set_savefile_name()
{
    strcpy(SAVEF, "SAVE");
    regularize(SAVEF+5);	/* avoid . or / in name */
}

#ifdef INSURANCE
void
save_savefile_name(fd)
FILE* fd;
{
	(void) fwrite((genericptr_t) SAVEF, 1, sizeof(SAVEF), fd);
}
#endif


#if defined(WIZARD) && !defined(MICRO)
/* change pre-existing savefile name to indicate an error savefile */
void
set_error_savefile()
{
	Strcat(SAVEF, ".e");
}
#endif


/* create save file, overwriting one if it already exists */
FILE*
create_savefile()
{
	const char *fq_save;
	FILE* fd;

	fq_save = fqname(SAVEF, SAVEPREFIX, 0);
	fd = fopen(fq_save, "wb");

	return fd;
}


/* open savefile for reading */
FILE*
open_savefile()
{
	const char *fq_save;
	FILE* fd;

	fq_save = fqname(SAVEF, SAVEPREFIX, 0);
	fd = fopen(fq_save, "rb");
	return fd;
}


/* delete savefile */
int
delete_savefile()
{
	(void) unlink(fqname(SAVEF, SAVEPREFIX, 0));
	return 0;	/* for restore_saved_game() (ex-xxxmain.c) test */
}


/* try to open up a save file and prepare to restore it */
FILE*
restore_saved_game()
{
	const char *fq_save;
	FILE* fd;

	set_savefile_name();
#ifdef MFLOPPY
	if (!saveDiskPrompt(1))
	    return -1;
#endif /* MFLOPPY */
	fq_save = fqname(SAVEF, SAVEPREFIX, 0);

	uncompress(fq_save);
	if ((fd = open_savefile()) == 0) return fd;

	if (!uptodate(fd, fq_save)) {
	    (void) fclose(fd),  fd = 0;
	    (void) delete_savefile();
	}
	return fd;
}

#if defined(UNIX) && defined(QT_GRAPHICS)
/*ARGSUSED*/
static char*
plname_from_file(filename)
const char* filename;
{
#ifdef STORE_PLNAME_IN_FILE
    int fd;
    char* result = 0;

    Strcpy(SAVEF,filename);
    uncompress(SAVEF);
    if ((fd = open_savefile()) >= 0) {
	if (uptodate(fd, filename)) {
	    char tplname[PL_NSIZ];
	    mread(fd, (genericptr_t) tplname, PL_NSIZ);
	    result = strdup(tplname);
	}
	(void) close(fd);
    }
    compress(SAVEF);

    return result;
#else
# if defined(UNIX) && defined(QT_GRAPHICS)
    /* Name not stored in save file, so we have to extract it from
       the filename, which loses information
       (eg. "/", "_", and "." characters are lost. */
    int k;
    int uid;
    char name[64]; /* more than PL_NSIZ */
    if ( sscanf( filename, "%*[^/]/%d%63[^.]" EXTSTR, &uid, name ) == 2 ) {
#undef EXTSTR
    /* "_" most likely means " ", which certainly looks nicer */
	for (k=0; name[k]; k++)
	    if ( name[k]=='_' )
		name[k]=' ';
	return strdup(name);
    } else
# endif
    {
	return 0;
    }
#endif
}
#endif /* defined(UNIX) && defined(QT_GRAPHICS) */

char**
get_saved_games()
{
	return 0;
}

void
free_saved_games(saved)
char** saved;
{
    if ( saved ) {
	int i=0;
	while (saved[i]) free((genericptr_t)saved[i++]);
	free((genericptr_t)saved);
    }
}


/* ----------  END SAVE FILE HANDLING ----------- */


/* ----------  BEGIN FILE COMPRESSION HANDLING ----------- */

/* compress file */
void
compress(filename)
const char *filename;
{
}


/* uncompress file if it exists */
void
uncompress(filename)
const char *filename;
{
}

/* ----------  END FILE COMPRESSION HANDLING ----------- */


/* ----------  BEGIN CONFIG FILE HANDLING ----------- */

const char *configfile =
#ifdef UNIX
			".nethackrc";
#else
# if defined(MAC) || defined(__BEOS__)
			"NetHack Defaults";
# else
#  if defined(MSDOS) || defined(WIN32)
			"defaults.nh";
#  else
			"NetHack.cnf";
#  endif
# endif
#endif


#ifdef MSDOS
/* conflict with speed-dial under windows
 * for XXX.cnf file so support of NetHack.cnf
 * is for backward compatibility only.
 * Preferred name (and first tried) is now defaults.nh but
 * the game will try the old name if there
 * is no defaults.nh.
 */
const char *backward_compat_configfile = "nethack.cnf"; 
#endif

#ifndef MFLOPPY
#define fopenp fopen
#endif

STATIC_OVL FILE *
fopen_config_file(filename)
const char *filename;
{
	FILE *fp;
#if defined(UNIX) || defined(VMS)
	char	tmp_config[BUFSZ];
	char *envp;
#endif

	/* "filename" is an environment variable, so it should hang around */
	/* if set, it is expected to be a full path name (if relevant) */
	if (filename) {
		if ((fp = fopenp(filename, "r")) != (FILE *)0) {
		    configfile = filename;
		    return(fp);
#if defined(UNIX) || defined(VMS)
		} else {
		    /* access() above probably caught most problems for UNIX */
		    raw_printf("Couldn't open requested config file %s (%d).",
					filename, errno);
		    wait_synch();
		    /* fall through to standard names */
#endif
		}
	}

#if defined(MICRO) || defined(MAC) || defined(__BEOS__) || defined(WIN32)
	if ((fp = fopenp(fqname(configfile, CONFIGPREFIX, 0), "r"))
								!= (FILE *)0)
		return(fp);
# ifdef MSDOS
	else if ((fp = fopenp(fqname(backward_compat_configfile,
					CONFIGPREFIX, 0), "r")) != (FILE *)0)
		return(fp);
# endif
#else
	/* constructed full path names don't need fqname() */
# ifdef VMS
	if ((fp = fopenp(fqname("nethackini", CONFIGPREFIX, 0), "r"))
								!= (FILE *)0) {
		configfile = "nethackini";
		return(fp);
	}
	if ((fp = fopenp("sys$login:nethack.ini", "r")) != (FILE *)0) {
		configfile = "nethack.ini";
		return(fp);
	}

	envp = nh_getenv("HOME");
	if (!envp)
		Strcpy(tmp_config, "NetHack.cnf");
	else
		Sprintf(tmp_config, "%s%s", envp, "NetHack.cnf");
	if ((fp = fopenp(tmp_config, "r")) != (FILE *)0)
		return(fp);
# else	/* should be only UNIX left */
	envp = nh_getenv("HOME");
	if (!envp)
		Strcpy(tmp_config, configfile);
	else
		Sprintf(tmp_config, "%s/%s", envp, configfile);
	if ((fp = fopenp(tmp_config, "r")) != (FILE *)0)
		return(fp);
# if defined(__APPLE__)
	/* try an alternative */
	if (envp) {
		Sprintf(tmp_config, "%s/%s", envp, "Library/Preferences/NetHack Defaults");
		if ((fp = fopenp(tmp_config, "r")) != (FILE *)0)
			return(fp);
		Sprintf(tmp_config, "%s/%s", envp, "Library/Preferences/NetHack Defaults.txt");
		if ((fp = fopenp(tmp_config, "r")) != (FILE *)0)
			return(fp);
	}
# endif
	if (errno != ENOENT) {
	    char *details;

	    /* e.g., problems when setuid NetHack can't search home
	     * directory restricted to user */

#if defined (NHSTDC) && !defined(NOTSTDC)
	    if ((details = strerror(errno)) == 0)
#endif
		details = "";
	    raw_printf("Couldn't open default config file %s %s(%d).",
		       tmp_config, details, errno);
	    wait_synch();
	}
# endif
#endif
	return (FILE *)0;

}


/*
 * Retrieve a list of integers from a file into a uchar array.
 *
 * NOTE: zeros are inserted unless modlist is TRUE, in which case the list
 *  location is unchanged.  Callers must handle zeros if modlist is FALSE.
 */
STATIC_OVL int
get_uchars(fp, buf, bufp, list, modlist, size, name)
    FILE *fp;		/* input file pointer */
    char *buf;		/* read buffer, must be of size BUFSZ */
    char *bufp;		/* current pointer */
    uchar *list;	/* return list */
    boolean modlist;	/* TRUE: list is being modified in place */
    int  size;		/* return list size */
    const char *name;		/* name of option for error message */
{
    unsigned int num = 0;
    int count = 0;
    boolean havenum = FALSE;

    while (1) {
	switch(*bufp) {
	    case ' ':  case '\0':
	    case '\t': case '\n':
		if (havenum) {
		    /* if modifying in place, don't insert zeros */
		    if (num || !modlist) list[count] = num;
		    count++;
		    num = 0;
		    havenum = FALSE;
		}
		if (count == size || !*bufp) return count;
		bufp++;
		break;

	    case '0': case '1': case '2': case '3':
	    case '4': case '5': case '6': case '7':
	    case '8': case '9':
		havenum = TRUE;
		num = num*10 + (*bufp-'0');
		bufp++;
		break;

	    case '\\':
		if (fp == (FILE *)0)
		    goto gi_error;
		do  {
		    if (!fgets(buf, BUFSZ, fp)) goto gi_error;
		} while (buf[0] == '#');
		bufp = buf;
		break;

	    default:
gi_error:
		raw_printf("Syntax error in %s", name);
		wait_synch();
		return count;
	}
    }
    /*NOTREACHED*/
}

#ifdef NOCWD_ASSUMPTIONS
STATIC_OVL void
adjust_prefix(bufp, prefixid)
char *bufp;
int prefixid;
{
	char *ptr;

	if (!bufp) return;
	/* Backward compatibility, ignore trailing ;n */ 
	if ((ptr = index(bufp, ';')) != 0) *ptr = '\0';
	if (strlen(bufp) > 0) {
		fqn_prefix[prefixid] = (char *)alloc(strlen(bufp)+2);
		Strcpy(fqn_prefix[prefixid], bufp);
		append_slash(fqn_prefix[prefixid]);
	}
}
#endif

#define match_varname(INP,NAM,LEN) match_optname(INP, NAM, LEN, TRUE)

/*ARGSUSED*/
int
parse_config_line(fp, buf, tmp_ramdisk, tmp_levels)
FILE		*fp;
char		*buf;
char		*tmp_ramdisk;
char		*tmp_levels;
{
#if (defined(macintosh) && (defined(__SC__) || defined(__MRC__))) || defined(__MWERKS__)
# pragma unused(tmp_ramdisk,tmp_levels)
#endif
	char		*bufp, *altp;
	uchar   translate[MAXPCHARS];
	int   len;

	if (*buf == '#')
		return 1;

	/* remove trailing whitespace */
	bufp = eos(buf);
	while (--bufp > buf && isspace(*bufp))
		continue;

	if (bufp <= buf)
		return 1;		/* skip all-blank lines */
	else
		*(bufp + 1) = '\0';	/* terminate line */

	/* find the '=' or ':' */
	bufp = index(buf, '=');
	altp = index(buf, ':');
	if (!bufp || (altp && altp < bufp)) bufp = altp;
	if (!bufp) return 0;

	/* skip  whitespace between '=' and value */
	do { ++bufp; } while (isspace(*bufp));

	/* Go through possible variables */
	/* some of these (at least LEVELS and SAVE) should now set the
	 * appropriate fqn_prefix[] rather than specialized variables
	 */
	if (match_varname(buf, "OPTIONS", 4)) {
		parseoptions(bufp, TRUE, TRUE);
		if (plname[0])		/* If a name was given */
			plnamesuffix();	/* set the character class */
#ifdef AUTOPICKUP_EXCEPTIONS
	} else if (match_varname(buf, "AUTOPICKUP_EXCEPTION", 5)) {
		add_autopickup_exception(bufp);
#endif
#ifdef NOCWD_ASSUMPTIONS
	} else if (match_varname(buf, "HACKDIR", 4)) {
		adjust_prefix(bufp, HACKPREFIX);
	} else if (match_varname(buf, "LEVELDIR", 4) ||
		   match_varname(buf, "LEVELS", 4)) {
		adjust_prefix(bufp, LEVELPREFIX);
	} else if (match_varname(buf, "SAVEDIR", 4)) {
		adjust_prefix(bufp, SAVEPREFIX);
	} else if (match_varname(buf, "BONESDIR", 5)) {
		adjust_prefix(bufp, BONESPREFIX);
	} else if (match_varname(buf, "DATADIR", 4)) {
		adjust_prefix(bufp, DATAPREFIX);
	} else if (match_varname(buf, "SCOREDIR", 4)) {
		adjust_prefix(bufp, SCOREPREFIX);
	} else if (match_varname(buf, "LOCKDIR", 4)) {
		adjust_prefix(bufp, LOCKPREFIX);
	} else if (match_varname(buf, "CONFIGDIR", 4)) {
		adjust_prefix(bufp, CONFIGPREFIX);
	} else if (match_varname(buf, "TROUBLEDIR", 4)) {
		adjust_prefix(bufp, TROUBLEPREFIX);
#else /*NOCWD_ASSUMPTIONS*/
# ifdef MICRO
	} else if (match_varname(buf, "HACKDIR", 4)) {
		(void) strncpy(hackdir, bufp, PATHLEN-1);
#  ifdef MFLOPPY
	} else if (match_varname(buf, "RAMDISK", 3)) {
				/* The following ifdef is NOT in the wrong
				 * place.  For now, we accept and silently
				 * ignore RAMDISK */
#   ifndef AMIGA
		(void) strncpy(tmp_ramdisk, bufp, PATHLEN-1);
#   endif
#  endif
	} else if (match_varname(buf, "LEVELS", 4)) {
		(void) strncpy(tmp_levels, bufp, PATHLEN-1);

	} else if (match_varname(buf, "SAVE", 4)) {
#  ifdef MFLOPPY
		extern	int saveprompt;
#  endif
		char *ptr;
		if ((ptr = index(bufp, ';')) != 0) {
			*ptr = '\0';
#  ifdef MFLOPPY
			if (*(ptr+1) == 'n' || *(ptr+1) == 'N') {
				saveprompt = FALSE;
			}
#  endif
		}
# ifdef	MFLOPPY
		else
		    saveprompt = flags.asksavedisk;
# endif

		(void) strncpy(SAVEP, bufp, SAVESIZE-1);
		append_slash(SAVEP);
# endif /* MICRO */
#endif /*NOCWD_ASSUMPTIONS*/

	} else if (match_varname(buf, "NAME", 4)) {
	    (void) strncpy(plname, bufp, PL_NSIZ-1);
	    plnamesuffix();
	} else if (match_varname(buf, "ROLE", 4) ||
		   match_varname(buf, "CHARACTER", 4)) {
	    if ((len = str2role(bufp)) >= 0)
	    	flags.initrole = len;
	} else if (match_varname(buf, "DOGNAME", 3)) {
	    (void) strncpy(dogname, bufp, PL_PSIZ-1);
	} else if (match_varname(buf, "CATNAME", 3)) {
	    (void) strncpy(catname, bufp, PL_PSIZ-1);

	} else if (match_varname(buf, "BOULDER", 3)) {
	    (void) get_uchars(fp, buf, bufp, &iflags.bouldersym, TRUE,
			      1, "BOULDER");
	} else if (match_varname(buf, "GRAPHICS", 4)) {
	    len = get_uchars(fp, buf, bufp, translate, FALSE,
			     MAXPCHARS, "GRAPHICS");
	    assign_graphics(translate, len, MAXPCHARS, 0);
	} else if (match_varname(buf, "DUNGEON", 4)) {
	    len = get_uchars(fp, buf, bufp, translate, FALSE,
			     MAXDCHARS, "DUNGEON");
	    assign_graphics(translate, len, MAXDCHARS, 0);
	} else if (match_varname(buf, "TRAPS", 4)) {
	    len = get_uchars(fp, buf, bufp, translate, FALSE,
			     MAXTCHARS, "TRAPS");
	    assign_graphics(translate, len, MAXTCHARS, MAXDCHARS);
	} else if (match_varname(buf, "EFFECTS", 4)) {
	    len = get_uchars(fp, buf, bufp, translate, FALSE,
			     MAXECHARS, "EFFECTS");
	    assign_graphics(translate, len, MAXECHARS, MAXDCHARS+MAXTCHARS);

	} else if (match_varname(buf, "OBJECTS", 3)) {
	    /* oc_syms[0] is the RANDOM object, unused */
	    (void) get_uchars(fp, buf, bufp, &(oc_syms[1]), TRUE,
					MAXOCLASSES-1, "OBJECTS");
	} else if (match_varname(buf, "MONSTERS", 3)) {
	    /* monsyms[0] is unused */
	    (void) get_uchars(fp, buf, bufp, &(monsyms[1]), TRUE,
					MAXMCLASSES-1, "MONSTERS");
	} else if (match_varname(buf, "WARNINGS", 5)) {
	    (void) get_uchars(fp, buf, bufp, translate, FALSE,
					WARNCOUNT, "WARNINGS");
	    assign_warnings(translate);
#ifdef WIZARD
	} else if (match_varname(buf, "WIZKIT", 6)) {
	    (void) strncpy(wizkit, bufp, WIZKIT_MAX-1);
#endif
#ifdef AMIGA
	} else if (match_varname(buf, "FONT", 4)) {
		char *t;

		if( t = strchr( buf+5, ':' ) )
		{
		    *t = 0;
		    amii_set_text_font( buf+5, atoi( t + 1 ) );
		    *t = ':';
		}
	} else if (match_varname(buf, "PATH", 4)) {
		(void) strncpy(PATH, bufp, PATHLEN-1);
	} else if (match_varname(buf, "DEPTH", 5)) {
		extern int amii_numcolors;
		int val = atoi( bufp );
		amii_numcolors = 1L << min( DEPTH, val );
	} else if (match_varname(buf, "DRIPENS", 7)) {
		int i, val;
		char *t;
		for (i = 0, t = strtok(bufp, ",/"); t != (char *)0;
				i < 20 && (t = strtok((char*)0, ",/")), ++i) {
			sscanf(t, "%d", &val );
			flags.amii_dripens[i] = val;
		}
	} else if (match_varname(buf, "SCREENMODE", 10 )) {
		extern long amii_scrnmode;
		if (!stricmp(bufp,"req"))
		    amii_scrnmode = 0xffffffff; /* Requester */
		else if( sscanf(bufp, "%x", &amii_scrnmode) != 1 )
		    amii_scrnmode = 0;
	} else if (match_varname(buf, "MSGPENS", 7)) {
		extern int amii_msgAPen, amii_msgBPen;
		char *t = strtok(bufp, ",/");
		if( t )
		{
		    sscanf(t, "%d", &amii_msgAPen);
		    if( t = strtok((char*)0, ",/") )
				sscanf(t, "%d", &amii_msgBPen);
		}
	} else if (match_varname(buf, "TEXTPENS", 8)) {
		extern int amii_textAPen, amii_textBPen;
		char *t = strtok(bufp, ",/");
		if( t )
		{
		    sscanf(t, "%d", &amii_textAPen);
		    if( t = strtok((char*)0, ",/") )
				sscanf(t, "%d", &amii_textBPen);
		}
	} else if (match_varname(buf, "MENUPENS", 8)) {
		extern int amii_menuAPen, amii_menuBPen;
		char *t = strtok(bufp, ",/");
		if( t )
		{
		    sscanf(t, "%d", &amii_menuAPen);
		    if( t = strtok((char*)0, ",/") )
				sscanf(t, "%d", &amii_menuBPen);
		}
	} else if (match_varname(buf, "STATUSPENS", 10)) {
		extern int amii_statAPen, amii_statBPen;
		char *t = strtok(bufp, ",/");
		if( t )
		{
		    sscanf(t, "%d", &amii_statAPen);
		    if( t = strtok((char*)0, ",/") )
				sscanf(t, "%d", &amii_statBPen);
		}
	} else if (match_varname(buf, "OTHERPENS", 9)) {
		extern int amii_otherAPen, amii_otherBPen;
		char *t = strtok(bufp, ",/");
		if( t )
		{
		    sscanf(t, "%d", &amii_otherAPen);
		    if( t = strtok((char*)0, ",/") )
				sscanf(t, "%d", &amii_otherBPen);
		}
	} else if (match_varname(buf, "PENS", 4)) {
		extern unsigned short amii_init_map[ AMII_MAXCOLORS ];
		int i;
		char *t;

		for (i = 0, t = strtok(bufp, ",/");
			i < AMII_MAXCOLORS && t != (char *)0;
			t = strtok((char *)0, ",/"), ++i)
		{
			sscanf(t, "%hx", &amii_init_map[i]);
		}
		amii_setpens( amii_numcolors = i );
	} else if (match_varname(buf, "FGPENS", 6)) {
		extern int foreg[ AMII_MAXCOLORS ];
		int i;
		char *t;

		for (i = 0, t = strtok(bufp, ",/");
			i < AMII_MAXCOLORS && t != (char *)0;
			t = strtok((char *)0, ",/"), ++i)
		{
			sscanf(t, "%d", &foreg[i]);
		}
	} else if (match_varname(buf, "BGPENS", 6)) {
		extern int backg[ AMII_MAXCOLORS ];
		int i;
		char *t;

		for (i = 0, t = strtok(bufp, ",/");
			i < AMII_MAXCOLORS && t != (char *)0;
			t = strtok((char *)0, ",/"), ++i)
		{
			sscanf(t, "%d", &backg[i]);
		}
#endif
#ifdef USER_SOUNDS
	} else if (match_varname(buf, "SOUNDDIR", 8)) {
		sounddir = (char *)strdup(bufp);
	} else if (match_varname(buf, "SOUND", 5)) {
		add_sound_mapping(bufp);
#endif
#ifdef QT_GRAPHICS
	/* These should move to wc_ options */
	} else if (match_varname(buf, "QT_TILEWIDTH", 12)) {
		extern char *qt_tilewidth;
		if (qt_tilewidth == NULL)	
			qt_tilewidth=(char *)strdup(bufp);
	} else if (match_varname(buf, "QT_TILEHEIGHT", 13)) {
		extern char *qt_tileheight;
		if (qt_tileheight == NULL)	
			qt_tileheight=(char *)strdup(bufp);
	} else if (match_varname(buf, "QT_FONTSIZE", 11)) {
		extern char *qt_fontsize;
		if (qt_fontsize == NULL)
			qt_fontsize=(char *)strdup(bufp);
	} else if (match_varname(buf, "QT_COMPACT", 10)) {
		extern int qt_compact_mode;
		qt_compact_mode = atoi(bufp);
#endif
	} else
		return 0;
	return 1;
}

void
read_config_file(filename)
const char *filename;
{
#define tmp_levels	(char *)0
#define tmp_ramdisk	(char *)0

#if defined(MICRO) || defined(WIN32)
#undef tmp_levels
	char	tmp_levels[PATHLEN];
# ifdef MFLOPPY
#  ifndef AMIGA
#undef tmp_ramdisk
	char	tmp_ramdisk[PATHLEN];
#  endif
# endif
#endif
	char	buf[4*BUFSZ];
	FILE	*fp;

	if (!(fp = fopen_config_file(filename))) return;

#if defined(MICRO) || defined(WIN32)
# ifdef MFLOPPY
#  ifndef AMIGA
	tmp_ramdisk[0] = 0;
#  endif
# endif
	tmp_levels[0] = 0;
#endif
	/* begin detection of duplicate configfile options */
	set_duplicate_opt_detection(1);

	while (fgets(buf, 4*BUFSZ, fp)) {
		if (!parse_config_line(fp, buf, tmp_ramdisk, tmp_levels)) {
			raw_printf("Bad option line:  \"%.50s\"", buf);
			wait_synch();
		}
	}
	(void) fclose(fp);
	
	/* turn off detection of duplicate configfile options */
	set_duplicate_opt_detection(0);

#if defined(MICRO) && !defined(NOCWD_ASSUMPTIONS)
	/* should be superseded by fqn_prefix[] */
# ifdef MFLOPPY
	Strcpy(permbones, tmp_levels);
#  ifndef AMIGA
	if (tmp_ramdisk[0]) {
		Strcpy(levels, tmp_ramdisk);
		if (strcmp(permbones, levels))		/* if not identical */
			ramdisk = TRUE;
	} else
#  endif /* AMIGA */
		Strcpy(levels, tmp_levels);

	Strcpy(bones, levels);
# endif /* MFLOPPY */
#endif /* MICRO */
	return;
}

#ifdef WIZARD
STATIC_OVL FILE *
fopen_wizkit_file()
{
	FILE *fp;
#if defined(VMS) || defined(UNIX)
	char	tmp_wizkit[BUFSZ];
#endif
	char *envp;

	envp = nh_getenv("WIZKIT");
	if (envp && *envp) (void) strncpy(wizkit, envp, WIZKIT_MAX - 1);
	if (!wizkit[0]) return (FILE *)0;

	if ((fp = fopenp(wizkit, "r")) != (FILE *)0) {
	    return(fp);
	} else {
	    /* access() above probably caught most problems for UNIX */
	    raw_printf("Couldn't open requested config file %s (%d).",
				wizkit, errno);
	    wait_synch();
	}

#if defined(MICRO) || defined(MAC) || defined(__BEOS__) || defined(WIN32)
	if ((fp = fopenp(fqname(wizkit, CONFIGPREFIX, 0), "r"))
								!= (FILE *)0)
		return(fp);
#else
# ifdef VMS
	envp = nh_getenv("HOME");
	if (envp)
		Sprintf(tmp_wizkit, "%s%s", envp, wizkit);
	else
		Sprintf(tmp_wizkit, "%s%s", "sys$login:", wizkit);
	if ((fp = fopenp(tmp_wizkit, "r")) != (FILE *)0)
		return(fp);
# else	/* should be only UNIX left */
	envp = nh_getenv("HOME");
	if (envp)
		Sprintf(tmp_wizkit, "%s/%s", envp, wizkit);
	else 	Strcpy(tmp_wizkit, wizkit);
	if ((fp = fopenp(tmp_wizkit, "r")) != (FILE *)0)
		return(fp);
	else if (errno != ENOENT) {
		/* e.g., problems when setuid NetHack can't search home
		 * directory restricted to user */
		raw_printf("Couldn't open default wizkit file %s (%d).",
					tmp_wizkit, errno);
		wait_synch();
	}
# endif
#endif
	return (FILE *)0;
}

void
read_wizkit()
{
	FILE *fp;
	char *ep, buf[BUFSZ];
	struct obj *otmp;
	boolean bad_items = FALSE, skip = FALSE;

	if (!wizard || !(fp = fopen_wizkit_file())) return;

	while (fgets(buf, (int)(sizeof buf), fp)) {
	    ep = index(buf, '\n');
	    if (skip) {	/* in case previous line was too long */
		if (ep) skip = FALSE; /* found newline; next line is normal */
	    } else {
		if (!ep) skip = TRUE; /* newline missing; discard next fgets */
		else *ep = '\0';		/* remove newline */

		if (buf[0]) {
			otmp = readobjnam(buf, (struct obj *)0, FALSE);
			if (otmp) {
			    if (otmp != &zeroobj)
				otmp = addinv(otmp);
			} else {
			    /* .60 limits output line width to 79 chars */
			    raw_printf("Bad wizkit item: \"%.60s\"", buf);
			    bad_items = TRUE;
			}
		}
	    }
	}
	if (bad_items)
	    wait_synch();
	(void) fclose(fp);
	return;
}

#endif /*WIZARD*/

/* ----------  END CONFIG FILE HANDLING ----------- */

/* ----------  BEGIN SCOREBOARD CREATION ----------- */

/* verify that we can write to the scoreboard file; if not, try to create one */
void
check_recordfile(dir)
const char *dir;
{
	const char *fq_record;
	FILE* fd;

	fq_record = fqname(RECORD, SCOREPREFIX, 0);
	fd = fopen(fq_record, "r+");
	if (fd != 0) {
	    (void) fclose(fd);	/* RECORD is accessible */
	} else if ((fd = fopen(fq_record, "w+")) >= 0) {
	    (void) fclose(fd);	/* RECORD newly created */
	} else {
	    raw_printf("Warning: cannot write scoreboard file %s", fq_record);
	    wait_synch();
	}
}

/* ----------  END SCOREBOARD CREATION ----------- */

/* ----------  BEGIN PANIC/IMPOSSIBLE LOG ----------- */

/*ARGSUSED*/
void
paniclog(type, reason)
const char *type;	/* panic, impossible, trickery */
const char *reason;	/* explanation */
{
#ifdef PANICLOG
	FILE *lfile;
	char buf[BUFSZ];

	if (!program_state.in_paniclog) {
		program_state.in_paniclog = 1;
		lfile = fopen_datafile(PANICLOG, "a", TROUBLEPREFIX);
		if (lfile) {
		    (void) fprintf(lfile, "%s %08ld: %s %s\n",
				   version_string(buf), yyyymmdd((time_t)0L),
				   type, reason);
		    (void) fclose(lfile);
		}
		program_state.in_paniclog = 0;
	}
#endif /* PANICLOG */
	return;
}

/* ----------  END PANIC/IMPOSSIBLE LOG ----------- */

#ifdef SELF_RECOVER

/* ----------  BEGIN INTERNAL RECOVER ----------- */
boolean
recover_savefile()
{
	int gfd, lfd, sfd;
	int lev, savelev, hpid;
	xchar levc;
	struct version_info version_data;
	int processed[256];
	char savename[SAVESIZE], errbuf[BUFSZ];

	for (lev = 0; lev < 256; lev++)
		processed[lev] = 0;

	/* level 0 file contains:
	 *	pid of creating process (ignored here)
	 *	level number for current level of save file
	 *	name of save file nethack would have created
	 *	and game state
	 */
	gfd = open_levelfile(0, errbuf);
	if (gfd < 0) {
	    raw_printf("%s\n", errbuf);
	    return FALSE;
	}
	if (read(gfd, (genericptr_t) &hpid, sizeof hpid) != sizeof hpid) {
	    raw_printf(
"\nCheckpoint data incompletely written or subsequently clobbered. Recovery impossible.");
	    (void)close(gfd);
	    return FALSE;
	}
	if (read(gfd, (genericptr_t) &savelev, sizeof(savelev))
							!= sizeof(savelev)) {
	    raw_printf("\nCheckpointing was not in effect for %s -- recovery impossible.\n",
			lock);
	    (void)close(gfd);
	    return FALSE;
	}
	if ((read(gfd, (genericptr_t) savename, sizeof savename)
		!= sizeof savename) ||
	    (read(gfd, (genericptr_t) &version_data, sizeof version_data)
		!= sizeof version_data)) {
	    raw_printf("\nError reading %s -- can't recover.\n", lock);
	    (void)close(gfd);
	    return FALSE;
	}

	/* save file should contain:
	 *	version info
	 *	current level (including pets)
	 *	(non-level-based) game state
	 *	other levels
	 */
	set_savefile_name();
	sfd = create_savefile();
	if (sfd < 0) {
	    raw_printf("\nCannot recover savefile %s.\n", SAVEF);
	    (void)close(gfd);
	    return FALSE;
	}

	lfd = open_levelfile(savelev, errbuf);
	if (lfd < 0) {
	    raw_printf("\n%s\n", errbuf);
	    (void)close(gfd);
	    (void)close(sfd);
	    delete_savefile();
	    return FALSE;
	}

	if (write(sfd, (genericptr_t) &version_data, sizeof version_data)
		!= sizeof version_data) {
	    raw_printf("\nError writing %s; recovery failed.", SAVEF);
	    (void)close(gfd);
	    (void)close(sfd);
	    delete_savefile();
	    return FALSE;
	}

	if (!copy_bytes(lfd, sfd)) {
		(void) close(lfd);
		(void) close(sfd);
		delete_savefile();
		return FALSE;
	}
	(void)close(lfd);
	processed[savelev] = 1;

	if (!copy_bytes(gfd, sfd)) {
		(void) close(lfd);
		(void) close(sfd);
		delete_savefile();
		return FALSE;
	}
	(void)close(gfd);
	processed[0] = 1;

	for (lev = 1; lev < 256; lev++) {
		/* level numbers are kept in xchars in save.c, so the
		 * maximum level number (for the endlevel) must be < 256
		 */
		if (lev != savelev) {
			lfd = open_levelfile(lev, (char *)0);
			if (lfd >= 0) {
				/* any or all of these may not exist */
				levc = (xchar) lev;
				write(sfd, (genericptr_t) &levc, sizeof(levc));
				if (!copy_bytes(lfd, sfd)) {
					(void) close(lfd);
					(void) close(sfd);
					delete_savefile();
					return FALSE;
				}
				(void)close(lfd);
				processed[lev] = 1;
			}
		}
	}
	(void)close(sfd);

#ifdef HOLD_LOCKFILE_OPEN
	really_close();
#endif
	/*
	 * We have a successful savefile!
	 * Only now do we erase the level files.
	 */
	for (lev = 0; lev < 256; lev++) {
		if (processed[lev]) {
			const char *fq_lock;
			set_levelfile_name(lock, lev);
			fq_lock = fqname(lock, LEVELPREFIX, 3);
			(void) unlink(fq_lock);
		}
	}
	return TRUE;
}

boolean
copy_bytes(ifd, ofd)
int ifd, ofd;
{
	char buf[BUFSIZ];
	int nfrom, nto;

	do {
		nfrom = read(ifd, buf, BUFSIZ);
		nto = write(ofd, buf, nfrom);
		if (nto != nfrom) return FALSE;
	} while (nfrom == BUFSIZ);
	return TRUE;
}

/* ----------  END INTERNAL RECOVER ----------- */
#endif /*SELF_RECOVER*/

/*files.c*/
