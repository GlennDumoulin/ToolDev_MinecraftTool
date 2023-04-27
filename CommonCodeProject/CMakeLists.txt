# Common Code Project subdirectory
# library --> not an executable
# header only library

add_library(CommonCode INTERFACE)
set(
	CommonCodeIncludeDir
	"${CMAKE_CURRENT_SOURCE_DIR}"
	PARENT_SCOPE
)