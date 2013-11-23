set(GENERATED_HEADERS post.h)
set(OUTPUT_HEADERS)
foreach(name ${GENERATED_HEADERS})
	set(OUTPUT_HEADER ${CMAKE_CURRENT_BINARY_DIR}/s11n/frontend_${name}
		${CMAKE_CURRENT_BINARY_DIR}/s11n/backend_${name})
	add_custom_command(
		OUTPUT ${OUTPUT_HEADER}
		COMMAND ${CMAKE_SOURCE_DIR}/util/serialization_codegen
			${CMAKE_CURRENT_BINARY_DIR}/s11n
			${CMAKE_CURRENT_SOURCE_DIR}/fbbs/${name}
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/fbbs/${name}
			${CMAKE_SOURCE_DIR}/util/serialization_codegen
	)
	set(OUTPUT_HEADERS ${OUTPUT_HEADERS} ${OUTPUT_HEADER})
endforeach(name)

add_custom_target(s11n DEPENDS ${OUTPUT_HEADERS})
