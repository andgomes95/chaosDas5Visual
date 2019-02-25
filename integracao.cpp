//Possui as funções MIDI
#include "MIDI_functions.c"
#include <omp.h>
//Para o open()
#include <fcntl.h>   
//Contem a biblioteca da manete
#include <linux/joystick.h>
//Para utilizar os threads, compilar com -pthread
#include <pthread.h>

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#define GL_GLEXT_PROTOTYPES 1
#include <unistd.h> 
#include <list> 
#include "shapes.cpp"
#define ESCAPE 27

//LINHA DE COMPILAÇÃO g++ integracao.cpp -o pratica -lX11 -lXi -lglut -lGL -lGLU -lm -lasound -lpthread -fopenmp

/***Variaveis Joystick***/
//As variáveis abaixo guardam informações sobre os botões da manete -> button[x] == 0 <--> o botão x não está pressionado
int button[12];
//axis[x] = valor da posição do eixo x (esse eixo pode ser horizontal ou vertical)
int axis[6];

/***Variaveis OpenGL***/
using namespace std; 

//Vetor de objetos a serem renderizados
list<obC> objetos; 
//instância da janela
int window;
//variaveis para screenshot
static GLubyte *pixels = NULL;
static const GLenum FORMAT = GL_RGBA;
static const GLuint FORMAT_NBYTES = 4;
static unsigned int nscreenshots = 0;
static unsigned int time_gl;
//Tamanho da tela
static const unsigned int HEIGHT = 400;
static const unsigned int WIDTH = 400;
//Angulos para rotação, em x, y e z
float angleX=0,angleY=0,angleZ=0;
//Variaveis auxiliar para seleção de tipo de objeto, de cor e de operação
int typeObject = 0;
int typeColor = 0;
int something = 0;
//variavel que define o quanto um objeto vai redimensionar
float valueSomething = 0.0;
//flags para definir em qual eixo o será girado o canvas
bool flagRotateX=false,flagRotateY=false,flagRotateZ=false;
//variaveis para definição de raio, posição do centro do objeto e da borda, no momento que será desenhado pela primeira vez
float raio=0.2,distX0=0.0,distY0=0.0,distX=0.0,distY=0.0;

//Assinatura de funções
static void limparTela();
void createObjectCanvas();
void change_2d_object_l1();
void change_2d_object_r1();
void change_3d_object_l2();
void change_3d_object_r2();


/***Funções Joystick***/
//Parâmetros de entrada:
//msg: Struct contendo informações sobre o evento de entrada da manete
//handle: Handle do sequenciador MIDI
//port_id: ID da porta de saída
void joystick_callback(struct js_event msg, snd_seq_t* handle, int port_id){
	if(msg.type == JS_EVENT_BUTTON){
//msg.number identifica o botão, msg.value diz se ele foi pressinado (1) ou solto (0)		
		button[msg.number] = msg.value;
//Este switch mapeia os botões da manete (excluindo o D-Pad)
//Esse mapeamento foi feito para a manete B-MAX (os valores variam de uma para a outra)		
		switch(msg.number){
			case 0:
				//Triângulo - Scalar
                something = 2;
			break;
			case 1:
				//Bolinha - rotate
                something = 4;
			break;
			case 2:
				//Cross - not enable 
			break;
			case 3:
				//Quadrado - Stop interactions
                something = 0;
			break;
			case 4:
				//L1 2d change
                change_2d_object_l1();
			break;
			case 5:
				//R1 2d change
                change_2d_object_r1();
			break;
			case 6:
				//L2 3d change
                change_3d_object_l2();
			break;
			case 7:
				//R2 3d change
                change_3d_object_r2();
			break;
			case 8:
				//Select - change color
                typeColor = rand()%8;
			break;
			case 9:
				//Start - PANIC BUTTON				
                limparTela();
			break;
			case 10:
				//L3
			break;
			case 11:
				//R3
			break;
		}
	}else if(msg.type == JS_EVENT_AXIS){
        /********************** MANIPULAR ESTA PARTE**********************/
//Nesse caso, msg.value guarda a posição do eixo		
		axis[msg.number] == msg.value;
//Este switch mapeia as alavancas e o D-PAD
		switch(msg.number){
		case 0:
//Para eixos horizontais, os valores crescem da esquerda para a direita, com o 0 com centro			
			//L3 Horizontal
		    if(msg.value > 0){
				//printf("AXIS 0 RIGHT\n");
			}else if(msg.value < 0){
				//printf("AXIS 0 LEFT\n");
			}else{
				//printf("AXIS 0 NEUTRAL\n");
			}
		break;
		case 1:
//Para eixoos verticais, os valores crescem de cima para baixo, com o 0 como centro
			//L3 Vertical
			if(msg.value > 0){
				//printf("AXIS 1 DOWN\n");
			}else if(msg.value < 0){
				//printf("AXIS 1 UP\n");
			}else{
				//printf("AXIS 1 NEUTRAL\n");
			}
		break;
		case 2:
			//R3 Horizontal				
			if(msg.value > 0){
				//printf("AXIS 2 DOWN\n");
			}else if(msg.value < 0){
				//printf("AXIS 2 UP\n");
			}else{
				//printf("AXIS 2 NEUTRAL\n");
			}
		break;
		case 3:
			//R3 Vertical				
			if(msg.value > 0){
				//printf("AXIS 3 DOWN\n"); 
			}else if(msg.value < 0){
				//printf("AXIS 3 UP\n");
			}else{
				//printf("AXIS 3 NEUTRAL\n");
			}
		break;
		case 4:
			//D-Pad Horizontal
			if(msg.value > 0){
				//printf("AXIS 4 RIGHT\n");
			}else if(msg.value < 0){
				//printf("AXIS 4 LEFT\n");
			}else{
				//printf("AXIS 4 NEUTRAL\n");
			}
		break;
		break;
		case 5:
			//D-pad Vertical
			if(msg.value > 0){
				//printf("AXIS 5 DOWN\n");
			}else if(msg.value < 0){
				//printf("AXIS 5 UP\n");
			}else{
				//printf("AXIS 5 NEUTRAL\n");
			}
		break;
		}
	}
}

void *thread(void *vargp){
	struct js_event msg;
	//Abre a conexão com a manete
	char* device = "/dev/input/js0";
	int fd = open(device, O_RDONLY);
	//Recebe o nome do dispositivo
	char name[128];
	if (ioctl(fd, JSIOCGNAME(sizeof(name)), name) < 0)
		strncpy(name, "Unknown", sizeof(name));
	//Recebe o número de eixos do dispositivo
	char number_of_axes;
	ioctl(fd, JSIOCGAXES, &number_of_axes);
	//Recebe o número de botões do dispositivo
	char number_of_buttons;
	ioctl(fd, JSIOCGBUTTONS, &number_of_buttons);
	//Abre um cliente com o sequenciador MIDI
	snd_seq_t* handle;
	int in_id, out_id;
	open_client(&handle, &in_id, &out_id);

	while(1){
		//Faz a leitura de um evento da manete, encerra a execução se não conseguir		
		if(read(fd, &msg, sizeof(struct js_event)) != sizeof(struct js_event)) {
			exit(1);
		}else{
			//Esse tipo de mensagem serve apenas para a inicialixação da manete e pode ser ignorada            		
			if (msg.type == JS_EVENT_INIT){
				continue;            		
			}else{
				//Chama a função de callback				
				joystick_callback(msg, handle, out_id);
			}
			usleep(10000);
		}
	}

	pthread_exit((void *)NULL);
}

//Função de inicialização da manete
void joystick_inicialize(){
	int i;
	//inicialização dos vetores do início do código	
	for(i=0; i<12; i++){
		button[i] = 0;
		if(i<6) axis[i] = 0;
	}
	pthread_t tid;
	pthread_create(&tid, NULL, thread, NULL);
	pthread_join(tid, NULL);
	pthread_exit((void *)NULL);
}

/***Funções OpenGL***/

static void init(void)  {
    glReadBuffer(GL_BACK);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glPixelStorei(GL_PACK_ALIGNMENT, 1); 
    /*(definir modos de armazenamento em pixels - 
        1º parametro -  pname: Especifica o nome simbólico do parâmetro a ser definido.
            Um valor afeta o empacotamento de dados de pixel na memória: GL_PACK_ALIGNMENT.
            O outro afeta a descompactação de dados de pixel da memória: GL_UNPACK_ALIGNMENT.)
        2º parametro - param: Especifica o valor para o qual pname está definido.*/
    glViewport(0, 0, WIDTH, HEIGHT);
    /* Seta a janela de exibição
    Parametros: GLint x,  GLint y,  GLsizei width,  GLsizei height*/
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);

    pixels = (GLubyte*)malloc(FORMAT_NBYTES * WIDTH * HEIGHT);
    time_gl = glutGet(GLUT_ELAPSED_TIME);
    /*glutGet recupera o estado GLUT simples representado por inteiros.
    GLUT_ELAPSED_TIME: Número de milissegundos desde que glutInit chamou (ou primeira chamada para glutGet (GLUT_ELAPSED_TIME)).
    */
}

static void deinit(void)  {
    /*liberar pixels*/
    free(pixels);
    objetos.clear();
}
/* Funcao que cria o print screen*/
static void create_ppm(char *prefix, int frame_id, unsigned int width, unsigned int height,
        unsigned int color_max, unsigned int pixel_nbytes, GLubyte *pixels) {
    size_t i, j, k, cur;
    enum Constants { max_filename = 256 };
    char filename[max_filename];
    snprintf(filename, max_filename, "%s%d.ppm", prefix, frame_id);
    FILE *f = fopen(filename, "w");
    fprintf(f, "P3\n%d %d\n%d\n", width, HEIGHT, 255);
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            cur = pixel_nbytes * ((height - i - 1) * width + j);
            
            fprintf(f, "%3d %3d %3d ", pixels[cur], pixels[cur + 1], pixels[cur + 2]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}
static void draw_scene(int,float,float,float,clr);
static void makeSomething(){
    switch(something){
        case 0:
            valueSomething = 0.000;
            flagRotateX=false;
            flagRotateY=false;
            flagRotateZ=false;
            break;
        case 1:
            angleX=0;
            angleY=0;
            angleZ=0;
            break;
        case 2:
            valueSomething = 0.001;
            break;
        case 3:
            flagRotateX=true;
            break;
        case 4:
            flagRotateY=true;
            break;
        case 5:
            flagRotateZ=true;
            
            break;
    }
}
bool big_image (const obC obj) { return (obj.raio>2.0); }

static void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    glRotatef(angleX,1.0,0.0,0.0);
    glRotatef(angleY,0.0,1.0,0.0);
    glRotatef(angleZ,0.0,0.0,1.0);
    objetos.remove_if (big_image);
    for (auto i = objetos.begin(); i!= objetos.end();++i){        
        draw_scene((*i).type,(*i).x0,(*i).y0,(*i).raio,(*i).color);
        makeSomething();
        (*i).raio = (*i).raio + valueSomething;
        if(something == 6){
            (*i).x0 = distX0;
            (*i).y0 = distY0;
        }
    }

    glutSwapBuffers();
    /*lê os pixels e salva na variavel "pixels"*/
    glReadPixels(0, 0, WIDTH, HEIGHT, FORMAT, GL_UNSIGNED_BYTE, pixels);
}

static void idle(void) {
    if(flagRotateX){
        angleX++; 
    }if(flagRotateY){
        angleY++; 
    }if(flagRotateZ){
        angleZ++; 
    }
    
    glutPostRedisplay();
}
static void limparTela(){
    objetos.clear();
}

static void draw_scene(int type, float x0, float y0, float raio, clr color) {
    switch(type){
        case 0:
            draw_circle(x0,y0,raio,color);
            break;
        case 1:
            draw_square(x0,y0,raio,color);
            break;
        case 2:
            draw_triangle(x0,y0,raio,color);
            break;
        case 3:
            draw_sphere(x0,y0,raio,color);
            break;
        case 4:
            draw_cube(x0,y0,raio,color);
            break;
        case 5:
            draw_cone(x0,y0,raio,color);
            break;
        case 6:
            draw_torus(x0,y0,raio,color);
            break;
        case 7:
            draw_teapot(x0,y0,raio,color);
            break;   
    }    
}


void changeRaio(){
    raio = sqrt(pow(distX-distX0,2)+pow(distY-distY0,2));
}

clr createColor(){
    clr color;
    switch(typeColor){
        case 0:
            color.r = 1.0f;
            color.g = 1.0f;
            color.b = 1.0f;
            break;
        case 1:
            color.r = 1.0f;
            color.g = 1.0f;
            color.b = 0.0f;
            break;
        case 2:
            color.r = 0.0f;
            color.g = 1.0f;
            color.b = 1.0f;
            break;
        case 3:
            color.r = 1.0f;
            color.g = 0.0f;
            color.b = 1.0f;
            break;
        case 4:
            color.r = 1.0f;
            color.g = 0.0f;
            color.b = 0.0f;
            break;
        case 5:
            color.r = 0.0f;
            color.g = 1.0f;
            color.b = 0.0f;
            break;
        case 6:
            color.r = 0.0f;
            color.g = 0.0f;
            color.b = 1.0f;
            break;
        case 7:
            color.r = 0.5f;
            color.g = 0.5f;
            color.b = 0.5f;
            break;
        case 8:
            color.r = 0.5f;
            color.g = 1.0f;
            color.b = 0.5f;
            break;

    }
    return color;
}

void createObjectCanvas(){
    changeRaio();
    obC objeto;
    objeto.type = typeObject;
    objeto.x0 = distX0;
    objeto.y0 = distY0;
    objeto.raio = raio;
    objeto.color = createColor();
    objetos.push_back(objeto);
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
        usleep(100);
        case ESCAPE:
            glutDestroyWindow(window); 
            exit(0);                   
        case 'q':
            typeColor = 0;
            break;
        case 'w':
            typeColor = 1;
            break;
        case 'e':
            typeColor = 2;
            break;
        case 'r':
            typeColor = 3;
            break;
        case 't':
            typeColor = 4;
            break;
        case 'y':
            typeColor = 5;
            break;
        case 'u':
            typeColor = 6;
            break;
        case 'i':
            typeColor = 7;
            break;
        case 'o':
            typeColor = 8;
            break;
        case 'p':
            typeColor = rand()%8;
            break;
        case 'a': 
            typeObject = 0;
            break;
        case 's':
            typeObject = 1;
            break;
        case 'd':
            typeObject = 2;
            break;
        case 'f':
            typeObject = 3;
            break;
        case 'g':
            typeObject = 4;
            break;
        case 'h':
            typeObject = 5;
            break;
        case 'j':
            typeObject = 6;
            break;
        case 'k':
            typeObject = 7;
            break;
        case 'l':
            limparTela();
            break;
        case 'z': 
            something = 0;
            break;
        case 'x':
            something = 1;
            break;
        case 'c':
            something = 2;
            break;
        case 'v':
            something = 3;
            break;
        case 'b':
            something = 4;
            break;
        case 'n':
            something = 5;
            break;
        case 'm':
            something = 6;
            break;
    }
    glutPostRedisplay();
}
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        puts("screenshot");
        create_ppm("tmp", nscreenshots, WIDTH, HEIGHT, 255, FORMAT_NBYTES, pixels);
        nscreenshots++;
    }
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
        distX0 = ((float)x/(float)HEIGHT-0.5)*2;
        distY0 = -((float)y/(float)WIDTH-0.5)*2;
        printf("%f\t%f\n\n",distX0,distY0);
    }
    if (button == GLUT_LEFT_BUTTON && state == GLUT_UP){
        distX = ((float)x/(float)HEIGHT-0.5)*2;
        distY = -((float)y/(float)WIDTH-0.5)*2;
        printf("%f\t%f\n\n",distX0,distY0);
        createObjectCanvas();
    }
}

void change_2d_object_l1(){
    if(typeObject>1 || typeObject<0){
        typeObject = 0;
    }else if(typeObject==0){
        typeObject = 1;
    }else if(typeObject==1){
        typeObject = 2;
    }
}

void change_2d_object_r1(){
    if(typeObject>2 || typeObject<1){
        typeObject = 2;
    }else if(typeObject==2){
        typeObject = 1;
    }else if(typeObject==1){
        typeObject = 0;
    }
}

void change_3d_object_l2(){
    if(typeObject>6 || typeObject<3){
        typeObject = 3;
    }else if(typeObject==3){
        typeObject = 4;
    }else if(typeObject==4){
        typeObject = 5;
    }else if(typeObject==5){
        typeObject = 6;
    }else if(typeObject==6){
        typeObject = 7;
    }
}

void change_3d_object_r2(){
    if(typeObject>7 || typeObject<4){
        typeObject = 7;
    }else if(typeObject==7){
        typeObject = 6;
    }else if(typeObject==6){
        typeObject = 5;
    }else if(typeObject==5){
        typeObject = 4;
    }else if(typeObject==4){
        typeObject = 3;
    }
}

int main_aux(int argc, char **argv) {
    GLint glut_display;
    
    glutInit(&argc, argv);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutInitWindowPosition(100, 100);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    window = glutCreateWindow(argv[0]);
    init();
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
   // glutFullScreen(); 
    glutDisplayFunc(display);

    glutIdleFunc(idle);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);
    atexit(deinit);
    glutMainLoop();
    return EXIT_SUCCESS;
}

int main (int argc, char *argv[]) {
    int nthreads, tid;

    /* Fork a team of threads giving them their own copies of variables */
    srand (time(NULL));
    #pragma omp parallel private(nthreads, tid)
    {

    /* Obtain thread number */
    tid = omp_get_thread_num();
    printf("Hello World from thread = %d\n", tid);

    /* Only master thread does this */
    if (tid == 0) 
        {
        nthreads = omp_get_num_threads();
        main_aux(argc,argv);
        }
    if(tid == 1)
    {
        joystick_inicialize();
    }

    }  /* All threads join master thread and disband */

}