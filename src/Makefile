#	NetHack Makefile.
#	SCCS Id: @(#)Makefile.src	3.4	2002/03/02

SHELL=/bin/sh

# Normally, the C compiler driver is used for linking:
LINK=$(CC)

# CFLAGS = -g -I../include
CFLAGS = -O -I../include -fvisibility=internal
LFLAGS = -shared

# make NetHack
GAME     = libnethack.so
# GAME     = nethack.prg

# used by `make depend' to reconstruct this Makefile; you shouldn't need this
AWK	= awk

# ----------------------------------------
#
# Nothing below this line should have to be changed.
#
# Other things that have to be reconfigured are in config.h,
# {unixconf.h, pcconf.h, tosconf.h}, and possibly system.h

MAKEDEFS = ../util/makedefs

# timestamp files to reduce `make' overhead and shorten .o dependency lists
CONFIG_H = ../src/config.h-t
HACK_H	= ../src/hack.h-t

# all .c that are part of the main NetHack program and are not operating- or
# windowing-system specific
HACKCSRC = allmain.c alloc.c apply.c artifact.c attrib.c ball.c bones.c \
	   botl.c cmd.c dbridge.c decl.c detect.c dig.c display.c dlb.c do.c \
	   do_name.c do_wear.c dog.c dogmove.c dokick.c dothrow.c drawing.c \
	   dungeon.c eat.c end.c engrave.c exper.c explode.c extralev.c \
	   files.c fountain.c hack.c hacklib.c invent.c light.c lock.c \
	   mail.c makemon.c mapglyph.c mcastu.c mhitm.c mhitu.c minion.c \
	   mklev.c mkmap.c \
	   mkmaze.c mkobj.c mkroom.c mon.c mondata.c monmove.c monst.c \
	   mplayer.c mthrowu.c muse.c music.c o_init.c objects.c objnam.c \
	   options.c pager.c pickup.c pline.c polyself.c potion.c pray.c \
	   priest.c quest.c questpgr.c read.c rect.c region.c restore.c rip.c \
	   rnd.c role.c rumors.c save.c shk.c shknam.c sit.c sounds.c sp_lev.c \
	   spell.c steal.c steed.c teleport.c timeout.c topten.c track.c trap.c \
	   u_init.c uhitm.c vault.c version.c vision.c weapon.c were.c wield.c \
	   windows.c wizard.c worm.c worn.c write.c zap.c \
	   ioctl.c unixmain.c unixtty.c unixunix.c unixres.c \
	   getline.c termcap.c topl.c wintty.c version.c

# generated source files (tile.c is handled separately via WINxxxSRC)
GENCSRC = monstr.c vis_tab.c	#tile.c

# all .h files except date.h, onames.h, pm.h, and vis_tab.h which would
# cause dependency loops if run through "make depend"
# and dgn_comp.h, dgn_file.h, lev_comp.h, special level & dungeon files.
#
HACKINCL = align.h amiconf.h artifact.h artilist.h attrib.h beconf.h color.h \
	config.h config1.h coord.h decl.h def_os2.h display.h dlb.h dungeon.h \
	edog.h emin.h engrave.h epri.h eshk.h extern.h flag.h func_tab.h \
	global.h hack.h lev.h macconf.h mfndpos.h micro.h mkroom.h \
	monattk.h mondata.h monflag.h monst.h monsym.h obj.h objclass.h \
	os2conf.h patchlevel.h pcconf.h permonst.h prop.h rect.h region.h rm.h \
	sp_lev.h spell.h system.h tcap.h timeout.h tosconf.h tradstdc.h \
	trampoli.h trap.h unixconf.h vault.h vision.h vmsconf.h wintty.h \
	winX.h winprocs.h wintype.h you.h youprop.h

HSOURCES = $(HACKINCL) date.h onames.h pm.h vis_tab.h\
		lev_comp.h dgn_comp.h dgn_file.h

# the following .o's _must_ be made before any others (for makedefs)
FIRSTOBJ = monst.o objects.o

HOBJ = $(FIRSTOBJ) allmain.o alloc.o apply.o artifact.o attrib.o ball.o \
	bones.o botl.o cmd.o dbridge.o decl.o detect.o dig.o display.o dlb.o \
	do.o do_name.o do_wear.o dog.o dogmove.o dokick.o dothrow.o \
	drawing.o dungeon.o eat.o end.o engrave.o exper.o explode.o \
	extralev.o files.o fountain.o hack.o hacklib.o invent.o light.o \
	lock.o mail.o makemon.o mapglyph.o mcastu.o mhitm.o mhitu.o \
	minion.o mklev.o mkmap.o \
	mkmaze.o mkobj.o mkroom.o mon.o mondata.o monmove.o monstr.o \
	mplayer.o mthrowu.o muse.o music.o o_init.o objnam.o options.o \
	pager.o pickup.o pline.o polyself.o potion.o pray.o priest.o \
	quest.o questpgr.o read.o rect.o region.o restore.o rip.o rnd.o \
	role.o rumors.o save.o shk.o shknam.o sit.o sounds.o sp_lev.o spell.o \
	steal.o steed.o teleport.o timeout.o topten.o track.o trap.o u_init.o \
	uhitm.o vault.o vision.o vis_tab.o weapon.o were.o wield.o windows.o \
	wizard.o worm.o worn.o write.o zap.o \
	ioctl.o unixmain.o unixtty.o unixunix.o unixres.o \
	getline.o termcap.o topl.o wintty.o version.o

$(GAME):	$(HOBJ) Makefile
	@echo "Loading ..."
	$(LINK) $(LFLAGS) -o $(GAME) $(HOBJ)
	@echo "$(GAME) is up to date."

DUMB.Setup:	../include/extern.h
	cp ../include/extern.h ../include/extern.h.BAK
	cat ../include/extern.h | \
		sed -e '/^E\ int\ /!b' \
			-e '/[^;/ 	]$$/N' \
			-e '/[(][*]occupation[)]/b' \
			-e '/[(][*]afternmv[)]/b' \
			-e '/float_down/b' \
			-e '/done1/b' \
			-e '/identify/b' \
			-e '/Hear_again/b' \
			-e '/hangup/b' \
			-e 's/^\(.*\)$$/\/\* \1 \/\*\*\//' | \
		sed -e '/^E\ void\ /!b' \
			-e '/[^;/ 	]$$/N' \
			-e 's/^\(.*\)$$/\/\* \1 \/\*\*\//' \
				>../include/extern.DUMB
	cp ../include/extern.DUMB ../include/extern.h
	@touch DUMB.Setup

all:	$(GAME)


#	dependencies for makedefs and its outputs, which the util
#	Makefile is responsible for keeping up to date
#

# special rules, to force update of makedefs, real dependencies should be
# below in the 'make depend' output.
monst.o:
	$(CC) $(CFLAGS) -c monst.c
	@rm -f $(MAKEDEFS)

objects.o:
	$(CC) $(CFLAGS) -c objects.c
	@rm -f $(MAKEDEFS)

$(MAKEDEFS): ../util/makedefs.c  $(CONFIG_H) ../include/permonst.h \
		../include/objclass.h ../include/monsym.h \
		../include/artilist.h ../include/dungeon.h ../include/obj.h \
		../include/monst.h ../include/you.h ../include/flag.h \
		../include/dlb.h ../include/patchlevel.h ../include/qtext.h
	@( cd ../util ; $(MAKE) makedefs)

../include/onames.h: $(MAKEDEFS)
	@( cd ../util ; $(MAKE) ../include/onames.h )
../include/pm.h: $(MAKEDEFS)
	@( cd ../util ; $(MAKE) ../include/pm.h )
monstr.c: $(MAKEDEFS)
	@( cd ../util ; $(MAKE) ../src/monstr.c )
../include/vis_tab.h: $(MAKEDEFS)
	@( cd ../util ; $(MAKE) ../include/vis_tab.h )
# makedefs -z makes both vis_tab.h and vis_tab.c, but writes the .h first
vis_tab.c: ../include/vis_tab.h
tile.c: ../win/share/tilemap.c $(HACK_H)
	@( cd ../util ; $(MAKE) ../src/tile.c )

#	date.h should be remade any time any of the source or include code
#	is modified.  Unfortunately, this would make the contents of this
#	file far more complex.  Since "hack.h" depends on most of the include
#	files, we kludge around this by making date.h dependent on hack.h,
#	even though it doesn't include this file.
#
#	hack.h depends on makedefs' output, so we know makedefs will be
#	up to date before being executed
../include/date.h:	$(VERSOURCES) $(HACK_H)
	../util/makedefs -v


tags: $(CSOURCES)
	@echo ctags -tw ...
	@ctags -tw $(CSOURCES)
	@( cd ../include ; ctags -tw $(HSOURCES) )
	@( cd ../util ; $(MAKE) tags )

clean:
	-rm -f *.o $(HACK_H) $(CONFIG_H)

spotless: clean
	-rm -f a.out core $(GAME) Sys*
	-rm -f ../include/date.h ../include/onames.h ../include/pm.h
	-rm -f monstr.c ../include/vis_tab.h vis_tab.c tile.c *.moc
	-rm -f ../win/gnome/gn_rip.h


depend: ../sys/unix/depend.awk \
		$(GENCSRC) $(HACKCSRC)
	$(AWK) -f ../sys/unix/depend.awk ../include/*.h \
		$(GENCSRC) $(HACKCSRC) >makedep
	@echo '/^# DO NOT DELETE THIS LINE OR CHANGE ANYTHING BEYOND IT/+2,$$d' >eddep
	@echo '$$r makedep' >>eddep
	@echo 'w' >>eddep
	@cp Makefile Makefile.bak
	ed - Makefile < eddep
	@rm -f eddep makedep
	@echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	@echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	@echo '# see make depend above' >> Makefile
	- diff Makefile.bak Makefile
	@rm -f Makefile.bak

# DO NOT DELETE THIS LINE OR CHANGE ANYTHING BEYOND IT

# config.h timestamp
$(CONFIG_H): ../include/config.h ../include/config1.h ../include/tradstdc.h \
		../include/global.h ../include/coord.h ../include/vmsconf.h \
		../include/system.h ../include/unixconf.h ../include/os2conf.h \
		../include/micro.h ../include/pcconf.h ../include/tosconf.h \
		../include/amiconf.h ../include/macconf.h ../include/beconf.h \
		../include/wceconf.h ../include/ntconf.h ../include/nhlan.h
	touch $(CONFIG_H)
# hack.h timestamp
$(HACK_H): ../include/hack.h $(CONFIG_H) ../include/align.h \
		../include/dungeon.h ../include/monsym.h ../include/mkroom.h \
		../include/objclass.h ../include/youprop.h ../include/prop.h \
		../include/permonst.h ../include/monattk.h \
		../include/monflag.h ../include/mondata.h ../include/pm.h \
		../include/wintype.h ../include/decl.h ../include/quest.h \
		../include/spell.h ../include/color.h ../include/obj.h \
		../include/you.h ../include/attrib.h ../include/monst.h \
		../include/skills.h ../include/onames.h ../include/timeout.h \
		../include/trap.h ../include/flag.h ../include/rm.h \
		../include/vision.h ../include/display.h ../include/engrave.h \
		../include/rect.h ../include/region.h ../include/winprocs.h \
		../include/wintty.h ../include/trampoli.h
	touch $(HACK_H)
#
monstr.o: monstr.c $(CONFIG_H)
vis_tab.o: vis_tab.c $(CONFIG_H) ../include/vis_tab.h
allmain.o: allmain.c $(HACK_H)
alloc.o: alloc.c $(CONFIG_H)
apply.o: apply.c $(HACK_H) ../include/edog.h
artifact.o: artifact.c $(HACK_H) ../include/artifact.h ../include/artilist.h
attrib.o: attrib.c $(HACK_H)
ball.o: ball.c $(HACK_H)
bones.o: bones.c $(HACK_H) ../include/lev.h
botl.o: botl.c $(HACK_H)
cmd.o: cmd.c $(HACK_H) ../include/func_tab.h
dbridge.o: dbridge.c $(HACK_H)
decl.o: decl.c $(HACK_H)
detect.o: detect.c $(HACK_H) ../include/artifact.h
dig.o: dig.c $(HACK_H) ../include/edog.h
display.o: display.c $(HACK_H)
dlb.o: dlb.c $(CONFIG_H) ../include/dlb.h
do.o: do.c $(HACK_H) ../include/lev.h
do_name.o: do_name.c $(HACK_H)
do_wear.o: do_wear.c $(HACK_H)
dog.o: dog.c $(HACK_H) ../include/edog.h
dogmove.o: dogmove.c $(HACK_H) ../include/mfndpos.h ../include/edog.h
dokick.o: dokick.c $(HACK_H) ../include/eshk.h
dothrow.o: dothrow.c $(HACK_H) ../include/edog.h
drawing.o: drawing.c $(HACK_H) ../include/tcap.h
dungeon.o: dungeon.c $(HACK_H) ../include/dgn_file.h ../include/dlb.h
eat.o: eat.c $(HACK_H)
end.o: end.c $(HACK_H) ../include/eshk.h ../include/dlb.h
engrave.o: engrave.c $(HACK_H) ../include/lev.h
exper.o: exper.c $(HACK_H)
explode.o: explode.c $(HACK_H)
extralev.o: extralev.c $(HACK_H)
files.o: files.c $(HACK_H) ../include/dlb.h
fountain.o: fountain.c $(HACK_H)
hack.o: hack.c $(HACK_H)
hacklib.o: hacklib.c $(HACK_H)
invent.o: invent.c $(HACK_H)
light.o: light.c $(HACK_H) ../include/lev.h
lock.o: lock.c $(HACK_H)
mail.o: mail.c $(HACK_H) ../include/mail.h
makemon.o: makemon.c $(HACK_H) ../include/epri.h ../include/emin.h \
		../include/edog.h
mapglyph.o: mapglyph.c $(HACK_H)
mcastu.o: mcastu.c $(HACK_H)
mhitm.o: mhitm.c $(HACK_H) ../include/artifact.h ../include/edog.h
mhitu.o: mhitu.c $(HACK_H) ../include/artifact.h ../include/edog.h
minion.o: minion.c $(HACK_H) ../include/emin.h ../include/epri.h
mklev.o: mklev.c $(HACK_H)
mkmap.o: mkmap.c $(HACK_H) ../include/sp_lev.h
mkmaze.o: mkmaze.c $(HACK_H) ../include/sp_lev.h ../include/lev.h
mkobj.o: mkobj.c $(HACK_H)
mkroom.o: mkroom.c $(HACK_H)
mon.o: mon.c $(HACK_H) ../include/mfndpos.h ../include/edog.h
mondata.o: mondata.c $(HACK_H) ../include/eshk.h ../include/epri.h
monmove.o: monmove.c $(HACK_H) ../include/mfndpos.h ../include/artifact.h \
		../include/epri.h
monst.o: monst.c $(CONFIG_H) ../include/permonst.h ../include/align.h \
		../include/monattk.h ../include/monflag.h ../include/monsym.h \
		../include/dungeon.h ../include/eshk.h ../include/vault.h \
		../include/epri.h ../include/color.h
mplayer.o: mplayer.c $(HACK_H)
mthrowu.o: mthrowu.c $(HACK_H)
muse.o: muse.c $(HACK_H) ../include/edog.h
music.o: music.c $(HACK_H) #interp.c
o_init.o: o_init.c $(HACK_H) ../include/lev.h
objects.o: objects.c $(CONFIG_H) ../include/obj.h ../include/objclass.h \
		../include/prop.h ../include/skills.h ../include/color.h
objnam.o: objnam.c $(HACK_H)
options.o: options.c $(CONFIG_H) ../include/objclass.h ../include/flag.h \
		$(HACK_H) ../include/tcap.h
pager.o: pager.c $(HACK_H) ../include/dlb.h
pickup.o: pickup.c $(HACK_H)
pline.o: pline.c $(HACK_H) ../include/epri.h ../include/edog.h
polyself.o: polyself.c $(HACK_H)
potion.o: potion.c $(HACK_H)
pray.o: pray.c $(HACK_H) ../include/epri.h
priest.o: priest.c $(HACK_H) ../include/mfndpos.h ../include/eshk.h \
		../include/epri.h ../include/emin.h
quest.o: quest.c $(HACK_H) ../include/qtext.h
questpgr.o: questpgr.c $(HACK_H) ../include/dlb.h ../include/qtext.h
read.o: read.c $(HACK_H)
rect.o: rect.c $(HACK_H)
region.o: region.c $(HACK_H) ../include/lev.h
restore.o: restore.c $(HACK_H) ../include/lev.h ../include/tcap.h
rip.o: rip.c $(HACK_H)
rnd.o: rnd.c $(HACK_H)
role.o: role.c $(HACK_H)
rumors.o: rumors.c $(HACK_H) ../include/lev.h ../include/dlb.h
save.o: save.c $(HACK_H) ../include/lev.h
shk.o: shk.c $(HACK_H) ../include/eshk.h
shknam.o: shknam.c $(HACK_H) ../include/eshk.h
sit.o: sit.c $(HACK_H) ../include/artifact.h
sounds.o: sounds.c $(HACK_H) ../include/edog.h
sp_lev.o: sp_lev.c $(HACK_H) ../include/dlb.h ../include/sp_lev.h
spell.o: spell.c $(HACK_H)
steal.o: steal.c $(HACK_H)
steed.o: steed.c $(HACK_H)
teleport.o: teleport.c $(HACK_H)
timeout.o: timeout.c $(HACK_H) ../include/lev.h
topten.o: topten.c $(HACK_H) ../include/dlb.h ../include/patchlevel.h
track.o: track.c $(HACK_H)
trap.o: trap.c $(HACK_H)
u_init.o: u_init.c $(HACK_H)
uhitm.o: uhitm.c $(HACK_H)
vault.o: vault.c $(HACK_H) ../include/vault.h
version.o: version.c $(HACK_H) ../include/date.h ../include/patchlevel.h
vision.o: vision.c $(HACK_H) ../include/vis_tab.h
weapon.o: weapon.c $(HACK_H)
were.o: were.c $(HACK_H)
wield.o: wield.c $(HACK_H)
windows.o: windows.c $(HACK_H) ../include/wingem.h ../include/winGnome.h
wizard.o: wizard.c $(HACK_H) ../include/qtext.h ../include/epri.h
worm.o: worm.c $(HACK_H) ../include/lev.h
worn.o: worn.c $(HACK_H)
write.o: write.c $(HACK_H)
zap.o: zap.c $(HACK_H)
ioctl.o: ioctl.c $(HACK_H) ../include/tcap.h
unixmain.o: unixmain.c $(HACK_H) ../include/dlb.h
unixtty.o: unixtty.c $(HACK_H)
unixunix.o: unixunix.c $(HACK_H)
unixres.o: unixres.c $(CONFIG_H)
getline.o: getline.c $(HACK_H) ../include/func_tab.h
termcap.o: termcap.c $(HACK_H) ../include/tcap.h
topl.o: topl.c $(HACK_H) ../include/tcap.h
wintty.o: wintty.c $(HACK_H) ../include/dlb.h ../include/patchlevel.h \
		../include/tcap.h
version.o: version.c $(HACK_H) ../include/date.h ../include/patchlevel.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
