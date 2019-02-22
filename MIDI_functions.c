#include <alsa/asoundlib.h> //para compilar com essa biblioteca, precisa usar o -lasound

//Algumas funções do ALSA retornam valores negativos para indicar falhas de execução. Este #define permite encapsular as chamadas de função em uma estrutura que verifica esse retorno e
//exibe uma mensagem de erro  
#define CHK(stmt, msg) if((stmt) < 0) {puts("ERROR: "#msg); exit(1);}

//Parâmetros de entrada:
//handle: Varíavel que guarda o handle do sequenciador
//in_port_id: Variável que guarda o id da porta de entrada
//out_port_id: Variável que guarda o ida da porta de saída
//Descrição:
//Abre o ALSA Sequencer para entrada e saida e cria uma porta para cada função
int open_client(snd_seq_t** handle, int* in_port_id, int* out_port_id){
	CHK(snd_seq_open(handle, "default", SND_SEQ_OPEN_DUPLEX, 0), "Could not open sequencer");
//Se não for necessário criar duas portas, basta mudar o valor acima para SND_SEQ_OPEN_INPUT (ou OUTPUT) e usar apenas uma das ultimas duas funções.
	CHK(snd_seq_set_client_name(*handle, "MIDI Client"), "Could not set client name");
//A função acima define o nome do cliente, nesse caso "MIDI Client"
	CHK(*out_port_id = snd_seq_create_simple_port(*handle, "Out-port", SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ, SND_SEQ_PORT_TYPE_APPLICATION), "Could not open port");
	CHK(*in_port_id = snd_seq_create_simple_port(*handle, "In-port", SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE, SND_SEQ_PORT_TYPE_APPLICATION), "Could not open port");
	return 0;
}

//Parâmetros de entrada:
//vel: Velocidade do evento (0 - 127)
//note: Nota do evento (0 - 127)
//channel: Canal utilizado (Nota: para as aplicações atuais o canal pode ser o 0)
//handle: Handle do sequenciador
//port_id: id da porta de origem
//Descrição:
//Cria e faz o output de um evento MIDI do tipo NOTEON/NOTEOFF
void send_note(unsigned char vel, unsigned char note, int channel, snd_seq_t* handle, int port_id){
//Declaração do evento de saida	
	snd_seq_event_t out_ev;
//Este trecho é necessário para preparar um evento para envio 	
	snd_seq_ev_clear(&out_ev);
	snd_seq_ev_set_source(&out_ev, port_id);
	snd_seq_ev_set_subs(&out_ev);
	snd_seq_ev_set_direct(&out_ev);
	snd_seq_ev_set_fixed(&out_ev);
//Se a velocidade do evento for 0, significa que ele é do tipo NOTEOFF 
	if(vel == 0){
		out_ev.type = SND_SEQ_EVENT_NOTEOFF;
	}else{
		out_ev.type = SND_SEQ_EVENT_NOTEON;		
	}
	out_ev.data.note.velocity = vel;
	out_ev.data.note.channel = channel;
	out_ev.data.note.note = note;
//Os dois comandos abaixo são utilizados para fazer o envio do evento criado
	snd_seq_event_output(handle, &out_ev);
	snd_seq_drain_output(handle);		
}

//Parâmetros de entrada
//param: Parâmetro do controle, indentifica qual controle foi utilizado (0 - 127)
//value: Valor do controle. Pode ser 127 ou 0 para um botão indicando se ele foi precionado ou solto respectivamente
//       Para botões de deslizar, o valor varia dentro dessa faixa
//channel: Canda de destino do evento (Nota: para as aplicações atuais o canal pode ser o 0)
//handle: Handle do sequenciador
//port_id: id da porta de origem.
void send_control(unsigned char param, unsigned char value, int channel, snd_seq_t* handle, int port_id){
//Declaração do evento de saida	
	snd_seq_event_t out_ev;
//Este trecho é necessário para preparar um evento para envio 	
	snd_seq_ev_clear(&out_ev);
	snd_seq_ev_set_source(&out_ev, port_id);
	snd_seq_ev_set_subs(&out_ev);
	snd_seq_ev_set_direct(&out_ev);
	snd_seq_ev_set_fixed(&out_ev);

	out_ev.type = SND_SEQ_EVENT_CONTROLLER;
	
	out_ev.data.control.param = param;
	out_ev.data.control.value = value;
	out_ev.data.control.channel = channel;
//Os dois comandos abaixo são utilizados para fazer o envio do evento criado
	snd_seq_event_output(handle, &out_ev);
	snd_seq_drain_output(handle);
}

//Parâmetros de entrada
//param: Parâmetro do controle, indentifica qual controle foi utilizado (0 - 127)
//value: Valor do pitchbend. Varia entre -8192 e 8191.
//channel: Canda de destino do evento (Nota: para as aplicações atuais o canal pode ser o 0)
//handle: Handle do sequenciador
//port_id: id da porta de origem.
void send_pitchbend(unsigned char param, int value, int channel, snd_seq_t* handle, int port_id){
//Declaração do evento de saida	
	snd_seq_event_t out_ev;
//Este trecho é necessário para preparar um evento para envio
	snd_seq_ev_clear(&out_ev);
	snd_seq_ev_set_source(&out_ev, port_id);
	snd_seq_ev_set_subs(&out_ev);
	snd_seq_ev_set_direct(&out_ev);
	snd_seq_ev_set_fixed(&out_ev);

	out_ev.type = SND_SEQ_EVENT_PITCHBEND;
	
	out_ev.data.control.param = param;
	out_ev.data.control.value = value;
	out_ev.data.control.channel = channel;
//Os dois comandos abaixo são utilizados para fazer o envio do evento criado
	snd_seq_event_output(handle, &out_ev);
	snd_seq_drain_output(handle);
}

//Parâmetros de entrada:
//seq_handle: Handle do sequenciador
//Descrição:
//Recebe o handle do sequenciador e retorna um evento MIDI recebido pela porta de entrada
//Nota: Eu acho que o snd_seq_event_input faz um malloc para preencher o ev. Nesse caso, seria necessário dar um free no ev depois. Preciso confirmar
snd_seq_event_t *midi_read(snd_seq_t* seq_handle)
{
	snd_seq_event_t* ev = NULL;
   	snd_seq_event_input(seq_handle, &ev);	
	return ev;
}
