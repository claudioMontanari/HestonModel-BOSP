###############################################################################
# The HestonFour Application
################################################################################

# Define CMake compatibility version
cmake_minimum_required (VERSION 2.6 FATAL_ERROR)

# The name of this project.
project (HestonFour)
include (${CONFIGPATH})

################################################################################
# Installation items
################################################################################

set (HESTONFOUR_PATH_BINS    "usr/bin")
set (HESTONFOUR_PATH_CONFIG  "etc/bbque")
set (HESTONFOUR_PATH_RECIPES "etc/bbque/recipes")
set (HESTONFOUR_PATH_DOCS    "usr/share/bbque/hestonfour")

set (CMAKE_INSTALL_RPATH "${CONFIG_BOSP_RUNTIME_PATH}/lib/bbque:${CONFIG_BOSP_RUNTIME_PATH}/lib")
set (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

################################################################################
# Common initialization settings
################################################################################

# Load additional CMake modules
list (APPEND CMAKE_MODULE_PATH
	"${BOSP_CMAKE_MODULES}")
list (APPEND CMAKE_MODULE_PATH
	"${PROJECT_SOURCE_DIR}/.cmake")

# Make sure the compiler can find include files
include_directories (
	${PROJECT_BINARY_DIR}/include
	${PROJECT_SOURCE_DIR}/include
)

message ( STATUS )
message ( STATUS "Checking dependencies..." )

# Setup the source tree versioning number
include (GetGitRevisionDescription)
git_describe(GIT_SHA1)
set (VERSION "${GIT_SHA1}")

# Check if running un UNIX like systems
if (UNIX)
	add_definitions(-DUNIX)
endif(UNIX)


################################################################################
# Build version specific configurations
################################################################################
# Choose the type of build, options are:
#    None Debug Release RelWithDebInfo MinSizeRel

# Set default release version to RELEASE
if (NOT CMAKE_BUILD_TYPE)
	set (CMAKE_BUILD_TYPE RelWithDebInfo)
endif (NOT CMAKE_BUILD_TYPE)

# Add flags common to all builds
set (BOSP_CPP11 "0x")
if (GCC_TAG GREATER "46")
	set (BOSP_CPP11 "11")
endif (GCC_TAG GREATER "46")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++${BOSP_CPP11} -Wall")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGCC_VERSION=${GCC_VERSION}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# These are used to clean-up unreferenced libraries symbols
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--gc-sections")

# Add "Generic-Linux" specific flags
if (CONFIG_TARGET_LINUX_NATIVE)
set (BBQUE_TARGET_OS "Native-Linux")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif (CONFIG_TARGET_LINUX_NATIVE)

# Add "ARM-Linux" specific flags
if (CONFIG_TARGET_LINUX_ARM)
set (BBQUE_TARGET_OS "ARM-Linux")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif (CONFIG_TARGET_LINUX_ARM)

# Add "Android-Linux" specific flags
if (CONFIG_TARGET_ANDROID)
set (BBQUE_TARGET_OS "Android-Linux")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I../../../out/include -D__ARM_ARCH_5__ -D__ARM_ARCH_5T__ -D__ARM_ARCH_5E__ -D__ARM_ARCH_5TE__ -D__STDINT_LIMITS -D_GLIBCXX_USE_C99_STDINT_TR1 -D__BIONIC__ -Wno-psabi -march=armv5te -mtune=xscale -msoft-float -fexceptions -frtti -mthumb -fomit-frame-pointer -fno-strict-aliasing -finline-limit=64 -DANDROID  -Wa,--noexecstack")
# -ffunction-sections -fdata-sections -Wl,--gc-sections - REMOVED becaus of missing symbols
endif (CONFIG_TARGET_ANDROID)

# Check target bitness
set (BBQUE_TARGET_ARCH 32)
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
  set (BBQUE_TARGET_ARCH 64)
endif (CMAKE_SIZEOF_VOID_P EQUAL 8)

set (BBQUE_BUILD_FLAVOR
	"${BBQUE_TARGET_OS} (${BBQUE_TARGET_ARCH}bit - ${CMAKE_BUILD_TYPE})")

# Options for build version: DEBUG
set (CMAKE_CXX_FLAGS_DEBUG "-O0 -g -Wextra -pedantic -DBBQUE_DEBUG -DDEBUG")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set (BBQUE_DEBUG ON)
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

# Options for build version: DESKTOP RELEASE
set (CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Options for build version: EMBEDDED RELEASE
set (CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG")


################################################################################
# Installation locations
################################################################################

# set default install prefix to project root directory
if (CMAKE_INSTALL_PREFIX STREQUAL "/usr/local")
	set (CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/out" )
endif (CMAKE_INSTALL_PREFIX STREQUAL "/usr/local")


################################################################################
# Lookup for library dependencies
################################################################################

# Force usage of BOSP provided BOOST installation
set (Boost_NO_SYSTEM_PATHS ON)
set (BOOST_ROOT ${CMAKE_INSTALL_PREFIX})
set (BOOST_INCLUDEDIR ${CMAKE_INSTALL_PREFIX})

# Check for required libs and packages (headers + lib)
find_package(Boost 1.45.0 REQUIRED program_options)

# Add compilation dependencies
include_directories(
	${Boost_INCLUDE_DIRS}
)

# Add linking dependencies
link_directories(
	${Boost_LIBRARY_DIRS}
)

################################################################################
# Subdirectories analysis
################################################################################

## Add package version definition
configure_file (
	"${PROJECT_SOURCE_DIR}/src/version.cc.in"
	"${PROJECT_BINARY_DIR}/src/version.cc" @ONLY
)

# Recurse into project subfolders
add_subdirectory (src)
install(DIRECTORY "${PROJECT_SOURCE_DIR}/recipes/"
	DESTINATION "${HESTONFOUR_PATH_RECIPES}"
	FILES_MATCHING PATTERN "*.recipe")

################################################################################
# Doxygen Documentation
################################################################################
# check if doxygen is even installed
find_package(Doxygen)
if (DOXYGEN_FOUND STREQUAL "NO")
	message(FATAL_ERROR "Doxygen not found. "
		"Please get a copy http://www.doxygen.org")
endif (DOXYGEN_FOUND STREQUAL "NO")
# prepare doxygen configuration file
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
	${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
# add doxygen as target
add_custom_target (doxygen ${DOXYGEN_EXECUTABLE}
	${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
# cleanup $build/api-doc on "make clean"
set_property(DIRECTORY APPEND PROPERTY
		     ADDITIONAL_MAKE_CLEAN_FILES api-doc)
# add doxygen as dependency to doc-target
get_target_property (DOC_TARGET doc TYPE)
if (NOT DOC_TARGET)
	add_custom_target(doc)
endif ()
add_dependencies (doc doxygen)
# install HTML API documentation and manual pages
set (DOC_PATH "${RTLIB_TUTORIAL_PATH_DOCS}")
install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/api-doc/html
	DESTINATION ${HESTONFOUR_PATH_DOCS} OPTIONAL)
# install man pages into packages, scope is now project root..
install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/api-doc/man/man3
	DESTINATION ${HESTONFOUR_PATH_DOCS} OPTIONAL)


################################################################################
# Configuration resume
################################################################################

# display status message for important variables
message ( STATUS )
message ( STATUS "===================[ HestonFour - Building System Configuration ]"
                 "====================" )
message ( STATUS "Version............... ${VERSION}" )
message ( STATUS "Build Flavor.......... ${BBQUE_BUILD_FLAVOR}" )
message ( STATUS "   Target............. ${CONFIG_TARGET_NAME} ${CONFIG_TARGET_DESCRIPTION}" )
message ( STATUS "   CFLAGS............. ${CMAKE_C_FLAGS}")
message ( STATUS " CXXFLAGS............. ${CMAKE_CXX_FLAGS}")
message ( STATUS "Installation prefix... ${CMAKE_INSTALL_PREFIX}" )
message ( STATUS "   Binary............. <prefix>/${HESTONFOUR_PATH_BINS}" )
message ( STATUS "   Recipes............ <prefix>/${HESTONFOUR_PATH_RECIPES}" )
message ( STATUS "   Documentation...... <prefix>/${HESTONFOUR_PATH_DOCS}" )
message ( STATUS "Using RTLib........... ${BBQUE_RTLIB_LIBRARY}" )
message ( STATUS "Boost library......... ${Boost_LIBRARY_DIRS}" )
message ( STATUS )
message ( STATUS "Default values could be changes at command line, with:")
message ( STATUS "  cmake -D<Variable>=<Value>" )
message ( STATUS "or use the '-i' cmake flag to run in interactive mode" )
message ( STATUS "======================================================"
                 "==========================" )
message(STATUS)

