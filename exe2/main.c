/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

 #include <stdio.h>
 #include "pico/stdlib.h"
 #include "hardware/gpio.h"
 #include "hardware/rtc.h"
 #include "pico/util/datetime.h"
 #include <string.h>
 #include "hardware/timer.h"
 
 const int ECHO_PIN1 = 18;
 const int TRIG_PIN1 = 19;

 const int ECHO_PIN2 = 12;
 const int TRIG_PIN2 = 13;
 
 volatile int time_init1 = 0;
 volatile int time_end1 = 0;
 volatile bool timeout_fired1 = false;
 volatile bool measuring1 = false;
 
 volatile int time_init2 = 0;
 volatile int time_end2 = 0;
 volatile bool timeout_fired2 = false;
 volatile bool measuring2 = false;
 
 const int TIMEOUT_US = 30000;
 
 int64_t alarm_callback1(alarm_id_t id, void *user_data)
 {
     timeout_fired1 = true;
     return 0;
 }
 
 int64_t alarm_callback2(alarm_id_t id, void *user_data)
 {
     timeout_fired2 = true;
     return 0;
 }
 
 void gpio_callback(uint gpio, uint32_t events)
{
    if (gpio == ECHO_PIN1)
    {
        if (events == GPIO_IRQ_EDGE_RISE)
        {
            time_init1 = to_us_since_boot(get_absolute_time());
            timeout_fired1 = false;
            add_alarm_in_us(TIMEOUT_US, alarm_callback1, NULL, false);
        }
        else if (events == GPIO_IRQ_EDGE_FALL)
        {
            time_end1 = to_us_since_boot(get_absolute_time());
            measuring1 = false;
        }
    }
    else if (gpio == ECHO_PIN2)
    {
        if (events == GPIO_IRQ_EDGE_RISE)
        {
            time_init2 = to_us_since_boot(get_absolute_time());
            timeout_fired2 = false;
            add_alarm_in_us(TIMEOUT_US, alarm_callback2, NULL, false);
        }
        else if (events == GPIO_IRQ_EDGE_FALL)
        {
            time_end2 = to_us_since_boot(get_absolute_time());
            measuring2 = false;
        }
    }
}
 
 
 // Função para medir distância
 void measure_distance_dual()
{
    gpio_put(TRIG_PIN1, 1);
    sleep_us(10);
    gpio_put(TRIG_PIN1, 0);
    measuring1 = true;

    gpio_put(TRIG_PIN2, 1);
    sleep_us(10);
    gpio_put(TRIG_PIN2, 0);
    measuring2 = true;

    sleep_ms(30);
}
 
 // Função para obter tempo atual
 void print_timestamp()
{
    datetime_t t = {0};
    rtc_get_datetime(&t);
    char datetime_buf[256];
    char *datetime_str = &datetime_buf[0];
    datetime_to_str(datetime_str, sizeof(datetime_buf), &t);
    printf("%s - ", datetime_str);
}
 
 // Função principal de leitura
 void read_sensor(bool is_running)
{
    if (!is_running)
        return;

    measure_distance_dual();

    // Leitura do Sensor 1
    if (timeout_fired1)
    {
        print_timestamp();
        printf("Sensor 1: Falha\n");
    }
    else if (time_end1 > time_init1)
    {
        int64_t duration1 = time_end1 - time_init1;
        float distance1 = (duration1 * 0.0343) / 2;
        print_timestamp();
        printf("Sensor 1: %.2f cm\n", distance1);
    }

    // Leitura do Sensor 2
    if (timeout_fired2)
    {
        print_timestamp();
        printf("Sensor 2: Falha\n");
    }
    else if (time_end2 > time_init2)
    {
        int64_t duration2 = time_end2 - time_init2;
        float distance2 = (duration2 * 0.0343) / 2;
        print_timestamp();
        printf("Sensor 2: %.2f cm\n", distance2);
    }
}

int main()
{
    stdio_init_all();

    gpio_init(ECHO_PIN1);
    gpio_set_dir(ECHO_PIN1, GPIO_IN);
    gpio_set_irq_enabled_with_callback(
        ECHO_PIN1, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    gpio_init(TRIG_PIN1);
    gpio_set_dir(TRIG_PIN1, GPIO_OUT);

    gpio_init(ECHO_PIN2);
    gpio_set_dir(ECHO_PIN2, GPIO_IN);
    gpio_set_irq_enabled_with_callback(
        ECHO_PIN2, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    gpio_init(TRIG_PIN2);
    gpio_set_dir(TRIG_PIN2, GPIO_OUT);

    datetime_t t = {
        .year = 2020,
        .month = 01,
        .day = 13,
        .dotw = 3, // 0 é domingo, então 3 é quarta-feira
        .hour = 11,
        .min = 20,
        .sec = 00};

    // Iniciar o RTC
    rtc_init();
    rtc_set_datetime(&t);
    //printf("Digite 'Start' para iniciar e 'Stop' para parar.\n");

    bool running = true;
    while (true)
    {
        // char command[10] = {0}; // Buffer para armazenar o comando
        // int index = 0;

        // while (index < 9)
        // {
        //     int caracter = getchar_timeout_us(100000); // Espera até 100ms

        //     if (caracter == PICO_ERROR_TIMEOUT)
        //     {
        //         break;
        //     }
        //     else if (caracter == '\n' || caracter == '\r')
        //     {
        //         command[index] = '\0';
        //         break;
        //     }
        //     else
        //     {
        //         command[index++] = (char)caracter;
        //     }
        // }

        // if (strcmp(command, "Start") == 0)
        // {
        //     running = true;
        //     printf("Leitura iniciada.\n");
        // }
        // else if (strcmp(command, "Stop") == 0)
        // {
        //     running = false;
        //     printf("Leitura parada.\n");
        // }

        read_sensor(running);
        sleep_ms(500);
    }
}
 