file(GLOB OMPPlusComponent_SRC
	CONFIGURE_DEPENDS
	"*.cpp"
)

add_library(omp_plus_component SHARED ${OMPPlusComponent_SRC})

target_link_libraries(omp_plus_component PRIVATE
	OMP-SDK
	OMP-Network
)

target_include_directories(omp_plus_component PRIVATE
	"${PROJECT_SOURCE_DIR}/Include"
	"${PROJECT_SOURCE_DIR}/Include/amx"
)

target_compile_definitions(omp_plus_component PRIVATE
	HAVE_STDINT_H=1
	PAWN_CELL_SIZE=32
)

target_compile_features(omp_plus_component PRIVATE cxx_std_17)

set_target_properties(omp_plus_component PROPERTIES
	OUTPUT_NAME "omp-plus"
	PREFIX ""
)
