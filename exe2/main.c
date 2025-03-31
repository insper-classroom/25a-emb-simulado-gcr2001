/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

 #include <stdio.h>
 #include <string.h> 
 #include "pico/stdlib.h"
 #include "hardware/gpio.h"
 #include "hardware/timer.h"
 #include "hardware/irq.h"
 
 const int ECHO_PIN_1 = 12;
 const int TRIGGER_PIN_1 = 13;
 
 volatile int alarm_1 = 0;
 volatile int echo_start_time_1 = 0;
 volatile int echo_end_time_1 = 0;
 
 const int ECHO_PIN_2 = 18;
 const int TRIGGER_PIN_2 = 19;
 
 volatile int alarm_2 = 0;
 volatile int echo_start_time_2 = 0;
 volatile int echo_end_time_2 = 0;
 
 volatile alarm_id_t echo_timeout_alarm_1;
 volatile alarm_id_t echo_timeout_alarm_2;
 
 float calcula_distancia_cm(uint64_t duracao_us) {
     // distancia = (duracao * velocidade_som) / 2
     // v_som = 343 m/s = 34300 cm / 1000000 us = 0.0343 cm/us
     return (duracao_us * 0.0343) / 2.0;
 }
 
 int64_t echo_timeout_callback_1(alarm_id_t id, void* user_data) {
     alarm_1 = 1;
     return 0;
 }
 
 int64_t echo_timeout_callback_2(alarm_id_t id, void* user_data) {
     alarm_2 = 1;
     return 0;
 }
 
 void echo_isr(uint gpio, uint32_t events) {
     if (gpio == ECHO_PIN_1) {
         if (events & GPIO_IRQ_EDGE_RISE) {
             echo_start_time_1 = get_absolute_time();
         }
         else if (events & GPIO_IRQ_EDGE_FALL) {
             echo_end_time_1 = get_absolute_time();
             cancel_alarm(echo_timeout_alarm_1);
         }
     }
 
     if (gpio == ECHO_PIN_2) {
         if (events & GPIO_IRQ_EDGE_RISE) {
             echo_start_time_2 = get_absolute_time();
         }
         else if (events & GPIO_IRQ_EDGE_FALL) {
             echo_end_time_2 = get_absolute_time();
             cancel_alarm(echo_timeout_alarm_2);
         }
     }
 }
 
 int main() {
     stdio_init_all();
 
     printf("oi \n");
     gpio_init(ECHO_PIN_1);
     gpio_init(TRIGGER_PIN_1);
     gpio_set_dir(ECHO_PIN_1, GPIO_IN);
     gpio_set_dir(TRIGGER_PIN_1, GPIO_OUT);
     gpio_put(TRIGGER_PIN_1, 0);
     gpio_set_irq_enabled_with_callback(ECHO_PIN_1, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &echo_isr);
 
 
     gpio_init(ECHO_PIN_2);
     gpio_init(TRIGGER_PIN_2);
     gpio_set_dir(ECHO_PIN_2, GPIO_IN);
     gpio_set_dir(TRIGGER_PIN_2, GPIO_OUT);
     gpio_put(TRIGGER_PIN_2, 0);
     gpio_set_irq_enabled_with_callback(ECHO_PIN_2, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &echo_isr);
 
     while (true) {
         alarm_1 = 0;
         echo_start_time_1 = 0;
         echo_end_time_1 = 0;
 
         alarm_2 = 0;
         echo_start_time_2 = 0;
         echo_end_time_2 = 0;
 
         sleep_us(100);
         gpio_put(TRIGGER_PIN_1, 1);
         gpio_put(TRIGGER_PIN_2, 1);
         sleep_us(10);
         gpio_put(TRIGGER_PIN_1, 0);
         gpio_put(TRIGGER_PIN_2, 0);
         echo_timeout_alarm_1 = add_alarm_in_ms(20, echo_timeout_callback_1, NULL, false);
         echo_timeout_alarm_2 = add_alarm_in_ms(20, echo_timeout_callback_2, NULL, false);
 
         while ((alarm_1 == 0) && (echo_end_time_1 == 0)) {}
 
         while ((alarm_2 == 0) && (echo_end_time_2 == 0)) {}
 
         if (alarm_1 == 1) {
             printf("Sensor 1 - dist: erro\n");
         }
         else {
             int dt_1 = echo_end_time_1 - echo_start_time_1;
             int distancia_1 = (int)((dt_1 * 0.0343) / 2.0);
             printf("Sensor 1 - dist: %d cm\n", distancia_1);
         }
 
         if (alarm_2 == 1) {
             printf("Sensor 2 - dist: erro\n");
         }
         else {
             int dt_2 = echo_end_time_2 - echo_start_time_2;
             int distancia_2 = (int)((dt_2 * 0.0343) / 2.0);
             printf("Sensor 2 - dist: %d cm\n", distancia_2);
         }
         cancel_alarm(echo_timeout_alarm_1);
         cancel_alarm(echo_timeout_alarm_2);
     }
 
     return 0;
 }
 