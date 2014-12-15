SUBDIRS = src po

EXTRA_DIST = config.rpath mkinstalldirs m4/Makefile.in mssh.schemas mssh_clusters.conf

SCHEMAS_FILE = mssh.schemas

schemadir = $(GCONF_SCHEMA_FILE_DIR)
schema_DATA = $(SCHEMAS_FILE)

if GCONF_SCHEMAS_INSTALL
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/$(schema_DATA)
endif

man1_MANS = mssh.1

ACLOCAL_AMFLAGS = -I m4

check-gettext:
	@if test x$(USE_NLS) != "xyes" ; then echo "Missing gettext. Rerun configure and check for" \
    "'checking whether to use NLS... yes'!" ; exit 1 ; fi

update-po: check-gettext
	@find $(srcdir)/src/ -name "*.c" -print | sort > $(srcdir)/po/POTFILES.in.2 ; \
    if diff $(srcdir)/po/POTFILES.in $(srcdir)/po/POTFILES.in.2 >/dev/null 2>&1 ; then \
        rm -f $(srcdir)/po/POTFILES.in.2 ; \
    else \
        mv $(srcdir)/po/POTFILES.in.2 $(srcdir)/po/POTFILES.in ; \
    fi
	cd po && $(MAKE) $(AM_MAKEFLAGS) update-po

update-gmo: check-gettext
	cd po && $(MAKE) $(AM_MAKEFLAGS) update-gmo

force-update-gmo: check-gettext
	touch po/*.po
	cd po && $(MAKE) $(AM_MAKEFLAGS) update-gmo

#force-update-gmo-%: check-gettext
#	@language=`echo $@ | sed s/force-update-gmo-//` ; \
#    if test ! -f po/$$language.po ; then echo "file po/$$language.po does not exist" ; exit 1 ; fi ; \
#    touch po/$$language.po ; \
#    cd po && $(MAKE) $(AM_MAKEFLAGS) update-gmo

.PHONY: check-gettext update-po update-gmo force-update-gmo
