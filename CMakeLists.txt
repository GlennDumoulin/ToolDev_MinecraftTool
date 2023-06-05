# minecrafttool

cmake_minimum_required(VERSION 3.22)

project(MinecraftTool VERSION 1.0.0)

add_subdirectory(CommonCodeProject)
message("CommonCode include directory: ${CommonCodeIncludeDir}")
add_subdirectory(CommandLineProject)
add_subdirectory(GUIProject)

target_include_directories(
	cmdMinecraftTool PUBLIC
	"${CommonCodeIncludeDir}"
)
target_include_directories(
	guiMinecraftTool PUBLIC
	"${CommonCodeIncludeDir}"
)

install(
	TARGETS
		cmdMinecraftTool
		guiMinecraftTool
	DESTINATION bin
)

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")

set(CPACK_NSIS_MODIFY_PATH ON)
include(CPack)