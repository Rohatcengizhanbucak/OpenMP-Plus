file(GLOB Common_SRC
	CONFIGURE_DEPENDS
	"*.cpp"
	"RakNet/*.cpp"
)

add_library(sampp_common STATIC ${Common_SRC})
target_link_libraries(sampp_common PUBLIC sampp_build_flags)
