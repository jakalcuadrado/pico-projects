/*
 * MQTT_protocol.h
 *
 * Created: 25/06/2019 11:04:14 a.m.
 *  Author: gmontanez
 */ 




//Protocol Definitions
#ifndef MQTT_PROTOCOL_H_
#define MQTT_PROTOCOL_H_

#include "SIM8XX.h"

//Server config
#define AIO_SERVER			"io.adafruit.com"		/* Adafruit server */
#define AIO_SERVER_PORT		"1883"					/* Server port */
#define AIO_BASE_URL		"/api/v2"				/* Base URL for api */
#define AIO_USERNAME		"cwdgdmsz"		/* Enter username here */
#define AIO_KEY				"HTKU3PUrDuUZ"			/* Enter AIO key here */
//#define AIO_FEED			"Enter Feed Key"		/* Enter feed key */

//#define AIO_USERNAME		"HTTPSPrueba.azure-devices.net/999/?api-version=2018-06-30"		/* Enter username here */
//#define AIO_KEY				"SharedAccessSignature sr=HTTPSPrueba.azure-devices.net&sig=ySfVlwQdZEfn8KMBiY1s6keTJBt93QpjnYkdeIg3YR8%3D&se=1594571776&skn=iothubowner"			/* Enter AIO key here */
//#define AIO_FEED			"Enter Feed Key"		/* Enter feed key */
//
//#define MQTT_BROKER			"iot.eclipse.org"
//#define MQTT_BROKER_PORT	"1883"

#define MQTT_BROKER			"postman.cloudmqtt.com"
#define MQTT_BROKER_PORT	"12032"

#define MQTT_TOPIC_PUBLISH_01 "IJA/CEAIOT/MAGDALENA/CIENAGA/LARINCONADA"

//#define MQTT_BROKER			  "HTTPSPrueba.azure-devices.net"
//#define MQTT_BROKER_PORT	  "1883"
//#define MQTT_TOPIC_PUBLISH_01 "devices/999/messages/events/"

#define MQTT_PROTOCOL_LEVEL		4

#define MQTT_CTRL_CONNECT		0x1
#define MQTT_CTRL_CONNECTACK	0x2
#define MQTT_CTRL_PUBLISH		0x3
#define MQTT_CTRL_PUBACK		0x4
#define MQTT_CTRL_PUBREC		0x5
#define MQTT_CTRL_PUBREL		0x6
#define MQTT_CTRL_PUBCOMP		0x7
#define MQTT_CTRL_SUBSCRIBE		0x8
#define MQTT_CTRL_SUBACK		0x9
#define MQTT_CTRL_UNSUBSCRIBE	0xA
#define MQTT_CTRL_UNSUBACK		0xB
#define MQTT_CTRL_PINGREQ		0xC
#define MQTT_CTRL_PINGRESP		0xD
#define MQTT_CTRL_DISCONNECT	0xE

#define MQTT_QOS_1				0x1
#define MQTT_QOS_0				0x0

/* Adjust as necessary, in seconds */
#define MQTT_CONN_KEEPALIVE		60

#define MQTT_CONN_USERNAMEFLAG	0x80
#define MQTT_CONN_PASSWORDFLAG	0x40
#define MQTT_CONN_WILLRETAIN	0x20
#define MQTT_CONN_WILLQOS_1		0x08
#define MQTT_CONN_WILLQOS_2		0x18
#define MQTT_CONN_WILLFLAG		0x04
#define MQTT_CONN_CLEANSESSION	0x02




uint16_t MQTT_connectpacket(uint8_t* packet);
uint16_t MQTT_publishPacket(uint8_t *packet, const char *topic, char *data, uint8_t qos);
uint16_t MQTT_subscribePacket(uint8_t *packet, const char *topic, uint8_t qos);

uint8_t* AddStringToBuf(uint8_t *_buf, const char *_string);
//uint16_t StringToUint16(uint8_t* _String);



#endif /* MQTT_PROTOCOL_H_ */