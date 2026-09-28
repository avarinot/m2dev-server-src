# Directory scope: applies to the server sources only, not to the vendored libraries.
if(ENABLE_WARNINGS AND NOT MSVC)
	# Unused parameters are everywhere in the handler/callback signatures and never point at a bug.
	add_compile_options(-Wall -Wextra -Wno-unused-parameter)
endif()

add_subdirectory(common)
add_subdirectory(libgame)
add_subdirectory(libpoly)
add_subdirectory(libsql)
add_subdirectory(libthecore)
add_subdirectory(liblua)
add_subdirectory(game)
add_subdirectory(db)
add_subdirectory(qc)

if (WIN32)
	set_target_properties(common PROPERTIES FOLDER lib)
	set_target_properties(libgame PROPERTIES FOLDER lib)
	set_target_properties(libpoly PROPERTIES FOLDER lib)
	set_target_properties(libsql PROPERTIES FOLDER lib)
	set_target_properties(libthecore PROPERTIES FOLDER lib)
	set_target_properties(liblua PROPERTIES FOLDER lib)
endif()