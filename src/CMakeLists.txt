
#----- Check for the required RTLib library
find_package(BbqRTLib REQUIRED)

#----- Add compilation dependencies
include_directories(${BBQUE_RTLIB_INCLUDE_DIR})

#----- Add "hestonfour" target application
set(HESTONFOUR_SRC version HestonWorker HestonFour_exc HestonFour_main)
add_executable(hestonfour ${HESTONFOUR_SRC})

#----- Linking dependencies
target_link_libraries(
	hestonfour
	${Boost_LIBRARIES}
	${BBQUE_RTLIB_LIBRARY}
)

# Use link path ad RPATH
set_property(TARGET hestonfour PROPERTY
	INSTALL_RPATH_USE_LINK_PATH TRUE)

#----- Install the HestonFour files
install (TARGETS hestonfour RUNTIME
	DESTINATION ${HESTONFOUR_PATH_BINS})

#----- Generate and Install HestonFour configuration file
configure_file (
	"${PROJECT_SOURCE_DIR}/HestonFour.conf.in"
	"${PROJECT_BINARY_DIR}/HestonFour.conf"
)
install (FILES "${PROJECT_BINARY_DIR}/HestonFour.conf"
	DESTINATION ${HESTONFOUR_PATH_CONFIG})
