add_library(extension SHARED 
	"extension.cpp"
	
	)

if (CMAKE_VERSION VERSION_GREATER 3.12)
	set_property(TARGET extension PROPERTY CXX_STANDARD 20)
endif()

set_target_properties(extension PROPERTIES
	OUTPUT_NAME "miniaudio"
	SUFFIX ".hdll")

if (WIN32)
	target_include_directories(extension PRIVATE "$ENV{HASHLINK}/include")
	target_link_libraries(extension PRIVATE "$ENV{HASHLINK}/libhl.lib")
endif()

add_subdirectory("lib/miniaudio")
target_include_directories(extension PRIVATE miniaudio miniaudio_common_options)
target_link_libraries(extension PRIVATE miniaudio miniaudio_common_options)