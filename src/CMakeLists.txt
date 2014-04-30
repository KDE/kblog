remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

set(kblog_SRCS
   blog.cpp
   blogpost.cpp
   blogcomment.cpp
   blogmedia.cpp
   blogger1.cpp
   gdata.cpp
   # livejournal.cpp
   metaweblog.cpp
   movabletype.cpp
   wordpressbuggy.cpp
   )

add_library(KF5Blog ${kblog_SRCS})

generate_export_header(KF5Blog BASE_NAME kblog)

add_library(KF5::Blog ALIAS KF5Blog)

target_include_directories(KF5Blog INTERFACE "$<INSTALL_INTERFACE:${KF5_INCLUDE_INSTALL_DIR}/KBlog>")
target_include_directories(KF5Blog PUBLIC "$<BUILD_INTERFACE:${KBlog_SOURCE_DIR}/src;${KBlog_BINARY_DIR}/src>")

target_link_libraries(KF5Blog
  KF5::CalendarCore
  KF5::CoreAddons
  KF5::KDELibs4Support
  KF5::KIOCore
  KF5::Syndication
  KF5::XmlRpcClient
)

set_target_properties(KF5Blog PROPERTIES
    VERSION ${KBLOG_VERSION_STRING}
    SOVERSION ${KBLOG_SOVERSION}
    EXPORT_NAME Blog
)

install(TARGETS KF5Blog EXPORT KF5BlogTargets ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

########### Generate Headers ###############
ecm_generate_headers(KBlog_CamelCase_HEADERS
  HEADER_NAMES
  Blog
  BlogComment
  Blogger1
  BlogMedia
  BlogPost
  GData
  MetaWeblog
  MovableType
  WordpressBuggy

  REQUIRED_HEADERS KBlog_HEADERS
)

install(FILES
    ${KBlog_BINARY_DIR}/src/kblog_export.h
    ${KBlog_HEADERS}
    ${KBlog_CamelCase_HEADERS}
    DESTINATION ${KF5_INCLUDE_INSTALL_DIR}/KBlog
    COMPONENT Devel
)

ecm_generate_pri_file(BASE_NAME KBlog LIB_NAME KF5Blog DEPS "CoreAddons KDELibs4Support KIOCore Syndication" FILENAME_VAR PRI_FILENAME)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})