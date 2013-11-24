set(SCRIPTS
	history/json2.js
	history/history.adapter.jquery.js
	history/history.html4.js
	history/history.js
	jquery.mustache.js)

if(UGLIFYJS)
	set(BUNDLE bundle.js)
	set(ABSOLUTE_BUNDLE ${CMAKE_CURRENT_BINARY_DIR}/${BUNDLE})

	set(FULL_PATHS)
	foreach(SCRIPT ${SCRIPTS})
		set(FULL_PATHS ${FULL_PATHS} ${CMAKE_CURRENT_SOURCE_DIR}/${SCRIPT})
	endforeach(SCRIPT)

	add_custom_command(OUTPUT ${BUNDLE}
		COMMAND cat ${FULL_PATHS} | ${UGLIFYJS} -nc > ${BUNDLE}
		DEPENDS ${SCRIPTS})
	add_custom_target(${BUNDLE} ALL DEPENDS ${ABSOLUTE_BUNDLE})
	install(FILES ${ABSOLUTE_BUNDLE} DESTINATION "${DOCUMENT_ROOT}/js")
endif(UGLIFYJS)

install(FILES ${SCRIPTS} DESTINATION "${DOCUMENT_ROOT}/js")
