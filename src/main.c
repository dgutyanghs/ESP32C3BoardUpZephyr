#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

int main(void)
{
    /* Print a single startup string to the UART console */
    printk("Hello from ESP32-C3 running on Zephyr RTOS!\n");

    int counter = 0;

    while (1) {
        printk("Running loop iteration: %d\n", counter++);
        k_msleep(1000); // Sleep for 1 second (1000 milliseconds)
    }

    return 0;
}
