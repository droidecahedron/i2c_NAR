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

## This sample also utilizes multiple IPC channels. Channel 2 is used for the RTC sync. Channel 4 is used to send data from appcore to netcore containing `0xDEADBEEF++`. Channel 6 is used to send data from the netcore to the appcore containing `0xBADF00D++`.
### So there is a mailbox set up in both cores in this case.

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
   *** Booting nRF Connect SDK v2.9.0-7787b2649840 ***
   *** Using Zephyr OS v3.7.99-1f8f3dc29142 ***
   [00:00:00.253,631] <inf> main: Synchronization using mbox driver
   [00:00:00.253,662] <inf> main: cpuapp main task
   [00:00:00.303,710] <inf> main: IPC send at 9951 ticks
   [00:00:00.353,698] <inf> main: IPC send at 11590 ticks
   [00:00:00.403,747] <inf> main: IPC send at 13229 ticks
   [00:00:00.453,765] <inf> main: IPC send at 14868 ticks
   [00:00:00.503,784] <inf> main: IPC send at 16507 ticks
   [00:00:00.553,771] <inf> main: IPC send at 18146 ticks
   [00:00:00.603,820] <inf> main: IPC send at 19785 ticks
   [00:00:00.653,839] <inf> main: IPC send at 21424 ticks
   [00:00:00.703,857] <inf> main: IPC send at 23063 ticks
   [00:00:00.753,875] <inf> main: IPC send at 24702 ticks
   [00:00:00.803,863] <inf> main: IPC send at 26341 ticks
   [00:00:00.853,942] <inf> main: IPC send at 27980 ticks
   [00:00:00.903,900] <inf> main: IPC send at 29619 ticks
   [00:00:00.953,948] <inf> main: IPC send at 31258 ticks
   [00:00:01.003,967] <inf> main: IPC send at 32897 ticks
   [00:00:01.253,753] <inf> main: app IPC send deadbeef
   [00:00:01.253,753] <inf> main: cpuapp main task
   [00:00:01.255,523] <inf> main: received data from netcore, size 7897 on channel 6
   [00:00:01.255,523] <inf> main: msg: badf00d
   [00:00:02.253,845] <inf> main: app IPC send deadbef0
   [00:00:02.253,875] <inf> main: cpuapp main task
   [00:00:02.255,645] <inf> main: received data from netcore, size 7897 on channel 6
   [00:00:02.255,645] <inf> main: msg: badf00e
   [00:00:03.253,997] <inf> main: app IPC send deadbef1
   [00:00:03.253,997] <inf> main: cpuapp main task
   [00:00:03.255,737] <inf> main: received data from netcore, size 7897 on channel 6
   [00:00:03.255,737] <inf> main: msg: badf00f
   [00:00:04.254,058] <inf> main: app IPC send deadbef2
   [00:00:04.254,089] <inf> main: cpuapp main task
   [00:00:04.255,920] <inf> main: received data from netcore, size 7897 on channel 6
   [00:00:04.255,920] <inf> main: msg: badf010
   [00:00:05.254,211] <inf> main: app IPC send deadbef3
   [00:00:05.254,211] <inf> main: cpuapp main task
   [00:00:05.256,011] <inf> main: received data from netcore, size 7897 on channel 6
   [00:00:05.256,011] <inf> main: msg: badf011
   [00:00:06.254,272] <inf> main: app IPC send deadbef4
   [00:00:06.254,302] <inf> main: cpuapp main task
   [00:00:06.256,103] <inf> main: received data from netcore, size 7897 on channel 6
   [00:00:06.256,103] <inf> main: msg: badf012


.. code-block:: console

   *** Booting nRF Connect SDK v2.9.0-7787b2649840 ***
   *** Using Zephyr OS v3.7.99-1f8f3dc29142 ***
   [00:00:00.000,640] <inf> net: Synchronization using mbox driver
   [00:00:00.000,671] <inf> net: i2c bus i2c@41013000 ready!
   [00:00:00.000,671] <inf> net: Netcore task, i2c device ready
   [00:00:00.048,980] <inf> net: Local timestamp: 1589, application core timestamp: 16235
   [00:00:00.098,999] <inf> net: Local timestamp: 3228, application core timestamp: 17874
   [00:00:00.149,017] <inf> net: Local timestamp: 4867, application core timestamp: 19513
   [00:00:00.199,035] <inf> net: Local timestamp: 6506, application core timestamp: 21152
   [00:00:00.249,053] <inf> net: Local timestamp: 8145, application core timestamp: 22791
   [00:00:00.299,072] <inf> net: Local timestamp: 9784, application core timestamp: 24430
   [00:00:00.349,090] <inf> net: Local timestamp: 11423, application core timestamp: 26069
   [00:00:14.446,929] <inf> net: received data from appcore, size 7897 on channel 4
   [00:00:14.446,929] <inf> net: msg: deadbefc
   [00:00:14.448,852] <inf> net: IPC send badf01a
   [00:00:14.448,852] <inf> net: Netcore task, i2c device ready
   [00:00:15.447,021] <inf> net: received data from appcore, size 7897 on channel 4
   [00:00:15.447,021] <inf> net: msg: deadbefd
   [00:00:15.448,913] <inf> net: IPC send badf01b
   [00:00:15.448,944] <inf> net: Netcore task, i2c device ready
   [00:00:16.447,082] <inf> net: received data from appcore, size 7897 on channel 4
   [00:00:16.447,082] <inf> net: msg: deadbefe
   [00:00:16.449,096] <inf> net: IPC send badf01c
   [00:00:16.449,127] <inf> net: Netcore task, i2c device ready
   [00:00:17.447,204] <inf> net: received data from appcore, size 7897 on channel 4
   [00:00:17.447,235] <inf> net: msg: deadbeff
   [00:00:17.449,188] <inf> net: IPC send badf01d
   [00:00:17.449,218] <inf> net: Netcore task, i2c device ready
   [00:00:18.447,265] <inf> net: received data from appcore, size 7897 on channel 4
   [00:00:18.447,265] <inf> net: msg: deadbf00

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
