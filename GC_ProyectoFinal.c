#include <GL/glut.h>
#include <string.h> 
#include <math.h>   
#include <stdlib.h>
#include <stdio.h>

//Texturas para los planetas
GLuint texMercurio = 0;
GLuint texVenus   = 0;
GLuint texTierra  = 0;  
GLuint texMarte   = 0;
GLuint texJupiter = 0;
GLuint texSaturno = 0;
GLuint texUrano   = 0;
GLuint texNeptuno = 0;

//Structs para cumplir con los requisitos del proyecto

//Struct para los planetas
typedef struct 
{
    float radio;         
    float distancia;    
    float velocidad; 
    float angulo;        
    float colorR, colorG, colorB;
} Planeta;

//Struct para la nave
typedef struct 
{
    float x, y, z;
    float rotacion;
} Nave;

//Struct para la lista
typedef struct Nodo 
{
    float posX, posY, posZ;
    struct Nodo* siguiente;
} Nodo;

//Struct para la pila
typedef struct NodoPila 
{
    float camX, camY, camZ;
    struct NodoPila* siguiente;
} NodoPila;

//Se usaron enum para la cola de escenas
enum Escenas 
{
    INTRO,              //0: Presentación (10s)
    DESPEDIDA,          //1: Se despiden (8s)
    VIAJE1,             //2: Viajando (4s)
    
    //Mercurio (2 escenas)
    MERCURIO_LEJOS,     //3: Vista del planeta (7s)
    MERCURIO_CERCA,     //4: Superficie con diálogo (9s)

    VIAJE2,             //5: Viajando (4s)

    //Venus (2 escenas)
    VENUS_LEJOS,        //6: Vista del planeta (7s)
    VENUS_CERCA,        //7: Superficie con diálogo (9s)

    VIAJE3,             //8: Viajando (4s)

    //Marte (2 escenas)
    MARTE_LEJOS,        //9: Vista del planeta (7s)
    MARTE_CERCA,        //10: Superficie con diálogo (9s)

    VIAJE4,             //11: Viajando (4s)

    //Jupiter (2 escenas)
    JUPITER_LEJOS,      //12: Vista del planeta (7s)
    JUPITER_CERCA,      //13: Superficie con diálogo (9s)

    VIAJE5,             //14: Viajando (4s)

    //Saturno (2 escenas)
    SATURNO_LEJOS,      //15: Vista del planeta (7s)
    SATURNO_CERCA,      //16: Superficie con diálogo (9s)

    REGRESO,            //17: Regresando (12s)
    CASTIGO,            //18: Castigados (14s)
    CREDITOS            //19: Fin (10s)
};


//Variables globales
//Cola de escenas (orden de la pelicula)
int cola[30];            //Arreglo que guarda las escenas en orden
int escenaActual = 0;    //En que escena estamos
int totalEscenas = 0;    //Cuantas escenas hay

//Control de tiempo
long tiempoInicio = 0;   //Cuando empezó la escena actual
long duracion = 0;       //Cuanto debe durar la escena
int pausado = 0;         //0 = corriendo, 1 = pausado

//Objetos del sistema solar
Planeta sol, mercurio, venus, tierra, marte, jupiter, saturno, urano, neptuno;
Nave nave;

float escalaNave = 0.4f;   //tamaño base de la nave
int naveConTripulacion = 1;   //1 = se dibujan adentro, 0 = nave vacia

//Estructuras
Nodo* listaFrames = NULL;      //Lista de frames
NodoPila* pilaCamara = NULL;   //Pila de cámaras

//Texturas
//Funcion para cargar texturas
GLuint cargarTexturaBMP(const char* filename) 
{
    FILE* f = fopen(filename, "rb");
    if (!f) {
        printf("No se pudo abrir la textura: %s\n", filename);
        return 0;
    }

    unsigned char header[54];
    if (fread(header, 1, 54, f) != 54) {
        printf("Archivo BMP invalido: %s\n", filename);
        fclose(f);
        return 0;
    }

    //Comprobar "BM"
    if (header[0] != 'B' || header[1] != 'M') {
        printf("No es un BMP valido: %s\n", filename);
        fclose(f);
        return 0;
    }

    unsigned int dataPos   = *(int*)&(header[0x0A]);
    unsigned int imageSize = *(int*)&(header[0x22]);
    int width  = *(int*)&(header[0x12]);
    int height = *(int*)&(header[0x16]);

    if (imageSize == 0) imageSize = width * height * 3;//24 bits
    if (dataPos == 0)   dataPos   = 54;

    unsigned char* data = (unsigned char*)malloc(imageSize);
    if (!data) {
        printf("No hay memoria para la textura: %s\n", filename);
        fclose(f);
        return 0;
    }

    fseek(f, dataPos, SEEK_SET);
    fread(data, 1, imageSize, f);
    fclose(f);

    //BMP viene en BGR, se pasa a RGB
    for (unsigned int i = 0; i < imageSize; i += 3) {
        unsigned char tmp = data[i];
        data[i]     = data[i + 2];
        data[i + 2] = tmp;
    }

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 width, height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, data);

    free(data);
    printf("Textura cargada: %s (id=%u)\n", filename, texID);
    return texID;
}


//Funciones 

//Funcion para cargar texturas
void cargarTexturasPlanetas() 
{
    texMercurio = cargarTexturaBMP("Texturas/Mercurio.bmp");
    texVenus    = cargarTexturaBMP("Texturas/Venus.bmp");
    texTierra   = cargarTexturaBMP("Texturas/Tierra.bmp");
    texMarte    = cargarTexturaBMP("Texturas/Marte.bmp");
    texJupiter  = cargarTexturaBMP("Texturas/Jupiter.bmp");
    texSaturno  = cargarTexturaBMP("Texturas/Saturno.bmp");
    texUrano    = cargarTexturaBMP("Texturas/Urano.bmp");
    texNeptuno  = cargarTexturaBMP("Texturas/Neptuno.bmp");
}

//Funciones de lista
void agregarALista(float x, float y, float z) 
{
    Nodo* nuevo = (Nodo*)malloc(sizeof(Nodo));
    nuevo->posX = x;
    nuevo->posY = y;
    nuevo->posZ = z;
    nuevo->siguiente = listaFrames;
    listaFrames = nuevo;
}

//Funcion para liberar la lista
void liberarLista() 
{
    while(listaFrames != NULL) 
    {
        Nodo* temp = listaFrames;
        listaFrames = listaFrames->siguiente;
        free(temp);
    }
}

//Funciones de pila
void pushCamara(float x, float y, float z) 
{
    NodoPila* nuevo = (NodoPila*)malloc(sizeof(NodoPila));
    nuevo->camX = x;
    nuevo->camY = y;
    nuevo->camZ = z;
    nuevo->siguiente = pilaCamara;
    pilaCamara = nuevo;
}

NodoPila* popCamara() 
{
    if(pilaCamara == NULL) return NULL;
    NodoPila* temp = pilaCamara;
    pilaCamara = pilaCamara->siguiente;
    return temp;
}

//Funcion para liberar la pila
void liberarPila() 
{
    while(pilaCamara != NULL) 
    {
        NodoPila* temp = popCamara();
        free(temp);
    }
}

//Funciones de inicializacion
void inicializar() 
{
    //Sol
    sol.radio = 1.0;
    sol.distancia = 0.0;
    sol.velocidad = 0.0;
    sol.angulo = 0.0;
    sol.colorR = 1.0; sol.colorG = 0.8; sol.colorB = 0.0;//Amarillo
    
    //Mercurio
    mercurio.radio = 0.15;
    mercurio.distancia = 2.5;
    mercurio.velocidad = 0.08;
    mercurio.angulo = 0.0;
    mercurio.colorR = 0.5; mercurio.colorG = 0.5; mercurio.colorB = 0.5;
    
    //Venus
    venus.radio = 0.25;
    venus.distancia = 3.5;
    venus.velocidad = 0.06;
    venus.angulo = 45.0;
    venus.colorR = 0.9; venus.colorG = 0.7; venus.colorB = 0.4;
    
    //Tierra
    tierra.radio = 0.3;
    tierra.distancia = 5.0;
    tierra.velocidad = 0.05;
    tierra.angulo = 0.0;
    tierra.colorR = 0.2; tierra.colorG = 0.5; tierra.colorB = 1.0;
    
    //Marte
    marte.radio = 0.25;
    marte.distancia = 7.0;
    marte.velocidad = 0.04;
    marte.angulo = 90.0;
    marte.colorR = 0.9; marte.colorG = 0.3; marte.colorB = 0.2;
    
    //Jupiter
    jupiter.radio = 0.7;
    jupiter.distancia = 10.0;
    jupiter.velocidad = 0.02;
    jupiter.angulo = 180.0;
    jupiter.colorR = 0.9; jupiter.colorG = 0.6; jupiter.colorB = 0.3;
    
    //Saturno
    saturno.radio = 0.6;
    saturno.distancia = 13.0;
    saturno.velocidad = 0.015;
    saturno.angulo = 270.0;
    saturno.colorR = 0.9; saturno.colorG = 0.8; saturno.colorB = 0.5;
    
    //Urano
    urano.radio = 0.4;
    urano.distancia = 16.0;
    urano.velocidad = 0.01;
    urano.angulo = 45.0;
    urano.colorR = 0.5; urano.colorG = 0.8; urano.colorB = 0.9;
    
    //Neptuno
    neptuno.radio = 0.4;
    neptuno.distancia = 19.0;
    neptuno.velocidad = 0.008;
    neptuno.angulo = 135.0;
    neptuno.colorR = 0.2; neptuno.colorG = 0.4; neptuno.colorB = 0.8;
    
    //Nave
    nave.x = 0.0;
    nave.y = -100.0;// Oculta al principio
    nave.z = 0.0;
    nave.rotacion = 0.0;
}

//Inicializacion de OPENGL
void iniciarOpenGL() 
{

    glEnable(GL_DEPTH_TEST);              //Activar 3D
    glClearColor(0.0, 0.0, 0.0, 1.0);     //Fondo negro
    
    //Configurar iluminacion
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    float luzAmbiente[] = {0.1, 0.1, 0.1, 1.0};
    float luzDifusa[] = {1.0, 1.0, 1.0, 1.0};
    float posLuz[] = {0.0, 0.0, 0.0, 1.0};//Luz en el Sol
    
    glLightfv(GL_LIGHT0, GL_AMBIENT, luzAmbiente);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, luzDifusa);
    glLightfv(GL_LIGHT0, GL_POSITION, posLuz);
    
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    
    //Configurar proyeccion
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50.0, 1.33, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);

    //Cargar las texturas de los planetas
    cargarTexturasPlanetas();
}

//Funciones de dibujo
//Dibuja un planeta en su orbita usando textura si texID != 0
void dibujarPlanetaTexturizado(Planeta p, GLuint texID) 
{
    glPushMatrix();
        //Rotar segun su angulo orbital
        glRotatef(p.angulo, 0.0f, 1.0f, 0.0f);
        //Moverse a la distancia del Sol
        glTranslatef(p.distancia, 0.0f, 0.0f);

        glPushMatrix();

        //Rotar para que la textura se vea bien
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

        if (texID != 0) 
        {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texID);

            GLUquadric *q = gluNewQuadric();
            gluQuadricTexture(q, GL_TRUE);
            glColor3f(1.0f, 1.0f, 1.0f);//No se tinte

            gluSphere(q, p.radio, 24, 24);
            gluDeleteQuadric(q);

            glDisable(GL_TEXTURE_2D);
        } else 
        {
            //Sin textura: usar color plano
            glColor3f(p.colorR, p.colorG, p.colorB);
            glutSolidSphere(p.radio, 20, 20);
        }
        glPopMatrix();

        //Anillo de Saturno sin textura
        if (p.distancia == saturno.distancia) 
        {
            glPushMatrix();
                glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

                glDisable(GL_TEXTURE_2D);
                glColor3f(0.8f, 0.7f, 0.5f);

                GLUquadric* q2 = gluNewQuadric();
                gluDisk(q2,
                        p.radio + 0.1f,//interno
                        p.radio + 0.4f,//externo
                        40, 1);
                gluDeleteQuadric(q2);
            glPopMatrix();
        }
    glPopMatrix();
}

//Dibuja estrellas de fondo (puntos blancos)
void dibujarEstrellas() 
{
    glDisable(GL_LIGHTING);
    glColor3f(1.0, 1.0, 1.0);
    glPointSize(2.0);
    
    glBegin(GL_POINTS);
        srand(12345);//Fijos para que no cambien
        for(int i = 0; i < 200; i++) 
        {
            float x = (rand() % 400 - 200) / 10.0;
            float y = (rand() % 400 - 200) / 10.0;
            float z = (rand() % 400 - 200) / 10.0;
            glVertex3f(x, y, z);
        }
    glEnd();
    
    glEnable(GL_LIGHTING);
}

//Dibuja texto en pantalla 2D
void dibujarTexto(const char* texto, float x, float y) 
{
    glDisable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, 800, 0, 600);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
            glLoadIdentity();
            glColor3f(1.0, 1.0, 1.0);
            glRasterPos2f(x, y);
            
            for(int i = 0; texto[i] != '\0'; i++) 
            {
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, texto[i]);
            }
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_LIGHTING);
}

//Dibuja texto centrado en la pantalla
void dibujarTextoCentrado(const char* texto, float y) 
{
    int len = strlen(texto);
    float ancho = len * 9.0f; // aprox 9 pixeles por caracter (Helvetica 18)
    float x = (800.0f - ancho) / 2.0f; // ventana 800x600
    dibujarTexto(texto, x, y);
}

//Dibujar personajes (Jerarquia de arbol)

//Dibuja un brazo o pierna
void dibujarBrazo(float largo, float grosor) 
{
    glPushMatrix();
        glScalef(grosor, largo, grosor);
        glutSolidCube(1.0);
    glPopMatrix();
}

//Pildora (Xancho, personaje principal)
//Estructura de arbol:
//Cuerpo (raíz)
//Parte inferior (roja)
//Parte superior (amarilla)
//Ojo izquierdo
//Ojo derecho
//Brazo izquierdo
//Brazo derecho
//Pierna izquierda
//Pierna derecha
void dibujarPildora() 
{
    glPushMatrix();

        //Deshabilitar texturas
        glDisable(GL_TEXTURE_2D);

        glDisable(GL_LIGHTING);

        glScalef(0.5, 0.5, 0.5);//Escala general
        
        //Parte inferior (roja)
        glColor3f(1.0, 0.2, 0.2);
        glPushMatrix();
            glTranslatef(0.0, -0.2, 0.0);
            glScalef(1.0, 1.2, 1.0);
            glutSolidSphere(0.3, 15, 15);
        glPopMatrix();
        
        //Parte superior (amarilla)
        glColor3f(1.0, 0.9, 0.1);
        glPushMatrix();
            glTranslatef(0.0, 0.2, 0.0);
            glScalef(1.0, 1.2, 1.0);
            glutSolidSphere(0.3, 15, 15);
        glPopMatrix();
        
        //Ojos
        glColor3f(1.0, 1.0, 1.0);
        glPushMatrix();
            glTranslatef(-0.12, 0.05, 0.28);
            glutSolidSphere(0.08, 10, 10);
            glColor3f(0.0, 0.0, 0.0);
            glTranslatef(0.0, 0.0, 0.05);
            glutSolidSphere(0.04, 8, 8);
        glPopMatrix();
        
        glColor3f(1.0, 1.0, 1.0);
        glPushMatrix();
            glTranslatef(0.12, 0.05, 0.28);
            glutSolidSphere(0.08, 10, 10);
            glColor3f(0.0, 0.0, 0.0);
            glTranslatef(0.0, 0.0, 0.05);
            glutSolidSphere(0.04, 8, 8);
        glPopMatrix();
        
        //Boca
        glColor3f(0.0, 0.0, 0.0);
        glPushMatrix();
            glTranslatef(0.0f, -0.10f, 0.28f);
            glScalef(0.35f, 0.18f, 0.1f);
            glutSolidSphere(0.25f, 16, 16);
        glPopMatrix();

        //Brazos
        glColor3f(1.0, 0.2, 0.2);
        glPushMatrix();
            glTranslatef(-0.35, 0.0, 0.0);
            glRotatef(45, 0.0, 0.0, 1.0);
            dibujarBrazo(0.4, 0.05);
        glPopMatrix();
        
        glPushMatrix();
            glTranslatef(0.35, 0.0, 0.0);
            glRotatef(-45, 0.0, 0.0, 1.0);
            dibujarBrazo(0.4, 0.05);
        glPopMatrix();

        //Pies
        glColor3f(1.0f, 0.2f, 0.2f);

        //Pierna izquierda
        glPushMatrix();
            glTranslatef(-0.12f, -0.55f, 0.0f);
            glPushMatrix();
                glScalef(0.06f, 0.30f, 0.06f);
                glutSolidCube(1.0f);
            glPopMatrix();

            //Pie
            glPushMatrix();
                glTranslatef(0.0f, -0.18f, 0.0f);
                glScalef(0.14f, 0.04f, 0.10f);
                glutSolidCube(1.0f);
            glPopMatrix();
        glPopMatrix();

        //Pierna derecha
        glPushMatrix();
            glTranslatef( 0.12f, -0.55f, 0.0f);
            glPushMatrix();
                glScalef(0.06f, 0.30f, 0.06f);
                glutSolidCube(1.0f);
            glPopMatrix();

            //Pie   
            glPushMatrix();
                glTranslatef(0.0f, -0.18f, 0.0f);
                glScalef(0.14f, 0.04f, 0.10f);
                glutSolidCube(1.0f);
            glPopMatrix();
        glPopMatrix();

        glEnable(GL_LIGHTING);

    glPopMatrix();
}

//Pastilla (Rony, personaje secundario)
//Estructura similar a Pildora pero mas simple
void dibujarPastilla() 
{
    glPushMatrix();

        //Deshabilitar texturas
        glDisable(GL_TEXTURE_2D);

        glDisable(GL_LIGHTING);

        glScalef(0.5, 0.5, 0.5);
        
        //Cuerpo
        glColor3f(0.95, 0.95, 0.95);
        glPushMatrix();
            glScalef(1.0, 1.0, 0.4);
            glutSolidSphere(0.4, 15, 15);
        glPopMatrix();
        
        //Ojos
        //Ojo izquierdo
        glPushMatrix();
            glColor3f(0.0f, 0.0f, 0.0f);
            glTranslatef(-0.15f, 0.10f, 0.18f);
            glutSolidSphere(0.05f, 8, 8);

            //Puntito blanco
            glColor3f(1.0f, 1.0f, 1.0f);
            glTranslatef(0.015f, 0.015f, 0.03f);
            glutSolidSphere(0.015f, 6, 6);
        glPopMatrix();
        
        //Ojo derecho
        glPushMatrix();
            glColor3f(0.0f, 0.0f, 0.0f);
            glTranslatef(0.15f, 0.10f, 0.18f);
            glutSolidSphere(0.05f, 8, 8);

            //Puntito blanco
            glColor3f(1.0f, 1.0f, 1.0f);
            glTranslatef(0.015f, 0.015f, 0.03f);
            glutSolidSphere(0.015f, 6, 6);
        glPopMatrix();


        //Boca
        glColor3f(0.0f, 0.0f, 0.0f); //Negro
        glPushMatrix();
            //Un poco debajo de los ojos
            glTranslatef(0.0f, -0.05f, 0.18f);
            //Linea horizontal con un cubo aplastado
            glScalef(0.3f, 0.05f, 0.05f);
            glutSolidCube(0.4f);
        glPopMatrix();
        
        //Brazos
        glColor3f(1.0, 1.0, 1.0);
        //Brazo izquierdo
        glPushMatrix();
            glTranslatef(-0.4, -0.1, 0.0);
            dibujarBrazo(0.3, 0.05);
        glPopMatrix();
        
        //Brazo derecho
        glPushMatrix();
            glTranslatef(0.4, -0.1, 0.0);
            dibujarBrazo(0.3, 0.05);
        glPopMatrix();

        //Pies
        glColor3f(1.0f, 1.0f, 1.0f);

        //Pierna izquierda
        glPushMatrix();
            glTranslatef(-0.12f, -0.43f, 0.0f);
            //Pierna
            glPushMatrix();
                glScalef(0.06f, 0.15f, 0.06f);
                glutSolidCube(1.0f);
            glPopMatrix();
            //Pie
            glPushMatrix();
                glTranslatef(0.0f, -0.10f, 0.0f);
                glScalef(0.14f, 0.04f, 0.10f);
                glutSolidCube(1.0f);
            glPopMatrix();
        glPopMatrix();

        //Pierna derecha
        glPushMatrix();
            glTranslatef(0.12f, -0.43f, 0.0f);
            //Pierna
            glPushMatrix();
                glScalef(0.06f, 0.15f, 0.06f);
                glutSolidCube(1.0f);
            glPopMatrix();
            //Pie
            glPushMatrix();
                glTranslatef(0.0f, -0.10f, 0.0f);
                glScalef(0.14f, 0.04f, 0.10f);
                glutSolidCube(1.0f);
            glPopMatrix();
        glPopMatrix();

        glEnable(GL_LIGHTING);

    glPopMatrix();
}

//Dibujar Dios: triangulo 3D con un ojo en medio
void dibujarDios() 
{
    glPushMatrix();

        //Deshabilitar texturas
        glDisable(GL_TEXTURE_2D);

        glScalef(0.6f, 0.6f, 0.6f);

        //Cuerpo
        glColor3f(0.4f, 1.0f, 0.2f);//verde

        glBegin(GL_TRIANGLES);
            //Vertices
            float vTop[3] =  { 0.0f,  0.6f,  0.0f};//punta
            float v1[3]  = {-0.6f, -0.3f,  0.4f};//izquierda
            float v2[3]  = { 0.6f, -0.3f,  0.4f};//derecha
            float v3[3]  = { 0.0f, -0.3f, -0.6f};//atras

            //Cara frontal
            glVertex3fv(vTop);
            glVertex3fv(v1);
            glVertex3fv(v2);

            //Cara derecha
            glVertex3fv(vTop);
            glVertex3fv(v2);
            glVertex3fv(v3);

            //Cara izquierda
            glVertex3fv(vTop);
            glVertex3fv(v3);
            glVertex3fv(v1);

            //Base (triangulo de abajo)
            glVertex3fv(v1);
            glVertex3fv(v3);
            glVertex3fv(v2);
        glEnd();

        //Ojo en la cara frontal
        glPushMatrix();
            //Frente del triángulo
            glTranslatef(0.0f, 0.05f, 0.32f);

            //Esclerótica (blanco)
            glColor3f(1.0f, 1.0f, 1.0f);
            glutSolidSphere(0.12f, 16, 16);

            //Iris (azul)
            glColor3f(0.0f, 0.3f, 0.9f);
            glTranslatef(0.0f, 0.0f, 0.06f);
            glutSolidSphere(0.07f, 16, 16);

            //Pupila (negra)
            glColor3f(0.0f, 0.0f, 0.0f);
            glTranslatef(0.0f, 0.0f, 0.04f);
            glutSolidSphere(0.035f, 12, 12);
        glPopMatrix();

    glPopMatrix();
}

//Dibujar nave
void dibujarNave() 
{
    glPushMatrix();

        //Deshabilitar texturas
        glDisable(GL_TEXTURE_2D);
        
        //Posicion
        glTranslatef(nave.x, nave.y, nave.z);
        glRotatef(nave.rotacion, 0.0, 1.0, 0.0);
        
        //Escala configurable segun la escena
        glScalef(escalaNave, escalaNave, escalaNave);

        //Cuerpo de la nave
        glColor3f(0.7f, 0.7f, 0.7f);
        glPushMatrix();
            glScalef(1.0f, 0.3f, 1.0f);
            glutSolidSphere(0.6, 20, 20);
        glPopMatrix();

        //Solo si naveConTripulacion == 1 dibujamos a los personajes
        if (naveConTripulacion) 
        {

            glDisable(GL_LIGHTING);

            //Pildora
            glPushMatrix();
                glTranslatef(-0.15f, 0.2f, 0.0f);
                glScalef(0.7f, 0.7f, 0.7f);
                dibujarPildora();
            glPopMatrix();

            //Pastilla
            glPushMatrix();
                glTranslatef( 0.15f, 0.2f, 0.0f);
                glScalef(0.7f, 0.7f, 0.7f);
                dibujarPastilla();
            glPopMatrix();

            glEnable(GL_LIGHTING);
        }

        //Cabina transparente   
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.3f, 0.6f, 1.0f, 0.3f);
        glPushMatrix();
            glTranslatef(0.0f, 0.25f, 0.0f);
            glutSolidSphere(0.35, 20, 20);
        glPopMatrix();
        glDisable(GL_BLEND);
    glPopMatrix();
}

//Personajes pisando planeta
void dibujarPersonajesEnPlaneta(Planeta p) 
{
    glPushMatrix();
        //Nos movemos al centro del planeta usando la misma transformación que en dibujarPlaneta
        glRotatef(p.angulo, 0.0f, 1.0f, 0.0f);
        glTranslatef(p.distancia, 0.0f, 0.0f);

        //Pildora (izquierda, sobre la superficie)
        glPushMatrix();
            glTranslatef(-p.radio * 0.7f, p.radio + 0.35f, 0.0f);
            glScalef(0.6f, 0.6f, 0.6f);
            dibujarPildora();
        glPopMatrix();

        //Pastilla (derecha, sobre la superficie)
        glPushMatrix();
            glTranslatef(p.radio * 0.7f, p.radio + 0.35f, 0.0f);
            glScalef(0.6f, 0.6f, 0.6f);
            dibujarPastilla();
        glPopMatrix();
    glPopMatrix();
}

//Dibujar planeta cerca para las escenas lejanas
void dibujarPlanetaCerca(Planeta p, GLuint texID) 
{
    glPushMatrix();
        float radioGrande = p.radio * 3.5f;

        glPushMatrix();
        //Rotar para que la textura se vea bien
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

        if (texID != 0) 
        {
            // Usar esfera con textura
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texID);

            GLUquadric* q = gluNewQuadric();
            gluQuadricTexture(q, GL_TRUE);
            glColor3f(1.0f, 1.0f, 1.0f);//que no se tinte

            gluSphere(q, radioGrande, 30, 30);
            gluDeleteQuadric(q);

            glDisable(GL_TEXTURE_2D);
        } else 
        {
            //Sin textura
            glColor3f(p.colorR, p.colorG, p.colorB);
            glutSolidSphere(radioGrande, 30, 30);
        }
        glPopMatrix();

        //Anillo de Saturno sin textura
        if (p.distancia == saturno.distancia) 
        {
            glPushMatrix();
                //Acostado
                glRotatef(90.0f, 1.0f, 0.0f, 0.0f);

                glDisable(GL_TEXTURE_2D);//no use textura
                glColor3f(0.85f, 0.75f, 0.55f);//color beige

                GLUquadric* q2 = gluNewQuadric();
                //Disco con hueco
                gluDisk(q2,
                        radioGrande + 0.3f,//interno
                        radioGrande + 1.6f,//externo
                        60, 1);
                gluDeleteQuadric(q2);
            glPopMatrix();
        }
    glPopMatrix();
}

//Dibujar superficie del planeta como en un horizonte para las escenas cercanas
void dibujarSuperficiePlaneta(Planeta p, GLuint texID, const char* dialogo1, const char* dialogo2) 
{

    //Rebote de los personajes
    long ms = glutGet(GLUT_ELAPSED_TIME);
    float t = ms / 1000.0f;

    //Rebote de cada personaje
    float bounce1 = sinf(t * 6.0f) * 0.15f;          //Píldora
    float bounce2 = sinf(t * 6.0f + 1.5f) * 0.12f;   //Pastilla


    //Planeta como suelo curvo
    glPushMatrix();
        //Radio grande fijo para que el horizonte se vea parecido en todos los planetas
        float R = 12.0f;

        //Bajamos el centro de la esfera en -R
        glTranslatef(0.0f, -R, 0.0f);

        if (texID != 0) 
        {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texID);

            GLUquadric* q = gluNewQuadric();
            gluQuadricTexture(q, GL_TRUE);
            glColor3f(1.0f, 1.0f, 1.0f);

            gluSphere(q, R, 40, 40);
            gluDeleteQuadric(q);

            glDisable(GL_TEXTURE_2D);
        } else 
        {
            glColor3f(p.colorR, p.colorG, p.colorB);
            glutSolidSphere(R, 40, 40);
        }
    glPopMatrix();

    //Personajes parados en la superficie

    //Pildora (izquierda)
    glPushMatrix();
        glTranslatef(-1.5f, 0.4f +bounce1, 2.0f);//x izquierda, y altura, z hacia la cámara
        glScalef(0.9f, 0.9f, 0.9f);
        dibujarPildora();
    glPopMatrix();

    //Pastilla (derecha)
    glPushMatrix();
        glTranslatef(1.5f, 0.4f +bounce2, 2.0f);//x derecha, y altura, z hacia la cámara
        glScalef(0.9f, 0.9f, 0.9f); 
        dibujarPastilla();
    glPopMatrix();

    //Dialogos (texto 2D arriba de la pantalla)
    dibujarTextoCentrado(dialogo1, 560);//texto Píldora
    dibujarTextoCentrado(dialogo2, 530);//texto Pastilla
}

//Funcion auxiliar: posicion real de un planeta en el mundo
void obtenerPosicionPlaneta(Planeta p, float *x, float *y, float *z) 
{
    //El planeta se dibuja
    //glRotatef(p.angulo, 0,1,0);
    //glTranslatef(p.distancia, 0, 0);
    //
    //Eso equivale a esta posición:
    //x = distancia * cos(angulo)
    //z = distancia * sin(angulo)
    //y = 0
    float rad = p.angulo * 3.14159f / 180.0f;
    *x = p.distancia * cosf(rad);
    *z = p.distancia * sinf(rad);
    *y = 0.0f;
}

//Funcion principal de dibujo
void display(void) 
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    //Obtener escena actual
    int escena = cola[escenaActual];
    long tiempoTranscurrido = glutGet(GLUT_ELAPSED_TIME) - tiempoInicio;
    float t = tiempoTranscurrido / 1000.0f;//Tiempo en segundos

    escalaNave = 0.4f;//tamaño normal por defecto

    //Configurar camara segun la escena    
    switch(escena) 
    {
        case INTRO:
            //Camara frente a los personajes
            gluLookAt(0.0, 0.0, 3.5,
                      0.0, 0.0, 0.0,
                      0.0, 1.0, 0.0);
            duracion = 10000;//10 segundos
            break;
            
        case DESPEDIDA:
            //Camara cerca de la Tierra
            gluLookAt(tierra.distancia - 2.0, 1.0, 2.0,
                      tierra.distancia, 0.0, 0.0,
                      0.0, 1.0, 0.0);
            duracion = 8000;//8 segundos
            break;
            
        case VIAJE1:
        case VIAJE2:
        case VIAJE3:
        case VIAJE4:
        case VIAJE5:
            //Camara panoramica del espacio
            gluLookAt(0.0, 20.0, -25.0,
                      0.0, 0.0, 0.0,
                      0.0, 1.0, 0.0);
            duracion = 4000;//4 segundos
            break;
            
        //Camaras para vista lejana de planetas
        case MERCURIO_LEJOS:
            gluLookAt(0.0, 1.5, 5.0,//cerca, porque Mercurio es chiquito
                      0.0, 0.5, 0.0,
                      0.0, 1.0, 0.0);
            duracion = 7000;//7 segundos
            break;

        case VENUS_LEJOS:
            gluLookAt(0.0, 1.7, 5.5,
                      0.0, 0.5, 0.0,
                      0.0, 1.0, 0.0);
            duracion = 7000;//7 segundos
            break;


        case MARTE_LEJOS:
            gluLookAt(0.0, 1.5, 5.5,//un poco mas lejos
                      0.0, 0.5, 0.0,
                      0.0, 1.0, 0.0);
            duracion = 7000;//7 segundos
            break;

        case JUPITER_LEJOS:
            //Mas lejos y un poco mas alto para que quepa el planeta + nave
            gluLookAt(0.0, 2.2, 7.5,//Z mas grande, Y un poco mas alto
                      0.0, 0.7, 0.0,//un poco por encima del centro
                      0.0, 1.0, 0.0);
            duracion = 7000;//7 segundos
            break;

        case SATURNO_LEJOS:
            //Aun un poco mas lejos porque los anillos ocupan mas
            gluLookAt(0.0, 2.4, 8.0,//alejado para que quepan anillos + nave
                      0.0, 0.7, 0.0,
                      0.0, 1.0, 0.0);
            duracion = 7000;//7 segundos
            break;

            
        //Camaras para superficie de cada planeta (primer plano)
        case MERCURIO_CERCA:
        case VENUS_CERCA:
        case MARTE_CERCA:
        case JUPITER_CERCA:
        case SATURNO_CERCA:
            gluLookAt(0.0, 1.5, 8.0,
                      0.0, 0.4, 0.0,
                      0.0, 1.0, 0.0);
            duracion = 9000;//9 segundos
            escalaNave = 1.2f;//nave grande en primer plano
            break;
            
        case REGRESO:
            //Camara viendo el regreso
            gluLookAt(0.0, 10.0, -15.0,
                      0.0, 0.0, 0.0,
                      0.0, 1.0, 0.0);
            duracion = 12000;//12 segundos
            break;
            
        case CASTIGO:
            //Camara en la Tierra
            gluLookAt(tierra.distancia - 2.0, 0.5, 2.0,
                      tierra.distancia, 0.0, 0.0,
                      0.0, 1.0, 0.0);
            duracion = 14000;//14 segundos
            break;
            
        case CREDITOS:
            //Camara muy lejos (pantalla negra)
            gluLookAt(0.0, 0.0, 100.0,
                      0.0, 0.0, 0.0,
                      0.0, 1.0, 0.0);
            duracion = 10000;//10 segundos
            break;
    }
    
    //Dibujar fondo (Estrellas y SS)
    if(escena != CREDITOS) 
    {
        //Estrellas
        //En superficies de planetas: estrellas giran
        if(escena == MERCURIO_CERCA || escena == VENUS_CERCA   || escena == MARTE_CERCA  ||
           escena == JUPITER_CERCA  || escena == SATURNO_CERCA ||
           escena == MERCURIO_LEJOS || escena == VENUS_LEJOS   || escena == MARTE_LEJOS  ||
           escena == JUPITER_LEJOS  || escena == SATURNO_LEJOS) {

            glPushMatrix();
                glRotatef(-t * 5.0f, 0.0f, 1.0f, 0.0f);
                dibujarEstrellas();
            glPopMatrix();
        } else 
        {
            //En las demas escenas las estrellas normales
            dibujarEstrellas();
        }

        //SS completo
        //No se dibuja en escenas de planetas ni lejos ni cerca
        if(escena != MERCURIO_LEJOS && escena != MERCURIO_CERCA &&
           escena != VENUS_LEJOS    && escena != VENUS_CERCA    &&
           escena != MARTE_LEJOS    && escena != MARTE_CERCA    &&
           escena != JUPITER_LEJOS  && escena != JUPITER_CERCA  &&
           escena != SATURNO_LEJOS  && escena != SATURNO_CERCA) 
           {
            glPushMatrix();

            //Solo si no estamos en la intro, dibujamos el sol
            if (escena != INTRO) 
            {
                //Sol (con emision de luz)
                glColor3f(sol.colorR, sol.colorG, sol.colorB);
                float emisionSol[] = {1.0f, 0.8f, 0.2f, 1.0f};
                glMaterialfv(GL_FRONT, GL_EMISSION, emisionSol);
                glutSolidSphere(sol.radio, 30, 30);
                float noEmision[] = {0.0f, 0.0f, 0.0f, 1.0f};
                glMaterialfv(GL_FRONT, GL_EMISSION, noEmision);
            }

            //Planetas: estos siempre se dibujan en las escenas que pasan este if,
            //incluyendo la intro asi si se ven de fondo atras de los personajes
            dibujarPlanetaTexturizado(mercurio, texMercurio);
            dibujarPlanetaTexturizado(venus, texVenus);
            dibujarPlanetaTexturizado(tierra, texTierra);
            dibujarPlanetaTexturizado(marte, texMarte);
            dibujarPlanetaTexturizado(jupiter, texJupiter);
            dibujarPlanetaTexturizado(saturno, texSaturno);
            dibujarPlanetaTexturizado(urano, texUrano);
            dibujarPlanetaTexturizado(neptuno, texNeptuno);

        glPopMatrix();
    }
}
    
    //Dibujar objetos segun la escena
    switch(escena) 
    {
        case INTRO: {
            //Presentacion: Personajes flotando
            float bounce = sin(t * 3.0f) * 0.1f;
            
            //Luz frontal para que se vean bien
            float luzFrente[] = {0.0f, 0.0f, 5.0f, 1.0f};
            glLightfv(GL_LIGHT0, GL_POSITION, luzFrente);
            
            //Pildora (izquierda)
            glPushMatrix();
                glTranslatef(-0.8f, bounce, 1.5f);
                glRotatef(t * 15.0f, 0.0f, 1.0f, 0.0f);
                dibujarPildora();
            glPopMatrix();
            
            //Pastilla (derecha)
            glPushMatrix();
                glTranslatef(0.8f, -bounce, 1.5f);
                glRotatef(-t * 15.0f, 0.0f, 1.0f, 0.0f);
                dibujarPastilla();
            glPopMatrix();
            
            //Regresar luz al Sol
            float luzSol[] = {0.0f, 0.0f, 0.0f, 1.0f};
            glLightfv(GL_LIGHT0, GL_POSITION, luzSol);
            
            dibujarTextoCentrado("Xancho y Rony: El Viaje Espacial", 550);
            break;
        }
        
        case DESPEDIDA: 
        {
            
            //Personajes mirando hacia arriba
            //Pildora
            glPushMatrix();
                glTranslatef(tierra.distancia - 0.5f, tierra.radio + -1.0f, -1.5f);
                glRotatef(-30.0f, 1.0f, 0.0f, 0.0f);
                dibujarPildora();
            glPopMatrix();
            
            //Pastilla
            glPushMatrix();
                glTranslatef(tierra.distancia + 0.5f, tierra.radio + -1.0f, -1.5f);
                glRotatef(-30.0f, 1.0f, 0.0f, 0.0f);
                dibujarPastilla();
            glPopMatrix();

            //Dios flotando arriba de ellos
            glPushMatrix();
                //Un poco por encima de la Tierra centrado
                glTranslatef(tierra.distancia, tierra.radio + 0.3f, 1.0f);
                //Rotacion para que se vea divino
                glRotatef(t * 50.0f, 0.0f, 1.0f, 0.0f);
                dibujarDios();
            glPopMatrix();
            
            dibujarTextoCentrado("Dios: Por ordenes mias deben visitar todos los planetas, no olviden ninguno", 60);
            break;
        }
        
        case VIAJE1:
        case VIAJE2:
        case VIAJE3:
        case VIAJE4:
        case VIAJE5: 
        {
            //Nave viajando por el espacio
            naveConTripulacion = 1; 
            dibujarNave();
            dibujarTextoCentrado("Viajando por el espacio...", 550);
            break;
        }
        
        //Escenas de Mercurio
        case MERCURIO_LEJOS: 
        {
            //Vista lejana del planeta
            naveConTripulacion = 1; 
            dibujarPlanetaCerca(mercurio, texMercurio);
            dibujarTextoCentrado("Llegando a Mercurio...", 550);
            dibujarNave();
            break;
        }
        
        case MERCURIO_CERCA: 
        {
            naveConTripulacion = 0; 
            dibujarSuperficiePlaneta(mercurio, texMercurio,
                "Xancho: Siento que me estoy derritiendo como queso ",
                "Rony: Es el planeta mas cercano al Sol pero tu aguanta, lo bueno soy una pastilla");
            dibujarNave();
            break;
        }  
        
                //Escenas de Venus
        case VENUS_LEJOS: 
        {
            naveConTripulacion = 1;
            dibujarPlanetaCerca(venus, texVenus);
            dibujarTextoCentrado("Llegando a Venus...", 550);
            dibujarNave();
            break;
        }

        case VENUS_CERCA: 
        {
            naveConTripulacion = 0;
            dibujarSuperficiePlaneta(venus, texVenus,
                "Rony: Este aire se siente super pesado y huele feo",
                "Xancho: Perdon fui yo... ah no, es el azufre de la atmosfera que es toxica y caliente");
            dibujarNave();
            break;
        }

        //Escenas de Marte
        case MARTE_LEJOS: 
        {
            naveConTripulacion = 1; 
            dibujarPlanetaCerca(marte, texMarte);
            dibujarTextoCentrado("Llegando a Marte...", 550);
            dibujarNave();
            break;
        }
        
        case MARTE_CERCA: 
        {
            naveConTripulacion = 0;     
            dibujarSuperficiePlaneta(marte, texMarte,
                "Rony: Mira dicen que hay rocas rojas por todos lados, tambien dicen que hay marcianos",
                "Xancho: Eso dicen pero no veo nada, ya mejor vamonos antes que salga Elon Musk");
            dibujarNave();
            break;
        }
        
        //Escenas de Jupiter
        case JUPITER_LEJOS: 
        {
            naveConTripulacion = 1; 
            dibujarPlanetaCerca(jupiter, texJupiter);
            dibujarTextoCentrado("Llegando a Jupiter...", 550);
            dibujarNave();
            break;
        }
        
        case JUPITER_CERCA: 
        {   
            naveConTripulacion = 0; 
            dibujarSuperficiePlaneta(jupiter, texJupiter,
                "Xancho: Me siento pesado, creo que por comer mucho",
                "Rony: No, es la gravedad es que este planeta es enorme");
            dibujarNave();
            break;
        }
        
        //Escenas de Saturno
        case SATURNO_LEJOS: 
        {
            naveConTripulacion = 1; 
            dibujarPlanetaCerca(saturno, texSaturno);
            dibujarTextoCentrado("Llegando a Saturno...", 550);
            dibujarNave();
            break;
        }
        
        case SATURNO_CERCA: 
        {
            naveConTripulacion = 0; 
            dibujarSuperficiePlaneta(saturno, texSaturno,
                "Rony: Mira hay particulas de hielo como nieve y piedras",
                "Xancho: Son los restos de los anillos cayendo, hay que llevarnos una piedra");
            dibujarNave();
            break;
        }
        
        //Escenas finales
        case REGRESO: 
        {
            dibujarNave();
            dibujarTextoCentrado("Regresando a la Tierra cansados...", 550);
            break;
        }
        
        case CASTIGO: 
        {
            //Personajes mirando hacia abajo
            //Pildora
            glPushMatrix();
                glTranslatef(tierra.distancia - 0.5f, tierra.radio + -1.0f, -1.0f);
                glRotatef(30.0f, 1.0f, 0.0f, 0.0f);
                dibujarPildora();
            glPopMatrix();
            
            //Pastilla
            glPushMatrix();
                glTranslatef(tierra.distancia + 0.5f, tierra.radio + -1.0f, -1.5f);
                glRotatef(30.0f, 1.0f, 0.0f, 0.0f);
                dibujarPastilla();
            glPopMatrix();

            //Dios flotando arriba de ellos
            glPushMatrix();
                //Un poco por encima de la Tierra centrado
                glTranslatef(tierra.distancia, tierra.radio + 0.1f, 1.0f);
                //Rotacion para que se vea como si fuera un ser divino
                glRotatef(t * 50.0f, 0.0f, 1.0f, 0.0f);
                dibujarDios();
            glPopMatrix();
            
            dibujarTextoCentrado("Dios: No visitaron Urano ni Neptuno", 550);
            dibujarTextoCentrado("Como castigo viviran en la Tierra para siempre y suerte con los humanos", 520);
            break;
        }
        
        case CREDITOS: 
        {
            glClearColor(0.0, 0.0, 0.0, 1.0);
            dibujarTextoCentrado("FIN", 330);
            dibujarTextoCentrado("Gracias por ver", 300);
            dibujarTextoCentrado("Proyecto de Graficacion por Computadora", 260);
            break;
        }
    }
    
    //Indicador de pausa
    if(pausado) 
    {
        dibujarTextoCentrado("PAUSADO (Presiona ESPACIO)", 580);
    }
    
    glutSwapBuffers();
}

//Funcion idle para la animacion continua
void idle(void) 
{
    //Si está pausado, no hacer nada
    if(pausado) 
    {
        return;
    }
    
    //Calcular tiempo transcurrido
    long tiempoActual = glutGet(GLUT_ELAPSED_TIME);
    long tiempoTranscurrido = tiempoActual - tiempoInicio;
    
    //Si ya terminó la escena se cambia a la siguiente
    if(tiempoTranscurrido > duracion) 
    {
        escenaActual++;//Avanzar a la siguiente escena
        
        //Si llega al final se detiene
        if(escenaActual >= totalEscenas) 
        {
            escenaActual = totalEscenas - 1;
            pausado = 1;//Pausar automaticamente
            return;
        }
        
        //Reiniciar reloj para la nueva escena
        tiempoInicio = tiempoActual;
        tiempoTranscurrido = 0;
    }
    
    //Animar planetas (girar alrededor del sol)
    mercurio.angulo += mercurio.velocidad;
    venus.angulo += venus.velocidad;
    tierra.angulo += tierra.velocidad;
    marte.angulo += marte.velocidad;
    jupiter.angulo += jupiter.velocidad;
    saturno.angulo += saturno.velocidad;
    urano.angulo += urano.velocidad;
    neptuno.angulo += neptuno.velocidad;
    
    //Animar nave (Segun la escena)
    int escena = cola[escenaActual];
    float t = tiempoTranscurrido / 1000.0;//Tiempo en segundos
    float progreso = (float)tiempoTranscurrido / (float)duracion;//0.0 a 1.0
    if (progreso < 0.0f) progreso = 0.0f;
    if (progreso > 1.0f) progreso = 1.0f;

    //Variables reutilizables para la animacion de la nave
    float inicioX = 0.0f, inicioY = 0.0f, inicioZ = 0.0f;
    float finX    = 0.0f, finY    = 0.0f, finZ    = 0.0f;
    
    switch (escena) 
    {
        
        //Escenas donde no queremos ver la nave
        case INTRO:
        case DESPEDIDA:
        case CASTIGO:
        case CREDITOS:
            nave.y = -100.0f;//La mandamos lejos
            break;

        //Lejos: nave flotando junto al planeta
        case MERCURIO_LEJOS:

            inicioX = -4.0f; inicioY = 2.5f; inicioZ = 8.0f;
            finX    =  1.2f; finY    = 0.8f; finZ    = 2.0f;    
            nave.x = inicioX + (finX - inicioX) * progreso;
            nave.y = inicioY + (finY - inicioY) * progreso;
            nave.z = inicioZ + (finZ - inicioZ) * progreso;

            nave.rotacion = -20.0f;
            break;

        case VENUS_LEJOS:

            inicioX =  4.0f; inicioY = 2.5f; inicioZ = 8.0f;
            finX    = -1.3f; finY    = 0.8f; finZ    = 2.0f;
            nave.x = inicioX + (finX - inicioX) * progreso;
            nave.y = inicioY + (finY - inicioY) * progreso;
            nave.z = inicioZ + (finZ - inicioZ) * progreso;

            nave.rotacion = 20.0f;
            break;

        case MARTE_LEJOS:

            inicioX =  4.5f; inicioY = 2.5f; inicioZ = 8.0f;
            finX    = -1.2f; finY    = 0.8f; finZ    = 2.0f;
            nave.x = inicioX + (finX - inicioX) * progreso;
            nave.y = inicioY + (finY - inicioY) * progreso;
            nave.z = inicioZ + (finZ - inicioZ) * progreso;

            nave.rotacion = 20.0f;
            break;

        case JUPITER_LEJOS:

            inicioX = -5.0f; inicioY = 3.0f; inicioZ = 9.0f;
            finX    =  1.5f; finY    = 1.0f; finZ    = 2.5f;
            nave.x = inicioX + (finX - inicioX) * progreso;
            nave.y = inicioY + (finY - inicioY) * progreso;
            nave.z = inicioZ + (finZ - inicioZ) * progreso;

            nave.rotacion = -30.0f;
            break;

        case SATURNO_LEJOS:

            inicioX =  5.0f; inicioY = 3.0f; inicioZ = 9.0f;
            finX    = -1.5f; finY    = 1.0f; finZ    = 2.5f;
            nave.x = inicioX + (finX - inicioX) * progreso;
            nave.y = inicioY + (finY - inicioY) * progreso;
            nave.z = inicioZ + (finZ - inicioZ) * progreso;

            nave.rotacion = 30.0f;
            break;

        //Cerca: nave aterrizada en el suelo del planeta
        case MERCURIO_CERCA:
        case VENUS_CERCA:   
        case MARTE_CERCA:
        case JUPITER_CERCA:
        case SATURNO_CERCA:
            //El horizonte está en y≈0 y los personajes en y=0.4, z=2.0
            //Ponemos la nave un poco detras de ellos
            nave.x = 0.0f;
            nave.y = 0.45f;//tocando el suelo
            nave.z = 0.8f;//mas cerca del origen (detras de los personajes)
            nave.rotacion = 0.0f;
            break;

        //Viajes: nave dando vueltas
        case VIAJE1:
        case VIAJE2:
        case VIAJE3:
        case VIAJE4:
        case VIAJE5: 
        {
            float radio = 12.0f;
            float angulo = t * 40.0f;
            nave.x = radio * cosf(angulo * 3.14159f / 180.0f);
            nave.z = radio * sinf(angulo * 3.14159f / 180.0f);
            nave.y = sinf(t * 2.0f) * 0.8f;
            nave.rotacion = -angulo;
            break;
    }

        //Regreso: animación hacia la Tierra
        case REGRESO: 
        {
            float inicioX = saturno.distancia - 3.0f;
            float finX    = tierra.distancia + 2.0f;

            nave.x = inicioX + (finX - inicioX) * progreso;
            nave.y = 3.0f - progreso * 2.5f;
            nave.z = 0.0f;
            nave.rotacion = 90.0f;
            break;
        }
    }

    glutPostRedisplay();
}

//Controles de teclado
void teclado(unsigned char tecla, int x, int y) 
{
    switch(tecla) 
    {
        case 27://ESC para salir
            //Liberar memoria antes de salir
            liberarLista();
            liberarPila();
            printf("\n¡Gracias por ver la pelicula!\n");
            exit(0);
            break;
            
        case ' '://ESPACIO para pausar/reanudar
            pausado = !pausado;
            printf(pausado ? "PAUSADO\n" : "REPRODUCIENDO\n");
            break;
            
        case 'r'://R para reiniciar
        case 'R':
            escenaActual = 0;
            tiempoInicio = glutGet(GLUT_ELAPSED_TIME);
            pausado = 0;
            nave.y = -100.0;
            printf(">> REINICIANDO PELICULA <<\n");
            break;
    }
    glutPostRedisplay();
}

//Menu con clic derecho
void menu(int opcion) 
{
    switch(opcion) 
    {
        case 1://Reproducir/Reanudar
            pausado = 0;
            printf(">>> REPRODUCIENDO <<<\n");
            break;
            
        case 2://Pausar
            pausado = 1;
            printf(">>> PAUSADO <<<\n");
            break;
            
        case 3://Reiniciar
            escenaActual = 0;
            tiempoInicio = glutGet(GLUT_ELAPSED_TIME);
            pausado = 0;
            nave.y = -100.0;
            printf(">>> REINICIANDO PELICULA <<<\n");
            break;
            
        case 4://Salir
            liberarLista();
            liberarPila();
            printf("\n¡Gracias por ver la pelicula!\n");
            exit(0);
            break;
    }
    glutPostRedisplay();
}

//Configurar la Pelicula
void configurarPelicula() 
{
    int i = 0;
    cola[i++] = INTRO;           //0: Presentación (10s)
    cola[i++] = DESPEDIDA;       //1: Despedida (8s)
    cola[i++] = VIAJE1;          //2: Viaje (4s)
    
    //Mercurio (2 escenas)
    cola[i++] = MERCURIO_LEJOS;  //3: Vista (7s)
    cola[i++] = MERCURIO_CERCA;  //4: Superficie (9s)
    
    cola[i++] = VIAJE2;          //5: Viaje (4s)

    //Venus (2 escenas)
    cola[i++] = VENUS_LEJOS;     //6: Vista (7s)
    cola[i++] = VENUS_CERCA;     //7: Superficie (9s)

    cola[i++] = VIAJE3;          //8: Viaje (4s)
    
    //Marte (2 escenas)
    cola[i++] = MARTE_LEJOS;     //9: Vista (7s)
    cola[i++] = MARTE_CERCA;     //10: Superficie (9s)
    
    cola[i++] = VIAJE4;          //11: Viaje (4s)
    
    //Jupiter (2 escenas)
    cola[i++] = JUPITER_LEJOS;   //12: Vista (7s)
    cola[i++] = JUPITER_CERCA;   //13: Superficie (9s)
    
    cola[i++] = VIAJE5;          //14: Viaje (4s)
    
    //Saturno (2 escenas)
    cola[i++] = SATURNO_LEJOS;   //15: Vista (7s)
    cola[i++] = SATURNO_CERCA;   //16: Superficie (9s)
    
    cola[i++] = REGRESO;         //17: Regreso (12s)
    cola[i++] = CASTIGO;         //18: Castigo (14s)
    cola[i++] = CREDITOS;        //19: Fin (10s)
    
    totalEscenas = i;//Total: 20 escenas

    //Inicializar reloj
    tiempoInicio = glutGet(GLUT_ELAPSED_TIME);
    
    //Agregar algunos frames a la lista
    agregarALista(0.0, 0.0, 0.0);
    agregarALista(1.0, 1.0, 0.0);
    agregarALista(2.0, 0.0, 0.0);
    
    //Agregar posición de camara a la pila
    pushCamara(0.0, 0.0, 3.5);
    pushCamara(0.0, 20.0, -25.0);
    
    printf("========================================\n");
    printf("   EL VIAJE DE XANCHO Y RONY\n");
    printf("========================================\n");
    printf("Duracion aproximada: 2:30 minutos \n");
    printf("Total de escenas: %d\n", totalEscenas);
    printf("========================================\n");
    printf("CONTROLES:\n");
    printf(" [ESPACIO] - Pausar / Reanudar\n");
    printf(" [R]       - Reiniciar pelicula\n");
    printf(" [ESC]     - Salir\n");
    printf(" [Clic Derecho] - Menu de opciones\n");
    printf("========================================\n\n");
}

//Funcion principal
int main(int argc, char** argv) 
{
    //Inicializar GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("El Viaje de Xancho y Rony - Proyecto Final");
    
    //Inicializar OpenGL y datos
    iniciarOpenGL();
    inicializar();
    configurarPelicula();
    
    //Crear menú (clic derecho)
    glutCreateMenu(menu);
    glutAddMenuEntry("Reproducir / Reanudar", 1);
    glutAddMenuEntry("Pausar", 2);
    glutAddMenuEntry("Reiniciar Pelicula", 3);
    glutAddMenuEntry("Salir", 4);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    
    //Registrar funciones callback
    glutDisplayFunc(display);//Que dibujar
    glutIdleFunc(idle);//Animacion continua
    glutKeyboardFunc(teclado);//Controles de teclado
    
    //Mensaje inicial
    printf("La pelicula esta por comenzar\n");
    
    glutMainLoop();
    
    return 0;
}