cmake_minimum_required (VERSION 3.8)

set(src1
	"snap.h"
	"snap.cpp"
	"obj.h"
	"obj.cpp"
	"glm.cpp"
	"glm.h"
	"main.cpp"
	"cgl_shader.cpp"
	"cgl_shader.h"
	"common.cpp"
	"common.h"
)
set(src2
	"blackhole.cpp"
	"blackhole.h"
	"bloom.cpp"
	"bloom.h"
	"cgl_shader.cpp"
	"cgl_shader.h"
	"common.cpp"
	"common.h"
	"draw_texture.cpp"
	"draw_texture.h"
	"input.cpp"
	"input.h"
	"sphere_tree.cpp"
	"sphere_tree.h"
)


# 添加源文件
add_executable (ZJUGalaxy "main.cpp" ${src1})
add_executable (BlackholeTest "main_bh_test.cpp" ${src2})

set(targets
	ZJUGalaxy
	BlackholeTest
)

# 要复制到执行目录的文件
set(copy_files
	gl/glut32.dll
	gl/glew32.dll
)

# 要复制到执行目录的目录
set(copy_diretories
	texture
	shader
)


find_package(OpenGL REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/gl/)

foreach(tg ${targets})

	target_link_libraries(${tg} ${OPENGL_LIBRARIES} ${CMAKE_SOURCE_DIR}/gl/glut32.lib ${CMAKE_SOURCE_DIR}/gl/glew32.lib)

	foreach(file ${copy_files})
		add_custom_command(
			TARGET ${tg} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
					${CMAKE_SOURCE_DIR}/${file}
					${CMAKE_CURRENT_BINARY_DIR})
	endforeach()

	foreach(diretory ${copy_diretories})
		add_custom_command(
				TARGET ${tg} POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_directory
						${CMAKE_SOURCE_DIR}/${diretory}
						${CMAKE_CURRENT_BINARY_DIR}/${diretory})
	endforeach()

endforeach()