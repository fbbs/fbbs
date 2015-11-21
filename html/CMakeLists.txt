function(publish_html HTML)
	set(DST ${CMAKE_CURRENT_BINARY_DIR}/${HTML}.htm)
	add_custom_target(${HTML}.htm ALL
		COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/generator
				${CMAKE_CURRENT_SOURCE_DIR}
				${CMAKE_BINARY_DIR}/less
				${HTML}.htm ${DST}
		DEPENDS ${HTML}.htm)
	install(FILES ${DST} DESTINATION ${DOCUMENT_ROOT}/${HTML}
		RENAME index.htm)
endfunction(publish_html)

publish_html(forum)
publish_html(index)
add_dependencies(index.htm style.homepage)
