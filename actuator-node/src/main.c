#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/sys/printk.h>

#define PWM_THREAD_STACK_SIZE 1024
#define PWM_THREAD_PRIORITY 5
#define SERVO_PERIOD PWM_MSEC(20)   // 20 ms = 50 Hz
#define SERVO_MIN_PULSE PWM_USEC(1000) // 1 ms pulse = ~0 deg
#define SERVO_MAX_PULSE PWM_USEC(2000) // 2 ms pulse = ~90 deg
#define SERVO_STEP_DELAY_MS 1000      // Delay between movements

#define PWM_NODE DT_ALIAS(pwmsignal)
static const struct pwm_dt_spec pwm_signal = PWM_DT_SPEC_GET(PWM_NODE);

void pwm_thread(void *arg1, void *arg2, void *arg3)
{
    if (!device_is_ready(pwm_signal.dev)) {
        printk("PWM device %s not ready\n", pwm_signal.dev->name);
        return;
    }

    while (1) {
        // Move to one extreme
        pwm_set_dt(&pwm_signal, SERVO_PERIOD, SERVO_MIN_PULSE);
        k_msleep(SERVO_STEP_DELAY_MS);

        // Move to the other extreme
        pwm_set_dt(&pwm_signal, SERVO_PERIOD, SERVO_MAX_PULSE);
        k_msleep(SERVO_STEP_DELAY_MS);
    }
}

int main(void)
{
    if (!pwm_is_ready_dt(&pwm_signal)) {
        printk("Error: PWM device %s is not ready\n", pwm_signal.dev->name);
        return 0;
    }

    return 0;
}

K_THREAD_DEFINE(pwm_thread_id, PWM_THREAD_STACK_SIZE, pwm_thread, NULL, NULL, NULL, PWM_THREAD_PRIORITY, 0, 0);
