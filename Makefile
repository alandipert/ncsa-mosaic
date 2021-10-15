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
	@echo "  freebsd -- x86 running freebsd DYNAMIC"
	@echo "  freebsd-static -- x86 running freebsd ALL STATIC"
	@echo "  freebsd-static-motif -- x86 running freebsd STATIC MOTIF"
	@echo "  linux -- x86 running Linux 1.2.13 DYNAMIC"
	@echo "  linux-static -- x86 running Linux 1.2.13 ALL STATIC"
	@echo "  linux-static-motif -- x86 running Linux 1.2.13 STATIC MOTIF"
	@echo "  unicos -- Cray PVP running UNICOS (tested with 10.0 and MOTIF)"
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

dev_freebsd: rm_and_touch freebsd
freebsd: rm_and_touch
	$(MAKE) -f makefiles/Makefile.freebsd DEV_ARCH=freebsd
p_freebsd: rm_and_touch
	$(MAKE) -f makefiles/Makefile.freebsd purifyd DEV_ARCH=freebsd
q_freebsd: rm_and_touch
	$(MAKE) -f makefiles/Makefile.freebsd quantifyd DEV_ARCH=freebsd

dev_freebsd_static: rm_and_touch freebsd_static
freebsd_static: rm_and_touch
	$(MAKE) -f makefiles/Makefile.freebsd staticd DEV_ARCH=freebsd
p_freebsd_static: rm_and_touch
	$(MAKE) -f makefiles/Makefile.freebsd p_staticd DEV_ARCH=freebsd
q_freebsd_static: rm_and_touch
	$(MAKE) -f makefiles/Makefile.freebsd q_staticd DEV_ARCH=freebsd

dev_freebsd_static_motif: rm_and_touch freebsd_static_motif
freebsd_static_motif: rm_and_touch
	$(MAKE) -f makefiles/Makefile.freebsd static_motifd DEV_ARCH=freebsd
p_freebsd_static_motif: rm_and_touch
	$(MAKE) -f makefiles/Makefile.freebsd p_static_motifd DEV_ARCH=freebsd
q_freebsd_static_motif: rm_and_touch
	$(MAKE) -f makefiles/Makefile.freebsd q_static_motifd DEV_ARCH=freebsd

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

dev_unicos: rm_and_touch unicos
unicos: rm_and_touch
	$(MAKE) -f makefiles/Makefile.unicos static_motifd DEV_ARCH=unicos
p_unicos: rm_and_touch
	$(MAKE) -f makefiles/Makefile.unicos p_static_motifd DEV_ARCH=unicos
q_unicos: rm_and_touch
	$(MAKE) -f makefiles/Makefile.unicos q_static_motifd DEV_ARCH=unicos

clean:
	cd libXmx; $(MAKE) clean
	cd libdtm; $(MAKE) clean
	cd libhtmlw; $(MAKE) clean
	cd libnet; $(MAKE) clean
	cd libnut; $(MAKE) clean
	cd libwww2; $(MAKE) clean
	cd src; $(MAKE) clean MOSAIC="Mosaic"
	@echo "Done cleaning..."
