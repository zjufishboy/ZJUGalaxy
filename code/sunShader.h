#pragma once
#include "common.h"


class SunShader : public CGLShader {
private:
	GLuint m_resolution, m_tex;
	GLfloat m_time;

public:
	SunShader() : CGLShader(nullptr, "shader/sun.fg") {
		m_resolution = glGetUniformLocationARB(programObject, "iResolution");
		m_time = glGetUniformLocationARB(programObject, "iTime");
		m_tex = glGetUniformLocationARB(programObject, "texSun");
	}
	void update(vec3 res, float time, GLuint tex) {
		glUniform3fARB(m_resolution, res.x, res.y, res.z);
		glUniform1fARB(m_time, time);

		/*glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);*/
		glUniform1fARB(m_tex, 0);
	}

};