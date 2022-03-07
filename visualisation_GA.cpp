#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>

using namespace std;    

// Visualisation of BSD and CF model obtained by the Genetic Algorithm (GA)


// Create multidimensional vector with corner-vertices of BSD or conformal model
vector<vector<vector<float>>> BSD()   
{
    // Load predicted data
    vector<string> predictedCoordinatesString;
    string coordinate;
    ifstream predictC("cf.txt");                    // "bsd.txt" or "cf.txt" : corner-vertices of BSD or CF model
    while ( !predictC.eof() )
    {
        while ( getline(predictC,coordinate) )
        {
            predictedCoordinatesString.push_back(coordinate) ;
        }
    }
    // convert string to Int vector 
    vector<float> predictedCoordinatesFl;
    for (int i=0; i<predictedCoordinatesString.size(); i++)
    {
        float num = atoi(predictedCoordinatesString.at(i).c_str());
        predictedCoordinatesFl.push_back(num);
    }

    // Create multidimensional vector, each individual quad-hexahedron is listed in a vector
    vector<vector<vector<float>>> ncoordinates;
    int p = 0;
    for (int c = 0; c < predictedCoordinatesFl.size()/24; c++)               // number of quad-hexahedrons
    {
        vector<vector<float>> cuboidVec;
        for (int j = 0; j < 24; j+=3)                                        // number of numerical values per quad-hexahedron
        {   
            vector<float> pointVec = {predictedCoordinatesFl[p+j+0], predictedCoordinatesFl[p+j+1], predictedCoordinatesFl[p+j+2]};
            cuboidVec.push_back(pointVec); 
        }
        ncoordinates.push_back(cuboidVec);
        p+=24;
    }

    return ncoordinates;            // return corner-vertices of BSD or CF to use in VISUALISATION
}

vector<float> PointCloud()   
{
    vector<string> predictedCoordinatesString;
    string coordinate;
    ifstream predictC("pnts.txt");                  // "pnts.txt" : vertices in Point cloud
    while ( !predictC.eof() )
    {
        while ( getline(predictC,coordinate) )
        {
            predictedCoordinatesString.push_back(coordinate) ;
        }
    }
    // convert string to Int vector 
    vector<float> predictedCoordinatesFl;
    for (int i=0; i<predictedCoordinatesString.size(); i++)
    {
        float num = atoi(predictedCoordinatesString.at(i).c_str());
        predictedCoordinatesFl.push_back(num);
    }
    // Create multidimensional vector, each individual cuboid is listed in a vector
    vector<vector<vector<float>>> ncoordinates;
    int p = 0;
    for (int c = 0; c < predictedCoordinatesFl.size()/24; c++)              // number of cuboids
    {
        vector<vector<float>> cuboidVec;
        for (int j = 0; j < 24; j+=3)                                        // number of numerical values per cuboid
        {   
            vector<float> pointVec = {predictedCoordinatesFl[p+j+0], predictedCoordinatesFl[p+j+1], predictedCoordinatesFl[p+j+2]};
            cuboidVec.push_back(pointVec); 
        }
        ncoordinates.push_back(cuboidVec);
        p+=24;
    }
    return predictedCoordinatesFl;            // return corner-vertices of Point cloud to use in VISUALIZATION
}



//------------------------------------------------------------------------------------------------------
// VISUALIZE Rectangles with OpenGL/glut                    ***         https://gist.github.com/hkulekci/2300262
//------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#define KEY_ESC 27 /* GLUT doesn't supply this */


// --- Initial parameters ---
// mouse operation: rotate around object
    int mouseDown = 0;
    float xrot = 10.0f;
    float yrot = -10.0f;
    float xdiff = 10.0f;
    float ydiff = 10.0f;
// translate shape
    float tra_x = 0.0f;
    float tra_y = 0.0f;
    float tra_z = 0.0f;
// View
    float grow_shrink = 72.0f;                  // fovy & zoom in/out
    float resize_f = 1.0f;                      
// Output screen
    #define SCREEN_WIDTH 640
    #define SCREEN_HEIGHT 480


// --- DRAW ---

void drawPointCloud()
{
    vector<float> coorN = PointCloud(); 
    int norm = 10;                                    
    for (int c = 0; c < coorN.size(); c++)
    {
        coorN[c] = coorN[c] / norm;
    }  

    //glTranslatef(tra_x-0.3f, tra_y-0.3f, tra_z-0.3f);   
    //glTranslatef(tra_x, tra_y, tra_z); 

    glPointSize(10.0f);  //10?
    glBegin(GL_POINTS);
    glColor3f(0.0f, 0.0f, 0.0f);

    for (int c = 0; c < coorN.size(); c+=3)
    {
        glVertex3f(coorN[c+0],     coorN[c+2],     -coorN[c+1]);
    }

    glEnd(); 

    
}

void drawBuildingFILL() 
{
        vector<vector<vector<float>>> coorN = BSD();                     // call coordinates vector from BSD() Function
        int norm = 10;                                      // divide by 10000, all numbers between 0 and 1 (normalized)
        for (int c = 0; c < coorN.size(); c++)
        {
            for (int p = 0; p < 8; p++)
            {
                for (int j = 0; j < 3; j++)
                {
                    coorN[c][p][j] = coorN[c][p][j] / norm;
                }
            }
        }  
        //glTranslatef(tra_x-0.3f, tra_y-0.3f, tra_z-0.3f);                       // translate shape to middle of the screen 
        glTranslatef(tra_x, tra_y, tra_z); 
        glBegin(GL_QUADS);

        // Color(RED, GREEN, BLUE)    0,0,0 = BLACK
        //glColor3f(0.8f, 0.8f, 0.8f);       // INPUT: light grey
        glColor3f(0.6f, 0.7f, 0.7f);       // OUTPUT: middle light blue-grey
        //glColor3f(0.4f, 0.5f, 0.6f);       // PREDICTION: blue-grey
        
        int nShape = 13;
        int x = 0; int y = 2; int z = 1;
        for (int c = 0; c < coorN.size(); c++)
        {
        // // TOP
            glVertex3f(coorN[c][6][x],     coorN[c][6][y],     -coorN[c][6][z]);
            glVertex3f(coorN[c][2][x],     coorN[c][2][y],     -coorN[c][2][z]);
            glVertex3f(coorN[c][1][x],     coorN[c][1][y],     -coorN[c][1][z]);
            glVertex3f(coorN[c][5][x],     coorN[c][5][y],     -coorN[c][5][z]); 
        // BOTTOM
            glVertex3f(coorN[c][4][x],     coorN[c][4][y],     -coorN[c][4][z]);
            glVertex3f(coorN[c][0][x],     coorN[c][0][y],     -coorN[c][0][z]);
            glVertex3f(coorN[c][3][x],     coorN[c][3][y],     -coorN[c][3][z]);
            glVertex3f(coorN[c][7][x],     coorN[c][7][y],     -coorN[c][7][z]); 
        // FRONT
            glVertex3f(coorN[c][5][x],     coorN[c][5][y],     -coorN[c][5][z]);
            glVertex3f(coorN[c][1][x],     coorN[c][1][y],     -coorN[c][1][z]);
            glVertex3f(coorN[c][0][x],     coorN[c][0][y],     -coorN[c][0][z]);
            glVertex3f(coorN[c][4][x],     coorN[c][4][y],     -coorN[c][4][z]); 
        // BACK
            glVertex3f(coorN[c][7][x],     coorN[c][7][y],     -coorN[c][7][z]);
            glVertex3f(coorN[c][3][x],     coorN[c][3][y],     -coorN[c][3][z]);
            glVertex3f(coorN[c][2][x],     coorN[c][2][y],     -coorN[c][2][z]);
            glVertex3f(coorN[c][6][x],     coorN[c][6][y],     -coorN[c][6][z]); 
        // LEFT
            glVertex3f(coorN[c][1][x],     coorN[c][1][y],     -coorN[c][1][z]);
            glVertex3f(coorN[c][2][x],     coorN[c][2][y],     -coorN[c][2][z]);
            glVertex3f(coorN[c][3][x],     coorN[c][3][y],     -coorN[c][3][z]);
            glVertex3f(coorN[c][0][x],     coorN[c][0][y],     -coorN[c][0][z]); 
        // RIGHT
            glVertex3f(coorN[c][6][x],     coorN[c][6][y],     -coorN[c][6][z]);
            glVertex3f(coorN[c][5][x],     coorN[c][5][y],     -coorN[c][5][z]);
            glVertex3f(coorN[c][4][x],     coorN[c][4][y],     -coorN[c][4][z]);
            glVertex3f(coorN[c][7][x],     coorN[c][7][y],     -coorN[c][7][z]); 
        }
        glEnd(); 
}
void drawBuildingLINE()
{
        vector<vector<vector<float>>> coorN = BSD();                     // call coordinates vector from BSD() Function
        float norm = 10;                                      // divide by 10000, all numbers between 0 and 1 (normalized)
        for (int c = 0; c < coorN.size(); c++)
        {
            for (int p = 0; p < 8; p++)
            {
                for (int j = 0; j < 3; j++)
                {
                    coorN[c][p][j] = coorN[c][p][j] / norm;
                }
            }
        }  
        glBegin(GL_QUADS);
        glColor3f(0.0f, 0.0f, 0.0f);     // black

        int nShape = 13;
        int x = 0; int y = 2; int z = 1;
        for (int c = 0; c < coorN.size(); c++)
        {
        // TOP
            glVertex3f(coorN[c][6][x],     coorN[c][6][y],     -coorN[c][6][z]);
            glVertex3f(coorN[c][2][x],     coorN[c][2][y],     -coorN[c][2][z]);
            glVertex3f(coorN[c][1][x],     coorN[c][1][y],     -coorN[c][1][z]);
            glVertex3f(coorN[c][5][x],     coorN[c][5][y],     -coorN[c][5][z]); 
        // BOTTOM
            glVertex3f(coorN[c][4][x],     coorN[c][4][y],     -coorN[c][4][z]);
            glVertex3f(coorN[c][0][x],     coorN[c][0][y],     -coorN[c][0][z]);
            glVertex3f(coorN[c][3][x],     coorN[c][3][y],     -coorN[c][3][z]);
            glVertex3f(coorN[c][7][x],     coorN[c][7][y],     -coorN[c][7][z]); 
        // FRONT
            glVertex3f(coorN[c][5][x],     coorN[c][5][y],     -coorN[c][5][z]);
            glVertex3f(coorN[c][1][x],     coorN[c][1][y],     -coorN[c][1][z]);
            glVertex3f(coorN[c][0][x],     coorN[c][0][y],     -coorN[c][0][z]);
            glVertex3f(coorN[c][4][x],     coorN[c][4][y],     -coorN[c][4][z]); 
        // BACK
            glVertex3f(coorN[c][7][x],     coorN[c][7][y],     -coorN[c][7][z]);
            glVertex3f(coorN[c][3][x],     coorN[c][3][y],     -coorN[c][3][z]);
            glVertex3f(coorN[c][2][x],     coorN[c][2][y],     -coorN[c][2][z]);
            glVertex3f(coorN[c][6][x],     coorN[c][6][y],     -coorN[c][6][z]); 
        // LEFT
            glVertex3f(coorN[c][1][x],     coorN[c][1][y],     -coorN[c][1][z]);
            glVertex3f(coorN[c][2][x],     coorN[c][2][y],     -coorN[c][2][z]);
            glVertex3f(coorN[c][3][x],     coorN[c][3][y],     -coorN[c][3][z]);
            glVertex3f(coorN[c][0][x],     coorN[c][0][y],     -coorN[c][0][z]); 
        // RIGHT
            glVertex3f(coorN[c][6][x],     coorN[c][6][y],     -coorN[c][6][z]);
            glVertex3f(coorN[c][5][x],     coorN[c][5][y],     -coorN[c][5][z]);
            glVertex3f(coorN[c][4][x],     coorN[c][4][y],     -coorN[c][4][z]);
            glVertex3f(coorN[c][7][x],     coorN[c][7][y],     -coorN[c][7][z]); 
        }
        glEnd(); 
}

// --- INITIALIZATION ---
int init(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);                    // background color(1f, 1f, 1f) & opaque(transparency)
	//glEnable(GL_DEPTH_TEST);                                    // enable depth-test, which shape is in front of another?
	//glDepthFunc(GL_LEQUAL);                                     // type of depth-test: LESS or EQUAL: Passes if the fragment's depth value is less than or equal to the stored depth value.
	//glClearDepth(1.0f);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);                  // wireframe view (GL_LINE), or GL_POINT or GL_FILL
    glShadeModel(GL_SMOOTH);                                      // shading: _SMOOTH or _FLAT
  
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);

    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return 1;
}

// --- DISPLAY ---
void display(void) 
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

    // camera position:
	gluLookAt(  0.0f, 0.0f, 1.0f,           // EYE
	            0.0f, 0.0f, -0.5f,           // AT
	            0.0f, 1.0f, 0.0f    );      // UP, corresponds to y-axis

	glRotatef(xrot, 1.0f, 0.0f, 0.0f);
	glRotatef(yrot, 0.0f, 1.0f, 0.0f);

    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    drawBuildingFILL();
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawBuildingLINE();
    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    drawPointCloud();

	glFlush();
	glutSwapBuffers();
}

// --- RESHAPE ---
void resize(int w, int h) 
{                                   
    // MATRIXMODE( "specifies which stack is the target for subsequent matrix operations.")
    //              mode: MODELVIEW, PROJECTION, TEXTURE, COLOR
	glMatrixMode(GL_PROJECTION);                // To operate on the Projection Matrix
	glLoadIdentity();

    // VIEWPORT(     x, y:           lower left corner of the viewport rectangle, in pixels. Initial = (0,0),
    //               width, height:  of the viewport. When a GL context is first attached to a window, width and height are set to the dimensions of that window
	glViewport(0, 0, w, h);

    // Set up a perspective projection matrix
    // PERSPECTIVE(  fovy:   field of view angle,  
    //               aspect: ratio of x (width) to y (height),
    //               zNear:  distance from viewer to near clipping plane (always positive),
    //               zFar:   distance from viewer to far  clipping plane (always positive)   )
	//gluPerspective(grow_shrink, resize_f*SCREEN_WIDTH/SCREEN_HEIGHT, 0.0f, 2.0f);
    // ORTHOGRPHIC( left, right, bottom, top, nearVal, farVal )
    glOrtho(-1.5, 1.5, -1.5, 1.5, 0, 2);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// --- VIEW OPTIONS --- 
// ZOOM VIEW:  IN (I) / OUT (O)
void keyboard(unsigned char key, int x, int y)
{
	switch(key)
	{
		case 27 : 
			exit(1); 
			break;

        // FORWARD
        case 'w':
			tra_z += 0.1f;
			break;
        // BACKWARD
		case 's':
			tra_z -= 0.1f;
			break;
        // LEFT
		case 'a':
			tra_x -= 0.1f;
			break;
        // RIGHT
		case 'd':
			tra_x += 0.1f;
			break;

        // Zoom OUT
		case 'o':
			grow_shrink++;
			resize(SCREEN_WIDTH, SCREEN_HEIGHT);			
			break;
        // Zoom IN
		case 'i':
			grow_shrink--;
			resize(SCREEN_WIDTH, SCREEN_HEIGHT);			
			break;
	}
	glutPostRedisplay();
}

// ORBIT VIEW:   Rotate with left mouse button around object shape 
void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		mouseDown = 1;

		xdiff = x - yrot;
		ydiff = -y + xrot;
	}
	else
		mouseDown = 0;
}
void mouseMotion(int x, int y)
{
	if (mouseDown)
	{
		yrot = x - xdiff;
		xrot = y + ydiff;

		glutPostRedisplay();
	}
}


// --- MAIN FUNCTION ---
int main(int argc, char *argv[])
{
	glutInit(&argc, argv);

	glutInitWindowPosition(0, 0);
	glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);

	glutInitDisplayMode(GLUT_RGB);

	glutCreateWindow("--- Building ---");

	glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);
	glutReshapeFunc(resize);

	if (!init())
		return 1;

	glutMainLoop();

	return 0;
}   
  