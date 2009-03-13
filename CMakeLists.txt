cmake_minimum_required(VERSION 2.6)

########### Look for glib-2.0 ###############

include( FindPkgConfig )
pkg_check_modules( GLIB2 REQUIRED glib-2.0 )
include_directories( ${GLIB2_INCLUDE_DIRS} )
link_directories( ${GLIB2_LIBRARY_DIRS} )

########### Look for check for unit tests ###############

option( WITH_CHECK   "Enable unit testing with check" OFF )
if ( WITH_CHECK )
  pkg_check_modules( CHECK check )
  include_directories( ${CHECK_INCLUDE_DIRS} )
  link_directories( ${CHECK_LIBRARY_DIRS} )
endif ( WITH_CHECK )

########### Look for Doxygen ###############

option( WITH_DOXYGEN "Build API docs using Doxygen" OFF )
if ( WITH_DOXYGEN )
  find_program( DOXYGEN doxygen )
endif ( WITH_DOXYGEN )

include( FIndDoxygen )
if (DOXYGEN)
  add_subdirectory(doxygen)
endif (DOXYGEN)

########### Look for BLAS ###############

option( WITH_BLAS   "Enable BLAS libraries" OFF )
if (WITH_BLAS)
  enable_language( Fortran )
  include( FindBLAS )
endif (WITH_BLAS)

########### Look for openMP ###############

find_file( HAVE_OMP_H omp.h )
if (HAVE_OMP_H)
  add_definitions( -DHAVE_OMP_H )
  message(STATUS "omp.h found: ${HAVE_OMP_H}")
else (HAVE_OMP_H)
  message("omp.h not found. Not using openMP.")
endif (HAVE_OMP_H)

########### Path to libintl  ###############
find_library(LIBINTL intl
             HINTS ${CMAKE_LIBINTL_DIR})
if (NOT LIBINTL)
  message(FATAL_ERROR "libintl not found. Try setting DCMAKE_LIBINTL_DIR.")
endif (NOT LIBINTL)
link_directories( ${CMAKE_LIBINTL_DIR} )
message(STATUS "libintl found: ${LIBINTL}")

add_subdirectory(deb)
add_subdirectory(rpm)
add_subdirectory(ew)

set( CPACK_PACKAGE_NAME sedflux )
set( CPACK_PACKAGE_INSTALL_DIRECTORY /usr/local/csdms/sedflux )
set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "sedflux is a basin-filling model" )
set( CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/License.txt )
set( CPACK_PACKAGE_VERSION_MAJOR "2" )
set( CPACK_PACKAGE_VERSION_MINOR "0" )
set( CPACK_PACKAGE_VERSION_PATCH "81" )

include( CPack )

########### install files ###############


########### Configuration Information ###############

if ( CMAKE_BUILD_TYPE MATCHES Release )
  set( cflags ${CMAKE_C_FLAGS_RELEASE} )
elseif ( CMAKE_BUILD_TYPE MATCHES Debug )
  set( cflags ${CMAKE_C_FLAGS_DEBUG} )
else ( )
  set( cflags ${CMAKE_C_FLAGS} )
endif ( CMAKE_BUILD_TYPE MATCHES Release )

message("------------------------------------------------------------------------")
message("Configuration:")
message("")
message("  Source code location:       ${CMAKE_SOURCE_DIR}")
message("  Build type:                 ${CMAKE_BUILD_TYPE}")
message("  Compiler:                   ${CMAKE_C_COMPILER}")
message("  Compiler flags:             ${cflags}")
message("  Host System Type:           ${CMAKE_HOST_SYSTEM}")
message("  Installation architecture:  ${CMAKE_SYSTEM}")
message("  Install path:               ${CMAKE_INSTALL_PREFIX}")
message("  With gtk+-2.0:              ${GTK_VERSION}")
message("  With check:                 ${CHECK_VERSION}")
message("  With doxygen:               ${DOXYGEN}")
message("  With flex:                  ${FLEX_VERSION}")
message("  HTML path:                  ${HTML_DIR}")
message("  BLAS path:                  ${with_blas_dir}")
message("  BLAS LIBS:                  ${BLAS_LIBS}")
message("  BLAS CFLAGS:                ${BLAS_CFLAGS}")
message("")
message("  See config.h for further configuration information.")
message("------------------------------------------------------------------------")



#original Makefile.am contents follow:

#ACLOCAL_AMFLAGS      = -I m4
#
#EXTRA_DIST           = bootstrap
#
#SUBDIRS              = deb  \
#                       rpm  \
#                       docs \
#                       ew
#
#MAINTAINERCLEANFILES = Makefile.in aclocal.m4 configure config-h.in \
#                       stamp-h.in
##all: package
#
##package: $(SUBDIRS)
#
##libutils: force_rebuild
##	@cd libutils; make
#
##libsed: libutils
##	@cd libsed/src && make datadir.h && cd .. &&  make
#
#utils: force_rebuild
#	@cd utils; make
#
#sed: utils
#	@cd sed && make datadir.h &&  make
#
#avulsion: utils sed
#	@cd avulsion; make 
#
#bing: utils sed
#	@cd bing; make
#
#compact: utils sed
#	@cd compact; make
#
#diffusion: utils sed
#	@cd diffusion; make
#
#failure: utils sed
#	@cd failure; make
#
#flow: utils sed
#	@cd flow; make
#
#hydrotrend: utils sed
#	@cd hydrotrend; make
#
#inflow: utils sed
#	@cd inflow; make
#
#muds: utils sed
#	@cd muds; make
#
#plume: utils sed
#	@cd plume; make
#
#quake: utils sed
#	@cd quake; make
#
#sakura: utils sed
#	@cd sakura; make
#
#sedflux: utils sed
#	@cd sedflux; make
#
#sedutils: utils sed
#	@cd sedutils; make
#
#squall: utils sed
#	@cd squall; make
#
#subside: utils sed
#	@cd subside; make
#
#xshore:  utils sed
#	@cd xshore; make
#
#message: force_rebuild
#	@echo "---------------------------------------"
#	@echo "--- Making package $@"
#	@echo "---------------------------------------"
#
#docs: force_rebuild
#	@cd docs; make install
#
#dist-deb: force_rebuild
#	cd deb \
#	&& make deb
#
#dist-rpm: force_rebuild
#	cd rpm \
#	&& make rpm
#
#force_rebuild:
#
##dist-hook:
##	cd docs && make all
##	mkdir -p $(distdir)/docs && \
##	cp -r $(top_builddir)/docs/html $(distdir)/docs && \
##	cp -r $(top_builddir)/docs/latex $(distdir)/docs
#
#dist-bin: distdir
#	mkdir -p $(distdir)/_inst
#	mkdir -p $(distdir)/_build
#	db_install_base=`$(am__cd) $(distdir)/_inst && pwd` \
#	   && cd $(distdir)/_build \
#	   && ../configure --srcdir=.. --prefix="$$db_install_base" --disable-shared --enable-static $(DISTCHECK_CONFIGURE_FLAGS) \
#	   && $(MAKE) $(AM_MAKEFLAGS) \
#	   && $(MAKE) $(AM_MAKEFLAGS) install \
#	   && cd .. \
#	   && mv _inst ${PACKAGE}-${VERSION} \
#	   && tar cvfz ../$(top_builddir)/${PACKAGE}_${VERSION}_@DPKG_ARCHITECTURE@.tar.gz ${PACKAGE}-${VERSION} \
#	   && rm -r _build ${PACKAGE}-${VERSION}
#	$(am__remove_distdir)
#
#nightly: distcheck
#	/Users/huttone/Desktop/googlecode_upload.py --summary="Nightly build (`date +%d/%m/%C%y`)" -P sedflux -u mcflugen -p f5e8p5y3 $(distdir).tar.gz
#