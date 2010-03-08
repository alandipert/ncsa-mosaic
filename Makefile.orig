##
## Toplevel Makefile for all Makefiles
##

##
## Scott Powers
##

all: dev_$(DEV_ARCH)

list: dev_
help: dev_
dev_::
	@echo "You must specify one of the following or set the environment variable"
	@echo "[DEV_ARCH] to one of the following:"
	@echo "  alpha -- DEC Alpha AXP running OSF/1"
	@echo "  bsdi -- x86 running BSDI BSD/OS 2.1"
	@echo "  dec -- DEC 2100 running Ultrix 4.3"
	@echo "  hp -- HP 9000/735 running HP-UX A.09.01"
	@echo "  ibm -- IBM RS6000 running AIX 4.4 BSD"
	@echo "  indy -- SGI Indy running IRIX 5.3"
	@echo "  linux -- x86 running Linux 1.2.13 DYNAMIC"
	@echo "  linux-static -- x86 running Linux 1.2.13 ALL STATIC"
	@echo "  linux-static-motif -- x86 running Linux 1.2.13 STATIC MOTIF"
	@echo "  sco -- x86 running SCO System V 3.2"
	@echo "  sgi -- SGI Iris running IRIS 4.0.2"
	@echo "  solaris-23 -- SPARCstation 20 running Solaris 2.3"
	@echo "  solaris-24 -- SPARCstation 20 running Solaris 2.4"
	@echo "  solaris-24-x86 -- x86 running Solaris 2.4 for x86"
	@echo "  sun -- SPARCserver 690MP running SunOS 4.1.3"
	@echo "  sun-lresolv -- SPARCserver 690MP running SunOS 4.1.3"
	@echo " "
	@echo "To make a Purify or Quantify version of Mosaic, put a 'p_' or"
	@echo "  'q_' in front of the platform."
	@echo "If your OS is not listed, you will need to copy one of the"
	@echo "  the Makefiles.OS in the 'makefiles' directory, edit it for"
	@echo "  your system, edit this Makefile to add your system, compile,"
	@echo "  and send in your changes to: mosaic-x@ncsa.uiuc.edu."

purifyd:
	$(MAKE) p_$(DEV_ARCH)

quantifyd:
	$(MAKE) q_$(DEV_ARCH)

rm_and_touch:
	@rm -f config.h
	@touch config.h

dev_alpha: rm_and_touch alpha
alpha: rm_and_touch
	$(MAKE) -f makefiles/Makefile.alpha DEV_ARCH=alpha
p_alpha: rm_and_touch
	$(MAKE) -f makefiles/Makefile.alpha purifyd DEV_ARCH=alpha
q_alpha: rm_and_touch
	$(MAKE) -f makefiles/Makefile.alpha quantifyd DEV_ARCH=alpha

dev_bsdi: rm_and_touch bsdi
bsdi: rm_and_touch
	$(MAKE) -f makefiles/Makefile.bsdi DEV_ARCH=bsdi
p_bsdi: rm_and_touch
	$(MAKE) -f makefiles/Makefile.bsdi purifyd DEV_ARCH=bsdi
q_bsdi: rm_and_touch
	$(MAKE) -f makefiles/Makefile.bsdi quantifyd DEV_ARCH=bsdi

dev_dec: rm_and_touch dec
dec: rm_and_touch
	$(MAKE) -f makefiles/Makefile.dec DEV_ARCH=dec
p_dec: rm_and_touch
	$(MAKE) -f makefiles/Makefile.dec purifyd DEV_ARCH=dec
q_dec: rm_and_touch
	$(MAKE) -f makefiles/Makefile.dec quantifyd DEV_ARCH=dec

dev_hp: rm_and_touch hp
hp: rm_and_touch
	$(MAKE) -f makefiles/Makefile.hp DEV_ARCH=hp
p_hp: rm_and_touch
	$(MAKE) -f makefiles/Makefile.hp purifyd DEV_ARCH=hp
q_hp: rm_and_touch
	$(MAKE) -f makefiles/Makefile.hp quantifyd DEV_ARCH=hp

dev_ibm: rm_and_touch ibm
ibm: rm_and_touch
	$(MAKE) -f makefiles/Makefile.ibm DEV_ARCH=ibm
p_ibm: rm_and_touch
	$(MAKE) -f makefiles/Makefile.ibm purifyd DEV_ARCH=ibm
q_ibm: rm_and_touch
	$(MAKE) -f makefiles/Makefile.ibm quantifyd DEV_ARCH=ibm

dev_indy: rm_and_touch indy
indy: rm_and_touch
	$(MAKE) -f makefiles/Makefile.indy DEV_ARCH=indy
p_indy: rm_and_touch
	$(MAKE) -f makefiles/Makefile.indy purifyd DEV_ARCH=indy
q_indy: rm_and_touch
	$(MAKE) -f makefiles/Makefile.indy quantifyd DEV_ARCH=indy

dev_linux: rm_and_touch linux
linux: rm_and_touch
	$(MAKE) -f makefiles/Makefile.linux DEV_ARCH=linux
p_linux: rm_and_touch
	$(MAKE) -f makefiles/Makefile.linux purifyd DEV_ARCH=linux
q_linux: rm_and_touch
	$(MAKE) -f makefiles/Makefile.linux quantifyd DEV_ARCH=linux

dev_linux_static: rm_and_touch linux_static
linux_static: rm_and_touch
	$(MAKE) -f makefiles/Makefile.linux staticd DEV_ARCH=linux
p_linux_static: rm_and_touch
	$(MAKE) -f makefiles/Makefile.linux p_staticd DEV_ARCH=linux
q_linux_static: rm_and_touch
	$(MAKE) -f makefiles/Makefile.linux q_staticd DEV_ARCH=linux

dev_linux_static_motif: rm_and_touch linux_static_motif
linux_static_motif: rm_and_touch
	$(MAKE) -f makefiles/Makefile.linux static_motifd DEV_ARCH=linux
p_linux_static_motif: rm_and_touch
	$(MAKE) -f makefiles/Makefile.linux p_static_motifd DEV_ARCH=linux
q_linux_static_motif: rm_and_touch
	$(MAKE) -f makefiles/Makefile.linux q_static_motifd DEV_ARCH=linux

dev_sco: rm_and_touch sco
sco: rm_and_touch
	$(MAKE) -f makefiles/Makefile.sco DEV_ARCH=sco
p_sco: rm_and_touch
	$(MAKE) -f makefiles/Makefile.sco purifyd DEV_ARCH=sco
q_sco: rm_and_touch
	$(MAKE) -f makefiles/Makefile.sco quantifyd DEV_ARCH=sco

dev_sgi: rm_and_touch sgi
sgi: rm_and_touch
	$(MAKE) -f makefiles/Makefile.sgi DEV_ARCH=sgi
p_sgi: rm_and_touch
	$(MAKE) -f makefiles/Makefile.sgi purifyd DEV_ARCH=sgi
q_sgi: rm_and_touch
	$(MAKE) -f makefiles/Makefile.sgi quantifyd DEV_ARCH=sgi

dev_solaris-23: rm_and_touch solaris-23
solaris-23: rm_and_touch
	$(MAKE) -f makefiles/Makefile.solaris-23 DEV_ARCH=solaris-23
p_solaris-23: rm_and_touch
	$(MAKE) -f makefiles/Makefile.solaris-23 purifyd DEV_ARCH=solaris-23
q_solaris-23: rm_and_touch
	$(MAKE) -f makefiles/Makefile.solaris-23 quantifyd DEV_ARCH=solaris-23

dev_solaris-24: rm_and_touch solaris-24
solaris-24: rm_and_touch
	$(MAKE) -f makefiles/Makefile.solaris-24 DEV_ARCH=solaris-24
p_solaris-24: rm_and_touch
	$(MAKE) -f makefiles/Makefile.solaris-24 purifyd DEV_ARCH=solaris-24
q_solaris-24: rm_and_touch
	$(MAKE) -f makefiles/Makefile.solaris-24 quantifyd DEV_ARCH=solaris-24

dev_solaris-24-x86: rm_and_touch solaris-24-x86
solaris-24-x86: rm_and_touch
	$(MAKE) -f makefiles/Makefile.solaris-24-x86 DEV_ARCH=solaris-24-x86
p_solaris-24-x86: rm_and_touch
	$(MAKE) -f makefiles/Makefile.solaris-24-x86 purifyd DEV_ARCH=solaris-24-x86
q_solaris-24-x86: rm_and_touch
	$(MAKE) -f makefiles/Makefile.solaris-24-x86 quantifyd DEV_ARCH=solaris-24-x86

dev_sun: rm_and_touch sun
sun: rm_and_touch
	$(MAKE) -f makefiles/Makefile.sun DEV_ARCH=sun
p_sun: rm_and_touch
	$(MAKE) -f makefiles/Makefile.sun purifyd DEV_ARCH=sun
q_sun: rm_and_touch
	$(MAKE) -f makefiles/Makefile.sun quantifyd DEV_ARCH=sun

dev_sun-lresolv: rm_and_touch sun-lresolv
sun-lresolv: rm_and_touch
	$(MAKE) -f makefiles/Makefile.sun-lresolv DEV_ARCH=sun-lresolv
p_sun-lresolv: rm_and_touch
	$(MAKE) -f makefiles/Makefile.sun-lresolv purifyd DEV_ARCH=sun-lresolv
q_sun-lresolv: rm_and_touch
	$(MAKE) -f makefiles/Makefile.sun-lresolv quantifyd DEV_ARCH=sun-lresolv

clean:
	cd libXmx; $(MAKE) clean
	cd libdtm; $(MAKE) clean
	cd libhtmlw; $(MAKE) clean
	cd libnet; $(MAKE) clean
	cd libnut; $(MAKE) clean
	cd libwww2; $(MAKE) clean
	cd src; $(MAKE) clean MOSAIC="Mosaic"
	@echo "Done cleaning..."
