
#include "obj.h"

extern GLuint objects[10];

void loadOBJ()
{
	//objects[0] = OBJ2List("obj/asteroid.obj");
	//objects[1] = OBJ2List("obj/rock.obj");
}

GLuint OBJ2List(char * filename) {
	GLMmodel *glm_model;
	GLuint list;
	glm_model = glmReadOBJ(filename);
	glmUnitize(glm_model);
	glmScale(glm_model, 5);
	glmFacetNormals(glm_model);
	glmVertexNormals(glm_model, 90);
	list = glmList(glm_model, GLM_TEXTURE | GLM_SMOOTH | GLM_MATERIAL);

	glmDelete(glm_model);
	return list;
}