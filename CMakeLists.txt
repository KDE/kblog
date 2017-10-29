cmake_minimum_required(VERSION 3.0)
set(PIM_VERSION "5.6.3")

project(KBlog VERSION ${PIM_VERSION})

# ECM setup
set(KF5_VERSION "5.35.0")
find_package(ECM ${KF5_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(GenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMGeneratePriFile)
include(CMakePackageConfigHelpers)
include(ECMSetupVersion)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(ECMCoverageOption)


set(KBLOG_LIB_VERSION ${PIM_VERSION})
set(CALENDARCORE_LIB_VERSION "5.6.3")
set(SYNDICATION_LIB_VERSION "5.6.3")
set(XMLRPCCLIENT_LIB_VERSION "5.6.3")
set(QT_REQUIRED_VERSION "5.7.0")
ecm_setup_version(PROJECT VARIABLE_PREFIX KBLOG
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kblog_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF5BlogConfigVersion.cmake"
                        SOVERSION 5)

########### Find packages ###########
find_package(KF5CoreAddons ${KF5_VERSION} CONFIG REQUIRED)
find_package(KF5KIO ${KF5_VERSION} CONFIG REQUIRED)

find_package(KF5CalendarCore ${CALENDARCORE_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5Syndication ${SYNDICATION_LIB_VERSION} CONFIG REQUIRED)
find_package(KF5XmlRpcClient ${XMLRPCCLIENT_LIB_VERSION} CONFIG REQUIRED)

add_definitions(-DTRANSLATION_DOMAIN=\"libkblog5\")
add_definitions("-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII")
add_definitions(-DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
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

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
