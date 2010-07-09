bin_SCRIPTS = voxywgw
CLEANFILES = $(bin_SCRIPTS)
EXTRA_DIST = voxywgw.orig.rb

do_subst = sed -e 's,[@]DATADIR[@],$(datadir),g'

voxywgw: voxywgw.orig.rb
	$(do_subst) < $(srcdir)/voxywgw.orig.rb > voxywgw
