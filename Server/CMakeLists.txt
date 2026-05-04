file(GLOB Server_SRC
	CONFIGURE_DEPENDS
	"*.cpp"
	"SDK/*.c"
	"SDK/*.cpp"
)

if(WIN32)
	list(APPEND Server_SRC "plugin.def")
endif()

add_library(sampp_server SHARED ${Server_SRC})

target_link_libraries(sampp_server PRIVATE sampp_common)

if(WIN32)
	target_link_libraries(sampp_server PRIVATE ws2_32)
endif()

set_target_properties(sampp_server PROPERTIES
	OUTPUT_NAME "sampp_server"
	PREFIX ""
)
