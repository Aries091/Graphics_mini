#include <windows.h>
#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#define PI 3.14152653597689786
#define RandomFactor 2.0
#define ESCAPE 27
#define TEXTID 3

unsigned int i;
int flag = 0, f = 2;
int vflag = 0;
GLfloat xt = 0.0, yt = 0.0, zt = 0.0;
GLfloat xangle = 0.0, yangle = 0.0, zangle = 0.0;
GLfloat X[3];
GLint ListNum; //The number of the display list
GLfloat OuterRadius = 2.4; //reservoir
GLfloat InnerRadius = 2.0;
GLint NumOfVerticesStone = 6; // reservoir shape
GLfloat StoneHeight = 0.5;
GLfloat WaterHeight = 0.45;

struct SVertex {
    GLfloat x, y, z;
};

class CDrop {
private:
    GLfloat time;
    SVertex ConstantSpeed;
    GLfloat AccFactor;

public:
    void SetConstantSpeed(SVertex NewSpeed);
    void SetAccFactor(GLfloat NewAccFactor);
    void SetTime(GLfloat NewTime);
    void GetNewPosition(SVertex *PositionVertex); //increments time, gets the new position
};

void CDrop::SetConstantSpeed(SVertex NewSpeed)
{
    ConstantSpeed = NewSpeed;
}

void CDrop::SetAccFactor(GLfloat NewAccFactor)
{
    AccFactor = NewAccFactor;
}

void CDrop::SetTime(GLfloat NewTime)
{
    time = NewTime;
}

void CDrop::GetNewPosition(SVertex *PositionVertex)
{
    SVertex Position;
    time += 0.15;
    Position.x = ConstantSpeed.x * time;
    Position.y = ConstantSpeed.y * time - AccFactor * time * time;
    Position.z = ConstantSpeed.z * time;
    PositionVertex->x = Position.x;
    PositionVertex->y = Position.y + WaterHeight;
    PositionVertex->z = Position.z;

    if (Position.y < 0.0)
    {
        time = time - int(time);
        if (time > 0.0) time -= 1.0;
    }
}

CDrop *FountainDrops;
SVertex *FountainVertices;
GLint Steps = 4; //a fountain has several steps, each with its own height
GLint RaysPerStep = 8;
GLint DropsPerRay = 80;
GLfloat DropsComplete = Steps * RaysPerStep * DropsPerRay;
GLfloat AngleOfDeepestStep = 80;
GLfloat AccFactor = 0.017; // Creating reservoir boundary

void CreateList(void)
{
    SVertex *Vertices = new SVertex[NumOfVerticesStone * 3]; //allocate mem for the required vertices
    ListNum = glGenLists(1);
    for (GLint i = 0; i < NumOfVerticesStone; i++)
    {
        Vertices[i].x = cos(2.0 * PI / NumOfVerticesStone * i) * OuterRadius;
        Vertices[i].y = StoneHeight; //Top
        Vertices[i].z = sin(2.0 * PI / NumOfVerticesStone * i) * OuterRadius;
    }
    for (GLint i = 0; i < NumOfVerticesStone; i++)
    {
        Vertices[i + NumOfVerticesStone * 1].x = cos(2.0 * PI / NumOfVerticesStone * i) * InnerRadius;
        Vertices[i + NumOfVerticesStone * 1].y = StoneHeight; //Top
        Vertices[i + NumOfVerticesStone * 1].z = sin(2.0 * PI / NumOfVerticesStone * i) * InnerRadius;
    }
    for (GLint i = 0; i < NumOfVerticesStone; i++)
    {
        Vertices[i + NumOfVerticesStone * 2].x = cos(2.0 * PI / NumOfVerticesStone * i) * OuterRadius;
        Vertices[i + NumOfVerticesStone * 2].y = 0.0; //Bottom
        Vertices[i + NumOfVerticesStone * 2].z = sin(2.0 * PI / NumOfVerticesStone * i) * OuterRadius;
    }
    glNewList(ListNum, GL_COMPILE);
    glBegin(GL_QUADS);
    //ground quad:
    glColor3ub(0, 105, 0);
    glVertex3f(-OuterRadius * 10.0, 0.0, OuterRadius * 10.0);
    glVertex3f(-OuterRadius * 10.0, 0.0, -OuterRadius * 10.0);
    glVertex3f(OuterRadius * 10.0, 0.0, -OuterRadius * 10.0);
    glVertex3f(OuterRadius * 10.0, 0.0, OuterRadius * 10.0);
    //stone:
     for (int j = 1; j < 3; j++) {
        if (j == 1)
            glColor3f(0, 0, 0.78);
        if (j == 2) glColor3f(1.7, 0.2, 0.6);
        for (GLint i = 0; i < NumOfVerticesStone - 1; i++) {
            glVertex3fv(&Vertices[i + NumOfVerticesStone * j].x);
            glVertex3fv(&Vertices[i].x);
            glVertex3fv(&Vertices[i + 1].x);
            glVertex3fv(&Vertices[i + NumOfVerticesStone * j + 1].x);
        }
        // Closing the last side
        glVertex3fv(&Vertices[NumOfVerticesStone * j + NumOfVerticesStone - 1].x);
        glVertex3fv(&Vertices[NumOfVerticesStone - 1].x);
        glVertex3fv(&Vertices[0].x);
        glVertex3fv(&Vertices[NumOfVerticesStone * j].x);
    }
    glEnd();
    //The "water":
    glTranslatef(0.0, WaterHeight - StoneHeight, 0.0);
    glBegin(GL_POLYGON);
    for (GLint i = 0; i < NumOfVerticesStone; i++)
    {
        glVertex3fv(&Vertices[i + NumOfVerticesStone].x);
        GLint m1, n1, p1;
        m1 = rand() % 255;
        n1 = rand() % 255;
        p1 = rand() % 255;
        glColor3ub(m1, n1, p1);
        glColor3f(1.0, 1.0, 1.0);
    }
    glEnd();
    glEndList();
}

GLfloat GetRandomFloat(GLfloat range)
{
    return (GLfloat)rand() / (GLfloat)RAND_MAX * range * RandomFactor;
}

void InitFountain(void)
{
    //This function needn't be and isn't speed optimized
    FountainDrops = new CDrop[(int)DropsComplete];
    FountainVertices = new SVertex[(int)DropsComplete];

    SVertex NewSpeed;
    GLfloat DropAccFactor; //different from AccFactor because of the random change
    GLfloat TimeNeeded;
    GLfloat StepAngle; //Angle, which the ray gets out of the fountain with
    GLfloat RayAngle; //Angle you see when you look down on the fountain

    for (unsigned int k = 0; k < static_cast<unsigned int>(Steps); k++)
    {
        for (unsigned int j = 0; j < static_cast<unsigned int>(RaysPerStep); j++)
        {
            for (unsigned int i = 0; i < static_cast<unsigned int>(DropsPerRay); i++)
            {
                DropAccFactor = AccFactor + GetRandomFloat(0.0005);
                StepAngle = AngleOfDeepestStep + 0.0 + (90.0 - AngleOfDeepestStep + 0.0) * GLfloat(k) / (Steps - 1) + GetRandomFloat(0.25 + 0.8 * (Steps - k - 1) / (Steps - 1));
                NewSpeed.x = cos(StepAngle * PI / 180.0) * (0.0 + 0.25 + 0.0375 * k);
                NewSpeed.y = sin(StepAngle * PI / 180.0) * (0.0 + 0.25 + 0.0375 * k);
                RayAngle = (GLfloat)j / (GLfloat)RaysPerStep * 360.0;
                NewSpeed.z = NewSpeed.x * sin(RayAngle * PI / 180.0);
                NewSpeed.x = NewSpeed.x * cos(RayAngle * PI / 180.0);
                TimeNeeded = -2.0 * NewSpeed.y / (2.0 * -DropAccFactor);

                FountainDrops[i + j * DropsPerRay + k * DropsPerRay * RaysPerStep].SetConstantSpeed(NewSpeed);
                FountainDrops[i + j * DropsPerRay + k * DropsPerRay * RaysPerStep].SetAccFactor(DropAccFactor);
                FountainDrops[i + j * DropsPerRay + k * DropsPerRay * RaysPerStep].SetTime(GetRandomFloat(TimeNeeded));
            }
        }
    }
}

void Init(void)
{
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    CreateList();
    InitFountain();
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_COLOR_MATERIAL);
}

void Reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat)width / (GLfloat)height, 1.0, 1000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0, 2.0, 12.0, 0.0, 2.0, 0.0, 0.0, 1.0, 0.0);
}

void Display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(xt, yt, -10.0 + zt);
    glRotatef(xangle, 1.0, 0.0, 0.0);
    glRotatef(yangle, 0.0, 1.0, 0.0);
    glRotatef(zangle, 0.0, 0.0, 1.0);
    glPushMatrix();
    glTranslatef(0.0, StoneHeight, 0.0);
    glCallList(ListNum);
    glPopMatrix();
    glBegin(GL_POINTS);
    for (unsigned int i = 0; i < DropsComplete; i++)
    {
        FountainDrops[i].GetNewPosition(&FountainVertices[i]);
        glVertex3fv(&FountainVertices[i].x);
        GLint m, n, p;
        m = rand() % 255;
        n = rand() % 255;
        p = rand() % 255;
        glColor3ub(m, n, p);
        glColor3f(0.0, 0.0, 1.0);
    }
    glEnd();
    glutSwapBuffers();
    glFlush();
}

void Animate(int value)
{
    glutPostRedisplay();
    glutTimerFunc(25, Animate, 0);
}

void Key(unsigned char key, int x, int y)
{
    switch (key)
    {
    case ESCAPE:
        exit(0);
        break;
        //for translating the structure
    case 'e': yt += 0.2; break;
    case 'q': yt -= 0.2; break;
    case 'w': xt -= 0.2; break;
    case 's': xt += 0.2; break;
    case 'a': zt += 0.2; break;
    case 'd': zt -= 0.2; break;
    //for rotating the stucture
    case 'i': xangle += 2.0; break;
    case 'k': xangle -= 2.0; break;
    case 'j': yangle += 2.0; break;
    case 'l': yangle -= 2.0; break;
    case 'u': zangle += 2.0; break;
    case 'o': zangle -= 2.0; break;
    }
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 800);
    glutCreateWindow("Water Fountain");
    Init();
    glutDisplayFunc(Display);
    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Key);
    glutTimerFunc(25, Animate, 0);
    glutMainLoop();
    return 0;
}
