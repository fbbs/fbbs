function(publish_html DIR)
	set(DST ${CMAKE_CURRENT_BINARY_DIR}/${DIR}.htm)
	add_custom_target(${DIR}-html ALL
		COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/generator
				${CMAKE_CURRENT_SOURCE_DIR}/${DIR}
				${CMAKE_BINARY_DIR}/less
				index.htm ${DST}
		DEPENDS ${DIR}/index.htm)
	install(FILES ${DST} DESTINATION ${DOCUMENT_ROOT}/${DIR}
		RENAME index.htm)
endfunction(publish_html)

publish_html(bbs)
publish_html(home)
add_dependencies(home-html style.homepage)
