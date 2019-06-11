cmake_minimum_required (VERSION 3.8)

# ����Դ�ļ�
add_executable (ZJUGalaxy "main.cpp")

# Ҫ���Ƶ�ִ��Ŀ¼���ļ�
set(copy_files
	glut/glut32.dll
)

# Ҫ���Ƶ�ִ��Ŀ¼��Ŀ¼
set(copy_diretories
	texture
)


find_package(OpenGL REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/glut/)

target_link_libraries(ZJUGalaxy ${OPENGL_LIBRARIES} ${CMAKE_SOURCE_DIR}/glut/glut32.lib)

foreach(file ${copy_files})
	add_custom_command(
        TARGET ZJUGalaxy POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/${file}
                ${CMAKE_CURRENT_BINARY_DIR})
endforeach()

foreach(diretory ${copy_diretories})
	add_custom_command(
			TARGET ZJUGalaxy POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_directory
					${CMAKE_SOURCE_DIR}/${diretory}
					${CMAKE_CURRENT_BINARY_DIR}/${diretory})
endforeach()