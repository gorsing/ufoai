# - try to find GTKSourceView3 module required for compiling UFORadiant
#  GTKSourceView3_INCLUDE_DIR   - Directories to include to use GTKSourceView3
#  GTKSourceView3_LIBRARY       - Files to link against to use GTKSourceView3
#  GTKSourceView3_FOUND         - GTKSourceView3 was found

find_path(GTKSourceView3_INCLUDE_DIR NAMES gtksourceview/gtksourceview.h
   PATH_SUFFIXES gtksourceview-3.0
   PATHS
   /usr/openwin/share/include
   /usr/lib/glib/include
   /opt/gnome/include
)

find_library(GTKSourceView3_LIBRARY
   NAMES  gtksourceview-3.0
   PATHS
   /lib
   /usr/lib
   /usr/openwin/lib
   /opt/gnome/lib
)

if (GTKSourceView3_INCLUDE_DIR AND
    GTKSourceView3_LIBRARY
   )
   set(GTKSourceView3_FOUND "YES")
   message(STATUS "GTKSourceView3 found: " ${GTKSourceView3_LIBRARY})
else()
   message(FATAL_ERROR "Couldn't find GTKSourceView3 !!")
endif()
