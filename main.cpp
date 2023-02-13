/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"

int segments[] = {
    0b00111111,
    0b00000110,
    0b01011011,
    0b01001111,
    0b01100110,
    0b01101101,
    0b01111101,
    0b00000111,
    0b01111111,
    0b01101111,
    0b01110111,
    0b01111100,
    0b00111001,
    0b01011110,
    0b01111001,
    0b01110001
};

PortOut output(PortC, 0xff);
DigitalOut seg1(PC_12);
DigitalOut seg2(PC_11);
bool curr = false;

DigitalOut led(LED1);
InterruptIn clicker(PA_1, PullDown);

int clicks = 0;
int int_time = 0;

void isr_tim6()
{
    int_time++;
    TIM6->SR = 0;
}

void clicker_rise()
{
    clicks++;
}

void isr_tim7() {
    curr = !curr;

    if (curr) {
        int seg2_num = clicks & 0b00001111;
        seg1 = false;
        seg2 = true;
        output = segments[seg2_num];
    } else {
        int seg1_num = (clicks & 0b11110000) >> 4;
        seg2 = false;
        seg1 = true;
        output = segments[seg1_num];
    }

    TIM7->SR = 0;
}

void init_timer7()
{
    RCC->APB1ENR |= 0b100000; //TIM7

    TIM7->PSC = 31999;
    TIM7->ARR= 5 - 1;
    TIM7->CNT = 0;
    TIM7->SR = 0;
    TIM7->DIER = 1; 

    TIM7->CR1 = 1;

    NVIC_SetVector(TIM7_IRQn,(uint32_t)&isr_tim7);
    HAL_NVIC_EnableIRQ(TIM7_IRQn);
}

void init_timer6()
{
    RCC->APB1ENR |= 0b10000; //TIM6

    TIM6->PSC = 31999;
    TIM6->ARR= (1000 * 60) - 1;
    TIM6->CNT = 0;
    TIM6->SR = 0;
    TIM6->DIER = 1; 

    TIM6->CR1 = 1;

    NVIC_SetVector(TIM6_IRQn,(uint32_t)&isr_tim6);
    HAL_NVIC_EnableIRQ(TIM6_IRQn);
}

int main() {
    init_timer6();
    init_timer7();

    clicker.rise(clicker_rise);
    clicker.enable_irq();

    __enable_irq();

    while (true) {
        while (int_time < 10) {
            // printf("int_time: %d\n", int_time);
            ThisThread::sleep_for(100ms);
        }
        
        printf("out: %d\n", clicks);
        int_time = 0;
        clicks = 0;

        led = !led;
    }
}
