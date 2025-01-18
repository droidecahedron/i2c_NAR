.. zephyr:code-sample:: nrf_sync_rtc
   :name: Synchronized RTC

   Synchronize system and network core RTC clocks.

Overview
********

Sample is showing how RTC clocks used for system clock on application and network
core are synchronized. The result of synchronization is an offset value on network
core which can be applied to the system tick for logging timestamping.

Sample is using IPM driver and IPC HAL to produce events which occur at same time on
both cores. Application core periodically reads current system tick and stores it in
the shared memory and triggers IPC task which results in the interrupt on the network
core. In the context of the IPC interrupt handler, network core is logging timestamp
from shared memory and local system tick updated by the offset. User can observe
that before synchronization procedure is completed, timestamps differ significantly
and once procedure is completed timestamps are synchronized. Network core timestamp
may be slightly behind (usually 1 tick) due to latency introduced by the
interrupt handling.

For simplicity and low latency sample is not using more sophisticated IPM protocols.

# This also enables I2C0 on the net core. This means you can't use UART0 on the net core, so you must use RTT Logging to view the netcore logs, but you can continue to use a UART for the app core logs.


Building the application for nrf5340dk/nrf5340/cpuapp
*****************************************************

.. zephyr-app-commands::
   :zephyr-app: samples/boards/nordic/nrf53_sync_rtc
   :board: nrf5340dk/nrf5340/cpuapp
   :goals: flash
   :flash-args: --hex-file build/nrf53_sync_rtc/zephyr/zephyr.hex
   :west-args: --sysbuild

Open a serial terminals (for example Minicom or PuTTY) and connect the board with the
following settings:

- Speed: 115200
- Data: 8 bits
- Parity: None
- Stop bits: 1

When you reset the development kit, the following messages (one for master and one for remote) will appear on the corresponding serial ports:

.. code-block:: console

   *** Booting Zephyr OS build v2.7.0-rc1-89-ge755863d66c9  ***
   [00:00:00.306,915] <inf> main: IPC send at 10056 ticks
   [00:00:00.356,903] <inf> main: IPC send at 11695 ticks
   [00:00:00.406,921] <inf> main: IPC send at 13334 ticks
   [00:00:00.456,939] <inf> main: IPC send at 14973 ticks
   [00:00:00.506,958] <inf> main: IPC send at 16612 ticks
   [00:00:00.556,976] <inf> main: IPC send at 18251 ticks
   [00:00:00.606,994] <inf> main: IPC send at 19890 ticks
   [00:00:00.657,012] <inf> main: IPC send at 21529 ticks
   [00:00:00.707,031] <inf> main: IPC send at 23168 ticks
   [00:00:00.757,049] <inf> main: IPC send at 24807 ticks
   [00:00:01.253,723] <inf> main: cpuapp main task
   [00:00:02.253,784] <inf> main: cpuapp main task
   [00:00:03.253,936] <inf> main: cpuapp main task


.. code-block:: console

   *** Booting nRF Connect SDK v2.9.0-7787b2649840 ***
   *** Using Zephyr OS v3.7.99-1f8f3dc29142 ***
   [00:00:00.000,640] <inf> net: Synchronization using mbox driver
   [00:00:00.000,671] <inf> net: i2c bus i2c@41013000 ready!
   [00:00:00.000,671] <inf> net: Netcore task, i2c device ready
   [00:00:00.048,980] <inf> net: Local timestamp: 1589, application core timestamp: 9951
   [00:00:00.098,999] <inf> net: Local timestamp: 3228, application core timestamp: 11590
   [00:00:00.149,017] <inf> net: Local timestamp: 4867, application core timestamp: 13229
   [00:00:00.199,035] <inf> net: Local timestamp: 6506, application core timestamp: 14868
   [00:00:00.249,053] <inf> net: Local timestamp: 8145, application core timestamp: 16507
   [00:00:00.519,744] <inf> sync_rtc: Updated timestamp to synchronized RTC by 8347 ticks (254730us)
   [00:00:00.553,802] <inf> net: Local timestamp: 18147, application core timestamp: 18146
   [00:00:08.256,469] <inf> net: Netcore task, i2c device ready
   [00:00:09.256,561] <inf> net: Netcore task, i2c device ready
   [00:00:10.256,652] <inf> net: Netcore task, i2c device ready

Observe that initially logging timestamps for the corresponding events on both cores
do not match. Same with local and remote timestamps reported on network core. After
RTC synchronization is completed they start to match.

.. _nrf53_sync_rtc_sample_build_bsim:

Building the application for the simulated nrf5340bsim
******************************************************

.. zephyr-app-commands::
   :zephyr-app: samples/boards/nordic/nrf53_sync_rtc
   :host-os: unix
   :board: nrf5340bsim/nrf5340/cpuapp
   :goals: build
   :west-args: --sysbuild

Then you can execute your application using:

.. code-block:: console

   $ ./build/zephyr/zephyr.exe -nosim
   # Press Ctrl+C to exit

You can expect a similar output as in the real HW in the invoking console.
