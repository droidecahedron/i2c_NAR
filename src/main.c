/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <hal/nrf_ipc.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/ipm.h>
#include <zephyr/drivers/mbox.h>

LOG_MODULE_REGISTER(main);

#if (CONFIG_SOC_SERIES_BSIM_NRFXX)
#include "nsi_cpu_if.h"

/* For simulation, we can define shared memory variables linkable from
 * other MCUs just by using NATIVE_SIMULATOR_IF
 */
NATIVE_SIMULATOR_IF_DATA uint32_t shared_cell_buffer;
static uint32_t shared_cell = (uintptr_t)&shared_cell_buffer;
#else
static uint32_t shared_cell = 0x20070000;
static uint32_t other_cell = 0x20070001;
static uint32_t fromnet_cell = 0x20070002;
#endif

static void timeout_handler(struct k_timer *timer)
{
	nrf_ipc_task_t task = offsetof(NRF_IPC_Type, TASKS_SEND[2]);
	uint32_t now = sys_clock_tick_get_32();

	*(volatile uint32_t *)shared_cell = now;
	nrf_ipc_task_trigger(NRF_IPC, task);

	LOG_INF("IPC send at %d ticks", now);

	/* Do it only for the first second. */
	if (now > sys_clock_hw_cycles_per_sec())
	{
		k_timer_stop(timer);
	}
}

K_TIMER_DEFINE(timer, timeout_handler, NULL);

static void mbox_callback(const struct device *dev, uint32_t channel,
						  void *user_data, struct mbox_msg *data)
{
	LOG_INF("received data from netcore, size %d on channel %d", data->size, channel);
	LOG_INF("msg: %x", *(volatile uint32_t *)fromnet_cell);
}

static int mbox_init(void)
{
	const struct device *dev;
	int err;

	dev = COND_CODE_1(CONFIG_MBOX, (DEVICE_DT_GET(DT_NODELABEL(mbox))), (NULL));
	if (dev == NULL)
	{
		return -ENODEV;
	}

	err = mbox_register_callback(dev, 6, mbox_callback, NULL);
	if (err < 0)
	{
		return err;
	}

	return mbox_set_enabled(dev, 6, true);
}

int main(void)
{
	LOG_INF("Synchronization using %s driver", IS_ENABLED(CONFIG_MBOX) ? "mbox" : "ipm");
	k_timer_start(&timer, K_MSEC(50), K_MSEC(50));

	uint32_t msg = 0xDEADBEEF;

	// Create a mailbox to hear back from the netcore
	int err;
	err = mbox_init();
	if (err < 0)
	{
		LOG_ERR("Failed to initialize app badfood listener (err:%d)", err);
	}

	while (1)
	{
		LOG_INF("cpuapp main task");
		k_sleep(K_MSEC(1000));
		// bother the netcore with some arbtirary data on another channel
		nrf_ipc_task_t task = offsetof(NRF_IPC_Type, TASKS_SEND[4]);
		
		*(volatile uint32_t *)other_cell = msg;
		nrf_ipc_task_trigger(NRF_IPC, task);
		LOG_INF("app IPC send %x", msg);
		msg++;
	}
	return 0;
}
