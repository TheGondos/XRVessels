file(GLOB FRAMEWORK_FILES ${CMAKE_CURRENT_SOURCE_DIR}/XRVessels/framework/framework/*.cpp)
file(GLOB XR1_LIB_FILES ${CMAKE_CURRENT_SOURCE_DIR}/XRVessels/DeltaGliderXR1/XR1Lib/*.cpp)
file(GLOB XR1_FILES ${CMAKE_CURRENT_SOURCE_DIR}/XRVessels/DeltaGliderXR1/DeltaGliderXR1/*.cpp)
file(GLOB XR2_FILES ${CMAKE_CURRENT_SOURCE_DIR}/XRVessels/XR2Ravenstar/XR2Ravenstar/*.cpp)
file(GLOB XR5_FILES ${CMAKE_CURRENT_SOURCE_DIR}/XRVessels/XR5Vanguard/XR5Vanguard/*.cpp)
file(GLOB XR1_BITMAPS ${CMAKE_CURRENT_SOURCE_DIR}/XRVessels/DeltaGliderXR1/DeltaGliderXR1/Bitmaps/*.bmp)
file(GLOB XR2_BITMAPS ${CMAKE_CURRENT_SOURCE_DIR}/XRVessels/XR2Ravenstar/XR2Ravenstar/Bitmaps/*.bmp)
file(GLOB XR5_BITMAPS ${CMAKE_CURRENT_SOURCE_DIR}/XRVessels/XR5Vanguard/XR5Vanguard/Bitmaps/*.bmp)
file(GLOB XR_CONFIGS ${CMAKE_CURRENT_SOURCE_DIR}/Orbiter/Config/Vessels/*)

add_custom_target(XRBitmaps ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${ORBITER_BINARY_BITMAP_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${ORBITER_BINARY_BITMAP_DIR}/DeltaGliderXR1
    COMMAND ${CMAKE_COMMAND} -E make_directory ${ORBITER_BINARY_BITMAP_DIR}/XR2Ravenstar
    COMMAND ${CMAKE_COMMAND} -E make_directory ${ORBITER_BINARY_BITMAP_DIR}/XR5Vanguard
    COMMAND ${CMAKE_COMMAND} -E copy ${XR1_BITMAPS} ${ORBITER_BINARY_BITMAP_DIR}/DeltaGliderXR1/
    COMMAND ${CMAKE_COMMAND} -E copy ${XR2_BITMAPS} ${ORBITER_BINARY_BITMAP_DIR}/XR2Ravenstar/
    COMMAND ${CMAKE_COMMAND} -E copy ${XR5_BITMAPS} ${ORBITER_BINARY_BITMAP_DIR}/XR5Vanguard/
)

install(FILES ${XR_CONFIGS}
    DESTINATION ${ORBITER_INSTALL_ROOT_DIR}/Config/Vessels
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/XRVessels/XR2Ravenstar/XR2RavenstarPrefs.cfg
    DESTINATION ${ORBITER_INSTALL_ROOT_DIR}/Config/
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/XRVessels/DeltaGliderXR1/DeltaGliderXR1Prefs.cfg
    DESTINATION ${ORBITER_INSTALL_ROOT_DIR}/Config/
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/XRVessels/XR5Vanguard/XR5VanguardPrefs.cfg
    DESTINATION ${ORBITER_INSTALL_ROOT_DIR}/Config/
)

install(DIRECTORY ${ORBITER_BINARY_BITMAP_DIR}/DeltaGliderXR1
	DESTINATION ${ORBITER_INSTALL_ROOT_DIR}/Bitmaps
)

install(DIRECTORY ${ORBITER_BINARY_BITMAP_DIR}/XR2Ravenstar
	DESTINATION ${ORBITER_INSTALL_ROOT_DIR}/Bitmaps
)

install(DIRECTORY ${ORBITER_BINARY_BITMAP_DIR}/XR5Vanguard
	DESTINATION ${ORBITER_INSTALL_ROOT_DIR}/Bitmaps
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Orbiter/Meshes
	DESTINATION ${ORBITER_INSTALL_ROOT_DIR}
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Orbiter/Scenarios
	DESTINATION ${ORBITER_INSTALL_ROOT_DIR}
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Orbiter/Textures
	DESTINATION ${ORBITER_INSTALL_ROOT_DIR}
)

add_library(DeltaGliderXR1 SHARED
    ${FRAMEWORK_FILES}
    ${XR1_LIB_FILES}
    ${XR1_FILES}
)

add_library(XR2Ravenstar SHARED
    ${FRAMEWORK_FILES}
    ${XR1_LIB_FILES}
    ${XR2_FILES}
)

add_library(XR5Vanguard SHARED
    ${FRAMEWORK_FILES}
    ${XR1_LIB_FILES}
    ${XR5_FILES}
)

add_dependencies(DeltaGliderXR1
	${OrbiterTgt}
	Orbitersdk
)

add_dependencies(XR2Ravenstar
	${OrbiterTgt}
	Orbitersdk
)

add_dependencies(XR5Vanguard
	${OrbiterTgt}
	Orbitersdk
)

target_link_libraries(DeltaGliderXR1
    imgui
    XRSound_dll
    imgui-knobs
)

target_link_libraries(XR2Ravenstar
    imgui
    XRSound_dll
)

target_link_libraries(XR5Vanguard
    imgui
    XRSound_dll
)

set_target_properties(DeltaGliderXR1
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${ORBITER_BINARY_PLUGIN_DIR}
	OUTPUT_NAME DeltaGliderXR1
)

set_target_properties(XR2Ravenstar
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${ORBITER_BINARY_PLUGIN_DIR}
	OUTPUT_NAME XR2Ravenstar
)

set_target_properties(XR5Vanguard
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${ORBITER_BINARY_PLUGIN_DIR}
	OUTPUT_NAME XR5Vanguard
)

target_include_directories(DeltaGliderXR1 PUBLIC
	${ORBITER_SOURCE_SDK_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/XRVessels/framework/framework
    ${ORBITER_BINARY_SDK_DIR}/XRSound
    ${CMAKE_CURRENT_SOURCE_DIR}/XRVessels/DeltaGliderXR1/DeltaGliderXR1
    ${CMAKE_CURRENT_SOURCE_DIR}/XRVessels/DeltaGliderXR1/XR1Lib
	PUBLIC ${ORBITER_SOURCE_MODULE_DIR}/ScnEditor      # for ScnEditorAPI.h
)

target_include_directories(XR2Ravenstar PUBLIC
	${ORBITER_SOURCE_SDK_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/XRVessels/framework/framework
    ${ORBITER_BINARY_SDK_DIR}/XRSound
    ${CMAKE_CURRENT_SOURCE_DIR}/XRVessels/XR2Ravenstar/XR2Ravenstar
    ${CMAKE_CURRENT_SOURCE_DIR}/XRVessels/DeltaGliderXR1/XR1Lib
	PUBLIC ${ORBITER_SOURCE_MODULE_DIR}/ScnEditor      # for ScnEditorAPI.h
)

target_include_directories(XR5Vanguard PUBLIC
	${ORBITER_SOURCE_SDK_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/XRVessels/framework/framework
    ${ORBITER_BINARY_SDK_DIR}/XRSound
    ${CMAKE_CURRENT_SOURCE_DIR}/XRVessels/XR5Vanguard/XR5Vanguard
    ${CMAKE_CURRENT_SOURCE_DIR}/XRVessels/DeltaGliderXR1/XR1Lib
	PUBLIC ${ORBITER_SOURCE_MODULE_DIR}/ScnEditor      # for ScnEditorAPI.h
)

install(TARGETS DeltaGliderXR1
#	RUNTIME
	DESTINATION ${ORBITER_INSTALL_MODULE_DIR}
)

install(TARGETS XR2Ravenstar
#	RUNTIME
	DESTINATION ${ORBITER_INSTALL_MODULE_DIR}
)

install(TARGETS XR5Vanguard
#	RUNTIME
	DESTINATION ${ORBITER_INSTALL_MODULE_DIR}
)

