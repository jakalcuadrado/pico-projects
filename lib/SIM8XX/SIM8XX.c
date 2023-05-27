#include "SIM8XX.h"
#include "string.h"
#include "MQTT_protocol.h"
#include <stdlib.h>

extern time_out_struct TimeOutSim8xx;

volatile uint8_t version_simxxx = VERSION2G;

char getData() {
	struct io_descriptor *io;
	uint8_t data;
	data=0;
	usart_sync_get_io_descriptor(&USART_SIM, &io);
	
	//watchdog_disable();
	io_read(io, &data , sizeof(data));
	//watchdog_enable();
	return data;
}

void getDataArray() {
	struct io_descriptor *io;
	usart_sync_get_io_descriptor(&USART_SIM, &io);
	uint8_t data[300];
	uint8_t dato;
	uint16_t i=0;
	for (i=0; i <300; i++)
	{
		data[i]=io_read(io, &dato , sizeof(dato));

	}

	usb_printf("dato: %s\r\n",data);
	//watchdog_enable();

}


void simxxx_sync(void) {
	/*
	int i=20;
	
	while(1) {
	
	while(!uart_ready()&&i) {
	delay_ms(500);
	printf("AT");
	//wdt_reset();
	i--;
	}
	
	char input;
	input = getData();
	if (input == 'A' || input == 'O'|| input == '\n') break;
	}
	*/
	simxxx_send_command("AT\r\n","OK");
	
	//simxxx_send_command("AT+IPR=115200\r\n","OK");
	//wdt_reset();
	/*
	char input[10]={0};
	
	i=0;
	while(1) {
	//wdt_reset();
	input[i] = getData();
	
	if (input[0] == 'S') {
	if (input[8] == 'y') break; //READY
	i++;
	}
	}
	*/
}


char simxxx_only_send_command(char command[]) {
	usb_printf(command);
	printf(command);
	return RETURN_OK;
}




char simxxx_send_command(char command[], char expected[]) {
	
	simxxx_reset_timeout();
	usb_printf(command);
	printf(command);
	//wdt_reset();
	char input[70];
	int tries = 3;
	watchdog_disable();
	while(tries) {
		
		int i = 0;
		
		//uint32_t  not_empty = usart_sync_is_rx_not_empty(io);
		while(TimeOutSim8xx.flag_counts != FLAG_OK ) {
			//watchdog_reset();
			input[i] = getData();
			
			if (input[i] == '\n') break;
			i++;
			if (i == 70) break;
		}
		if (TimeOutSim8xx.flag_counts == FLAG_OK){
			usb_printf(KRED"TimeOut SIM!!\r\n"KNRM);
			watchdog_enable();
			return RETURN_ERROR;
		}
		
		//wdt_reset();
		input[i] = 0;
		if(strstr(input, expected)) {
			usb_printf("msj: %s\r\n",input);
			return RETURN_OK;
			}else if(strstr(input, "ERROR")){
			usb_printf("msj: %s\r\n",input);
			watchdog_enable();
			return RETURN_ERROR;
		}
		tries--;
	}
	usb_printf("RETURN_ERROR, tries: %d msj: %s\r\n",tries,input);
	
	watchdog_enable();
	return RETURN_ERROR;
}

uint8_t simxxx_power(void) //Power Down or power Up
{
	
	gpio_set_pin_level(SIM8XX_POWER_KEY, false);
	delay_ms(3500);
	gpio_set_pin_level(SIM8XX_POWER_KEY, true);
	delay_ms(5000);
	usb_printf("Power signal sent.\r\n");
	if(simxxx_send_command("AT\r\n","OK") == RETURN_OK){
		delay_ms(1);
		usb_printf("SIM TURNED ON\r\n");
		return 1;
		}else{
		delay_ms(1);
		usb_printf("SIM TURNED OFF\r\n");
		return 0;
	}
}



char simxxx_get_version(char expected[]) {
	simxxx_reset_timeout();
	char input[150];
	uint8_t i = 0;
	while(TimeOutSim8xx.flag_counts != FLAG_OK) {
		input[i] = getData();
		i++;
		//if (input[i] == '\n') break;
		if (i == 150) break;
		
		if(strstr(input, expected)) {
			if(strstr(input, "SIM808") || strstr(input, "SIM868")) {
				version_simxxx = VERSION2G;
				usb_printf("SIM808 or SIM868 version 2G SIM\r\n");
				return RETURN_OK;
			}
			if(strstr(input, "SIM5360")) {
				version_simxxx = VERSION3G;
				usb_printf("SIM5360 version 3G SIM\r\n");
				return RETURN_OK;
			}
		}
		if(strstr(input, "ERROR")) {
			version_simxxx=VERSION2G;
			usb_printf("Default version 2G SIM\r\n");
			return RETURN_ERROR;
		}
		
	}
	version_simxxx=VERSION2G;
	usb_printf("Default version 2G SIM\r\n");
	return RETURN_ERROR;
}



char simxxx_send_command_expected(char command[], char expected[]) {
	simxxx_reset_timeout();
	usb_printf(command);
	printf(command);
	char input[150];
	uint8_t i = 0;
	while(TimeOutSim8xx.flag_counts != FLAG_OK) {
		input[i] = getData();
		i++;
		//if (input[i] == '\n') break;
		if (i == 150) break;
		
		if(strstr(input, expected)) {
			usb_printf("Msj: %s\r\n",input);
			return RETURN_OK;
		}
		if(strstr(input, "ERROR")) {
			usb_printf("ERROR!!!\r\n",input);
			return RETURN_ERROR;
		}
		
	}
		usb_printf("ERROR!!! MSJ: %s\r\n",input);
	return RETURN_ERROR;
}


char simxxx_send_command_encrypt(uint8_t command[], uint16_t length_buffer, char expected[]) {
	
	usb_printf("Sending data ......\r\n");
	//usb_printf("\"");
	printf("\"");
	for(uint16_t k = 0; k < length_buffer; k++){
		delay_ms(10);
		//usb_printf("%02X",command[k]);
		printf("%02X",command[k]);
	}
	//usb_printf("\"\r\n");
	printf("\"\r\n");
	
	//wdt_reset();
	//char input[30];
	//int tries = 3;
	//while(tries) {
	//
	//int i = 0;
	//while(1) {
	//input[i] = getData();
	//if (input[i] == '\n') break;
	//i++;
	//if (i == 29) break;
	//}
	////wdt_reset();
	//input[i] = 0;
	//if(strstr(input, expected)) {
	//usb_printf("%s\r\n",input);
	//return RETURN_OK;
	//}else if(strstr(input, "ERROR")){
	//usb_printf("%s\r\n",input);
	//return RETURN_ERROR;
	//}
	//tries--;
	//}
	return RETURN_OK;
}


char simxxx_3G_send_command_encrypt(uint8_t command[], uint16_t length_buffer) {
	usb_printf("Sending data ......\r\n");
	//usb_printf("\"");
	printf("\"");
	for(uint16_t k = 0; k < length_buffer; k++){
		//delay_ms(10);
		//usb_printf("%02X",command[k]);
		printf("%02X",command[k]);
	}
	//usb_printf("\"\r\n");
	printf("\"\r\n");
	
	simxxx_reset_timeout();
	char input[60];
	uint8_t i = 0;

	//uint32_t  not_empty = usart_sync_is_rx_not_empty(io);
	while(TimeOutSim8xx.flag_counts != FLAG_OK ) {
		input[i] = getData();
		//if (input[i] == '\n') break;
		i++;
		if (i == 60) break;
		
		if(strstr(input, "+CHTTPSNOTIFY: PEER CLOSED") || strstr(input, "+CHTTPS: RECV EVENT")) {       ///expected OK
			usb_printf("OK message sended!!\r\n");
			return RETURN_OK;

		}
		
		if(strstr(input, "ERROR")) {
			//usb_printf("Msj5: %s\r\n",input);
			usb_printf(KRED"ERROR in command!!\r\n"KNRM);
			delay_ms(1);
			return RETURN_ERROR;
		}
		
	}
	if (TimeOutSim8xx.flag_counts == FLAG_OK){
		usb_printf("Msj4: %s\r\n",input);
		usb_printf(KRED"TimeOut SIM!!\r\n"KNRM);
		return RETURN_ERROR;
	}
	return RETURN_ERROR;
}


char simxxx_get_timestamp(Time *gsmData) {
	
	if(version_simxxx){	//For SIM8XX
		simxxx_only_send_command("AT+CIPGSMLOC=2,1\r\n");
		if (simxxx_get_date(gsmData,"OK"))
		{
			return RETURN_OK;
		}else{
			return RETURN_ERROR;
		}
	}
	
	if(!version_simxxx){	//For SIM53XX
		simxxx_only_send_command("AT+CASSISTLOC=1\r\n");
		if (simxxx_3G_get_date(gsmData,"+CASSISTLOC: 0") )
		{
			return RETURN_OK;
		}else{
			return RETURN_ERROR;
		}
	}
	return RETURN_OK;
}


char simxxx_get_date(Time *gsmData, char expected[]) {

	simxxx_reset_timeout();
	char input[50];
	char *strings[7];
	char *time[7];
	//uint8_t operation_mode;
	uint8_t i = 0;
	uint8_t tries = 3;
	while(tries) {
		
		i = 0;
		//uint32_t  not_empty = usart_sync_is_rx_not_empty(io);
		while(TimeOutSim8xx.flag_counts != FLAG_OK ) {
			input[i] = getData();
			//if (input[i] == '\n') break;
			i++;
			if (i == 50) break;
			
			if(strstr(input, expected)) {       ///expected OK
				//usb_printf("Msj: %s\r\n",input);
				int i = 0;
				strings[i] = strtok( input, "\r\n" );
				//usb_printf("msj0: %s and i: %d\r\n",strings[i],i);
				while( strings[i] != NULL  ){strings[++i] = strtok( NULL, "\r\n"  );/*usb_printf("msj1: %s and i: %d\r\n",strings[i],i);*/}
				i = 0;
				strings[i] = strtok( strings[0], "," );
				while( strings[i] != NULL  ){strings[++i] = strtok( NULL, ","  );usb_printf("msj1: %s and i: %d\r\n",strings[i],i);}
				
				
				//time[0]=strings[2];
				i = 0;
				time[i] = strtok( strings[2], ":" );	//en ocaciones se jode aca
				//usb_printf("msj0: %s and i: %d\r\n",time[i],i);
				while( time[i] != NULL  ){
					if (i==0 && time[i]!=NULL){
						gsmData->hour=atoi(time[i]);
					}else if (i==0)
					{
						gsmData->hour=0;
					}
					if (i==1 && time[i]!=NULL){
						gsmData->min=atoi(time[i]);
					}else if (i==1)
					{
						gsmData->min=0;
					}
					if (i==2 && time[i]!=NULL){
						gsmData->sec=atoi(time[i]);
					}else if (i==2)
					{
						gsmData->sec=0;
					}
					time[++i] = strtok( NULL, ":"  );
					//usb_printf("msj1: %s and i: %d\r\n",time[i],i);
				}
				/*
				usb_printf("hora %s minutos %s segundos %s\r\n",time_str[0],time_str[1],time_str[2]);
				if((time[0] != NULL) && (time[1]=! NULL) && (time[2]!=NULL)){
				gsmData->hour=atoi(time[0]);
				gsmData->min = atoi(time[1]);
				gsmData->second=atoi(time[2]);
				}else{
				gsmData->hour=0;
				gsmData->min = 0;
				gsmData->second=0;
				}
				*/
				//usb_printf("2hora %d minutos %s segundos %s\r\n",gsmData->hour,gsmData->min,gsmData->second);
				
				
				
				
				i = 0;
				//usb_printf("Fecha %s\r\n",strings[1]);
				strings[i] = strtok( strings[1], "/" );
				//usb_printf("msj0: %s and i: %d\r\n",strings[i],i);
				while( strings[i] != NULL  ){
					if (i==0 && strings[i]!=NULL){
						gsmData->year=atoi(strings[i]);
					}else if (i==0)
					{
						gsmData->year=0;
					}
					if (i==1 && strings[i]!=NULL){
						gsmData->mon=atoi(strings[i]);
					}else if (i==1)
					{
						gsmData->mon=0;
					}
					if (i==2 && strings[i]!=NULL){
						gsmData->date=atoi(strings[i]);
					}else if (i==2)
					{
						gsmData->date=0;
					}
					
					strings[++i] = strtok( NULL, "/"  );
					//usb_printf("msj1: %s and i: %d\r\n",strings[i],i);
				}
				/*
				usb_printf("Mes %s dia %s a?o %s\r\n",strings[1],strings[2],strings[0]);
				if((strings[1] =! NULL) && (strings[2] =! NULL) && (strings[0] != NULL)){
				gsmData->month=atoi(strings[1]);
				gsmData->day = atoi(strings[2]);
				gsmData->year=atoi(strings[0]);
				}else{
				gsmData->month=0;
				gsmData->day = 0;
				gsmData->year=0;
				}
				usb_printf("2Mes %s dia %s a?o %s\r\n",strings[1],strings[2],strings[0]);
				
				*/
				
				
				usb_printf("Current Time from GSM: %02d/%02d/%04d %02d:%02d:%02d+0\r\n",gsmData->mon,gsmData->date,gsmData->year,gsmData->hour,gsmData->min,gsmData->sec);
				
				//return_value=String(year+":"+month+":"+day+":"+time);
				//if(msj_echo)Serial.println(return_value);
				////2018-04-25T10:55:59
				//free(chr);
				//Serial_SIM808.flush();
				//if(msj_echo) Serial.println(return_value);
				//return return_value;
				return RETURN_OK;
				
				
				
				/*
				//usb_printf("Mensaje: %s\r\n",input);
				i = 0;
				strings[i] = strtok( input, "\"" );
				while( strings[i] != NULL  )strings[++i] = strtok( NULL, "\""  );
				operation_mode=atoi(strings[1]);
				usb_printf("mode  de operacion: %d\r\n",operation_mode);
				System_Set_Mode(operation_mode);
				if (operation_mode <1)
				{
				return RETURN_ERROR;
				}
				
				return RETURN_OK;
				}else if(strstr(input, "ERROR")){
				usb_printf("read_http_status2: %s\r\n",input);
				return RETURN_ERROR;
				*/
			}
			
			
		}
		if (TimeOutSim8xx.flag_counts == FLAG_OK){
			usb_printf(KRED"TimeOut SIM!!\r\n"KNRM);
			return RETURN_ERROR;
		}
		
		//wdt_reset();
		
		
		
		tries--;
	}
	return RETURN_ERROR;
	
}


char simxxx_3G_get_date(Time *gsmData, char expected[]) {
	
	simxxx_reset_timeout();
	char input[200];
	char *strings[7];
	char *time[7];
	//uint8_t operation_mode;
	uint8_t i = 0;
	uint8_t tries = 3;
	u_int32_t num, aux;
	
	while(tries) {
		
		i = 0;
		//uint32_t  not_empty = usart_sync_is_rx_not_empty(io);
		while(TimeOutSim8xx.flag_counts != FLAG_OK ) {
			input[i] = getData();
			//if (input[i] == '\n') break;
			i++;
			if (i == 200) break;
			
			if(strstr(input, expected)) {       ///expected OK
				//usb_printf("Msj: %s\r\n",input);
				int i = 0;
				strings[i] = strtok( input, "\r\n" );
				//usb_printf("msj0: %s and i: %d\r\n",strings[i],i);
				while( strings[i] != NULL  ){strings[++i] = strtok( NULL, "\r\n"  );/*usb_printf("msj1: %s and i: %d\r\n",strings[i],i);*/}

				time[0]=strings[1];
				i = 0;
				strings[i] = strtok( time[0], ":" );
				while( strings[i] != NULL  ){strings[++i] = strtok( NULL, ":"  );/*usb_printf("msj2: %s and i: %d\r\n",strings[i],i);*/}
				
				time[0]=strings[1];
				i = 0;
				strings[i] = strtok( time[0], "," );
				while( strings[i] != NULL  ){strings[++i] = strtok( NULL, ","  );/*usb_printf("msj3: %s and i: %d\r\n",strings[i],i);*/}
				
				//For date....
				num = atoi(strings[2]);
				aux = num/10000;
				num = num%10000;
				gsmData->date = (uint8_t)aux;
				
				aux = num/100;
				num = num%100;
				gsmData->mon = (uint8_t)aux;
				
				aux = num/1;
				num = num%1;
				gsmData->year = (uint16_t) (aux + 2000);
				
				//For Time....
				num = atoi(strings[3]);
				aux = num/10000;
				num = num%10000;
				gsmData->hour = (uint8_t)aux;
				
				aux = num/100;
				num = num%100;
				gsmData->min = (uint8_t)aux;
				
				aux = num/1;
				num = num%1;
				gsmData->sec = (uint8_t)aux;
				
				//usb_printf("Current Time from GSM: %02d/%02d/%04d %02d:%02d:%02d+0\r\n",gsmData->mon,gsmData->date,gsmData->year,gsmData->hour,gsmData->min,gsmData->sec);
				return RETURN_OK;

			}
			
			if(strstr(input, "ERROR")) {
				//usb_printf("Msj5: %s\r\n",input);
				usb_printf(KRED"ERROR in command!!\r\n"KNRM);
				delay_ms(1);
				return RETURN_ERROR;
			}
			if(strstr(input, "+CASSISTLOC: 4")) {
				//usb_printf("Msj6: %s\r\n",input);
				usb_printf(KRED"Network ERROR!!\r\n"KNRM);
				delay_ms(1);
				return RETURN_ERROR;
			}
			
			
		}
		if (TimeOutSim8xx.flag_counts == FLAG_OK){
			usb_printf("Msj4: %s\r\n",input);
			usb_printf(KRED"TimeOut SIM!!\r\n"KNRM);
			return RETURN_ERROR;
		}
		
		
		tries--;
	}
	return RETURN_ERROR;
	
}



char simxxx_read_http_response(char expected[]) {
	simxxx_reset_timeout();
	char input[30];
	char *strings[7];
	uint8_t operation_mode;
	uint8_t i = 0;
	while(TimeOutSim8xx.flag_counts != FLAG_OK) {
		input[i] = getData();
		i++;
		//if (input[i] == '\n') break;
		if (i == 30) break;
		
		if(strstr(input, expected)) {
			
			usb_printf("Mensaje: %s\r\n",input);
			i = 0;
			strings[i] = strtok( input, "\"" );
			while( strings[i] != NULL  )strings[++i] = strtok( NULL, "\""  );
			operation_mode=atoi(strings[1]);
			usb_printf("mode  de operacion: %d\r\n",operation_mode);
			System_Set_Mode(operation_mode);
			if (operation_mode <1)
			{
				return RETURN_ERROR;
			}
			
			return RETURN_OK;
			}else if(strstr(input, "ERROR")){
			usb_printf("read_http_status2: %s\r\n",input);
			return RETURN_ERROR;
		}
		
		
	}
	System_Set_Mode(0);
	usb_printf("Mensaje: %s\r\n",input);
	return RETURN_ERROR;
	
}




char simxxx_3G_read_http_response(char expected[]) {
	simxxx_reset_timeout();
	char input[500];
	char *strings[20];
	uint16_t i = 0;
	uint8_t count_dato=0, count_error=0;
	uint8_t flag_end_message=0;
	
	while(TimeOutSim8xx.flag_counts != FLAG_OK) {
		
		input[i] = getData();
		//usb_printf(KGRN "%s %d %d\r\n"KNRM,input,i,contador);
		
		//Search "CV: 0"
		if(count_dato == 0){
			if(input[i] == 'C' ){
				count_dato=1;
				//usb_printf("encontro C!!\r\n");
				}else{
				count_dato = 0;
			}
			i++;
			if (i == 500) break;
			continue;
		}
		if(count_dato == 1){
			if(input[i] == 'V'){
				count_dato =2;
				//usb_printf("encontro V!!\r\n");
				}else{
				count_dato = 0;
			}
			i++;
			if (i == 500) break;
			continue;
		}
		if(count_dato == 2){
			if(input[i] == ':'){
				count_dato =3;
				//usb_printf("encontro :!!\r\n");
				}else{
				count_dato = 0;
			}
			i++;
			if (i == 500) break;
			continue;
		}
		if(count_dato == 3){
			if(input[i] == ' '){
				count_dato =4;
				//usb_printf("encontro espacio!!\r\n");
				}else{
				count_dato = 0;
			}
			i++;
			if (i == 500) break;
			continue;
		}
		if(count_dato == 4){
			if(input[i] == '0'){
				count_dato ++;
				usb_printf("Found message\r\n");
				flag_end_message=1;
				break;
				}else{
				count_dato=0;
			}
		}
		if (i == 500) break;
		
		/////Search "ERROR"
		if(count_error == 0){
			if(input[i] == 'E' ){
				count_error=1;
				//usb_printf("encontro E!!\r\n");
				}else{
				count_error = 0;
			}
			i++;
			if (i == 500) break;
			continue;
		}
		if(count_error == 1){
			if(input[i] == 'R'){
				count_error =2;
				//usb_printf("encontro R!!\r\n");
				}else{
				count_error = 0;
			}
			i++;
			if (i == 500) break;
			continue;
		}
		if(count_error == 2){
			if(input[i] == 'R'){
				count_error =3;
				//usb_printf("encontro R!!\r\n");
				}else{
				count_error = 0;
			}
			i++;
			if (i == 500) break;
			continue;
		}
		if(count_error == 3){
			if(input[i] == 'O'){
				count_error =4;
				//usb_printf("encontro O!!\r\n");
				}else{
				count_error = 0;
			}
			i++;
			if (i == 500) break;
			continue;
		}
		if(count_error == 4){
			if(input[i] == 'R'){
				count_error ++;
				usb_printf(KRED"ERROR!!\r\n"KNRM);
				return RETURN_ERROR;
				}else{
				count_error=0;
			}
		}
		
		if (i == 500) break;
		i++;
		
	}
	
	
	if(flag_end_message) {
		i = 0;
		//usb_printf("EL MENSAJE ES1: %s \r\n",input);
		char *oldstr = malloc(sizeof(input));
		strcpy(oldstr,input);
		
		strings[i] = strtok( input, " " );
		while( strings[i] != NULL  ){strings[++i] = strtok( NULL, " "  );/*usb_printf("msj1: %s and i: %d\r\n",strings[i],i);*/}
		//usb_printf("Status HTTP: %s\r\n",strings[2]);
		
		if (simxxx_http_response_status(atoi(strings[2])))
		{
			//simxxx_3G_http_response(input);
			//usb_printf("EL MENSAJE ES2: %s \r\n",oldstr);
			i=0;
			strings[i] = strtok( oldstr, "\"" );
			//usb_printf("msj1: %s and i: %d\r\n",strings[i],i);
			while( strings[i] != NULL  ){strings[++i] = strtok( NULL, "\""  );/*usb_printf("msj1: %s and i: %d\r\n",strings[i],i);*/}
			usb_printf("Modo de operacion: %s\r\n",strings[1]);
				
			System_Set_Mode(atoi(strings[1]));
			
			free(oldstr);
			return RETURN_OK;
		}else{
			System_Set_Mode(0);
			free(oldstr);
			return RETURN_ERROR;
		}
		
		free(oldstr);
		//usb_printf("msj12: %s\r\n",strlen(input));
		
		
	}
	
	
	
	System_Set_Mode(0);
	usb_printf(KRED"Msj2: %s\r\n"KNRM,input);
	return RETURN_ERROR;

	
}



char simxxx_3G_send_command(char command[], char expected[]) {
	
	simxxx_reset_timeout();
	usb_printf(command);
	printf(command);

	char input[20];
	uint8_t i = 0;
	while(TimeOutSim8xx.flag_counts != FLAG_OK) {
		input[i] = getData();
		i++;
		if (i == 20) break;
		
		if(strstr(input, expected)) {
			
			usb_printf("Msj: %s\r\n",input);
			return RETURN_OK;
		}else
		if(strstr(input, "ERROR")){
			usb_printf("ERROR!: %s\r\n",input);
			return RETURN_ERROR;
		}
		
	}
	System_Set_Mode(0);
	usb_printf(KRED"Msj2: %s\r\n"KNRM,input);
	return RETURN_ERROR;
}

char simxxx_3G_response_length(char expected[], uint16_t *length_buffer) {
	simxxx_reset_timeout();
	char input[50];
	char *strings[2];
	char *dato[3];
	uint8_t i = 0;
	while(TimeOutSim8xx.flag_counts != FLAG_OK) {
		input[i] = getData();
		i++;
		if (i == 50) break;
		
		if(strstr(input, expected)) {
			
			//usb_printf("Msj: %s\r\n",input);
			i=0;
			strings[i] = strtok( input, "," );
			while( strings[i] != NULL  ){strings[++i] = strtok( NULL, ","  );/*usb_printf("msj1: %s and i: %d\r\n",strings[i],i);*/}
			
			i=0;
			dato[0] = strings[1];
			strings[i] = strtok( dato[0], "\r\n" );
			//usb_printf("msj2: %s and i: %d\r\n",strings[i],i);
			while( strings[i] != NULL  ){strings[++i] = strtok( NULL, "\r\n"  );/*usb_printf("msj2: %s and i: %d\r\n",strings[i],i);*/}
			
			*length_buffer=atoi(strings[0]);
			usb_printf("El tamano del buffer es de: %d\r\n",*length_buffer);
			
			return RETURN_OK;
		}else
		if(strstr(input, "ERROR")){
			usb_printf("read_http_status2: %s\r\n",input);
			return RETURN_ERROR;
		}
		
		
	}
	System_Set_Mode(0);
	usb_printf(KRED"Msj2: %s\r\n"KNRM,input);
	return RETURN_ERROR;
	
}



char simxxx_read_signal(mesuared_data *gsmData, char expected[]) {
	simxxx_reset_timeout();
	char input[40];
	char *strings[7];
	//uint8_t operation_mode;
	uint8_t i = 0;
	while(TimeOutSim8xx.flag_counts != FLAG_OK) {
		input[i] = getData();
		//usb_printf("msj1: %s and i: %d\r\n",input[i],i);
		i++;
		//if (input[i] == '\n') break;
		if (i == 40) break;
		
		if(strstr(input, expected)) {
			
			//usb_printf("Mensaje: %s\r\n",input);
			i = 0;
			strings[i] = strtok( input, "," );
			strings[i] = strtok( strings[i], ":" );
			while( strings[i] != NULL  ){
				if (i==1 && strings[i]!=NULL){
					gsmData->signal_strength=atoi(strings[i]);
				}else if (i==1)
				{
					gsmData->signal_strength=0;
				}
				
				
				strings[++i] = strtok( NULL, ":"  );
				//usb_printf("msj1: %s and i: %d\r\n",strings[i],i);
			}
			
			usb_printf("Signal GSM Value: %d\r\n",gsmData->signal_strength);
			
			return RETURN_OK;
			
			
			}else if(strstr(input, "ERROR")){
			usb_printf("read_signal: %s\r\n",input);
			return RETURN_ERROR;
		}
		
		
	}
	
	return RETURN_ERROR;
	
}



char simxxx_network_status(char command[], char expected[]) {
	simxxx_reset_timeout();
	usb_printf(command);
	printf(command);
	char input[200];
	char *strings[7];
	char *dato[4];
	//uint8_t operation_mode;
	uint8_t i = 0;
	uint8_t response=0;
	while(TimeOutSim8xx.flag_counts != FLAG_OK) {
		input[i] = getData();
		//usb_printf("msj1: %s and i: %d\r\n",input[i],i);
		i++;
		//if (input[i] == '\n') break;
		if (i == 200) break;
		
		if(strstr(input, expected)) {
			
			usb_printf("msj: ");
			i=0;
			strings[i] = strtok( input, "," );
			while( strings[i] != NULL  ){strings[++i] = strtok( NULL, ","  );/*usb_printf("msj1: %s and i: %d\r\n",strings[i],i);*/}
			
			i=0;
			dato[0] = strings[1];
			strings[i] = strtok( dato[0], "\r\n" );
			//usb_printf("msj2: %s and i: %d\r\n",strings[i],i);
			while( strings[i] != NULL  ){strings[++i] = strtok( NULL, "\r\n"  );/*usb_printf("msj2: %s and i: %d\r\n",strings[i],i);*/}
			
			response=atoi(strings[0]);
			switch (response)
			{
				case 0:
					usb_printf(KRED"Not registered, ME is not currently searching a new operator to register to\r\n"KNRM);
					return RETURN_ERROR;
					break;
				case 1:
					usb_printf("Registered, home network\r\n");
					return RETURN_OK;
					break;
				case 2:
					usb_printf(KRED"Not registered, but ME is currently searching a new operator to register to\r\n"KNRM);
					return RETURN_ERROR;
					break;
				case 3:
					usb_printf(KRED"Registration denied\r\n"KNRM);
					return RETURN_ERROR;
					break;
				case 4:
					usb_printf(KRED"Unknown\r\n"KNRM);
					return RETURN_ERROR;
					break;				
				case 5:
					usb_printf("Registered, roaming\r\n");
					return RETURN_OK;
					break;
				default:
					usb_printf(KRED"Unknown ERROR!!\r\n"KNRM);
					return RETURN_ERROR;
					break;	
			}		
			
		}else if(strstr(input, "ERROR")){
			usb_printf("read_signal: %s\r\n",input);
			return RETURN_ERROR;
		}
		
	}
	
	return RETURN_ERROR;
	
}



char simxxx_read_http_status(char expected[]) {
	simxxx_reset_timeout();
	char *strings[7];
	char input[40];
	char status[15];
	int tries = 2;
	uint8_t i;
	while(tries) {
		i = 0;
		while(TimeOutSim8xx.flag_counts != FLAG_OK) {
			//usb_printf("bandera: %d ",TimeOutSim8xx.flag_counts);
			//usb_printf("input: %s ",input[i]);
			//wdt_reset();
			input[i] = getData();
			//if (input[i] == '\n') break;
			i++;
			if (i == 40) break;
			
			if(strstr(input, expected)) {
				i = 0;
				//usb_printf("mensaje1: %s\r\n",input);
				while(TimeOutSim8xx.flag_counts != FLAG_OK) {
					status[i] = getData();
					i++;
					if (i == 15) break;
					if(strstr(status, "\r\n")) {
						//usb_printf("Mensaje: %s\r\n",status);
						i = 0;
						strings[i] = strtok( status, "," );
						while( strings[i] != NULL  )strings[++i] = strtok( NULL, ","  );
						usb_printf("Status HTTP: %s\r\n",strings[1]);
						return simxxx_http_response_status (atoi(strings[1]));
					}
				}
				
			}
		}
		if (TimeOutSim8xx.flag_counts == FLAG_OK){
			usb_printf("mensaje1: %s\r\n",input);
			usb_printf(KRED"TimeOut SIM!!\r\n"KNRM);
			return RETURN_ERROR;
		}
		tries--;
		
	}
	

	
	return RETURN_ERROR;
}













/*

char simxxx_read_http_status(void) {
simxxx_reset_timeout();
char input[40];
int tries = 2;
while(tries) {
int i = 0;
while(TimeOutSim8xx.flag_counts != FLAG_OK) {
//wdt_reset();
input[i] = getData();
//if (input[i] == '\n') break;
i++;
if (i == 40) break;
}
if (TimeOutSim8xx.flag_counts == FLAG_OK){
usb_printf("TimeOut SIM!!\r\n");
return RETURN_ERROR;
}
//wdt_reset();
usb_printf("read_http_status1: %s\r\n",input);
if (input[15] == '2') {
usb_printf("read_http_status2: %s\r\n",input);
return RETURN_OK;
}else if(input[0] == 'E'){
usb_printf("read_http_status3: %s\r\n",input);
return RETURN_ERROR;
}
tries--;
}
return RETURN_ERROR;
}
*/

void simxxx_reset_timeout(void){
	TimeOutSim8xx.flag_counts = FLAG_NO;
	TimeOutSim8xx.counts = 0;
}

char simxxx_open(void) {
	
	//Set TimeOut
	TimeOutSim8xx.flag_counts = FLAG_NO;
	TimeOutSim8xx.num_counts = TIME_OUT_MAX;
	TimeOutSim8xx.counts = 0;
	
	//FUNCIONA PARA PRENDER O APAGAR EL SIM
	uint8_t retires;
	watchdog_disable();
	for (retires=0; retires<2;retires++){
		gpio_set_pin_level(COMMPWR_SIM8XX,false);
		delay_ms(1000);
		gpio_set_pin_level(COMMPWR_SIM8XX,true);
		if (simxxx_power() != 1){
			usb_printf("SIM POWER DOWN\r\n");
			}else{
			usb_printf("SIM Power ON\r\n");
			break;
		}
	}
	if (retires>=2){
		usb_printf("SIM FATAL ERROR\r\n");
		watchdog_enable();
		return RETURN_ERROR;
	}
	
	//usb_printf("Waiting to authenticate\r\n");
	//delay_ms(60000);
	//usb_printf("Waiting to authenticate\r\n");
	//delay_ms(60000);
	usb_printf("Waiting to authenticate\r\n");


	delay_ms(30000);
	usb_printf("Done\r\n");
	
	watchdog_enable();
	
	
	//gpio_set_pin_level(simxxx_POWER_KEY, false);	//quitar
	//delay_ms(1500);//quitar
	//gpio_set_pin_level(simxxx_POWER_KEY, true);//quitar
	//delay_ms(500);//quitar

	watchdog_reset();
	// sync baud (send AT 'til OK)
	simxxx_sync();

	// disable echo
	uint8_t i=0;

	
	for (i=0; i<NUM_RETRIES;i++){
		watchdog_reset();
		if(simxxx_send_command("ATE0\r\n", "OK") == RETURN_OK){
			break;
		}
	}
	if(NUM_RETRIES<=i) return RETURN_ERROR;
		
		
	for (i=0; i<NUM_RETRIES;i++){
		watchdog_reset();
		simxxx_only_send_command("ATI\r\n");
		if(simxxx_get_version("OK") == RETURN_OK){
			break;
		}
	}
	if(NUM_RETRIES<=i) return RETURN_ERROR;
	
	
	
	if(version_simxxx){	//For SIM8XX
		for (i=0; i<NUM_RETRIES;i++){
			watchdog_reset();
			if(simxxx_send_command("AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n", "OK") == RETURN_OK){
				break;
			}
		}
		if(NUM_RETRIES<=i) return RETURN_ERROR;

		
		for (i=0; i<NUM_RETRIES;i++){
			watchdog_reset();
			if(simxxx_send_command("AT+SAPBR=3,1,\"APN\",\"" SIMXXX_NETWORK_APN "\"\r\n", "OK") == RETURN_OK){
				break;
			}
		}
		if(NUM_RETRIES<=i) return RETURN_ERROR;
		
		for (i=0; i<NUM_RETRIES;i++){
			watchdog_reset();
			if(simxxx_send_command("AT+SAPBR=3,1,\"USER\",\"" SIMXXX_NETWORK_USER "\"\r\n", "OK") == RETURN_OK){
				break;
			}
		}
		if(NUM_RETRIES<=i) return RETURN_ERROR;
		
		for (i=0; i<NUM_RETRIES;i++){
			watchdog_reset();
			if(simxxx_send_command("AT+SAPBR=3,1,\"PWD\",\"" SIMXXX_NETWORK_PASSWORD "\"\r\n", "OK") == RETURN_OK){
				break;
			}
		}
		if(NUM_RETRIES<=i) return RETURN_ERROR;
		

		for (i=0; i<NUM_RETRIES;i++){
			watchdog_reset();
			if(simxxx_send_command("AT+SAPBR=1,1\r\n", "OK") == RETURN_OK){
				break;
			}
		}
		if(NUM_RETRIES<=i) return RETURN_ERROR;
	}
	
	if(!version_simxxx){ //For SIM53XX
		for (i=0; i<NUM_RETRIES;i++){
			watchdog_reset();
			if(simxxx_send_command("AT+CGSOCKCONT=1,\"IP\",\"" SIMXXX_NETWORK_APN "\"\r\n", "OK") == RETURN_OK){
				break;
			}
		}
		if(NUM_RETRIES<=i) return RETURN_ERROR;
		
		for (i=0; i<NUM_RETRIES;i++){
			watchdog_reset();
			if(simxxx_send_command("AT+CSOCKAUTH=1,1,\"" SIMXXX_NETWORK_USER "\",\"" SIMXXX_NETWORK_PASSWORD "\"\r\n", "OK") == RETURN_OK){
				break;
			}
		}
		if(NUM_RETRIES<=i) return RETURN_ERROR;
		
		for (i=0; i<NUM_RETRIES;i++){
			watchdog_reset();
			if(simxxx_send_command("AT+CSOCKSETPN=1\r\n", "OK") == RETURN_OK){
				break;
			}
		}
		if(NUM_RETRIES<=i) return RETURN_ERROR;
		
		for (i=0; i<NUM_RETRIES;i++){
			watchdog_reset();
			if(simxxx_send_command("AT+CIPMODE=0\r\n", "OK") == RETURN_OK){
				break;
			}
		}
		if(NUM_RETRIES<=i) return RETURN_ERROR;

		for (i=0; i<NUM_RETRIES;i++){
			watchdog_reset();
			if(simxxx_send_command("AT+NETOPEN\r\n", "OK") == RETURN_OK){
				break;
			}
		}
		if(NUM_RETRIES<=i) return RETURN_ERROR;

		
		
	}
	delay_ms(10000);
	watchdog_reset();
	
	//simxxx_send_command("AT+SAPBR=2,1\r\n", "OK");
	//ret=simxxx_send_command("AT+SAPBR=1,1\r\n", "OK");
	
	//if (ret==RETURN_ERROR) {
	//ret=simxxx_send_command("AT+SAPBR=1,1\r\n", "OK");
	//if (ret==RETURN_ERROR) {
	//return RETURN_ERROR;
	//}
	//}
	
	

	
	
	return RETURN_OK;
}

/*
void simxxx_close(void) {
// close context
simxxx_send_command("AT+SAPBR=0,1\r\n", "OK");
// poweroff_sim800
PORTD &= ~(1<<DDD2);
}
*/


uint8_t simxxx_decimal_number(uint8_t number) {
	
	if(number<10){
		number = number*10;
	}

	return number;
	
}





unsigned char simxxx_report_status(mesuared_data *gsmData, uint16_t ID) {
	
	uint16_t length_buffer;
	char buffer[100];
	uint8_t i=0;
	char payload[600];
	
	watchdog_reset();
	
	if(version_simxxx){	//For SIM8XX
		simxxx_send_command("AT+HTTPTERM\r\n", "OK");
		
		for (i=0; i<NUM_RETRIES;i++){
			watchdog_reset();
			if(simxxx_send_command("AT+HTTPINIT\r\n", "OK") == RETURN_OK){
				break;
			}
		}
		if(NUM_RETRIES<=i) return RETURN_ERROR;

		for (i=0; i<NUM_RETRIES;i++){
			watchdog_reset();
			if(simxxx_send_command("AT+HTTPPARA = \"CID\",1\r\n", "OK") == RETURN_OK){
				break;
			}
		}
		if(NUM_RETRIES<=i) return RETURN_ERROR;
		
		for (i=0; i<NUM_RETRIES;i++){
			watchdog_reset();
			if(simxxx_send_command("AT+HTTPPARA=\"URL\",\"" CLOUD_HOST "\"\r\n", "OK") == RETURN_OK){
				break;
			}
		}
		if(NUM_RETRIES<=i) return RETURN_ERROR;

		for (i=0; i<NUM_RETRIES;i++){
			watchdog_reset();
			if(simxxx_send_command("AT+HTTPPARA=\"CONTENT\",\"" CLOUD_CONTENT_TYPE "\"\r\n", "OK") == RETURN_OK){
				break;
			}
		}
		if(NUM_RETRIES<=i) return RETURN_ERROR;
		
		watchdog_reset();

		if(NUM_US == 2){
			sprintf(payload, "{\"id\":\"%02d\",\"fecha\":\"%04d-%02d-%02dT%02d:%02d:%02d\",\"bateria\":{\"voltaje\":\"%02d.%02d\",\"corriente\":\"%02d.%02d\"},\"carga\":{\"voltaje\":\"%02d.%02d\",\"corriente\":\"%02d.%02d\"},\"nivel\":\"%03d\",\"eventos\":{\"banderas\":\"%02d\",\"diferenciaNivel\":\"%03d\"}}",
			ID, gsmData->year, gsmData->month, gsmData->day, gsmData->hour, gsmData->minute, gsmData->second, gsmData->battery.volt_integer, (gsmData->battery.volt_decimal),
			gsmData->battery.curr_integer, (gsmData->battery.curr_decimal), gsmData->load.volt_integer, (gsmData->load.volt_decimal), gsmData->load.curr_integer,
			(gsmData->load.curr_decimal),	gsmData->level_int, gsmData->signal_strength, gsmData->level_10deg_int);
		}
		if(NUM_US == 1){
			sprintf(payload, "{\"id\":\"%02d\",\"fecha\":\"%04d-%02d-%02dT%02d:%02d:%02d\",\"bateria\":{\"voltaje\":\"%02d.%02d\",\"corriente\":\"%02d.%02d\"},\"carga\":{\"voltaje\":\"%02d.%02d\",\"corriente\":\"%02d.%02d\"},\"nivel\":\"%03d\",\"eventos\":{\"banderas\":\"%02d\",\"diferenciaNivel\":\"%03d\"}}",
			ID, gsmData->year, gsmData->month, gsmData->day, gsmData->hour, gsmData->minute, gsmData->second, gsmData->battery.volt_integer, (gsmData->battery.volt_decimal),
			gsmData->battery.curr_integer, (gsmData->battery.curr_decimal), gsmData->load.volt_integer, (gsmData->load.volt_decimal), gsmData->load.curr_integer,
			(gsmData->load.curr_decimal),	gsmData->level_int, gsmData->signal_strength, gsmData->level_int);
		}

		

		usb_printf("El mensaje sin cifrar es:");
		usb_printf("%s\r\n", payload);
		length_buffer = length_AES_buff(payload);
		uint8_t buffer_aes[length_buffer];

		cifrado_AES(payload,&buffer_aes[0],length_buffer);
		delay_ms(10);
		//usb_printf("El tamaño es: %d\n\r",(sizeof(payload_encrypt)/sizeof(char)));
		
		//for(uint16_t k = 0; k < 224; k++){
		//delay_ms(10);
		//usb_printf("%02X",buffer_aes[k]);
		////printf("%02X",payload_encrypt[k]);
		//}
		
		snprintf(buffer,100,"AT+HTTPDATA=%d,8000\r\n",(length_buffer*2)+2); //*2 porque each byte se presenta con dos hex y se suman dos de las comillas del mensaje
		
		for (i=0; i<NUM_RETRIES;i++){
			watchdog_reset();
			if(simxxx_send_command(buffer, "DOWNLOAD") == RETURN_OK){
				break;
			}
		}
		if(NUM_RETRIES<=i) return RETURN_ERROR;
		
		delay_ms(500);
		for (i=0; i<NUM_RETRIES;i++){
			watchdog_reset();
			if(simxxx_send_command_encrypt(buffer_aes,length_buffer ,"OK") == RETURN_OK){
				break;
			}
		}
		if(NUM_RETRIES<=i) return RETURN_ERROR;
		
		
		for (i=0; i<NUM_RETRIES;i++){
			watchdog_reset();
			if(simxxx_send_command("AT+CGATT=1\r\n", "OK") == RETURN_OK){
				break;
			}
		}
		if(NUM_RETRIES<=i) return RETURN_ERROR;
		
		
		delay_ms(1000);
		for (i=0; i<NUM_RETRIES;i++){
			watchdog_reset();
			simxxx_only_send_command("AT+HTTPACTION=1\r\n");
			if(simxxx_read_http_status("HTTPACTION") == RETURN_OK){
			break;
			}
		}
		if(NUM_RETRIES<=i) return RETURN_ERROR;

		for (i=0; i<NUM_RETRIES;i++){
			watchdog_reset();
			simxxx_only_send_command("AT+HTTPREAD\r\n");
			if(simxxx_read_http_response("OK") == RETURN_OK){
				break;
			}
		}
		if(NUM_RETRIES<=i) return RETURN_ERROR;
		
		delay_ms(5000);
		for (i=0; i<NUM_RETRIES;i++){
			watchdog_reset();
			if(simxxx_send_command("AT+HTTPTERM\r\n", "OK") == RETURN_OK){
				break;
			}
		}
		if(NUM_RETRIES<=i) return RETURN_ERROR;
	}
		
		
	if(!version_simxxx){ //For SIM53XX
		
		for (i=0; i<NUM_RETRIES;i++){
			watchdog_reset();
			if(simxxx_network_status("AT+CREG?\r\n","OK") == RETURN_OK){
				break;
			}
		}
		if(NUM_RETRIES<=i) return RETURN_ERROR;
		
		simxxx_send_command("AT+CHTTPSSTOP\r\n", "OK");
		simxxx_send_command("AT+CHTTPSCLSE\r\n", "OK");
		
		for (i=0; i<NUM_RETRIES_SENDING;i++){
			watchdog_reset();
			if(simxxx_send_command("AT+CHTTPSSTART\r\n","OK") == RETURN_OK){
				break;
			}
		}
		if(NUM_RETRIES<=i) return RETURN_ERROR;
		
		for (i=0; i<NUM_RETRIES_SENDING;i++){
			watchdog_reset();
			if(simxxx_send_command("AT+CHTTPSOPSE=\"sendmessagecornareceaiot.azurewebsites.net\",80,1\r\n","OK") == RETURN_OK){
				break;
			}
		}
		if(NUM_RETRIES<=i) return RETURN_ERROR;

		watchdog_reset();
	
		if(NUM_US == 2){
			sprintf(payload, "{\"id\":\"%02d\",\"fecha\":\"%04d-%02d-%02dT%02d:%02d:%02d\",\"bateria\":{\"voltaje\":\"%02d.%02d\",\"corriente\":\"%02d.%02d\"},\"carga\":{\"voltaje\":\"%02d.%02d\",\"corriente\":\"%02d.%02d\"},\"nivel\":\"%03d\",\"eventos\":{\"banderas\":\"%02d\",\"diferenciaNivel\":\"%03d\"}}",
			ID, gsmData->year, gsmData->month, gsmData->day, gsmData->hour, gsmData->minute, gsmData->second, gsmData->battery.volt_integer, (gsmData->battery.volt_decimal),
			gsmData->battery.curr_integer, (gsmData->battery.curr_decimal), gsmData->load.volt_integer, (gsmData->load.volt_decimal), gsmData->load.curr_integer,
			(gsmData->load.curr_decimal),	gsmData->level_int, gsmData->signal_strength, gsmData->level_10deg_int);
		}
		if(NUM_US == 1){
			sprintf(payload, "{\"id\":\"%02d\",\"fecha\":\"%04d-%02d-%02dT%02d:%02d:%02d\",\"bateria\":{\"voltaje\":\"%02d.%02d\",\"corriente\":\"%02d.%02d\"},\"carga\":{\"voltaje\":\"%02d.%02d\",\"corriente\":\"%02d.%02d\"},\"nivel\":\"%03d\",\"eventos\":{\"banderas\":\"%02d\",\"diferenciaNivel\":\"%03d\"}}",
			ID, gsmData->year, gsmData->month, gsmData->day, gsmData->hour, gsmData->minute, gsmData->second, gsmData->battery.volt_integer, (gsmData->battery.volt_decimal),
			gsmData->battery.curr_integer, (gsmData->battery.curr_decimal), gsmData->load.volt_integer, (gsmData->load.volt_decimal), gsmData->load.curr_integer,
			(gsmData->load.curr_decimal),	gsmData->level_int, gsmData->signal_strength, gsmData->level_int);
		}
		
		usb_printf("El mensaje sin cifrar es:");
		usb_printf("%s\r\n", payload);
		length_buffer = length_AES_buff(payload);
		uint8_t buffer_aes[length_buffer];
		cifrado_AES(payload,&buffer_aes[0],length_buffer);
		delay_ms(10);
		
		char post_headers[200];
		sprintf(post_headers, "POST /device/message HTTP/1.1\r\nHost: sendmessagecornareceaiot.azurewebsites.net\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n",((length_buffer*2)+2));
		
		sprintf(buffer,"AT+CHTTPSSEND=%d\r\n",strlen(post_headers)+((length_buffer*2)+2)+2); //*2 porque each byte se presenta con dos hex y se suman dos de las comillas del mensaje

		for (i=0; i<NUM_RETRIES;i++){
			watchdog_reset();
			if( simxxx_3G_send_command(buffer,">") == RETURN_OK){
				break;
			}
		}
		
		if(NUM_RETRIES<=i) return RETURN_ERROR;


		for (i=0; i<NUM_RETRIES;i++){
			watchdog_reset();
			if(simxxx_only_send_command(post_headers) == RETURN_OK){
				break;
			}
		}
		
		if(NUM_RETRIES<=i) return RETURN_ERROR;
		
		for (i=0; i<NUM_RETRIES;i++){
			watchdog_reset();
			if(simxxx_3G_send_command_encrypt(buffer_aes,length_buffer) == RETURN_OK){
				break;
			}
		}
		if(NUM_RETRIES<=i) return RETURN_ERROR;
		
		uint16_t length_buffer;
		for (i=0; i<NUM_RETRIES;i++){
			watchdog_reset();
			simxxx_only_send_command("AT+CHTTPSRECV?\r\n");
			if(simxxx_3G_response_length("OK", &length_buffer) == RETURN_OK){
				break;
			}
		}
		if(NUM_RETRIES<=i) return RETURN_ERROR;
		
		if (length_buffer > (uint16_t) 500)
		{
			usb_printf("Response overflow\r\n");
			return RETURN_ERROR;
		}
		
		
		for (i=0; i<NUM_RETRIES;i++){
			watchdog_reset();
			simxxx_only_send_command("AT+CHTTPSRECV=500\r\n");
			if(simxxx_3G_read_http_response("+CHTTPSRECV: 0") == RETURN_OK){
				break;
			}
		}
		if(NUM_RETRIES<=i) return RETURN_ERROR;
		
		for (i=0; i<NUM_RETRIES;i++){
			watchdog_reset();
			if(simxxx_send_command("AT+CHTTPSCLSE\r\n","OK") == RETURN_OK){
				break;
			}
		}
		if(NUM_RETRIES<=i) return RETURN_ERROR;
		
		for (i=0; i<NUM_RETRIES;i++){
			watchdog_reset();
			if(simxxx_send_command("AT+CHTTPSSTOP\r\n","OK") == RETURN_OK){
				break;
			}
		}
		if(NUM_RETRIES<=i) return RETURN_ERROR;
		
	}
	return RETURN_OK;

}


char simxxx_3G_http_response(char input[]){
	uint8_t i=0;
	char *strings[4];
	i=0;
	strings[i] = strtok( input, "\"" );
	while( strings[i] != NULL  ){strings[++i] = strtok( NULL, "\""  );/*usb_printf("msj1: %s and i: %d\r\n",strings[i],i);*/}
	
	System_Set_Mode(atoi(strings[1]));			
	
	return RETURN_OK;	
}


char simxxx_http_response_status(uint16_t response){

	switch(response){
		case 200:
		usb_printf("200 OK\r\n");
		return RETURN_OK;
		break;
		case 302:
		usb_printf(KRED"302 Found\r\n"KNRM);
		return RETURN_ERROR;
		break;
		case 400:
		usb_printf(KRED"400 Bad Request\r\n"KNRM);
		return RETURN_ERROR;
		break;
		case 401:
		usb_printf(KRED"401 Unauthorized\r\n"KNRM);
		return RETURN_ERROR;
		break;
		case 402:
		usb_printf(KRED"402 Payment Required\r\n"KNRM);
		return RETURN_ERROR;
		break;
		case 403:
		usb_printf(KRED"403 Forbidden\r\n"KNRM);
		return RETURN_ERROR;
		break;
		case 404:
		usb_printf(KRED"404 Not Found\r\n"KNRM);
		return RETURN_ERROR;
		break;
		case 500:
		usb_printf(KRED"500 Internal Server Error\r\n"KNRM);
		return RETURN_ERROR;
		break;
		default:
		usb_printf(KRED"Error # %d\r\n"KNRM,response);
		return RETURN_ERROR;
		break;
	}
}

uint8_t simxxx_TCPClient_Start()
{
	simxxx_send_command("AT+CIPHEAD=0\r\n","OK");
	
	simxxx_send_command("AT+CIPSRIP=0\r\n","OK");
	
	for (int i=0; i<NUM_RETRIES;i++){
		watchdog_reset();
		//if(simxxx_send_command("AT+CIPOPEN=5,\"TCP\",\"" MQTT_BROKER "\"," MQTT_BROKER_PORT "\r\n","+CIPOPEN: 5,0") == RETURN_OK){
		if(simxxx_send_command_expected("AT+CIPOPEN=5,\"TCP\",\"" MQTT_BROKER "\"," MQTT_BROKER_PORT "\r\n","+CIPOPEN: 5,0") == RETURN_OK){
			return RETURN_OK;
			break;
		}
	}	
	return RETURN_ERROR_TO_RESET;
	                           
}

uint8_t simxxx_TCPClient_Send(char* Data, uint16_t _length)
{
	char error=0;
	char temp_buffer[30];
	sprintf(temp_buffer,"AT+CIPSEND=5,%d\r\n",_length);
	error=simxxx_3G_send_command(temp_buffer,">");
	if (error==RETURN_ERROR)
	{
		return RETURN_ERROR;
	}

	watchdog_reset();
	for(uint16_t k = 0; k < _length; k++){
		//delay_ms(10);
		//usb_printf("%02X",command[k]);
		printf("%c",Data[k]);
		usb_printf("%c",Data[k]);
	}

	
	simxxx_reset_timeout();
	char input[60];
	uint8_t i = 0;
	watchdog_reset();
	//uint32_t  not_empty = usart_sync_is_rx_not_empty(io);
	while(TimeOutSim8xx.flag_counts != FLAG_OK ) {
		input[i] = getData();
		//if (input[i] == '\n') break;
		i++;
		if (i == 60) break;
		memset(temp_buffer, 0, sizeof(temp_buffer));
		sprintf(temp_buffer,"+CIPSEND: 5,%d,%d\r\n",_length,_length);
		if(strstr(input, temp_buffer)) {       ///expected OK
			usb_printf("Msj4: %s\r\n",input);
			usb_printf("OK message sended!!\r\n");
			delay_ms(10);
			return RETURN_OK;

		}
	
		if(strstr(input, "ERROR")) {
			//usb_printf("Msj5: %s\r\n",input);
			usb_printf(KRED"ERROR in command!!\r\n"KNRM);
			delay_ms(1);
			return RETURN_ERROR;
		}
		
	}
	
	
	if (TimeOutSim8xx.flag_counts == FLAG_OK){
		usb_printf("Msj4: %s\r\n",input);
		usb_printf(KRED"TimeOut SIM!!\r\n"KNRM);
		return RETURN_ERROR;
	}
	return RETURN_ERROR;
}

char simxxx_TCPClient_Close()
{
	char error=0;
	error=simxxx_send_command_expected("AT+CIPCLOSE=5\r\n","+CIPCLOSE: 5,0\r\n");
	return error;
}

uint16_t simxxx_Create_Payload(mesuared_data *gsmData, uint16_t ID,char * payload) {
	
	

	sprintf(payload, "{\"id\":%d,\"DT\":\"%04d-%02d-%02dT%02d:%02d:%02d\",\"BT\":{\"V\":\"%02d.%02d\",\"I\":\"%02d.%02d\"},\"LD\":{\"V\":\"%02d.%02d\",\"I\":\"%02d.%02d\"},\"SN\":{\"L1\":%d,\"%02X\":%d,\"%02X\":%d,\"%02X\":%d,\"%02X\":%d,\"%02X\":%d,\"%02X\":%d,\"%02X\":%d,\"%02X\":%d,\"%02X\":%d,\"%02X\":%d},\"EV\":{\"SS\":%d,\"CT\":%u}}",
					ID, gsmData->year,
					gsmData->month, gsmData->day,
					gsmData->hour, gsmData->minute,
					gsmData->second,
					gsmData->battery.volt_integer,
					(gsmData->battery.volt_decimal),
					gsmData->battery.curr_integer,
					(gsmData->battery.curr_decimal),
					gsmData->load.volt_integer,
					(gsmData->load.volt_decimal),
					gsmData->load.curr_integer,
					(gsmData->load.curr_decimal),
					gsmData->level_int,
					gsmData->temperature_sensor_id[0],
					gsmData->temperature_array[0],
					gsmData->temperature_sensor_id[1],
					gsmData->temperature_array[1],
					gsmData->temperature_sensor_id[2],
					gsmData->temperature_array[2],
					gsmData->temperature_sensor_id[3],
					gsmData->temperature_array[3],
					gsmData->temperature_sensor_id[4],
					gsmData->temperature_array[4],
					gsmData->temperature_sensor_id[5],
					gsmData->temperature_array[5],
					gsmData->temperature_sensor_id[6],
					gsmData->temperature_array[6],
					gsmData->temperature_sensor_id[7],
					gsmData->temperature_array[7],
					gsmData->temperature_sensor_id[8],
					gsmData->temperature_array[8],
					gsmData->temperature_sensor_id[9],
					gsmData->temperature_array[9],
					gsmData->signal_strength,
					gsmData->credit
					);
			
					
	//sprintf(payload, "{\"id\":\"%02d\",\"fecha\":\"%04d-%02d-%02dT%02d:%02d:%02d\",\"bateria\":{\"voltaje\":\"%02d.%02d\",\"corriente\":\"%02d.%02d\"},\"carga\":{\"voltaje\":\"%02d.%02d\",\"corriente\":\"%02d.%02d\"},\"nivel\":\"%03d\",\"eventos\":{\"banderas\":\"%02d\",\"diferenciaNivel\":\"%03d\"}}",
	//ID, gsmData->year, gsmData->month, gsmData->day, gsmData->hour, gsmData->minute, gsmData->second, gsmData->battery.volt_integer, (gsmData->battery.volt_decimal),
	//gsmData->battery.curr_integer, (gsmData->battery.curr_decimal), gsmData->load.volt_integer, (gsmData->load.volt_decimal), gsmData->load.curr_integer,
	//(gsmData->load.curr_decimal),	gsmData->level_int, gsmData->signal_strength, gsmData->level_10deg_int);
		
			


		usb_printf("El mensaje sin cifrar es (%d):",strlen(payload));
		usb_printf("%s\r\n", payload);
	return strlen(payload);

}

