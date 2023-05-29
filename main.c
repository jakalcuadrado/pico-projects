/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "board_definitions.h"
#include "string.h"
#include "hw_config.h"

enum State
{
    State_Init,
    State_Get_Time,
    State_Read_Sensors,
    State_Treat_data,
    State_Wait,
    State_Reset
};

enum State current_State=State_Init;

#include "hardware/uart.h"
#define UART_ID uart0
#define BAUD_RATE 115200
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE
#define UART_TX_PIN 0
#define UART_RX_PIN 1

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
//sensor_t sensor_temperature_array_2;

uint8_t romcnt_array_1 = 0;     // No of temperature sensor found
uint8_t romcnt_array_2 = 0;     // No of temperature sensor found
uint8_t roms_array_1[40] = {0}; // temperature sensor name.
uint8_t roms_array_2[40] = {0}; // temperature sensor name.

// Prototype functions
void Sense_State_Function();

//Global variables
mesuared_data mesuaredData;

char getData()
{
    uint8_t data;
    data = 0;
    data = uart_getc(UART_ID);
    return data;
}

char simxxx_send_command(char command[], char expected[])
{

    uart_puts(UART_ID, command);
    printf(command);
    // wdt_reset();
    char input[70];
    int tries = 3;
    while (tries)
    {

        int i = 0;

        while (true)//TimeOutSim8xx.flag_counts != FLAG_OK)
        {
            // watchdog_reset();
            input[i] = uart_getc(UART_ID);

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
    uart_puts(UART_ID, command);
    printf("\r\n");
    printf(command);
    printf("\r\n");

        int i = 0;

    while (true) // TimeOutSim8xx.flag_counts != FLAG_OK)
    {
        // watchdog_reset();
        input[i] = uart_getc(UART_ID);

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

    printf("msj: %s\r\n", input);

    printf("char12: %c\r\n", input[bit_possition]);

    if (input[bit_possition] != '1')
    {
        printf("RETURN_ERROR, msj: %s\r\n",  input);
        return RETURN_ERROR;
    }     
    
    printf("RETURN_OK, msj: %s\r\n",  input);

    return RETURN_OK;
}

bool repeating_timer_callback(struct repeating_timer *t)
{
    printf("!!!!!Main Timer!!!!!\r\n");
    current_State = State_Get_Time;
    return true;
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
    

    // Initialize chosen serial port
    stdio_init_all();

    if (watchdog_caused_reboot())
    {
        printf("Rebooted by Watchdog!\n");
        return 0;
    }
    else
    {
        printf("Clean boot\n");
    }

    // Set up our UART with the required speed.
    uart_init(UART_ID, BAUD_RATE);

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Wait for user to press 'enter' to continue
    printf("\r\nPress 'enter' to start.\r\n");
    while (true)
    {
        buf[0] = getchar();
        if ((buf[0] == '\r') || (buf[0] == '\n'))
        {
            break;
        }
    }
    printf("\r\nUart Test. Press 'enter' to start.\r\n");
    while (true)
    {
        buf[0] = getchar();
        if ((buf[0] == '\r') || (buf[0] == '\n'))
        {
            break;
        }
    }
    // Send out a string, with CR/LF conversions
    uart_puts(UART_ID, " Hello, UART!\n");
    uart_puts(UART_ID, "55555");

    
    

    struct repeating_timer timer_sampling;
    add_repeating_timer_ms(30000, repeating_timer_callback, NULL, &timer_sampling);
    //watchdog_enable(60000, 1);
    // Wait for user to press 'enter' to continue
    printf("\r\nSD card test. Press 'enter' to start.\r\n");
    while (true)
    {
        if(current_State ==State_Init){
            printf("\r\nState INIT\r\n");

            simxxx_send_command("AT\r\n", "OK");
            simxxx_send_command("AT\r\n", "OK");
            simxxx_send_command("AT\r\n", "OK");
            simxxx_send_command("AT+CGNSPWR=1\r\n", "OK");
            current_State = State_Get_Time;
            current_State = State_Read_Sensors;
        }

        if (current_State == State_Get_Time)
        {
            printf("\r\nState GET TIME\r\n");
            current_State = State_Wait;
            if(simxxx_read_time("AT+CGNSINF\r\n", 12)){
                current_State = State_Read_Sensors;
            }
               
        }

        if (current_State == State_Read_Sensors)
        {
            uint32_t status=0;
            printf("\r\nState Read Sensors\r\n");
            //save_and_disable_interrupts();
            Sense_State_Function();
            //restore_interrupts(status);
            current_State = State_Read_Sensors;
        }
        

        if (current_State == State_Wait)
        {
            printf("\r\nState WAIT\r\n");
            sleep_ms(5000);
        }
        
        

    }
    while (true)
    {
        buf[0] = getchar();
        if ((buf[0] == '\r') || (buf[0] == '\n'))
        {
            break;
        }
    }

    // Initialize SD card
    if (!sd_init_driver())
    {
        printf("ERROR: Could not initialize SD card\r\n");
        while (true)
            ;
    }

    // Mount drive
    fr = f_mount(&fs, "0:", 1);
    if (fr != FR_OK)
    {
        printf("ERROR: Could not mount filesystem (%d)\r\n", fr);
        while (true)
            ;
    }

    // Open file for writing ()
    fr = f_open(&fil, filename, FA_WRITE | FA_OPEN_APPEND);
    if (fr != FR_OK)
    {
        printf("ERROR: Could not open file (%d)\r\n", fr);
        while (true)
            ;
    }

    // Write something to file
    ret = f_printf(&fil, "This is another test\r\n");
    if (ret < 0)
    {
        printf("ERROR: Could not write to file (%d)\r\n", ret);
        f_close(&fil);
        while (true)
            ;
    }

    ret = f_printf(&fil, "of writing to an SD card.\r\n");
    if (ret < 0)
    {
        printf("ERROR: Could not write to file (%d)\r\n", ret);
        f_close(&fil);
        while (true)
            ;
    }

    ret = f_printf(&fil, "A successor to the programming language B, C was originally developed at Bell Labs by Ritchie between 1972 and 1973 to construct utilities running on Unix. It was applied to re-implementing the kernel of the Unix operating system.[8] During the 1980s, C gradually gained popularity. It has become one of the most widely used programming languages,[9][10] with C compilers available for practically all modern computer architectures and operating systems. C has been standardized by ANSI since 1989 (ANSI C) and by the International Organization for Standardization (ISO).C is an imperative procedural language, supporting structured programming, lexical variable scope and recursion, with a static type system. It was designed to be compiled to provide low-level access to memory and language constructs that map efficiently to machine instructions, all with minimal runtime support. Despite its low-level capabilities, the language was designed to encourage cross-platform programming. A standards-compliant C program written with portability in mind can be compiled for a wide variety of computer platforms and operating systems with few changes to its source code.[11] Since 2000, C has consistently ranked among the top two languages in the TIOBE index, a measure of the popularity of programming languages.[12]\r\n");
    if (ret < 0)
    {
        printf("ERROR: Could not write to file (%d)\r\n", ret);
        f_close(&fil);
        while (true)
            ;
    }

    // Close file
    fr = f_close(&fil);
    if (fr != FR_OK)
    {
        printf("ERROR: Could not close file (%d)\r\n", fr);
        while (true)
            ;
    }

    // Open file for reading
    fr = f_open(&fil, filename, FA_READ);
    if (fr != FR_OK)
    {
        printf("ERROR: Could not open file (%d)\r\n", fr);
        while (true)
            ;
    }

    // Print every line in file over serial
    printf("Reading from file '%s':\r\n", filename);
    printf("---\r\n");
    while (f_gets(buf, sizeof(buf), &fil))
    {
        printf(buf);
    }
    printf("\r\n---\r\n");

    // Close file
    fr = f_close(&fil);
    if (fr != FR_OK)
    {
        printf("ERROR: Could not close file (%d)\r\n", fr);
        while (true)
            ;
    }

    // Unmount drive
    f_unmount("0:");

    // Loop forever doing nothing
    while (true)
    {
        sleep_ms(1000);
    }
}

void Sense_State_Function()
{
    float voltage, current;
    int16_t *result_ptr = mesuaredData.temperature_array;
    uint8_t error = 1;
    uint8_t error_1 = 1;
    uint8_t error_2 = 1;
    uint8_t temp_tries = 3;

    /// Get river level in main sensor
    //mesuaredData.level_int = MB7040_get_avg_measure(&i2c_US_main, ADD_I2C_US_MAIN);
    //printf("Measure in main sensor: %d cm \r\n", mesuaredData.level_int);

    //mesuaredData.level_10deg_int = 0;

    // Sensors detection.
    if (onewireInit(&sensor_temperature_array_1))
    {
        printf("Sensor Detected!!!!! \r\n");
    }
    
    onewireInit(&sensor_temperature_array_1);

        while (error_1 != DS18B20_ERROR_OK && temp_tries != 0)
    {
        temp_tries--;
      
        error_1 = ds18b20search(&sensor_temperature_array_1, &romcnt_array_1, roms_array_1, 40);
        
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


}
