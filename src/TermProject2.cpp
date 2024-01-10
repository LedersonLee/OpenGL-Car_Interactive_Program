#include <stdio.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include "LoadShaders.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <time.h>
#include "Primi.h"
#include <vector>
#define _USE_MATH_DEFINES
#include <math.h>

GLuint program;
int idx_selected = 0;
int projection_mode = 0;
std::vector<Model*> models;
glm::mat4 R;
int button_pressed[3] = { GLUT_UP,GLUT_UP ,GLUT_UP };
int mouse_pos[2] = { 0,0 };
bool specialKeyON[4] = { false, }; //F1, F2, right, left 


ModelState model_state;

glm::mat4 parallel (double r, double aspect, double n, double f) {
	double l = -r;
	double width = 2 * r;
	double height = width / aspect;
	double t = height / 2;
	double b = -t;
	return glm::ortho(l, r, b, t, n, f);
}

struct Camera {
	enum { ORTHOGRAPHIC, PERSPECTIVE };
	glm::vec3 eye;
	glm::vec3 center;
	glm::vec3 up;
	int projection_mode;
	float z_near;
	float z_far;
	float fovy;
	float x_right;

	Camera() :
		eye(0, 0, 8),
		center(0, 0, 0),
		up(0, 1, 0),
		projection_mode(PERSPECTIVE),
		z_near(0.01f),
		z_far(100.0f),
		fovy((float)(M_PI / 180.0 * (30.0))),
		x_right(1.2f)
	{}

	glm::mat4 get_viewing() { return glm::lookAt(eye, center, up); }
	glm::mat4 get_projection(float aspect) {
		glm::mat4 P;

		switch(projection_mode){
		case ORTHOGRAPHIC:
			P = parallel(x_right, aspect, z_near, z_far);
			break;
		case PERSPECTIVE:
			P = glm::perspective(fovy, aspect, z_near, z_far);
			break;
		}
		return P;
	}
}camera;

void mouse(int button, int state, int x, int y) {
	button_pressed[button] = state;
	mouse_pos[0] = x;
	mouse_pos[1] = y;
}

void motion(int x, int y) {
	using namespace glm;

	int modifiers = glutGetModifiers();
	int is_alt_active = modifiers & GLUT_ACTIVE_ALT;
	int is_ctrl_active = modifiers & GLUT_ACTIVE_CTRL;
	int is_shift_active = modifiers & GLUT_ACTIVE_SHIFT;

	int w = glutGet(GLUT_WINDOW_WIDTH);
	int h = glutGet(GLUT_WINDOW_HEIGHT);
	GLfloat dx = 1.f * (x - mouse_pos[0]) / w;
	GLfloat dy = -1.f * (y - mouse_pos[1]) / h;

	/*add code here to deal with mouse motion*/
	if (button_pressed[GLUT_LEFT_BUTTON] == GLUT_DOWN) {
		if (is_alt_active) {
			vec4 disp(camera.eye - camera.center, 1);
			GLfloat alpha = 2.0f;
			mat4 V = camera.get_viewing(); //camera position, camera 방향
			mat4 Rx = rotate(mat4(1.0f), alpha * dy, vec3(transpose(V)[0]));
			mat4 Ry = rotate(mat4(1.0f), -alpha * dx, vec3(0, 1, 0));
			mat4 R = Ry * Rx;
			camera.eye = camera.center + vec3(R * disp);
			camera.up = mat3(R) * camera.up;
			printf("Tumble tool used\n");
		}
	}
	if (button_pressed[GLUT_MIDDLE_BUTTON] == GLUT_DOWN) {
		if (is_alt_active) {
			//eye와 center의 position이 dx, dy에 따라 함께 변해야 함
			mat4 V = camera.get_viewing(); //camera position, camera 방향
			camera.eye += dx * vec3(transpose(V)[0]); //dx(크기) * 카메라의 X축 (방향) ==vector 크으으으으으
			camera.center += dx * vec3(transpose(V)[0]); 
			camera.eye += dy * vec3(transpose(V)[1]);	//dy(크기) * 카메라의 Y축 (방향) 성공해서 기분이 너무 좋아요~
			camera.center += dy * vec3(transpose(V)[1]);

			printf("Track tool used\n");
		}
	}
	mouse_pos[0] = x;
	mouse_pos[1] = y;

	glutPostRedisplay();
}


void init()
{
	srand(clock());
	models.push_back(new CubePrimitive(1.0f, 1.0f, 1.0f));
	models.push_back(new SpherePrimitive(0.5f, 15, 15));
	models.push_back(new ConePrimitive(0.5f, 1.0f, 10));
	models.push_back(new CylinderPrimitive(0.5f, 1.0f, 15));
	models.push_back(new TorusPrimitive(0.3f, 0.3f, 10, 10));
	models.push_back(new CarModel(
		(CubePrimitive*)models[0], (SpherePrimitive*)models[1], (ConePrimitive*)models[2], (CylinderPrimitive*)models[3], (TorusPrimitive*)models[4]));
	
	ShaderInfo shaders[] = {
	{GL_VERTEX_SHADER, "shader.vert"},
	{GL_FRAGMENT_SHADER, "shader.frag"},
	{GL_NONE, NULL}
	};
	
	program = LoadShaders(shaders);
	glUseProgram(program);
	
	int num_of_models = (int)models.size();
	
	for (int i = 0; i < num_of_models; ++i) {
		models[i]->init(program);
	}
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
}

void release_models()
{
	int n = (int)models.size();
	for (int i = 0; i < n; ++i) {
		if (models[i]) {
			delete models[i];
			models[i] = NULL;
		}
	}
	models.clear();
}
void display()
{
	using namespace glm;
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1, 1);
	glUniform1i(4, 0);
	mat4 Transf(1.0f);
	
	
	glm::mat4 V = lookAt(camera.eye, camera.center, camera.up);
	glm::mat4 P(1.0);
	int width = glutGet(GLUT_WINDOW_WIDTH);
	int height = glutGet(GLUT_WINDOW_HEIGHT);
	double aspect = 1.0 * width / height;
	P = glm::perspective(M_PI / 180.0 * (30.0), aspect, 0.01, 10.0);
	
	
	
	glUniformMatrix4fv(2, 1, GL_FALSE, value_ptr(V));
	glUniformMatrix4fv(3, 1, GL_FALSE, value_ptr(P));
	
	// Draw the selected model.
	models[5]->draw();
	
	glDisable(GL_POLYGON_OFFSET_FILL);
	glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
		idx_selected = key - '1';
		glutPostRedisplay();
		break;
	}
}
void keyboardSpecial(int key, int x, int y)
{
	int quadrant = 1 + (int)abs(model_state.theta / 1.6f) % 4;
	printf("사분면 : % d\n", quadrant);

	switch (key) {
	case GLUT_KEY_F1:
		specialKeyON[0] = true;
		model_state.theta -= 0.05f;
		glutPostRedisplay();
		
		break;
	case GLUT_KEY_F2:
		specialKeyON[1] = true;
		model_state.theta += 0.05f;
		glutPostRedisplay();
		break;
	case GLUT_KEY_RIGHT:
		specialKeyON[2] = true;
		using namespace glm;
		R = rotate(mat4(1.0f), model_state.theta, vec3(0.0f, 1.0f, 0.0f));
		if (quadrant == 2 || quadrant == 3)  //차 앞 방향이 2, 3사분면을 가리킬 때 left, right 키 반대로 적용되는 것 방지를 위함
			model_state.pos += 0.05f * vec3(R[0]); //뒤로 이동
		else 
			model_state.pos -= 0.05f * vec3(R[0]); //뒤로 이동
		model_state.rotOnlyTire += 0.05f; //차 바퀴 뒤로 회전

		glutPostRedisplay();
		break;

	case GLUT_KEY_LEFT:
		specialKeyON[3] = true;
		using namespace glm;
		R = rotate(mat4(1.0f), model_state.theta, vec3(0.0f, 1.0f, 0.0f));
		
		if (quadrant == 2 || quadrant == 3)  //차 앞 방향이 2, 3사분면을 가리킬 때 left, right 키 반대로 적용되는 것 방지를 위함
			model_state.pos-= 0.05f * vec3(R[0]); //앞으로 이동
		else
			model_state.pos += 0.05f * vec3(R[0]); //앞으로 이동
		model_state.rotOnlyTire -= 0.05f; //차 바퀴 앞으로 회전
		glutPostRedisplay();
		break;
	}
}

void mouse_wheel(int wheel, int dir, int x, int y) {
	int is_alt_active = glutGetModifiers() & GLUT_ACTIVE_ALT;

	if (is_alt_active) {
		glm::vec3 disp = camera.eye - camera.center;
		if (dir > 0)
			camera.eye = camera.center + 0.95f * disp;
		else
			camera.eye = camera.center + 1.05f * disp;
		printf("Dolly tool used\n");

	}

	glutPostRedisplay();
}
void main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(512, 512);
	glutCreateWindow("Drawing Primitives");
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		exit(EXIT_FAILURE);
	}
	init();
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyboardSpecial);
	glutMouseFunc(mouse);
	glutMouseWheelFunc(mouse_wheel);
	glutMotionFunc(motion);

	glutMainLoop();
	release_models();
}
