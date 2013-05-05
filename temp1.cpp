#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <vector>
#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif


#include "glm.h"
#include "glm.cpp"
#include <SOIL.h>

#define PI 3.141592653589
#define DEG2RAD(deg) (deg * PI / 180)
#define ESCAPE 27
#define PAGE_UP 73
#define PAGE_DOWN 81
#define UP_ARROW 72
#define DOWN_ARROW 80
#define LEFT_ARROW 75
#define RIGHT_ARROW 77

using namespace std;


/********** STRUCTS TO CONSTRUCT BOXES AROUND THE OBJECTS ************************/

typedef struct point
{
	float x, y, z;
}point;
typedef struct box
{
	point min, max;
}box;
typedef struct boxed_object
{
	char *name;
	vector<box> boxes;
}boxed_object;

/***********************************************************************************/


/************* STRUCTS TO CONSTRUCT THE WORLD, TEXTURES ******************************/

typedef struct
{
	GLfloat x, y, z;     // 3d coords.
	GLfloat u, v;        // texture coords.
} VERTEX;

typedef struct
{
	VERTEX vertex[3];    // 3 vertices array
	int tex;
	int id;
} TRIANGLE;

typedef struct
{
	int numtriangles;    // number of triangles in the sector
	TRIANGLE* triangle;  // pointer to array of triangles.
} SECTOR;

SECTOR sector1;
SECTOR sector2;

typedef struct
{
	unsigned long sizeX;
	unsigned long sizeY;
	char *data;
} Image;

int numtriangles;
/***********************************************************************************/


/****************************  VARIABLES FOR GAME MODE  ****************************/

int w, h;
int view_mode = 0;	// 0: 1st Person , 1: Third Person , 2: Tower View
float xcenter, zcenter;
float dhelicopter = 10;
float x_look_helicopter=0, z_look_helicopter=0;
float x_helicopter=0, z_helicopter=0;
int tile_index = 2;
float  tile_width=6, tile_height=6, tile_dist=10;
float connecting_dist = 0, connecting_offset = 0.06;
int n = 5, m = 5;
// Starting index for connecting tiles
int indx1, id1, indx2, id2, indx3, id3;

/***********************************************************************************/


/****************************  VARIABLES FOR STEVE ********************************/

int direction; 		// 0 : Still  1: Moving
float _angle = 180;
float leg_angle = 0;
float arm_angle = 0;
float lookup =0;
float offset = 2;	// Speed of arm, leg rotations
float xtrans = 0, ztrans = 0, ytrans = 0.2, ytrans_old = 0.2;

/***********************************************************************************/


/*************************** VARIABLES FOR GAME LOGIC *******************************/

float dist = 0, fall_dist = 0;
int jump_flag = 0, fall_flag = 0;
float yjump=0, tjump=0;

int current_id = 50; 	// to store the current position of steve

GLMmodel* pmodel1 = NULL;

vector<boxed_object> boxed_obj;		// To store all boxed objs.
int boxindex = 0;			// To store the number of objects.

/***********************************************************************************/


/********** GLOBAL VARIABLES FOR TEXTURES,  LIGHTING *******************************/

int loop;             // general loop variable
GLuint texture[3];       // storage for 3 textures;
GLuint texture2[3];
GLuint texture3[3];
GLuint texture4[3];
GLuint texture5[3];
GLuint texture6[3];
GLuint texture7[3];


int light = 0;           // lighting on/off
int blend = 0;        // blending on/off


GLfloat LightAmbient[]  = {0.5f, 0.5f, 0.5f, 1.0f};
GLfloat LightDiffuse[]  = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat LightPosition[] = {0.0f, 0.0f, 2.0f, 1.0f};

GLuint filter = 0;       // texture filtering method to use (nearest, linear, linear + mipmaps)
GLUquadric *quad;
float disc_color = 50, disc_offset = 0.1;
float disc_angle = 0, disc_angle_offset = 0.02;


/***********************************************************************************/


/********** FUNCTION DEFINITIONS **********/
void initRendering();
void handleResize(int w, int h);

void specialKeyRelease (int key, int x, int y);
void specialKeyPressed(int key, int x, int y);
void keyPressed(unsigned char key, int x, int y);
void mouse2(int button, int state, int x, int y);
void mouse(int button, int state, int x, int y);

void mousePassive(int x, int y);

void drawScene();
GLvoid drawSteve(GLMmodel* model, GLuint mode);
GLvoid DrawGLScene();
void update(int value);

void loadmodel();
void SetupWorld();
void create_connecting(int id, int indx, int flag, float offset, int flag2);
void create_support(int id, int indx, int flag2);
int add_to_triangles(float xleft, float zleft, int indx, int id, int flag);

float terrain_collision();
void vert_collisions();
int inside_rectangle_check(int id);
int inside_rectangle_check_x(int id);
int inside_rectangle_check_z(int id);

void drawbox(box b); // for debugging
void check_warnings(GLMmodel* model, GLuint mode);

/********* INTEGRATED ******************/
void readstr(FILE *f, char *string);
int ImageLoad(char *filename, Image *image);
void generate_filter(Image* image1, GLuint* texture, char *name);
float rad(float angle);
GLvoid LoadGLTextures();
static unsigned int getint(FILE *fp);
static unsigned int getshort(FILE *fp);

int main(int argc, char** argv)
{
	SetupWorld();

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_ALPHA);

	w = glutGet(GLUT_SCREEN_WIDTH);
	h = glutGet(GLUT_SCREEN_HEIGHT);
	glutInitWindowSize(w, h);
	glutCreateWindow("ASSIGNMENT2");
	initRendering();

	glutDisplayFunc(drawScene);
	glutIdleFunc(drawScene);
	glutKeyboardFunc(keyPressed);

	glutMouseFunc(mouse2);
	glutMotionFunc(mousePassive);

	glutSpecialUpFunc(specialKeyRelease);

	glutSpecialFunc(specialKeyPressed);

	glutReshapeFunc(handleResize);
	glutTimerFunc(25, update, 0);

	loadmodel();


	glutMainLoop();
	printf("glutMainLoop\n");
	return 0;
}

void glutPrint(float x, float y, void* font, char* text, float r, float g, float b, float a)
{
    if(!text || !strlen(text)) return;
    bool blending = false;
    if(glIsEnabled(GL_BLEND)) blending = true;
    glEnable(GL_BLEND);
    glColor4f(r,g,b,a);
    glRasterPos2f(x,y);
    while (*text) {
        glutBitmapCharacter(font, *text);
        text++;
    }
    if(!blending) glDisable(GL_BLEND);
}


void drawScene()
{
	float x1, z1, d=1.5;
	int i, j;
	float xleft, zleft;

	z1 = -d * (cos(-_angle * PI / 180.0));
	x1 = d * sin(-_angle * PI / 180.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	void* glutFonts[7] = {
    GLUT_BITMAP_9_BY_15,
    GLUT_BITMAP_8_BY_13,
    GLUT_BITMAP_TIMES_ROMAN_10,
    GLUT_BITMAP_TIMES_ROMAN_24,
    GLUT_BITMAP_HELVETICA_10,
    GLUT_BITMAP_HELVETICA_12,
    GLUT_BITMAP_HELVETICA_18
};

	/*

	GLfloat lightpos[] = {-50, 6, -70, 1.0};
  	glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
  	printf("%f %f\n",xtrans, ztrans );
  	//GLfloat stevepos[] = {-xtrans+x1, ytrans, -ztrans+z1, 1.0};
  	GLfloat stevepos[] = {-xtrans, ytrans + 2, -ztrans, 1.0};
  	//GLfloat stevedir[] = {-x1, 0 , -z1, 1.0};
  	GLfloat stevedir[] = {0, -1 , 0, 1.0};
  	GLfloat steveDiffuse[]  = {1.0f, 1.0f, 1.0f, 1.0f};


  	glLightfv(GL_LIGHT2, GL_POSITION, stevepos);
  	glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, stevedir);
  	glLightfv(GL_LIGHT2, GL_DIFFUSE, steveDiffuse);
  	glLightf (GL_LIGHT2, GL_SPOT_CUTOFF, 30.f);
  	glLightfv(GL_LIGHT2, GL_AMBIENT, LightAmbient);

  	*/

	//glEnable(GL_LIGHTING);
	glPushMatrix();

	glDisable(GL_TEXTURE_2D);
	glTranslatef(0, 0, -1);
	glutPrint(0.0f, 0.1f, glutFonts[4], "Hello World!", 1.0f, 0.0f, 0.0f, 2.5f);
	glEnable(GL_TEXTURE_2D);

	glPopMatrix();

	if(view_mode==0)	// First Person Mode
		gluLookAt(-xtrans, ytrans+0.2, -ztrans, -xtrans+x1, ytrans+0.2+lookup, -ztrans+z1, 0, 1, 0);
	else if(view_mode==1)	// Third Person Mode
	{
		gluLookAt(-xtrans-x1, 0.5+ytrans, -ztrans-z1, -xtrans, 0.05+ytrans, -ztrans, 0, 1, 0);
	//	glDisable(GL_LIGHTING);
	}
	else if(view_mode==2)	// Tower Mode
		gluLookAt(-xcenter, 3 , -zcenter, -xtrans, ytrans, -ztrans, 0, 1, 0);
	else if(view_mode==3) 	// Helicopter Mode
		gluLookAt(-xcenter, 3 , -zcenter, 0, z_look_helicopter/20 + z_helicopter/10.0, x_look_helicopter/20 + x_helicopter/10.0, 0, 1, 0);
	else if(view_mode==4)	// Tile View
	{
		i = (2 * tile_index)/10;
		j = (2 * tile_index)%10;
		xleft = 1.0 * tile_width * i - tile_width / 2.0 + i * tile_dist;
		zleft = 1.0 * tile_height * j - tile_height / 2.0 + j * tile_dist;
		gluLookAt(sector1.triangle[2*tile_index].vertex[0].x + 1,
			sector1.triangle[2*tile_index].vertex[0].y + 1 ,
			sector1.triangle[2*tile_index].vertex[0].z + 2,
			-xtrans, 0, -ztrans, 0, 1, 0);
	}
	

	glPushMatrix();

	if(ztrans == 0)
		ztrans = d;

	glRotatef(180.0, 0, 1, 0);
	glTranslatef(xtrans, ytrans, ztrans);
	glRotatef(_angle, 0.0, 1.0, 0.0);

	glScalef(0.2, 0.2, 0.2);

	//drawSteve(pmodel1 , GLM_SMOOTH | GLM_MATERIAL | GLM_TEXTURE);
	drawSteve(pmodel1 , GLM_SMOOTH | GLM_TEXTURE);





	glPopMatrix();

DrawGLScene();


	glutSwapBuffers();
}

void render_text(char *string)
{
	for (int i=0;i<sizeof(string);i++)
	{
		glutStrokeCharacter(GLUT_STROKE_ROMAN, string[i]);

	}
}
//Called every 25 milliseconds
void update(int value)
{
	// fall flag = 0 means currently, Steve is not falling
	// fall flag = 1 means currently, Steve is falling
	// jump flag = 0 means currently, Steve is on the ground (not jumping)
	// jump flag = 1 means currently, Steve is in air (jumping)
	int i;
	vert_collisions();

	connecting_dist += connecting_offset;
	if(connecting_dist > tile_dist-1 || connecting_dist < -1.0)
	{
		connecting_offset*=-1;
		disc_angle_offset = 0;
	}

	if(connecting_dist > 0 && connecting_dist < tile_dist-2)
	{
		disc_angle_offset = 0.02;
		create_connecting(id1, indx1, 0, connecting_dist, 1);
		create_connecting(id2, indx2, 1, connecting_dist, 1);
	}
	create_support(id3, indx3, 1);

	if(!jump_flag)
	{
		if(current_id >= id1 && current_id < id2)
		{
			if(connecting_dist > 0 && connecting_dist < tile_dist - 2)
			{
				ztrans -= connecting_offset;
			//	_angle = disc_angle;
			}
		}
		if(current_id >= id2 && current_id < (id2 + (n-1)*(m-1)) )
		{
			if(connecting_dist > 0 && connecting_dist < tile_dist - 2)
			{
				xtrans -= connecting_offset;
			//	_angle = disc_angle;
			}
		}
	}



	if(fall_flag==0)
	{
		while(inside_rectangle_check(current_id)==0)
		{
			fall_dist = ytrans - terrain_collision();
			if(fall_dist > 0.001)
			{
				fall_flag = 1;
			}
			else
			{
				fall_flag = 0;
				ytrans = ytrans_old;
			}
			tjump = 0;
			jump_flag = 0;
		}
		// For tiles moving up and down
		if(!fall_flag && inside_rectangle_check(current_id) && !jump_flag )
			ytrans = sector1.triangle[current_id*2].vertex[0].y + 0.2;

	}
	if(fall_flag==1)
	{
		tjump += 0.07;
		ytrans -= 0.05*tjump;
		fall_dist -= 0.05*tjump;
		direction = 1;
		if(fall_dist < 0.3)
		{
			direction = 0;
			fall_flag=0;
			ytrans_old = ytrans;

			if(current_id ==0) // TODO : HANDLE JUMP IN WATER
			{
				printf("Steve Jumped into water\n");
				exit(0);
			}
		}
	}


	if(jump_flag!=1)
	{
		ytrans_old = ytrans;
	}
	float term;
	if(jump_flag==1)
	{
		tjump += 0.07;
		yjump = 0.4 * tjump - 0.1 * tjump * tjump + ytrans_old;
		ytrans = yjump;
		fall_flag=2;
		term = ytrans_old;

		if(current_id != 0)
		{
			for(i=4; i<sector1.numtriangles; i++)
			{
				if(sector1.triangle[i].id == current_id)
					continue;
				if(inside_rectangle_check(sector1.triangle[i].id))
				{
					if(sector1.triangle[i].vertex[0].y+0.2 > term)
						term = sector1.triangle[i].vertex[0].y+0.2;
				}
			}
		}
		if(yjump < term)
		{
			yjump = 0;
			tjump = 0;
			jump_flag = 0;
			fall_flag=0;
			ytrans = term;
			direction = 0;
			offset = 2;
		}
	}
	if(direction)
	{
		leg_angle += offset;
		arm_angle += (offset/1.2);
	}

	if (_angle > 360)
	{
		_angle -= 360;
	}
	if(fabs(leg_angle) > 30)
	{
		offset *=-1;
	}


	glutPostRedisplay();
	glutTimerFunc(25, update, 0);
}
// loads the world from a text file.
void SetupWorld()
{
	float x, y, z, u, v;
	int vert,localtex;
	FILE *filein;        // file to load the world from
	char oneline[255];

	filein = fopen("Data/world2.txt", "rt");

	readstr(filein, oneline);
	sscanf(oneline, "NUMPOLLIES %d\n", &numtriangles);

	sector1.numtriangles = numtriangles;
//	sector1.triangle = (TRIANGLE *) malloc(sizeof(TRIANGLE)*numtriangles);
	sector1.triangle = (TRIANGLE *) malloc(sizeof(TRIANGLE)*10000);


	for (loop = 0; loop < (int)numtriangles; loop++)
	{
		for (vert = 0; vert < 3; vert++) {
			readstr(filein,oneline);
			sscanf(oneline, "%d %f %f %f %f %f",&localtex, &x, &y, &z, &u, &v);
			sector1.triangle[loop].tex=localtex;
			sector1.triangle[loop].vertex[vert].x = x;
			sector1.triangle[loop].vertex[vert].y = y;
			sector1.triangle[loop].vertex[vert].z = z;
			sector1.triangle[loop].vertex[vert].u = u;
			sector1.triangle[loop].vertex[vert].v = v;

			sector1.triangle[loop].id = loop/2;

		}
	}
	fclose(filein);
	int i=0, j=0,id = 2, indx;

	float xleft, zleft;
	// Floors

	for(i=0; i<n; i++)
	{
		for(j=0; j<m; j++)
		{
//			rand1 = rand()%100;
//			rand2 = rand()%2;
//			if(rand2 == 0)
//				rand1*=-1;
//			y = rand1/200.0;
			y = 0;
			xleft = 1.0 * tile_width * i - tile_width / 2.0 + i * tile_dist;
			zleft = 1.0 * tile_height * j - tile_height / 2.0 + j * tile_dist;

			if(i==n/2 && j==m/2)
			{
				xcenter = -(xleft + tile_width /2.0);
				zcenter = -(zleft + tile_height /2.0);
			}

			if(id==current_id)
			{
				xtrans = -(xleft + tile_width /2.0);
				ztrans = -(zleft + tile_height /2.0);
				ytrans = y+0.2;
				ytrans_old = ytrans;

			}

			indx = 4 + i*2*n + 2*j;
			if(i==7 && j==6)
			{

				sector1.triangle[indx].tex = 7;
			}
			else
				sector1.triangle[indx].tex = 1;

			sector1.triangle[indx].id = id;

			sector1.triangle[indx].vertex[0].x = xleft;
			sector1.triangle[indx].vertex[0].z = zleft;
			sector1.triangle[indx].vertex[0].y = y;

			sector1.triangle[indx].vertex[0].u = 0;
			sector1.triangle[indx].vertex[0].v = 12;


			sector1.triangle[indx].vertex[1].x = xleft;
			sector1.triangle[indx].vertex[1].z = zleft+ tile_height;
			sector1.triangle[indx].vertex[1].y = y;

			sector1.triangle[indx].vertex[1].u = 0;
			sector1.triangle[indx].vertex[1].v = 0;


			sector1.triangle[indx].vertex[2].x = xleft+tile_width;
			sector1.triangle[indx].vertex[2].z = zleft+tile_height;
			sector1.triangle[indx].vertex[2].y = y;

			sector1.triangle[indx].vertex[2].u = 12;
			sector1.triangle[indx].vertex[2].v = 0;

			indx = 4+ i*2*n+2*j+1;

			if(i==7 && j==6)
				sector1.triangle[indx].tex = 7;
			else
				sector1.triangle[indx].tex = 1;
			sector1.triangle[indx].id = id;

			sector1.triangle[indx].vertex[0].x = xleft;
			sector1.triangle[indx].vertex[0].z = zleft;
			sector1.triangle[indx].vertex[0].y = y;

			sector1.triangle[indx].vertex[0].u = 0;
			sector1.triangle[indx].vertex[0].v = 12;


			sector1.triangle[indx].vertex[1].x = xleft+tile_width;
			sector1.triangle[indx].vertex[1].z = zleft;
			sector1.triangle[indx].vertex[1].y = y;

			sector1.triangle[indx].vertex[1].u = 0;
			sector1.triangle[indx].vertex[1].v = 0;


			sector1.triangle[indx].vertex[2].x = xleft+tile_width;
			sector1.triangle[indx].vertex[2].z = zleft+tile_height;
			sector1.triangle[indx].vertex[2].y = y;

			sector1.triangle[indx].vertex[2].u = 12;
			sector1.triangle[indx].vertex[2].v = 0;

			sector1.numtriangles+=2;
			numtriangles+=2;
			id++;


		}
	}
	id1 = id;
	indx1 = indx;

	create_connecting(id1, indx1, 0, 0, 0);
	create_connecting(id2, indx2, 1, 0, 0);

	create_support(id3, indx3, 0);


	filein = fopen("Data/world3.txt", "rt");

	readstr(filein, oneline);
	sscanf(oneline, "NUMPOLLIES %d\n", &numtriangles);


//	sector1.triangle = (TRIANGLE *) malloc(sizeof(TRIANGLE)*numtriangles);
//	sector1.triangle = (TRIANGLE *) malloc(sizeof(TRIANGLE)*1000);


	for (loop = sector1.numtriangles; loop < sector1.numtriangles + (int)numtriangles; loop++)
	{
		for (vert = 0; vert < 3; vert++) {
			readstr(filein,oneline);
			sscanf(oneline, "%d %f %f %f %f %f",&localtex, &x, &y, &z, &u, &v);
			sector1.triangle[loop].tex=localtex;
			sector1.triangle[loop].vertex[vert].x = x;
			sector1.triangle[loop].vertex[vert].y = y;
			sector1.triangle[loop].vertex[vert].z = z;
			sector1.triangle[loop].vertex[vert].u = u;
			sector1.triangle[loop].vertex[vert].v = v;

			sector1.triangle[loop].id = loop/2;

		}
	}
	int ref = sector1.numtriangles;

	//sector1.numtriangles += numtriangles;

	fclose(filein);
	printf("done\n");

	int start;

	for(i=0; i<n; i++)
	{
		for(j=0; j<m; j++)
		{
			printf("%d %d\n",i, j );
			start = sector1.numtriangles;
			xleft = 1.0 * tile_width * i - tile_width / 2.0 + i * tile_dist + tile_height/2;
			zleft = 1.0 * tile_height * j - tile_height / 2.0 + j * tile_dist + tile_width/2;

			for (loop = 0; loop <  (int)numtriangles; loop++)
			{
				for (vert = 0; vert < 3; vert++) {
					sector1.triangle[start + loop].tex=sector1.triangle[ref + loop].tex;
					sector1.triangle[start + loop].vertex[vert].x = sector1.triangle[ref + loop].vertex[vert].x + xleft;
					sector1.triangle[start + loop].vertex[vert].y = sector1.triangle[ref + loop].vertex[vert].y;
					sector1.triangle[start + loop].vertex[vert].z = sector1.triangle[ref + loop].vertex[vert].z + zleft;
					sector1.triangle[start + loop].vertex[vert].u = sector1.triangle[ref + loop].vertex[vert].u;
					sector1.triangle[start + loop].vertex[vert].v = sector1.triangle[ref + loop].vertex[vert].v;
					sector1.triangle[start + loop].id = (loop+ start)/2;

				}
			}
			sector1.numtriangles += numtriangles;
		}
	}


printf("dnoe2\n");
	return;
}

int inside_rectangle_check(int id)
{
	float xleft, xright, zdown, zup;
	int i, j;
	for(i = 2*id; i< 2*id+2; i++)
	{
		for(j=0; j<3; j++)
		{
			if(i==2*id && j==0)
			{
				xleft = sector1.triangle[i].vertex[j].x ;
				zdown = sector1.triangle[i].vertex[j].z;
				xright = sector1.triangle[i].vertex[j].x ;
				zup = sector1.triangle[i].vertex[j].z;
			}
			else
			{
				if(sector1.triangle[i].vertex[j].x < xleft)
				{
					xleft = sector1.triangle[i].vertex[j].x;
				}
				if(sector1.triangle[i].vertex[j].x > xright)
				{
					xright = sector1.triangle[i].vertex[j].x;
				}

				if(sector1.triangle[i].vertex[j].z < zdown)
				{
					zdown = sector1.triangle[i].vertex[j].z;
				}
				if(sector1.triangle[i].vertex[j].z > zup)
				{
					zup = sector1.triangle[i].vertex[j].z;
				}
			}
		}
	}
	if( ((-xtrans) < xright) && ((-xtrans) > xleft) && (-ztrans < zup) && (-ztrans > zdown))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


float terrain_collision()
{
	float maxx, temp;
	ytrans -=0.2;
	int i, vert, flag=0, vflag=0;

	for(i=0; i<sector1.numtriangles; i++)
	{
		vflag=0;
		for (vert = 1; vert < 3; vert++)
		{
			if(sector1.triangle[i].vertex[vert].y != sector1.triangle[i].vertex[vert-1].y)
			{
				vflag=1;
				break;
			}
		}
		if(vflag==1)
		{
			continue;
		}

		if(inside_rectangle_check(sector1.triangle[i].id) == 0)
			continue;

		for (vert = 0; vert < 3; vert++)
		{
			if(sector1.triangle[i].id == current_id)
				continue;
			if( sector1.triangle[i].vertex[vert].y < ytrans + 0.1)
			{
				temp =  sector1.triangle[i].vertex[vert].y;

				if(flag==0)
				{
					maxx = temp;
					flag=1;
					current_id = sector1.triangle[i].id;
				}
				else
				{
					if(temp > maxx)
					{
						maxx = temp;
						current_id = sector1.triangle[i].id;
					}
				}
			}
		}
	}
	if(flag==0)
		maxx=ytrans;
	return maxx+0.2;
}
void vert_collisions()
{
	int xflag = 0, zflag = 0, yflag = 0, i;
	for(i=0; i<sector1.numtriangles; i++)
	{
		xflag = 0; yflag = 0; zflag = 0;

		if(sector1.triangle[i].vertex[0].y == sector1.triangle[i].vertex[1].y)
		{
			if(sector1.triangle[i].vertex[1].y == sector1.triangle[i].vertex[2].y)
				yflag = 1;
		}
		if(sector1.triangle[i].vertex[0].x == sector1.triangle[i].vertex[1].x)
		{
			if(sector1.triangle[i].vertex[1].x == sector1.triangle[i].vertex[2].x)
				xflag = 1;
		}
		if(sector1.triangle[i].vertex[0].z == sector1.triangle[i].vertex[1].z)
		{
			if(sector1.triangle[i].vertex[1].z == sector1.triangle[i].vertex[2].z)
				zflag = 1;
		}
		if(yflag)
			continue;
		if(xflag)
		{
			if(inside_rectangle_check_x(sector1.triangle[i].id))
			{



				if(-sector1.triangle[i].vertex[0].x - xtrans > 0 && -sector1.triangle[i].vertex[0].x - xtrans < 0.2)
				{
					xtrans = -sector1.triangle[i].vertex[0].x - 0.22;
				}
				if(xtrans + sector1.triangle[i].vertex[0].x > 0 && xtrans + sector1.triangle[i].vertex[0].x < 0.2)
				{
					xtrans = -sector1.triangle[i].vertex[0].x + 0.22;
				}
			}
		}
		if(zflag)
		{

			if(inside_rectangle_check_z(sector1.triangle[i].id))
			{
				if(-sector1.triangle[i].vertex[0].z - ztrans > 0 && -sector1.triangle[i].vertex[0].z - ztrans < 0.2)
				{
					ztrans = -sector1.triangle[i].vertex[0].z - 0.22;
				}
				if(ztrans + sector1.triangle[i].vertex[0].z > 0 && ztrans + sector1.triangle[i].vertex[0].z < 0.2)
				{
					ztrans = -sector1.triangle[i].vertex[0].z + 0.22;
				}
			}
		}
	}

}
int inside_rectangle_check_x(int id)
{
	float yleft, yright, zdown, zup;
	int i, j;
	for(i = 2*id; i< 2*id+2; i++)
	{
		for(j=0; j<3; j++)
		{
			if(i==2*id && j==0)
			{
				yleft = sector1.triangle[i].vertex[j].y;
				zdown = sector1.triangle[i].vertex[j].z;
				yright = sector1.triangle[i].vertex[j].y;
				zup = sector1.triangle[i].vertex[j].z;
			}
			else
			{
				if(sector1.triangle[i].vertex[j].y < yleft)
				{
					yleft = sector1.triangle[i].vertex[j].y;
				}
				if(sector1.triangle[i].vertex[j].y > yright)
				{
					yright = sector1.triangle[i].vertex[j].y;
				}

				if(sector1.triangle[i].vertex[j].z < zdown)
				{
					zdown = sector1.triangle[i].vertex[j].z;
				}
				if(sector1.triangle[i].vertex[j].z > zup)
				{
					zup = sector1.triangle[i].vertex[j].z;
				}
			}
		}
	}



	if( ((ytrans) < yright) && ((ytrans) > yleft) && (-ztrans < zup) && (-ztrans > zdown))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
int inside_rectangle_check_z(int id)
{
	float yleft, yright, xdown, xup;
	int i, j;
	for(i = 2*id; i< 2*id+2; i++)
	{
		for(j=0; j<3; j++)
		{
			if(i==2*id && j==0)
			{
				yleft = sector1.triangle[i].vertex[j].y;
				xdown = sector1.triangle[i].vertex[j].x;
				yright = sector1.triangle[i].vertex[j].y;
				xup = sector1.triangle[i].vertex[j].x;
			}
			else
			{
				if(sector1.triangle[i].vertex[j].y < yleft)
				{
					yleft = sector1.triangle[i].vertex[j].y;
				}
				if(sector1.triangle[i].vertex[j].y > yright)
				{
					yright = sector1.triangle[i].vertex[j].y;
				}

				if(sector1.triangle[i].vertex[j].x < xdown)
				{
					xdown = sector1.triangle[i].vertex[j].x;
				}
				if(sector1.triangle[i].vertex[j].x > xup)
				{
					xup = sector1.triangle[i].vertex[j].x;
				}
			}

		}

	}
	if( ((ytrans) < yright) && ((ytrans) > yleft) && (-xtrans < xup) && (-xtrans > xdown))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
GLvoid drawSteve(GLMmodel* model, GLuint mode)
{
	static GLuint i;
	static GLMgroup* group;
	static GLMtriangle* triangle;
	static GLMmaterial* material;



	assert(model);
	assert(model->vertices);
	GLuint textureObject = 0;
	textureObject = SOIL_load_OGL_texture( "herobrine2.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
						SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y |
						SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureObject);

	check_warnings(model, mode);

	group = model->groups;

	while (group)
	{
		glPushMatrix();
		if(direction != 0)
		{
			if(strcmp(" leg1.R", group->name)==0)
			{
				glRotatef(leg_angle, 1, 0, 0);
			}
			if( strcmp(" leg2.R", group->name)==0)
			{
				glRotatef(leg_angle, 1, 0, 0);

			}
			if(strcmp(" leg1.L", group->name)==0 || strcmp(" leg2.L", group->name)==0 )
			{
				glRotatef(-1.0*leg_angle, 1, 0, 0);
			}
			if(strcmp(" arm1.L", group->name)==0)
			{
				glTranslatef(0, 0.5, 0);
				glRotatef(arm_angle, 1, 0, 0);
				glTranslatef(0, -0.5, 0);
			}
			if(strcmp(" arm1.R", group->name)==0)
			{
				glTranslatef(0, 0.5, 0);
				glRotatef(-1.0 * arm_angle, 1, 0, 0);
				glTranslatef(0, -0.5, 0);
			}
			if(strcmp(" arm2.L", group->name)==0)
			{
				glTranslatef(0, 0.5, 0);
				glRotatef(arm_angle, 1, 0, 0);
				glTranslatef(0, -0.5, 0);
			}
			if(strcmp(" arm2.R", group->name)==0)
			{
				glTranslatef(0, 0.5, 0);
				glRotatef(-1.0 * arm_angle, 1, 0, 0);
				glTranslatef(0, -0.5, 0);
			}

			/*
			if(strcmp(" arm2.R", group->name)==0 )
			{
				glTranslatef(0, 0.27, 0);
				glRotatef(-arm_angle-30, 1, 0, 0);
				glTranslatef(0, -0.27, 0);
			}
			if( strcmp(" arm2.L", group->name)==0 )
			{
				glTranslatef(0, 0.27, 0);
				glRotatef(arm_angle-30, 1, 0, 0);
				glTranslatef(0, -0.27, 0);
			}
			*/
		}
		material = &model->materials[group->material];
		if (mode & GLM_MATERIAL)
		{
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material->ambient);
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material->diffuse);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material->specular);
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material->shininess);
		}

		if (mode & GLM_COLOR)
		{
			glColor3fv(material->diffuse);
		}
		glEnable(GL_SMOOTH);
		glShadeModel(GL_SMOOTH);
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		glBegin(GL_TRIANGLES);
		for (i = 0; i < group->numtriangles; i++)
		{

			triangle = &T(group->triangles[i]);

			if (mode & GLM_FLAT)
				glNormal3fv(&model->facetnorms[3 * triangle->findex]);

			if (mode & GLM_SMOOTH)
				glNormal3fv(&model->normals[3 * triangle->nindices[0]]);
			if (mode & GLM_TEXTURE)
				glTexCoord2fv(&model->texcoords[2 * triangle->tindices[0]]);

			glVertex3fv(&model->vertices[3 * triangle->vindices[0]]);	// Vertex #1

			if (mode & GLM_SMOOTH)
				glNormal3fv(&model->normals[3 * triangle->nindices[1]]);
			if (mode & GLM_TEXTURE)
				glTexCoord2fv(&model->texcoords[2 * triangle->tindices[1]]);

			glVertex3fv(&model->vertices[3 * triangle->vindices[1]]);	// Vertex #2

			if (mode & GLM_SMOOTH)
				glNormal3fv(&model->normals[3 * triangle->nindices[2]]);
			if (mode & GLM_TEXTURE)
				glTexCoord2fv(&model->texcoords[2 * triangle->tindices[2]]);

			glVertex3fv(&model->vertices[3 * triangle->vindices[2]]);	// Vertex #3

			/*
			for (k=0;k<3;k++)
			{
				GLuint index = model->triangles[group->triangles[i]].vindices[j];
				x = (model->vertices[index*3 + 0]) * cos(_angle * PI / 180.0) + xtrans;
				y = model->vertices[index*3 + 1];
				z = (model->vertices[index*3 + 2]) * sin(_angle * PI / 180.0) + ztrans;
				if(i==0)
				{
					b.min.x = x;
					b.min.y = y;
					b.min.z = z;

					b.max.x = x;
					b.max.y = y;
					b.max.z = z;
				}
				else
				{
					if (b.min.x>x) b.min.x = x;
					if (b.min.y>y) b.min.y = y;
					if (b.min.z>z) b.min.z = z;

					if (b.max.x<x) b.max.x = x;
					if (b.max.y<y) b.max.y = y;
					if (b.max.z<z) b.max.z = z;
				}
			}
			*/
		}
		glEnd();
//		if( group->numtriangles >0)
//			boxed_obj[j].boxes.push_back(b);
		group = group->next;
		glPopMatrix();
	}
}
void specialKeyPressed(int key, int x, int y)
{
	/* avoid thrashing this procedure */
	usleep(100);
	switch (key) {
		case GLUT_KEY_UP: // walk forward (bob head)

			direction = 1;
			dist = 0.1f;
			ztrans += dist * cos(_angle * PI / 180.0);
			xtrans += dist * sin(_angle * PI / 180.0);
			break;

		case GLUT_KEY_DOWN: // walk back (bob head)
			direction = 1;
			dist = -0.05f;
			ztrans += dist * cos(_angle * PI / 180.0);
			xtrans += dist * sin(_angle * PI / 180.0);

			break;

		case GLUT_KEY_LEFT: // look left
			_angle += 3;
			if(_angle >= 360)
				_angle -=360;
			break;

		case GLUT_KEY_RIGHT: // look right
			_angle -= 3;
			if(_angle >= 360)
				_angle -=360;
			break;

		default:
			printf ("Special key %d pressed. No action there yet.\n", key);
			break;
	}
}
void keyPressed(unsigned char key, int x, int y)
{
	/* avoid thrashing this procedure */
	usleep(100);

	switch (key) {
		case ESCAPE: // kill everything.
			/* exit the program...normal termination. */
			exit(1);
			break; // redundant.

		case 'f':
		case 'F': // switch the filter
			printf("F/f pressed; filter is: %d\n", filter);
			filter++;                           // switch the current value of filter, between 0/1/2;
			if (filter > 2) {
				filter = 0;
			}
			printf("Filter is now: %d\n", filter);
			break;

		case 'l':
		case 'L': // switch the lighting
			printf("L/l pressed; lighting is: %d\n", light);
			light = light ? 0 : 1;              // switch the current value of light, between 0 and 1.
			if (light) {
				glEnable(GL_LIGHTING);
			} else {
				glDisable(GL_LIGHTING);
			}
			printf("Lighting is now: %d\n", light);
			break;

		case 'v':
		case 'V':
			view_mode++;
			view_mode%=5;
			break;
		case 'n':
		case 'N':
			if(view_mode == 4)
			{
				tile_index++;
				tile_index-=2;
				tile_index%=100;
				tile_index+=2;
			}
			break;
		case 'p':
		case 'P':
			if(view_mode == 4)
			{
				tile_index--;
				if(tile_index < 2)
				{
					tile_index +=100;
				}
			}
			break;
		case 'j':
		case 'J':
			if(fall_flag==0)
			{
				jump_flag = 1;
				yjump = 0;
				tjump = 0;
				direction = 5;
				offset = 5;
			}
			break;
		case 'w':
		case 'W':
			lookup += 0.1;
			break;
		case 's':
		case 'S':
			lookup -= 0.1;
			break;
		default:
			break;
	}
}

void specialKeyRelease (int key, int x, int y)
{
	if ( key == GLUT_KEY_UP || key == GLUT_KEY_DOWN )
	{
		direction = 0;
	}
}

void mouse2(int button, int state, int x, int y)
{
	if(view_mode ==3)
	{
		if(state ==1)
		{
			x_look_helicopter = (x - w/2)/3.0;
			z_look_helicopter = (y - h/2)/3.0;
		}
	}
	return;
}
void mousePassive(int x, int y)
{
	if(view_mode == 3)
	{
		x_helicopter = (x - w/2)/3.0;
		z_helicopter = (y - h/2)/3.0;
		x_look_helicopter = x_helicopter;
		z_look_helicopter = z_helicopter;
	}
}

void initRendering()
{
	LoadGLTextures();

	glEnable(GL_TEXTURE_2D);                    // Enable texture mapping.
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);          // Set the blending function for translucency (note off at init time)
	glClearColor(115.0/255.0, 210.0/255.0, 249.0/255.0, 0.0f);	// This Will Clear The Background Color To Black
//	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);	// This Will Clear The Background Color To Black
	glClearDepth(1.0);				// Enables Clearing Of The Depth Buffer
	glDepthFunc(GL_LESS);                       // type of depth test to do.
	glEnable(GL_DEPTH_TEST);                    // enables depth testing.
	glShadeModel(GL_SMOOTH);			// Enables Smooth Color Shading

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();				// Reset The Projection Matrix

	gluPerspective(45.0f,(GLfloat)w/(GLfloat)h,0.1f,100.0f);	// Calculate The Aspect Ratio Of The Window

	glMatrixMode(GL_MODELVIEW);

//	glEnable(GL_LIGHTING);
	// set up lights.
//	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
	//glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
	//glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
//	glEnable(GL_LIGHT0);
//	glEnable(GL_LIGHT1);
//	glEnable(GL_LIGHT2);

	quad = gluNewQuadric ();

}

void handleResize(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (float)w / (float)h, 0.3, 200.0);

	glMatrixMode(GL_MODELVIEW);
}

void drawbox(box b) // for debugging
{
	glColor3f(1.0f, 1.0f , 0.0f);

	glBegin (GL_LINE_LOOP);
	glVertex3f(b.max.x,b.max.y,b.min.z); // 0
	glVertex3f(b.min.x,b.max.y,b.min.z); // 1
	glVertex3f(b.min.x,b.min.y,b.min.z); // 2
	glVertex3f(b.max.x,b.min.y,b.min.z); // 3
	glEnd();

	glBegin (GL_LINE_LOOP);
	glVertex3f(b.max.x,b.min.y,b.max.z); // 4
	glVertex3f(b.max.x,b.max.y,b.max.z); // 5
	glVertex3f(b.min.x,b.max.y,b.max.z); // 6
	glVertex3f(b.min.x,b.min.y,b.max.z); // 7
	glEnd();

	glBegin (GL_LINE_LOOP);
	glVertex3f(b.max.x,b.max.y,b.min.z); // 0
	glVertex3f(b.max.x,b.max.y,b.max.z); // 5
	glVertex3f(b.min.x,b.max.y,b.max.z); // 6
	glVertex3f(b.min.x,b.max.y,b.min.z); // 1
	glEnd();

	glBegin (GL_LINE_LOOP);
	glVertex3f(b.max.x,b.min.y,b.max.z); // 4
	glVertex3f(b.min.x,b.min.y,b.max.z); // 7
	glVertex3f(b.min.x,b.min.y,b.min.z); // 2
	glVertex3f(b.max.x,b.min.y,b.min.z); // 3
	glEnd();

}
float rad(float angle)
{
	return angle * PI / 180.0;
}

void readstr(FILE *f, char *string)
{
	do {
		fgets(string, 255, f); // read the line
	} while ((string[0] == '/') || (string[0] == '\n'));
	return;
}
void create_support(int id, int indx, int flag2)
{
	int i, j;
	float xleft, zleft;
	for(i=0; i<n; i++)
	{
		for(j=0; j<m; j++)
		{
			xleft = 1.0 * tile_width * i - tile_width / 2.0 + i * tile_dist;
			zleft = 1.0 * tile_height * j - tile_height / 2.0 + j * tile_dist;



			indx = add_to_triangles(xleft, zleft, indx, id-1, 0);
			id++;

			indx = add_to_triangles(xleft + tile_height, zleft, indx, id-1, 0);
			id++;

			indx = add_to_triangles(xleft, zleft, indx, id-1, 1);
			id++;

			indx = add_to_triangles(xleft , zleft + tile_width, indx, id-1, 1);


			if(flag2==0)
			{
				sector1.numtriangles+=8;
				numtriangles+=8;
			}

		}
	}
}
int add_to_triangles(float xleft, float zleft, int indx, int id, int flag)
{
	float offset1, offset2;
	if(flag==0)
	{
		offset1 = xleft;
		offset2 = zleft + tile_width;
	}
	else if(flag==1)
	{
		offset1 = xleft + tile_width;
		offset2 = zleft;

	}
			indx++;
			sector1.triangle[indx].tex = 6;
			sector1.triangle[indx].id = id;

			sector1.triangle[indx].vertex[0].x = xleft;
			sector1.triangle[indx].vertex[0].z = zleft;
			sector1.triangle[indx].vertex[0].y = 0;
			sector1.triangle[indx].vertex[0].u = 0;
			sector1.triangle[indx].vertex[0].v = 6;

			sector1.triangle[indx].vertex[1].x = offset1;
			sector1.triangle[indx].vertex[1].z = offset2;
			sector1.triangle[indx].vertex[1].y = 0;
			sector1.triangle[indx].vertex[1].u = 0;
			sector1.triangle[indx].vertex[1].v = 0;

			sector1.triangle[indx].vertex[2].x = xleft;
			sector1.triangle[indx].vertex[2].z = zleft;
			sector1.triangle[indx].vertex[2].y = -10;
			sector1.triangle[indx].vertex[2].u = 6;
			sector1.triangle[indx].vertex[2].v = 0;

			indx++;
			sector1.triangle[indx].tex = 6;
			sector1.triangle[indx].id = id;

			sector1.triangle[indx].vertex[0].x = offset1;
			sector1.triangle[indx].vertex[0].z = offset2;
			sector1.triangle[indx].vertex[0].y = 0;
			sector1.triangle[indx].vertex[0].u = 0;
			sector1.triangle[indx].vertex[0].v = 6;

			sector1.triangle[indx].vertex[1].x = xleft;
			sector1.triangle[indx].vertex[1].z = zleft;
			sector1.triangle[indx].vertex[1].y = -10;
			sector1.triangle[indx].vertex[1].u = 0;
			sector1.triangle[indx].vertex[1].v = 0;

			sector1.triangle[indx].vertex[2].x = offset1;
			sector1.triangle[indx].vertex[2].z = offset2;
			sector1.triangle[indx].vertex[2].y = -10;
			sector1.triangle[indx].vertex[2].u = 6;
			sector1.triangle[indx].vertex[2].v = 0;
			return indx;

}

void create_connecting(int id, int indx, int flag, float offset, int flag2)
{
	int i=0, j=0, rand1, rand2;
	float xleft, zleft, y;
	for(i=0; i<n-1; i++)
	{
		for(j=0; j<m-1; j++)
		{
			if(flag2==0)
			{
				rand1 = rand()%100;
				rand2 = rand()%2;
				if(rand2 == 0)
					rand1*=-1;
				y = rand1/200.0;
			}

			//y = 0;
			xleft = 1.0 * tile_width * i - tile_width / 2.0 + i * tile_dist ;
			zleft = 1.0 * tile_height * j - tile_height / 2.0 + j * tile_dist ;
			if(flag2==0)
			{
				offset = rand() % 400;
				offset/=100;
			}

			if(flag==0)
			{
				xleft = xleft + tile_width/2 - 1;
				zleft = zleft + tile_height + offset;
			}
			else if(flag==1)
			{
				xleft = xleft + tile_width + offset;
				zleft = zleft + tile_height/2 - 1;
			}

			indx++;

			sector1.triangle[indx].tex = 5;
			sector1.triangle[indx].id = id;

			sector1.triangle[indx].vertex[0].x = xleft;
			sector1.triangle[indx].vertex[0].z = zleft;
			if(!flag2)
			sector1.triangle[indx].vertex[0].y = y;

			sector1.triangle[indx].vertex[0].u = 0;
			sector1.triangle[indx].vertex[0].v = 6;

			sector1.triangle[indx].vertex[1].x = xleft;
			sector1.triangle[indx].vertex[1].z = zleft+ tile_height/3;
			if(!flag2)
			sector1.triangle[indx].vertex[1].y = y;

			sector1.triangle[indx].vertex[1].u = 0;
			sector1.triangle[indx].vertex[1].v = 0;

			sector1.triangle[indx].vertex[2].x = xleft+tile_width/3;
			sector1.triangle[indx].vertex[2].z = zleft+tile_height/3;
			if(!flag2)
			sector1.triangle[indx].vertex[2].y = y;

			sector1.triangle[indx].vertex[2].u = 6;
			sector1.triangle[indx].vertex[2].v = 0;

			indx++;

			sector1.triangle[indx].tex = 5;
			sector1.triangle[indx].id = id;

			sector1.triangle[indx].vertex[0].x = xleft;
			sector1.triangle[indx].vertex[0].z = zleft;
			if(!flag2)
			sector1.triangle[indx].vertex[0].y = y;

			sector1.triangle[indx].vertex[0].u = 0;
			sector1.triangle[indx].vertex[0].v = 6;

			sector1.triangle[indx].vertex[1].x = xleft+tile_width/3;
			sector1.triangle[indx].vertex[1].z = zleft;
			if(!flag2)
			sector1.triangle[indx].vertex[1].y = y;

			sector1.triangle[indx].vertex[1].u = 0;
			sector1.triangle[indx].vertex[1].v = 0;

			sector1.triangle[indx].vertex[2].x = xleft+tile_width/3;
			sector1.triangle[indx].vertex[2].z = zleft+tile_height/3;
			if(!flag2)
			sector1.triangle[indx].vertex[2].y = y;

			sector1.triangle[indx].vertex[2].u = 6;
			sector1.triangle[indx].vertex[2].v = 0;

			if(flag2==0)
			{

				sector1.numtriangles+=2;
				numtriangles+=2;
			}
			id++;


		}
	}
	if(!flag)
	{
		indx2 = indx;
		id2 = id;
	}
	else
	{
		indx3 = indx;
		id3 = id;
	}
}



void generate_filter(Image* image1, GLuint* texture, char *name)
{
	image1 = (Image *) malloc(sizeof(Image));
	if (image1 == NULL) {
		printf("Error allocating space for image");
		exit(0);
	}
	if (!ImageLoad(name, image1)) {
		exit(1);
	}
	glGenTextures(3, &texture[0]);

	// nearest filtered texture
	glBindTexture(GL_TEXTURE_2D, texture[0]);   // 2d texture (x and y size)
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST); // scale cheaply when image bigger than texture
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST); // scale cheaply when image smalled than texture
	glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->sizeX, image1->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image1->data);

	// linear filtered texture
	glBindTexture(GL_TEXTURE_2D, texture[1]);   // 2d texture (x and y size)
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // scale linearly when image bigger than texture
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // scale linearly when image smalled than texture
	glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->sizeX, image1->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image1->data);

	// mipmapped texture
	glBindTexture(GL_TEXTURE_2D, texture[2]);   // 2d texture (x and y size)
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // scale linearly when image bigger than texture
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST); // scale mipmap when image smalled than texture
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, image1->sizeX, image1->sizeY, GL_RGB, GL_UNSIGNED_BYTE, image1->data);

}
GLvoid LoadGLTextures()
{
	Image *image1;
	char arr[100];
	strcpy(arr,"Data/grass.bmp");
	generate_filter(image1, texture, arr);

	Image *image2;
	strcpy(arr,"Data/sky.bmp");
	generate_filter(image2, texture2, arr);

	Image *image3;
	strcpy(arr,"Data/ground.bmp");
	generate_filter(image3, texture3, arr);

	Image *image4;
	strcpy(arr,"Data/water4.bmp");
	generate_filter(image4, texture4, arr);

	Image *image5;
	strcpy(arr,"Data/ground.bmp");
	generate_filter(image5, texture5, arr);

	Image *image6;
	strcpy(arr,"Data/mud3.bmp");
	generate_filter(image6, texture6, arr);

	Image *image7;
	strcpy(arr,"Data/minidoor.bmp");
	generate_filter(image7, texture7, arr);

};

GLvoid DrawGLScene()
{
	glPushMatrix();
	//glDisable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);
		glTranslatef(-50, 6, -90);
				glColor3f(1, 1, 1);
		glColor3f(255.0/255.0, 255.0/255.0, 0.0/255.0);

		gluDisk (quad, 0.0, 3, 18, 1);

	//	glEnable(GL_COLOR_MATERIAL);
		glColor3f(1, 1, 1);
		glDisable(GL_COLOR_MATERIAL);
	//	glEnable(GL_LIGHTING);
		glPopMatrix();

	GLfloat x_m, y_m, z_m, u_m, v_m;
	int numtriangles;

	numtriangles = sector1.numtriangles;

	for (loop=0; loop<(int)numtriangles; loop++) {        // loop through all the triangles
		if(sector1.triangle[loop].tex==1)
		{
			glEnable(GL_TEXTURE_2D); // Enable texture mapping.
			glBindTexture(GL_TEXTURE_2D, texture[filter]); // 2d texture (x and y size)
		}
		else if(sector1.triangle[loop].tex==2)
		{
			glEnable(GL_TEXTURE_2D); // Enable texture mapping.
			glBindTexture(GL_TEXTURE_2D, texture2[2]); // 2d texture (x and y size)
		}
		else if(sector1.triangle[loop].tex==3)
		{
			glEnable(GL_TEXTURE_2D); // Enable texture mapping.
			glBindTexture(GL_TEXTURE_2D, texture3[filter]); // pick the texture.
		}
		else if(sector1.triangle[loop].tex==4)
		{
			glEnable(GL_TEXTURE_2D); // Enable texture mapping.
			glBindTexture(GL_TEXTURE_2D, texture4[2]); // 2d texture (x and y size)
		}

		else if(sector1.triangle[loop].tex==5)
		{
		//	glEnable(GL_TEXTURE_2D); // Enable texture mapping.
		//	glBindTexture(GL_TEXTURE_2D, texture5[2]); // 2d texture (x and y size)
			glDisable(GL_TEXTURE_2D);
			glEnable(GL_COLOR_MATERIAL);
			glColor3f(85/255.0, 85/255.0, 85/255.0);

		}

		else if(sector1.triangle[loop].tex==6)
		{
			glEnable(GL_TEXTURE_2D); // Enable texture mapping.
			if((sector1.triangle[loop].id)%2==0)
				glBindTexture(GL_TEXTURE_2D, texture6[2]); // 2d texture (x and y size)
			else
				glBindTexture(GL_TEXTURE_2D, texture6[0]); // 2d texture (x and y size)
		}

		else if(sector1.triangle[loop].tex==7)
		{
			glEnable(GL_TEXTURE_2D); // Enable texture mapping.
			glBindTexture(GL_TEXTURE_2D, texture7[filter]); // 2d texture (x and y size)
		}

		if(sector1.triangle[loop].tex!=5)
		{
		glBegin(GL_TRIANGLES);
		glNormal3f( 0.0f, 0.0f, 1.0f);

		x_m = sector1.triangle[loop].vertex[0].x;
		y_m = sector1.triangle[loop].vertex[0].y;
		z_m = sector1.triangle[loop].vertex[0].z;
		u_m = sector1.triangle[loop].vertex[0].u;
		v_m = sector1.triangle[loop].vertex[0].v;
		glTexCoord2f(u_m,v_m);
		glVertex3f(x_m,y_m,z_m);

		x_m = sector1.triangle[loop].vertex[1].x;
		y_m = sector1.triangle[loop].vertex[1].y;
		z_m = sector1.triangle[loop].vertex[1].z;
		u_m = sector1.triangle[loop].vertex[1].u;
		v_m = sector1.triangle[loop].vertex[1].v;
		glTexCoord2f(u_m,v_m);
		glVertex3f(x_m,y_m,z_m);

		x_m = sector1.triangle[loop].vertex[2].x;
		y_m = sector1.triangle[loop].vertex[2].y;
		z_m = sector1.triangle[loop].vertex[2].z;
		u_m = sector1.triangle[loop].vertex[2].u;
		v_m = sector1.triangle[loop].vertex[2].v;
		glTexCoord2f(u_m,v_m);
		glVertex3f(x_m,y_m,z_m);

		glEnd();
		}
		else
		{
			glPushMatrix();
			glTranslatef(sector1.triangle[loop].vertex[0].x, sector1.triangle[loop].vertex[0].y, sector1.triangle[loop].vertex[0].z);
			glTranslatef(tile_width/6, 0, tile_height/6);
			glRotatef(90, 1, 0, 0);
			disc_angle+= disc_angle_offset;
			if(disc_angle > 360)
				disc_angle -= 360;
			glRotatef(disc_angle, 0, 0, 1);
			glRectf(-tile_width/6,tile_width/6, tile_height/6, -tile_height/6);
			glPopMatrix();
		}

		if(sector1.triangle[loop].tex==5)
		{


			glPushMatrix();
			glTranslatef(sector1.triangle[loop].vertex[0].x, sector1.triangle[loop].vertex[0].y, sector1.triangle[loop].vertex[0].z);
			glTranslatef(tile_width/6, 0.01, tile_height/6);
			glRotatef(90, 1, 0, 0);

			disc_color+= disc_offset;
			if(disc_color > 250)
				disc_offset = disc_offset * -1;
			if(disc_color < 50)
				disc_offset = disc_offset * -1;
			glColor3f(disc_color/ 255.0, 0, 0);

			gluDisk (quad, 0.3, 0.5, 18, 2);

			glEnable(GL_COLOR_MATERIAL);
			glColor3f(1, 1, 1);
			glDisable(GL_COLOR_MATERIAL);

			glPopMatrix();

		}


	}
}

static unsigned int getint(FILE *fp)
{
	int c, c1, c2, c3;

	// get 4 bytes
	c = getc(fp);
	c1 = getc(fp);
	c2 = getc(fp);
	c3 = getc(fp);

	return ((unsigned int) c) +
		(((unsigned int) c1) << 8) +
		(((unsigned int) c2) << 16) +
		(((unsigned int) c3) << 24);
}
static unsigned int getshort(FILE *fp)
{
	int c, c1;

	//get 2 bytes
	c = getc(fp);
	c1 = getc(fp);

	return ((unsigned int) c) + (((unsigned int) c1) << 8);
}
int ImageLoad(char *filename, Image *image)
{
	FILE *file;
	unsigned long size;                 // size of the image in bytes.
	unsigned long i;                    // standard counter.
	unsigned short int planes;          // number of planes in image (must be 1)
	unsigned short int bpp;             // number of bits per pixel (must be 24)
	char temp;                          // used to convert bgr to rgb color.

	// make sure the file is there.
	if ((file = fopen(filename, "rb"))==NULL) {
		printf("File Not Found : %s\n",filename);
		return 0;
	}

	// seek through the bmp header, up to the width/height:
	fseek(file, 18, SEEK_CUR);

	// No 100% errorchecking anymore!!!

	// read the width
	image->sizeX = getint (file);

	// read the height
	image->sizeY = getint (file);

	// calculate the size (assuming 24 bits or 3 bytes per pixel).
	size = image->sizeX * image->sizeY * 3;

	// read the planes
	planes = getshort(file);
	if (planes != 1) {
		printf("Planes from %s is not 1: %u\n", filename, planes);
		return 0;
	}

	// read the bpp
	bpp = getshort(file);
	if (bpp != 24) {
		printf("Bpp from %s is not 24: %u\n", filename, bpp);
		return 0;
	}

	// seek past the rest of the bitmap header.
	fseek(file, 24, SEEK_CUR);

	// read the data.
	image->data = (char *) malloc(size);
	if (image->data == NULL) {
		printf("Error allocating memory for color-corrected image data");
		return 0;
	}

	if ((i = fread(image->data, size, 1, file)) != 1) {
		printf("Error reading image data from %s.\n", filename);
		return 0;
	}

	for (i=0;i<size;i+=3) { // reverse all of the colors. (bgr -> rgb)
		temp = image->data[i];
		image->data[i] = image->data[i+2];
		image->data[i+2] = temp;
	}
	return 1;
}
void loadmodel()
{
	char *temp = new char[20];
	if (!pmodel1)
	{
		// this is the call that actualy reads the OBJ and creates the model object
		strcpy(temp, "steve2.obj");

		pmodel1 = glmReadOBJ(temp);
		if (!pmodel1) exit(0);
		glmUnitize(pmodel1);
		glmFacetNormals(pmodel1);
		glmVertexNormals(pmodel1, 90.0);
	}
//	boxed_object object;
//	strcpy(object.name, temp);
//	boxed_obj.push_back(object);
//	boxindex++;
}
void check_warnings(GLMmodel* model, GLuint mode)
{
	/* do a bit of warning */
	if (mode & GLM_FLAT && !model->facetnorms) {
		printf("glmDraw() warning: flat render mode requested "
				"with no facet normals defined.\n");
		mode &= ~GLM_FLAT;
	}
	if (mode & GLM_SMOOTH && !model->normals) {
		printf("glmDraw() warning: smooth render mode requested "
				"with no normals defined.\n");
		mode &= ~GLM_SMOOTH;
	}
	if (mode & GLM_TEXTURE && !model->texcoords) {
		printf("glmDraw() warning: texture render mode requested "
				"with no texture coordinates defined.\n");
		mode &= ~GLM_TEXTURE;
	}
	if (mode & GLM_FLAT && mode & GLM_SMOOTH) {
		printf("glmDraw() warning: flat render mode requested "
				"and smooth render mode requested (using smooth).\n");
		mode &= ~GLM_FLAT;
	}
	if (mode & GLM_COLOR && !model->materials) {
		printf("glmDraw() warning: color render mode requested "
				"with no materials defined.\n");
		mode &= ~GLM_COLOR;
	}
	if (mode & GLM_MATERIAL && !model->materials) {
		printf("glmDraw() warning: material render mode requested "
				"with no materials defined.\n");
		mode &= ~GLM_MATERIAL;
	}
	if (mode & GLM_COLOR && mode & GLM_MATERIAL) {
		printf("glmDraw() warning: color and material render mode requested "
				"using only material mode.\n");
		mode &= ~GLM_COLOR;
	}
	if (mode & GLM_COLOR)
		glEnable(GL_COLOR_MATERIAL);
	else if (mode & GLM_MATERIAL)
		glDisable(GL_COLOR_MATERIAL);

}
