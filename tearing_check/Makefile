ARCHPOSTFIX=`uname -m |sed -e 's/x86_64/64/' -e 's/x86//' -e 's/i686//'`

#Compile xscreenshot

#TODO : the '.so' extension is required for XScreenshot build job to perform a successful cross compile on build server
#This is ugly, the build job must be fixed and '.so' extension removed
gnu_c: tiffloader
	gcc tearing.c tiffloader/* -lX11 -o tearing${ARCHPOSTFIX}.so

#copy tiffloader sources from spcified input directory
tiffloader:

ifdef TIFF_LOADER_SRC_FOLDER
	sh copy_tiff_src.sh ${TIFF_LOADER_SRC_FOLDER}
endif

#Running tiffloader rule every time
.PHONY: tiffloader
