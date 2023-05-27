#ifndef simxxx_H_
#define simxxx_H_

#include "System_User.h"
#include "ds1302.h"
#include <stdbool.h>

//Version SIMXXX
#define VERSION2G		1
#define VERSION3G		0

#define RETURN_OK		1
#define RETURN_ERROR	0
#define RETURN_ERROR_TO_RESET	2

#define TIME_OUT_MAX			120	//Seconds
#define NUM_RETRIES				3	
#define NUM_RETRIES_SENDING		1	

//Define for select cellular network variables
//#define CLARO
//#define TIGO
//#define MOVISTAR
#define AVANTEL
//#define EXITO
//#define ETB

	#ifdef CLARO
		#define SIMXXX_NETWORK_APN		"internet.comcel.com.co"
		#define SIMXXX_NETWORK_USER		"comcel"
		#define SIMXXX_NETWORK_PASSWORD	"comcel"
	#endif

	#ifdef TIGO
		#define SIMXXX_NETWORK_APN		"web.colombiamovil.com.co"
		#define SIMXXX_NETWORK_USER		""
		#define SIMXXX_NETWORK_PASSWORD	""
	#endif

	#ifdef MOVISTAR
		#define SIMXXX_NETWORK_APN		"internet.movistar.com.co"
		#define SIMXXX_NETWORK_USER		"movistar"
		#define SIMXXX_NETWORK_PASSWORD	"movistar"
	#endif
	
	#ifdef AVANTEL
		#define SIMXXX_NETWORK_APN		"lte.avantel.com.co"
		#define SIMXXX_NETWORK_USER		""
		#define SIMXXX_NETWORK_PASSWORD	""
	#endif

	#ifdef EXITO
		#define SIMXXX_NETWORK_APN		"movilexito.net.co"
		#define SIMXXX_NETWORK_USER		""
		#define SIMXXX_NETWORK_PASSWORD	""
	#endif
	
	#ifdef ETB
		#define SIMXXX_NETWORK_APN		"moviletb.net.co"
		#define SIMXXX_NETWORK_USER		"etb"
		#define SIMXXX_NETWORK_PASSWORD	"etb"
	#endif


//Cloud
//#define CLOUD_HOST "http://sendmessagecornareiot.azurewebsites.net/device/message"
#define CLOUD_HOST "http://sendmessagecornareceaiot.azurewebsites.net/device/message"
	
#define CLOUD_CONTENT_TYPE "application/json"

char getData() ;
void simxxx_sync(void) ;
char simxxx_get_version(char expected[]) ;
char simxxx_get_timestamp(Time *gsmData);
char simxxx_only_send_command(char command[]) ;
char simxxx_send_command(char command[], char expected[]) ;
char simxxx_network_status(char command[], char expected[]);
uint8_t simxxx_power(void);
char simxxx_send_command_encrypt(uint8_t command[], uint16_t length_buffer, char expected[]) ;
char simxxx_read_http_response(char expected[]) ;
char simxxx_read_http_status(char expected[]);
void simxxx_reset_timeout(void);
char simxxx_open(void);
unsigned char simxxx_report_status(mesuared_data *gsmData, uint16_t ID);
char simxxx_get_date(Time *gsmData, char expected[]);
char simxxx_http_response_status (uint16_t response);
uint8_t simxxx_decimal_number(uint8_t number);
char simxxx_read_signal(mesuared_data *gsmData, char expected[]);
char simxxx_3G_send_command_encrypt(uint8_t command[], uint16_t length_buffer);
char simxxx_3G_get_date(Time *gsmData, char expected[]);
char simxxx_3G_response_length(char expected[], uint16_t *length_buffer);
char simxxx_3G_send_command(char command[], char expected[]);
char simxxx_3G_http_response(char input[]);
char simxxx_send_command_expected(char command[], char expected[]);

//TCP functions
uint8_t simxxx_TCPClient_Start();
uint8_t simxxx_TCPClient_Send(char* Data, uint16_t _length);
char simxxx_TCPClient_Close();
uint16_t simxxx_Create_Payload(mesuared_data *gsmData, uint16_t ID, char * payload);

   
#endif /* simxxx_H_ */