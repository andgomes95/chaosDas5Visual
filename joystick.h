#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/joystick.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

//Estrutura que encapsula as informações sobre a entrada recebida da manete
//to do: Eu acho que essa struct é redundante pois é a exata mesma struct js_event da biblioteca joystick.h
typedef struct{
	unsigned int type; //JS_EVENT_BUTTON, JS_EVENT_AXIS ou JS_EVENT_INIT
	unsigned int id; //Identifica qual botão/eixo foi acionado
	int value; //1(pressionado) ou 0(solto) para botões, [-32767, +32767] para eixos
	unsigned int time; //tempo em que o botão foi pressionado em milisegundos
}t_mosaic_button_event;

//Definição da assinatura da função callback de evento
typedef void (t_mosaic_joystick_event_callback_function)(
                t_mosaic_button_event *event
                );

//Definição da função callback de registro
//to do: Talvez, um callback diferente para esta situação não seja necessário, acho que as informações sobre o dispositivo deveriam ser salvas na estrutura t_mosaic_device_data
typedef void (t_mosaic_joystick_register_callback_function)(
                char * device,
                char * name,
                char number_of_axes,
                char number_of_buttons,
                int driver_version
                );

//Estrutura de cada device aberto, incluindo o caminho e as funções de callback
typedef struct {
    t_mosaic_joystick_event_callback_function * event_callback_function;
    t_mosaic_joystick_register_callback_function * register_callback_function;
    char * device;
} t_mosaic_device_data;

//Thread que abre e gerencia as informações e entradas de cada manete
void *joystick_thread(void *data){
	//Se uma função de callback não foi especificada a thread se encerra logo no início
	if (((t_mosaic_device_data *)data)->event_callback_function == NULL){
		pthread_exit((void *)NULL);
	}
	//Abertura do arquivo de entrada da manete
	int fd = open(((t_mosaic_device_data *)data)->device, O_RDONLY);
	//A chamada abaixo informa o nome do dispositivo
	//Se o nome não puder ser recebido ele é definido como "Unknow"	
	char name[128];
	if (ioctl(fd, JSIOCGNAME(sizeof(name)), name) < 0)
		strncpy(name, "Unknown", sizeof(name));
	//A chamada abaixo informa o número de eixos da manete
	char number_of_axes;
	ioctl(fd, JSIOCGAXES, &number_of_axes);
	//A chamada abaixo informa o número de botões da manete
	char number_of_buttons;
	ioctl(fd, JSIOCGBUTTONS, &number_of_buttons);
	//A chamada abaixo informa a versão do driver do dispositivo
	int driver_version;
	ioctl(fd,JSIOCGVERSION,&driver_version);
	//Se houver uma função de callback para registro das informações, ela é chamada aqui
	if (((t_mosaic_device_data *)data)->register_callback_function != NULL)
		((t_mosaic_device_data *)data)->register_callback_function(
			((t_mosaic_device_data *)data)->device,
			name,
			number_of_axes,
			number_of_buttons,
			driver_version);
	//A partir daqui os eventos das entradas da manete começam a serem lidos
	t_mosaic_button_event *btn_event = (t_mosaic_button_event *) malloc(sizeof(btn_event));
	struct js_event msg;
	//Primeiro os eventos de inicialização da manete são recebidos e ignorados
	do{
		//Nesse e no laço abaixo, se ocorreu um erro na hora de ler o evento a thread se encerrra		
		if(read(fd, &msg, sizeof(struct js_event)) != sizeof(struct js_event)){
			printf("JOYSTICK INITIALAZING ERROR\n");			
			pthread_exit((void *)NULL);
		}
	//O laço abaixo faz o tratamento dos eventos verdadeiros
	}while(msg.type == JS_EVENT_INIT);	
	while(1) {
		if(read(fd, &msg, sizeof(struct js_event)) != sizeof(struct js_event)){
			printf("EVENT READING ERROR\n");			
			pthread_exit((void *)NULL);
		}	
		btn_event->type = msg.type;
		btn_event->id = msg.number;
		btn_event->value = msg.value;
		btn_event->time = msg.time;
		//A chamada da função de callback para os eventos se encontra aqui
		((t_mosaic_device_data *)data)->event_callback_function(btn_event);
	}
}

//Parâmetros de entrada:
//device: string contendo o caminho para o arquivo de entrada
//event_callback_function: ponteiro para a função de callback para os eventos
//register_callback_function: ponteiro para a função de callback para o registro de informações
void joystick_inicialize(const char * device,
                t_mosaic_joystick_event_callback_function * event_callback_function,
                t_mosaic_joystick_register_callback_function * register_callback_function){
	t_mosaic_device_data * data = (t_mosaic_device_data *) malloc(sizeof(t_mosaic_device_data));
	data->device = (char *) malloc(strlen(device));
	strcpy(data->device, device);
	data->event_callback_function = event_callback_function;
	data->register_callback_function = register_callback_function;
	pthread_t tid;
	pthread_create(&tid, NULL, joystick_thread, data);
}


