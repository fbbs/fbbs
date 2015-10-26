function(publish_css COMPILED LESS FILES)
	set(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${COMPILED}.css)
	add_custom_command(OUTPUT ${OUTPUT}
			COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && ${LESSC} --clean-css ${LESS} ${OUTPUT}
			DEPENDS ${LESS} ${FILES})
	add_custom_target(${COMPILED} ALL DEPENDS ${OUTPUT})
	install(FILES ${OUTPUT} ${LESS} DESTINATION ${DOCUMENT_ROOT}/css)
endfunction(publish_css)

set(FILES default.less navigation.less reset.less var.less footer.less
		board.less)
publish_css(style.default "fbbs.less" "${FILES}")
