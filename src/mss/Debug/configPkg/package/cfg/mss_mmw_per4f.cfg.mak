# invoke SourceDir generated makefile for mss_mmw.per4f
mss_mmw.per4f: .libraries,mss_mmw.per4f
.libraries,mss_mmw.per4f: package/cfg/mss_mmw_per4f.xdl
	$(MAKE) -f C:\Users\a0232274\60GH_Dev\pplcount_mss_68xx/src/makefile.libs

clean::
	$(MAKE) -f C:\Users\a0232274\60GH_Dev\pplcount_mss_68xx/src/makefile.libs clean

