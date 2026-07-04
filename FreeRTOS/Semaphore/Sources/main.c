#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define RCC_AHB1ENR   (*(volatile uint32_t*)0x40023830)

#define GPIOA_MODER   (*(volatile uint32_t*)0x40020000)
#define GPIOA_IDR     (*(volatile uint32_t*)0x40020010)
#define GPIOA_PUPDR   (*(volatile uint32_t*)0x4002000C)

#define GPIOD_MODER   (*(volatile uint32_t*)0x40020C00)
#define GPIOD_ODR     (*(volatile uint32_t*)0x40020C14)

#define SCB_CPACR     (*(volatile uint32_t*)0xE000ED88)

#define RCC_APB2ENR   (*(volatile uint32_t*)0x40023844)

#define SYSCFG_EXTICR1 (*(volatile uint32_t*)0x40013808)

#define EXTI_IMR      (*(volatile uint32_t*)0x40013C00)
#define EXTI_RTSR     (*(volatile uint32_t*)0x40013C08)
#define EXTI_PR       (*(volatile uint32_t*)0x40013C14)

#define NVIC_ISER0    (*(volatile uint32_t*)0xE000E100)

#define NVIC_IPR1 (*(volatile uint32_t*)0xE000E404)



TaskHandle_t LedTaskHandle = NULL;
SemaphoreHandle_t ButtonSemaphore = NULL;
volatile uint8_t buttonBusy = 0;

/*---------------- LED Task ----------------*/
void LedTask(void *pvParameters)
{
    (void)pvParameters;

    while (1)
    {
    	xSemaphoreTake(ButtonSemaphore, portMAX_DELAY);

        GPIOD_ODR |= (1 << 12);
        vTaskDelay(pdMS_TO_TICKS(500));

        GPIOD_ODR |= (1 << 13);
        vTaskDelay(pdMS_TO_TICKS(500));

        GPIOD_ODR |= (1 << 14);
        vTaskDelay(pdMS_TO_TICKS(500));

        GPIOD_ODR |= (1 << 15);
        vTaskDelay(pdMS_TO_TICKS(500));

        GPIOD_ODR &= ~((1<<12)|(1<<13)|(1<<14)|(1<<15));
        buttonBusy = 0;


    }
}
void EXTI0_IRQHandler(void)
{


    BaseType_t xHigherPriorityTaskWoken = pdFALSE;


    if (EXTI_PR & (1 << 0))
    {
        EXTI_PR = (1 << 0);


        if(buttonBusy == 0)
               {
                   buttonBusy = 1;

                   xSemaphoreGiveFromISR(ButtonSemaphore,
                                         &xHigherPriorityTaskWoken);
               }

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}


/*---------------- Button Task ----------------*/
void ButtonTask(void *pvParameters)
{
    (void)pvParameters;

    while (1)
    {
        if (GPIOA_IDR & (1 << 0))
        {
            vTaskDelay(pdMS_TO_TICKS(20));

            if (GPIOA_IDR & (1 << 0))
            {
            	xSemaphoreGive(ButtonSemaphore);

                while (GPIOA_IDR & (1 << 0));

                vTaskDelay(pdMS_TO_TICKS(20));
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}


int main(void)
{
    /* Enable FPU */
    SCB_CPACR |= (0xF << 20);

    /* Enable GPIOA & GPIOD Clock */
    RCC_AHB1ENR |= (1<<0);
    RCC_AHB1ENR |= (1<<3);

    /* PA0 Input */
    GPIOA_MODER &= ~(3U << (0*2));

    /* Pull-down on PA0 */
    GPIOA_PUPDR &= ~(3U << (0*2));
    GPIOA_PUPDR |=  (2U << (0*2));

    /* PD12-PD15 Output */
    for(int i=12;i<=15;i++)
    {
        GPIOD_MODER &= ~(3U << (i*2));
        GPIOD_MODER |=  (1U << (i*2));
    }

    ButtonSemaphore = xSemaphoreCreateBinary();

    configASSERT(ButtonSemaphore != NULL);

    xTaskCreate(
        LedTask,
        "LED",
        256,
        NULL,
        2,
        &LedTaskHandle);

 #if 0
    xTaskCreate(
        ButtonTask,
        "BUTTON",
        256,
        NULL,
        3,
        NULL);
 #endif
    /* Enable SYSCFG clock */
    RCC_APB2ENR |= (1 << 14);

    /* EXTI0 source = PA0 */
    SYSCFG_EXTICR1 &= ~(0xF << 0);

    /* Unmask EXTI0 */
    EXTI_IMR |= (1 << 0);

    /* Rising edge trigger */
    EXTI_RTSR |= (1 << 0);

    /* EXTI0 interrupt priority = 5 */
    NVIC_IPR1 &= ~(0xFF << 16);   // Clear IRQ6 priority field
    NVIC_IPR1 |=  (5 << 20);      // Set priority to 5

   /* Enable EXTI0 interrupt in NVIC */
    NVIC_ISER0 |= (1 << 6);



    vTaskStartScheduler();

    while(1);
}
