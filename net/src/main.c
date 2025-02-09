/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/mbox.h>
#include <hal/nrf_ipc.h>
#include <zephyr/drivers/ipm.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/timer/nrf_rtc_timer.h>
#include <zephyr/logging/log.h>

#include <zephyr/drivers/i2c.h>

LOG_MODULE_REGISTER(net);

#if (CONFIG_SOC_SERIES_BSIM_NRFXX)
extern uint32_t shared_cell_buffer;
static uint32_t shared_cell = (uintptr_t)&shared_cell_buffer;
#else
static uint32_t shared_cell = 0x20070000;
static uint32_t other_cell = 0x20070001;
static uint32_t fromnet_cell = 0x20070002;
#endif

#define I2C0_NODE DT_NODELABEL(mysensor)

static void sync_callback(void)
{
	int32_t offset = z_nrf_rtc_timer_nrf53net_offset_get();

	__ASSERT(offset >= 0, "Synchronization should be completed");

	uint32_t timestamp = sys_clock_tick_get_32() + offset;
	uint32_t app_timestamp = *(volatile uint32_t *)shared_cell;

	LOG_INF("Local timestamp: %u, application core timestamp: %u",
			timestamp, app_timestamp);
}

static void mbox_callback(const struct device *dev, uint32_t channel,
						  void *user_data, struct mbox_msg *data)
{
	sync_callback();
}

static void other_mbox_callback(const struct device *dev, uint32_t channel,
								void *user_data, struct mbox_msg *data)
{
	LOG_INF("received data from appcore, size %d on channel %d", data->size, channel);
	LOG_INF("msg: %x", *(volatile uint32_t *)other_cell);
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

	err = mbox_register_callback(dev, 2, mbox_callback, NULL);
	if (err < 0)
	{
		return err;
	}

	return mbox_set_enabled(dev, 2, true);
}

static int other_mbox_init(void)
{
	const struct device *dev;
	int err;

	dev = COND_CODE_1(CONFIG_MBOX, (DEVICE_DT_GET(DT_NODELABEL(mbox))), (NULL));
	if (dev == NULL)
	{
		return -ENODEV;
	}

	err = mbox_register_callback(dev, 4, other_mbox_callback, NULL);
	if (err < 0)
	{
		return err;
	}

	return mbox_set_enabled(dev, 4, true);
}

int main(void)
{
	int err;
	uint32_t msg = 0xBADF00D;

	LOG_INF("Synchronization using mbox driver");
	err = mbox_init();
	if (err < 0)
	{
		LOG_ERR("Failed to initialize sync RTC listener (err:%d)", err);
	}
	err = other_mbox_init();
	if (err < 0)
	{
		LOG_ERR("Failed to initialize net deadbeef listener (err:%d)", err);
	}

	// i2c inits
	static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C0_NODE);
	if (!device_is_ready(dev_i2c.bus))
	{
		LOG_ERR("I2C bus %s is not ready!", dev_i2c.bus->name);
		return -1;
	}
	else
	{
		LOG_INF("i2c bus %s ready!", dev_i2c.bus->name);
	}

	while (1)
	{
		LOG_INF("Netcore task, i2c device ready");
		k_sleep(K_MSEC(1000));
		//bother the appcore with some arbitrary data on another another channel
		// bother the netcore with some arbtirary data on another channel
		nrf_ipc_task_t task = offsetof(NRF_IPC_Type, TASKS_SEND[6]);
		
		*(volatile uint32_t *)fromnet_cell = msg;
		nrf_ipc_task_trigger(NRF_IPC, task);
		LOG_INF("IPC send %x", msg);
		msg++;
	}

	return 0;
}
