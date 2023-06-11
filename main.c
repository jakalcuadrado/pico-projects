/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "board_definitions.h"
#include "string.h"
#include "stdlib.h"
#include "hw_config.h"

enum State
{
    State_Init,
    State_Get_Time,
    State_Read_Sensors,
    State_Treat_data,
    State_Save_data,
    State_Wait,
    State_Reset
};

enum State current_State=State_Init;

#include "hardware/uart.h"
#define UART_SIM uart0
#define UART_SIM_BAUD_RATE 115200
#define UART_SIM_DATA_BITS 8
#define UART_SIM_STOP_BITS 1
#define UART_SIM_PARITY UART_PARITY_NONE
#define UART_SIM_TX_PIN 0
#define UART_SIM_RX_PIN 1

#define UART_LVL uart1
#define UART_LVL_BAUD_RATE 9600
#define UART_LVL_DATA_BITS 8
#define UART_LVL_STOP_BITS 1
#define UART_LVL_PARITY UART_PARITY_NONE
#define UART_LVL_TX_PIN 4
#define UART_LVL_RX_PIN 5
#define LVL_PIN 2

#include "sd_card.h"
#include "ff.h"

#include "hardware/timer.h"

#define RETURN_OK 1
#define RETURN_ERROR 0
#define RETURN_ERROR_TO_RESET 2

#include "hardware/watchdog.h"
#include "lib/ds18b20/ds18b20.h"
#include "lib/ds18b20/romsearch.h"
sensor_t sensor_temperature_array_1;
sensor_t sensor_temperature_array_2;

uint8_t romcnt_array_1 = 0;     // No of temperature sensor found
uint8_t romcnt_array_2 = 0;     // No of temperature sensor found
uint8_t roms_array_1[40] = {0}; // temperature sensor name.
uint8_t roms_array_2[40] = {0}; // temperature sensor name.

// Prototype functions
void temp_sensor_read();

//Global variables
mesuared_data mesuaredData;
char payload[150];

void simxxx_powerUp()
{
    gpio_init(6);
    gpio_set_dir(6, GPIO_OUT);
    gpio_put(6, false);

    gpio_put(6, true);
    sleep_ms(2000);
    gpio_put(6, false);
    sleep_ms(2000);
    sleep_ms(5000);
    sleep_ms(5000);
}

char simxxx_send_command(char command[], char expected[])
{
      
    uart_puts(UART_SIM, command);
    hw_clear_bits(&uart_get_hw(UART_SIM)->rsr, UART_UARTRSR_BITS);
    printf(command);
    // wdt_reset();
    char input[70];
    int tries = 3;
    while (tries)
    {

        int i = 0;
        sleep_ms(500);
        while (uart_is_readable(UART_SIM)) // TimeOutSim8xx.flag_counts != FLAG_OK)
        {
            
            // watchdog_reset();
            input[i] = uart_getc(UART_SIM);

            if (input[i] == '\n')
                break;
            i++;
            if (i == 70)
                break;
        }
        /*
        if (TimeOutSim8xx.flag_counts == FLAG_OK)
        {
            printf(KRED "TimeOut SIM!!\r\n" KNRM);
            watchdog_enable();
            return RETURN_ERROR;
        }
        */

        // wdt_reset();
        
        input[i] = 0;
        if (strstr(input, expected))
        {
            printf("msj: %s\r\n", input);
            return RETURN_OK;
        }
        else if (strstr(input, "ERROR"))
        {
            printf("msj: %s\r\n", input);
            
            return RETURN_ERROR;
        }
        tries--;
    }
    printf("RETURN_ERROR, msj: %s\r\n", input);

    
    return RETURN_ERROR;
}

char simxxx_read_time(char command[], int bit_possition)
{

    // wdt_reset();
    char input[70];
    // FIFO Rx Clean
    hw_clear_bits(&uart_get_hw(UART_SIM)->rsr, UART_UARTRSR_BITS);
    uart_puts(UART_SIM, command);
    printf("\r\n");
    printf(command);
    printf("\r\n");

    int i = 0;
    sleep_ms(100);

    while (uart_is_readable(UART_SIM)) // TimeOutSim8xx.flag_counts != FLAG_OK)
    {
        // watchdog_reset();
        input[i] = uart_getc(UART_SIM);

        //if (input[i] == '\n')
        //    break;
        i++;
        if (i == 70)
            break;
    }
    /*
    if (TimeOutSim8xx.flag_counts == FLAG_OK)
    {
        printf(KRED "TimeOut SIM!!\r\n" KNRM);
        watchdog_enable();
        return RETURN_ERROR;
    }
    */

    printf("msj: %s\r\n", input);

    printf("char%d: %c\r\n", bit_possition, input[bit_possition]);
    printf("char%d: %c\r\n", bit_possition + 1, input[bit_possition + 1]);

    if (input[bit_possition] != '2' && input[bit_possition+1] != '0')
    {
        printf("RETURN_ERROR, msj: %s\r\n",  input);
        return RETURN_ERROR;
    }     
    
    printf("RETURN_OK, msj: %s\r\n",  input);

    char subbuff[4];
    memcpy(subbuff, &input[bit_possition], 4);
    mesuaredData.year = atoi(subbuff);

    memcpy(subbuff, &input[bit_possition+4], 2);
    subbuff[2] = '\0';
    mesuaredData.month = atoi(subbuff);

    memcpy(subbuff, &input[bit_possition+6], 2);
    subbuff[2] = '\0';
    mesuaredData.day = atoi(subbuff);

    memcpy(subbuff, &input[bit_possition + 8], 2);
    subbuff[2] = '\0';
    mesuaredData.hour = atoi(subbuff);

    memcpy(subbuff, &input[bit_possition+10], 2);
    subbuff[2] = '\0';
    mesuaredData.minute = atoi(subbuff);

    memcpy(subbuff, &input[bit_possition+12], 2);
    subbuff[2] = '\0';
    mesuaredData.second = atoi(subbuff);

    printf("Current Time from GSM: %02d/%02d/%04d %02d:%02d:%02d+0\r\n", mesuaredData.day, mesuaredData.month, mesuaredData.year, mesuaredData.hour, mesuaredData.minute, mesuaredData.second);

    return RETURN_OK;
}

bool repeating_timer_callback(struct repeating_timer *t)
{
    printf("!!!!!Main Timer!!!!!\r\n");
    current_State = State_Get_Time;
    return true;
}

void lvl_sensor_read(){
    //FIFO Rx Clean
    hw_clear_bits(&uart_get_hw(UART_LVL)->rsr, UART_UARTRSR_BITS);
    gpio_put(LVL_PIN, true);
    
    printf("Reading level sensor\r\n");
    sleep_ms(100);
    char input_l[9];
    char aux_ch='a';
    int k=0;
    //gpio_put(LVL_PIN, true);
   

    while (uart_is_readable(UART_LVL)) 
    {
        

        aux_ch = uart_getc(UART_LVL);

        if (aux_ch == 'R')
            break;
    }

    while (uart_is_readable(UART_LVL)) 
    {
        aux_ch = uart_getc(UART_LVL);

       
        input_l[k] = aux_ch;
        k++;
        if (k == 8)
            break;
    }

    gpio_put(LVL_PIN, false);
    sleep_ms(100);
    input_l[k]='\0';

    char subbuff[5];
    subbuff[0] = input_l[0];
    subbuff[1] = input_l[1];
    subbuff[2] = input_l[2];
    subbuff[3] = input_l[3];
    subbuff[4] = '\0';

    printf("LVL: %s", subbuff);
    printf("\r\n");
    mesuaredData.level = atoi(subbuff);
}

int main()
{

    FRESULT fr;
    FATFS fs;
    FIL fil;
    int ret;
    char buf[100];
    char filename[] = "test00.txt";

   


    sensor_temperature_array_1.pin = 16;
    sensor_temperature_array_2.pin = 15;

    mesuaredData.level = 0;
    mesuaredData.temperature_array[0] = 0;
    mesuaredData.temperature_array[1] = 0;
    mesuaredData.temperature_array[2] = 0;
    mesuaredData.temperature_array[3] = 0;
    mesuaredData.temperature_array[4] = 0;
    mesuaredData.temperature_array[5] = 0;
    mesuaredData.temperature_array[6] = 0;
    mesuaredData.temperature_array[7] = 0;
    mesuaredData.temperature_array[8] = 0;
    mesuaredData.temperature_array[9] = 0;

    if (watchdog_caused_reboot())
    {
        printf("Rebooted by Watchdog!\n");
        return 0;
    }
    else
    {
        printf("Clean boot\n");
    }

    // Set up our UART_SIM with the required speed.
    uart_init(UART_SIM, UART_SIM_BAUD_RATE);
    uart_set_hw_flow(UART_SIM, false, false);
    gpio_set_function(UART_SIM_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_SIM_RX_PIN, GPIO_FUNC_UART);

    // Set up our UART_LVL with the required speed.
    uart_init(UART_LVL, UART_LVL_BAUD_RATE);
    uart_set_format(UART_LVL, UART_LVL_DATA_BITS, UART_LVL_STOP_BITS, UART_LVL_PARITY);
    //uart_set_hw_flow(UART_LVL, false, false);
    gpio_set_function(UART_LVL_RX_PIN, GPIO_FUNC_UART);
    //uart_set_fifo_enabled(UART_LVL, false);

    gpio_init(LVL_PIN);
    gpio_set_dir(LVL_PIN, GPIO_OUT);
    gpio_put(LVL_PIN, false);

    hw_clear_bits(&uart_get_hw(uart1)->rsr, UART_UARTRSR_BITS);

    stdio_init_all();

    simxxx_powerUp();

    printf("\r\nRushboard Init.\r\n");


    sprintf(payload, "{\"date\":\"%02d/%02d/%04d %02d:%02d:%02d\",\"level\":%d,\"T00\":%d,\"T01\":%d,\"T02\":%d,\"T03\":%d,\"T04\":%d,\"T05\":%d,\"T06\":%d,\"T07\":%d,\"T08\":%d,\"T09\":%d}\r\n",
            mesuaredData.day, mesuaredData.month, mesuaredData.year, mesuaredData.hour, mesuaredData.minute, mesuaredData.second,
            mesuaredData.level,
            mesuaredData.temperature_array[0],
            mesuaredData.temperature_array[1],
            mesuaredData.temperature_array[2],
            mesuaredData.temperature_array[3],
            mesuaredData.temperature_array[4],
            mesuaredData.temperature_array[5],
            mesuaredData.temperature_array[6],
            mesuaredData.temperature_array[7],
            mesuaredData.temperature_array[8],
            mesuaredData.temperature_array[9]);
    printf(payload);

    // Wait to turn on
    sleep_ms(5000);

    struct repeating_timer timer_sampling;
    add_repeating_timer_ms(30000, repeating_timer_callback, NULL, &timer_sampling);
    // watchdog_enable(60000, 1);



    while (true)
    {
        if(current_State ==State_Init){
            printf("\r\nState INIT\r\n");

            uart_puts(UART_SIM, "AT\r\n");
            printf("AT\r\n");
            sleep_ms(500);

            simxxx_send_command("AT\r\n", "OK");
            simxxx_send_command("AT\r\n", "OK");
            simxxx_send_command("ATE0\r\n", "OK");
            simxxx_send_command("AT+CGNSPWR=1\r\n", "OK");
            current_State = State_Get_Time;
        }

        if (current_State == State_Get_Time)
        {
            printf("\r\nState GET TIME\r\n");
            current_State = State_Wait;
            if(simxxx_read_time("AT+CGNSINF\r\n", 16)){
                current_State = State_Read_Sensors;
            }
            printf("Current Time from GSM: %02d/%02d/%04d %02d:%02d:%02d+0\r\n", mesuaredData.day, mesuaredData.month, mesuaredData.year, mesuaredData.hour, mesuaredData.minute, mesuaredData.second);
           }

        if (current_State == State_Read_Sensors)
        {
            printf("\r\nState Read Sensors\r\n");
            uint32_t status=0;
            printf("\r\nState Read Sensors\r\n");
            
            temp_sensor_read();
            
            lvl_sensor_read();
            sleep_ms(100);
            lvl_sensor_read();
            sleep_ms(100);
            lvl_sensor_read();
            


            current_State = State_Save_data;
        }

        if (current_State== State_Save_data)
        {
            printf("\r\nState Save Data\r\n");
            cancel_repeating_timer(&timer_sampling);

            sprintf(payload, "{\"date\":\"%02d/%02d/%04d %02d:%02d:%02d\",\"level\":%04d,\"%02X\":%d,\"%02X\":%d,\"%02X\":%d,\"%02X\":%d,\"%02X\":%d,\"%02X\":%d,\"%02X\":%d,\"%02X\":%d,\"%02X\":%d,\"%02X\":%d}\r\n",
                    mesuaredData.day, mesuaredData.month, mesuaredData.year, mesuaredData.hour, mesuaredData.minute, mesuaredData.second,
                    mesuaredData.level,
                    mesuaredData.temperature_sensor_id[0],
                    mesuaredData.temperature_array[0],
                    mesuaredData.temperature_sensor_id[1],
                    mesuaredData.temperature_array[1],
                    mesuaredData.temperature_sensor_id[2],
                    mesuaredData.temperature_array[2],
                    mesuaredData.temperature_sensor_id[3],
                    mesuaredData.temperature_array[3],
                    mesuaredData.temperature_sensor_id[4],
                    mesuaredData.temperature_array[4],
                    mesuaredData.temperature_sensor_id[5],
                    mesuaredData.temperature_array[5],
                    mesuaredData.temperature_sensor_id[6],
                    mesuaredData.temperature_array[6],
                    mesuaredData.temperature_sensor_id[7],
                    mesuaredData.temperature_array[7],
                    mesuaredData.temperature_sensor_id[8],
                    mesuaredData.temperature_array[8],
                    mesuaredData.temperature_sensor_id[9],
                    mesuaredData.temperature_array[9]);
            printf(payload);
            

            // Initialize SD card
            if (!sd_init_driver())
            {
                printf("ERROR: Could not initialize SD card\r\n");
            }
            // Mount drive
            fr = f_mount(&fs, "0:", 1);
            if (fr != FR_OK)
            {
                printf("ERROR: Could not mount filesystem (%d)\r\n", fr);
            }

            // Open file for writing ()
            fr = f_open(&fil, filename, FA_WRITE | FA_OPEN_APPEND);
            if (fr != FR_OK)
            {
                printf("ERROR: Could not open file (%d)\r\n", fr);
               
            }

            // Write something to file
            ret = f_printf(&fil, payload);
            if (ret < 0)
            {
                printf("ERROR: Could not write to file (%d)\r\n", ret);
                f_close(&fil);
                
            }

            // Close file
            fr = f_close(&fil);
            if (fr != FR_OK)
            {
                printf("ERROR: Could not close file (%d)\r\n", fr);
                
            }

            // Unmount drive
            f_unmount("0:");

            add_repeating_timer_ms(30000, repeating_timer_callback, NULL, &timer_sampling);

            current_State=State_Wait;
        }
        

        if (current_State == State_Wait)
        {
            printf("\r\nState WAIT\r\n");
            sleep_ms(5000);
        }
        
        

    }
    

        
}

void temp_sensor_read()
{

    printf("Reading Temperature Array\r\n");

    float voltage, current;
    int16_t *result_ptr = mesuaredData.temperature_array;
    uint8_t error = 1;
    uint8_t error_1 = 1;
    uint8_t error_2 = 1;
    uint8_t temp_tries = 3;

    // Sensors detection.
    while (error_1 != DS18B20_ERROR_OK && temp_tries != 0)
    {
        temp_tries--;
        error_1 = ds18b20search(&sensor_temperature_array_1, &romcnt_array_1, roms_array_1, 40);
    }

    temp_tries = 3;
    // Sensors detection.
    while (error_2 != DS18B20_ERROR_OK && temp_tries != 0)
    {
        temp_tries--;
        error_2 = ds18b20search(&sensor_temperature_array_2, &romcnt_array_2, roms_array_2, 40);

    }

    if (error_1 == DS18B20_ERROR_OK || error_2 == DS18B20_ERROR_OK)
    {
        printf("\r\n");
        printf("%d Sensor found!!!\r\n", romcnt_array_1 + romcnt_array_2);
        printf("Array one \r\n");

        for (int j = 0; j < romcnt_array_1; j++)
        {
            for (int i = 0; i < 8; i++)
            {
                printf("%02X", roms_array_1[i + j * 8]);
            }

            printf("\r\n");
        }

        printf("Array two \r\n");
        for (int j = 0; j < romcnt_array_2; j++)
        {
            for (int i = 0; i < 8; i++)
            {
                printf("%02X", roms_array_2[i + j * 8]);
            }

            printf("\r\n");
        }

        printf("\r\n");
    }
    else if (error == ONEWIRE_ERROR_COMM)
    {
        printf("error COMM \r\n");
    }

    for (int i = 0; i < romcnt_array_1; i++)
    {
        error_1 |= ds18b20convert(&sensor_temperature_array_1, roms_array_1 + i * 8);
    }

    for (int i = 0; i < romcnt_array_2; i++)
    {
        error_2 |= ds18b20convert(&sensor_temperature_array_2, roms_array_2 + i * 8);
    }

    if (error_1 == DS18B20_ERROR_OK || error_2 == DS18B20_ERROR_OK)
    {
        printf("Sensors Ready \r\n");
    }
    else
    {

        printf("Sensors convert error \r\n");
    }

    // Delay (sensor needs time to perform conversion)
    sleep_ms(1000);

    printf("doing for \r\n");
    temp_tries = 3;
    for (int i = 0; i < romcnt_array_1; i++)
    {
        error = ds18b20read(&sensor_temperature_array_1, roms_array_1 + i * 8, result_ptr++);

        if (error == DS18B20_ERROR_OK)
        {
            mesuaredData.temperature_sensor_id[i] = roms_array_1[i * 8 + 7];

            printf("Sensor %02X value: %d \r\n", mesuaredData.temperature_sensor_id[i], mesuaredData.temperature_array[i]);
        }
        else if (error == DS18B20_ERROR_PULL)
        {
            printf("Sensor %02X no read \r\n", roms_array_1[i * 8 + 7]);
            if (temp_tries != 0)
            {
                i--;
                result_ptr--;
                temp_tries--;
            }
        }
        else if (error == ONEWIRE_ERROR_COMM)
        {
            printf("Sensor %02X no read comm \r\n", roms_array_1[i * 8 + 7]);
            if (temp_tries != 0)
            {
                i--;
                result_ptr--;
                temp_tries--;
            }
        }
    }
    temp_tries = 3;

    for (int i = 0; i < romcnt_array_2; i++)
    {

        error = ds18b20read(&sensor_temperature_array_2, roms_array_2 + i * 8, result_ptr++);

        if (error == DS18B20_ERROR_OK)
        {
            mesuaredData.temperature_sensor_id[i + romcnt_array_1] = roms_array_2[i * 8 + 7];

            printf("Sensor %02X value: %d \r\n", mesuaredData.temperature_sensor_id[i + romcnt_array_1], mesuaredData.temperature_array[i + romcnt_array_1]);
        }
        else if (error == DS18B20_ERROR_PULL)
        {
            printf("Sensor %02X no read \r\n", roms_array_2[i * 8 + 7]);
            if (temp_tries != 0)
            {
                i--;
                result_ptr--;
                temp_tries--;
            }
        }
        else if (error == ONEWIRE_ERROR_COMM)
        {
            printf("Sensor %02X no read comm \r\n", roms_array_1[i * 8 + 7]);
            if (temp_tries != 0)
            {
                i--;
                result_ptr--;
                temp_tries--;
            }
        }
    }
}