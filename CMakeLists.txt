cmake_minimum_required(VERSION 3.5)
set(PIM_VERSION "5.14.40")

project(KBlog VERSION ${PIM_VERSION})

# ECM setup
set(KF5_MIN_VERSION "5.70.0")

find_package(ECM ${KF5_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMGeneratePriFile)

include(ECMSetupVersion)
include(FeatureSummary)
include(ECMQtDeclareLoggingCategory)



set(KBLOG_LIB_VERSION ${PIM_VERSION})
set(SYNDICATION_LIB_VERSION "5.14.40")
set(XMLRPCCLIENT_LIB_VERSION "5.14.40")
set(QT_REQUIRED_VERSION "5.13.0")
ecm_setup_version(PROJECT VARIABLE_PREFIX KBLOG
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kblog_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5BlogConfigVersion.cmake"
                        SOVERSION 5)

########### Find packages ###########
find_package(KF5CoreAddons ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5KIO ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5I18n ${KF5_MIN_VERSION} CONFIG REQUIRED)

find_package(KF5CalendarCore ${KF5_MIN_VERSION} CONFIG REQUIRED)
find_package(KF5Syndication ${SYNDICATION_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5XmlRpcClient ${XMLRPCCLIENT_LIB_VERSION} CONFIG REQUIRED)

add_definitions(-DTRANSLATION_DOMAIN=\"libkblog5\")
add_definitions(-DQT_NO_FOREACH)
add_definitions(-DQT_NO_KEYWORDS)
if (EXISTS "${CMAKE_SOURCE_DIR}/.git")
   add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050f00)
   add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x054700)
endif()

########### Targets ###########
add_subdirectory(src)

if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()

########### CMake Config Files ###########
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF5Blog")

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF5BlogConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5BlogConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF5BlogConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF5BlogConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF5BlogTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF5BlogTargets.cmake NAMESPACE KF5::)

install(FILES
   ${CMAKE_CURRENT_BINARY_DIR}/kblog_version.h
   DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}
   COMPONENT Devel
)
install(FILES kblog.renamecategories kblog.categories  DESTINATION  ${KDE_INSTALL_LOGGINGCATEGORIESDIR})

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
