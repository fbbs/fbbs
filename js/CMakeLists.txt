function(publish_js MINIFIED SCRIPTS)
	set(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${MINIFIED}.min.js)
	if(UGLIFYJS)
		set(OUTPUT ${OUTPUT} ${CMAKE_CURRENT_BINARY_DIR}/${MINIFIED}.map.js)
		add_custom_command(OUTPUT ${OUTPUT}
				COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR}
						&& cp ${SCRIPTS} ${CMAKE_CURRENT_BINARY_DIR}
				COMMAND ${UGLIFYJS} ${SCRIPTS} -c -m -o ${MINIFIED}.min.js
						--source-map ${MINIFIED}.map.js -p relative
				DEPENDS ${SCRIPTS})
	else(UGLIFYJS)
		add_custom_command(OUTPUT ${OUTPUT}
				COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && cat ${SCRIPTS} > ${OUTPUT}
				DEPENDS ${SCRIPTS})
	endif(UGLIFYJS)
	add_custom_target(${MINIFIED} ALL DEPENDS ${OUTPUT})
	install(FILES ${OUTPUT} ${SCRIPTS} DESTINATION ${DOCUMENT_ROOT}/js)
endfunction(publish_js)

set(SCRIPTS compat.js store.js app.js util.js ui.js session.js board.js
	post.js main.js user.js)
publish_js(app-201509 "${SCRIPTS}")

set(SCRIPTS compat.js store.js cookie.js session.js homepage.js)
publish_js(homepage "${SCRIPTS}")
