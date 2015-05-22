#include "mesh.h"
#include "glew.h"
#include "glut.h"
#include "FreeImage.h"
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#define MIPMAP
#define TESTSCENE 1
#define MAX_NUM_OBJECT 50
#define MAX_NUM_TEXTURE 50
using namespace std;

struct vertex {
	float x, y, z;
};

// function define
void load_scene();
void load_view();
void load_light();
void LoadTexture(string, int);
void LoadCubeTexture(string[6], int); 
void display();
void reshape(GLsizei, GLsizei);
void keyboard(unsigned char, int, int);
void rotate(int);

// parameter define
string skip, temp, cube[6];
int windowSize[2];
int xOrigin = -1, yOrigin = -1;
int rotate_value = 0;
int NUM_OBJECT = 0;
int texture_mode[MAX_NUM_OBJECT];
int texture_table[MAX_NUM_OBJECT];
// scene
mesh* object[MAX_NUM_OBJECT];
vertex scale_value[MAX_NUM_OBJECT];
vertex rotation_axis_vector[MAX_NUM_OBJECT];
vertex transfer_vector[MAX_NUM_OBJECT];
float angle[MAX_NUM_OBJECT];
GLuint texObject[MAX_NUM_TEXTURE];
// viewing
vertex eye, vat, vup;
int fovy, dnear, dfar;
int viewport_x, viewport_y;
GLsizei viewport_width, viewport_height;

int main(int argc,char** argv)
{
	load_view();
	glutInit(&argc, argv);
	glutInitWindowSize(viewport_width, viewport_height);
	glutInitWindowPosition(viewport_x, viewport_y);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutCreateWindow("HW2");
	glewInit();
	glGenTextures(MAX_NUM_TEXTURE, texObject);
	load_scene();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMainLoop();
	return 0;
}

void load_scene()
{
	ifstream input;
	stringstream ss;
	int i = 0, j = 0, mode = 0;
	
	if(TESTSCENE == 1)
		input.open("Scene\\TestScene1\\park.scene");
	if(TESTSCENE == 2)
		input.open("Scene\\TestScene2\\Chess.scene");

	while(input >> skip)
	{
		if(skip == "model") {
			input >> skip;
			if(TESTSCENE == 1)
				skip = "Scene\\TestScene1\\" + skip;
			if(TESTSCENE == 2)
				skip = "Scene\\TestScene2\\" + skip;
			object[i] = new mesh(skip.c_str());
			input >> scale_value[i].x >> scale_value[i].y >> scale_value[i].z;
			input >> angle[i] >> rotation_axis_vector[i].x >> rotation_axis_vector[i].y >> rotation_axis_vector[i].z;
			input >> transfer_vector[i].x >> transfer_vector[i].y >> transfer_vector[i].z;
			texture_mode[i] = mode;
			texture_table[i] = j-1;
			i++;
		}
		else if(skip == "single-texture"){
			mode = 1;
			getline(input, skip);
			ss.str("");
			ss.clear();
			ss << skip;
			
			ss >> temp;
			if(TESTSCENE == 1)
				temp = "Scene\\TestScene1\\" + temp;
			if(TESTSCENE == 2)
				temp = "Scene\\TestScene2\\" + temp;
			LoadTexture(temp, j);
			j++;
		}
		else if(skip == "multi-texture"){
			mode = 2;
			getline(input, skip);
			ss.str("");
			ss.clear();
			ss << skip;
			
			while(ss >> temp) {
				if(TESTSCENE == 1)
					temp = "Scene\\TestScene1\\" + temp;
				if(TESTSCENE == 2)
					temp = "Scene\\TestScene2\\" + temp;
				LoadTexture(temp, j);
				j++;
			}
		}
		else if(skip == "cube-map") {
			mode = 3;
			getline(input, skip);
			ss.str("");
			ss.clear();
			ss << skip;
			int k = 0;
			while(ss >> temp) {
				if(TESTSCENE == 1)
					temp = "Scene\\TestScene1\\" + temp;
				if(TESTSCENE == 2)
					temp = "Scene\\TestScene2\\" + temp;
				cube[k++] = temp;
			}
			LoadCubeTexture(cube, j);
			j++;
		}
		else {
			mode = 0;
		}
	}
	NUM_OBJECT = i;
	input.close();
}

void load_view()
{
	ifstream input;
	if(TESTSCENE == 1)
		input.open("Scene\\TestScene1\\park.view");
	if(TESTSCENE == 2)
		input.open("Scene\\TestScene2\\Chess.view");
	
	input >> skip >> eye.x >> eye.y >> eye.z;
	input >> skip >> vat.x >> vat.y >> vat.z;
	input >> skip >> vup.x >> vup.y >> vup.z;
	input >> skip >> fovy;
	input >> skip >> dnear;
	input >> skip >> dfar;
	input >> skip >> viewport_x >> viewport_y >> viewport_width >> viewport_height;
	
	input.close();
}

void load_light()
{	
	ifstream input;
	if(TESTSCENE == 1)
		input.open("Scene\\TestScene1\\park.light");
	if(TESTSCENE == 2)
		input.open("Scene\\TestScene2\\Chess.light");
	
	glShadeModel(GL_SMOOTH);

	// z buffer enable
	glEnable(GL_DEPTH_TEST);

	// enable lighting
	glEnable(GL_LIGHTING);
	
	int i=0;
	float temp_x, temp_y, temp_z;
	
	while(input >> skip && skip == "light" && i<8)
	{
		input >> temp_x >> temp_y >> temp_z;
		GLfloat light_position[] = {temp_x, temp_y, temp_z, 1.0f};
		
		input >> temp_x >> temp_y >> temp_z;
		GLfloat light_ambient[] = {temp_x, temp_y, temp_z, 1.0f};
		
		input >> temp_x >> temp_y >> temp_z;
		GLfloat light_diffuse[] = {temp_x, temp_y, temp_z, 1.0f};
		
		input >> temp_x >> temp_y >> temp_z;
		GLfloat light_specular[] = {temp_x, temp_y, temp_z, 1.0f};
		
		// set light property
		glEnable(GL_LIGHT0+i);
		glLightfv(GL_LIGHT0+i, GL_POSITION, light_position);
		glLightfv(GL_LIGHT0+i, GL_DIFFUSE, light_diffuse);
		glLightfv(GL_LIGHT0+i, GL_SPECULAR, light_specular);
		glLightfv(GL_LIGHT0+i, GL_AMBIENT, light_ambient);
		i++;
	}
	
	input >> temp_x >> temp_y >> temp_z;
	GLfloat ambient[] = {temp_x, temp_y, temp_z};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

	input.close();
}

void LoadTexture(string Filename, int index)
{
    char* pFilename;
	pFilename = (char*)Filename.c_str();
	
	FIBITMAP *pImage = FreeImage_Load(FreeImage_GetFileType(pFilename, 0), pFilename);
    FIBITMAP *p32BitsImage = FreeImage_ConvertTo32Bits(pImage);
    int iWidth = FreeImage_GetWidth(p32BitsImage);
    int iHeight = FreeImage_GetHeight(p32BitsImage);

	glBindTexture(GL_TEXTURE_2D, texObject[index]);
#ifdef MIPMAP
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, iWidth, iHeight, GL_BGRA, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(p32BitsImage));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
#else
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, iWidth, iHeight,
		0, GL_BGRA, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(p32BitsImage));
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#endif
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    FreeImage_Unload(p32BitsImage);
    FreeImage_Unload(pImage);
}

void LoadCubeTexture(string Filename[6], int index) 
{
	char* pFilename[6];
	for(int i=0; i<6; i++)
		pFilename[i] = (char*)Filename[i].c_str();
	
	glBindTexture(GL_TEXTURE_CUBE_MAP, texObject[index]);
	for(int i=0; i<6; i++){
		FIBITMAP *pImage = FreeImage_Load(FreeImage_GetFileType(pFilename[i], 0), pFilename[i]);
		FIBITMAP *p32BitsImage = FreeImage_ConvertTo32Bits(pImage);
		int iWidth = FreeImage_GetWidth(pImage);
		int iHeight = FreeImage_GetHeight(pImage);
#ifdef MIPMAP
		gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 4, iWidth, iHeight, GL_BGR, GL_UNSIGNED_BYTE, (void*)FreeImage_GetBits(pImage));
#else
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA,iWidth, iHeight, 0, GL_BGR, GL_UNSIGNED_BYTE,(void*)FreeImage_GetBits(pImage));
#endif
		FreeImage_Unload(p32BitsImage);
		FreeImage_Unload(pImage);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void display()
{
	// clear the buffer
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);      //清除用color
	glClearDepth(1.0f);                        // Depth Buffer (就是z buffer) Setup
	glEnable(GL_DEPTH_TEST);                   // Enables Depth Testing
	glDepthFunc(GL_LEQUAL);                    // The Type Of Depth Test To Do
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//這行把畫面清成黑色並且清除z buffer
	
	// viewport transformation
	glViewport(viewport_x, viewport_y, windowSize[0], windowSize[1]);

	// projection transformation
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (GLfloat)windowSize[0]/(GLfloat)windowSize[1], dnear, dfar);
	
	// viewing and modeling transformation
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(	eye.x, eye.y, eye.z,	// eye
				vat.x, vat.y, vat.z,    // center
				vup.x, vup.y, vup.z);   // up

	//注意light位置的設定，要在gluLookAt之後
	load_light();

	for(int t=0, k=0; k<NUM_OBJECT; k++)
	{
		glPushMatrix();//儲存現在的矩陣 (目前是modelview)
		glTranslatef(transfer_vector[k].x, transfer_vector[k].y, transfer_vector[k].z);
		glRotatef(angle[k], rotation_axis_vector[k].x, rotation_axis_vector[k].y, rotation_axis_vector[k].z);
		glScalef(scale_value[k].x, scale_value[k].y, scale_value[k].z);
		int lastMaterial = -1;

		for(int i=0; i<object[k]->fTotal; i++)
		{	
			// set material property if this face used different material
			if(lastMaterial != object[k]->faceList[i].m)
			{
				lastMaterial = (int)object[k]->faceList[i].m;
				glMaterialfv(GL_FRONT, GL_AMBIENT  , object[k]->mList[lastMaterial].Ka);
				glMaterialfv(GL_FRONT, GL_DIFFUSE  , object[k]->mList[lastMaterial].Kd);
				glMaterialfv(GL_FRONT, GL_SPECULAR , object[k]->mList[lastMaterial].Ks);
				glMaterialfv(GL_FRONT, GL_SHININESS, &object[k]->mList[lastMaterial].Ns);

				//you can obtain the texture name by object[k]->mList[lastMaterial].map_Kd
				//load them once in the main function before mainloop
				//bind them in display function here
			}

			if(texture_mode[k] == 1) {
				glActiveTexture(GL_TEXTURE0);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, texObject[texture_table[k]]);
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);				
				glEnable(GL_ALPHA_TEST);
				glAlphaFunc(GL_GREATER, 0.5f);	
			}
			if(texture_mode[k] == 2) {
				glActiveTexture(GL_TEXTURE0);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, texObject[texture_table[k]-1]);
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
				glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);

				glActiveTexture(GL_TEXTURE1);
				glEnable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, texObject[texture_table[k]]);
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
				glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
			}
			if(texture_mode[k] == 3) {
				glActiveTexture(GL_TEXTURE0);
				glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
				glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
				glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
				glEnable(GL_TEXTURE_GEN_S);
				glEnable(GL_TEXTURE_GEN_T);
				glEnable(GL_TEXTURE_GEN_R);
				glEnable(GL_TEXTURE_CUBE_MAP);
				glBindTexture(GL_TEXTURE_CUBE_MAP, texObject[texture_table[k]]);
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
				glEnable(GL_ALPHA_TEST);
				glAlphaFunc(GL_GREATER, 0.5f);
			}

			glBegin(GL_TRIANGLES);
			for(int j=0; j<3; j++)
			{
				if(texture_mode[k] == 1 || texture_mode[k] == 3)
					glTexCoord2fv(object[k]->tList[object[k]->faceList[i][j].t].ptr);
				if(texture_mode[k] == 2) {
					glMultiTexCoord2fv(GL_TEXTURE0, object[k]->tList[object[k]->faceList[i][j].t].ptr);
					glMultiTexCoord2fv(GL_TEXTURE1, object[k]->tList[object[k]->faceList[i][j].t].ptr);
				}
				
				glNormal3fv(object[k]->nList[object[k]->faceList[i][j].n].ptr);
				glVertex3fv(object[k]->vList[object[k]->faceList[i][j].v].ptr);
			}
			glEnd();
			
			if(texture_mode[k] == 1) {
				glActiveTexture(GL_TEXTURE0);
				glDisable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			if(texture_mode[k] == 2) {
				glActiveTexture(GL_TEXTURE0);
				glDisable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, 0);
				
				glActiveTexture(GL_TEXTURE1);
				glDisable(GL_TEXTURE_2D);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			if(texture_mode[k] == 3) {
				glActiveTexture(GL_TEXTURE0);
				glDisable(GL_TEXTURE_CUBE_MAP);
				glDisable(GL_TEXTURE_GEN_R);
				glDisable(GL_TEXTURE_GEN_T);
				glDisable(GL_TEXTURE_GEN_S);
				glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
			}
		}

		glPopMatrix();//拿stack中的矩陣到指定矩陣(modelview),會回到 I * ModelView
	}
	
	glutSwapBuffers();
}

void reshape(GLsizei w, GLsizei h)
{
	windowSize[0] = w;
	windowSize[1] = h;
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
		fovy += 1;
		break;
		
	case 's':
		fovy -= 1;
		break;

	case 'd':
		rotate_value += 1;
		rotate(rotate_value);
		break;

	case 'a':
		rotate_value -= 1;
		rotate(rotate_value);
		break;
	
	default:
		break;
	}

	glutPostRedisplay();
}

void rotate(int x)
{
	float theta;
	float toRadian = 0.01745;
	float Xsquare = (eye.x - vat.x) * (eye.x - vat.x);
	float Zsquare = (eye.z - vat.z) * (eye.z - vat.z);
	float radius = sqrt(Xsquare + Zsquare);

	if(TESTSCENE == 1)
		theta = 45 + x;
	if(TESTSCENE == 2) 
		theta = 45 + x;

	eye.x = vat.x + radius * sin(theta*toRadian);
	eye.z = vat.z + radius * cos(theta*toRadian);

	glutPostRedisplay();
}