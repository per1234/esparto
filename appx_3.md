![Esparto Logo](/assets/espartotitle.png)
# APPENDIX 3
## System variables

As a general rule, you should not need any of these values, still less change any. Changing any of these variables to a "bad" value can resukt in anything from a minor inconvenience / confusion through to permanent crash / reboot cycle requiring factory reset.

Danger Level for bad values:

* 0 = Mild inconvenience, confusing / inconsistent results and/or behaviour
* 1 = Major proplems, possibly recoverable. Loss of data, loss of connection, possible crash / reboot cycle
* 2 = Don't even _*think*_ of messing with these. Will cause major malfunction and almost certain crash / reboot cycle, possibly requirinf factory reset to repair.

| UID | Name                 | Function                           | Danger Level |
|-----|----------------------|------------------------------------|--------------|
| 0   | ESPARTO_VERSION      | Current system version             | 0            |
| 1   | ESPARTO_BOOT_COUNT   | #times up since last FR            | 0            |
| 2   | ESPARTO_LOG_STATS    | 1= send stats to MQTT, 0=don't     | 0            |
| 3   | ESPARTO_IP_ADDRESS   | hazard a guess…                    | 2            |
| 4   | ESPARTO_AP_FALLBACK  | mSec before AP mode                | 1            |
| 5   | ESPARTO_DNS_PORT     | captive portal                     | 1            |
| 6   | ESPARTO_HEAP_FACTOR  | % headroom on heap throttling      | 2            |
| 7   | ESPARTO_HEAP_HOLD    | mSec before heap un-throttle       | 1            |
| 8   | ESPARTO_HEAP_PCENT   | min % of heap as Hne               | 2            |
| 9   | ESPARTO_IDLE_TIME    | when / whenever idle loop          | 1            |
| 10  | ESPARTO_SYS_LOCKED   | this lot appears in lists when =1  | 0            |
| 11  | ESPARTO_MQTT_RETRY   | mSec  retries on MQTT fail         | 1            |
| 12  | ESPARTO_PIN_HOLD     | mSec before sigmaPins un-throttle  | 2            |
| 13  | ESPARTO_Q_MAX        | task queue size                    | 2            |
| 14  | ESPARTO_SOX_HOLD     | mSec before socks un-throttle      | 2            |
| 15  | ESPARTO_SOX_LIMIT    | max nSocks per sec before heapfail | 2            |
| 16  | ESPARTO_SOX_OVRIDE   | mSec tail-off of  SOX_LIMIT        | 2            |
| 17  | ESPARTO_SOX_PEAK     | absvalue fed to limit              | 2            |
| 18  | ESPARTO_WEB_PORT     |                                    | 1            |
| 19  | ESPARTO_MQTT_USER    | current MQTT username (if any)     | 1            |
| 20  | ESPARTO_MQTT_PASS    | current MQTT password  (if any)    | 1            |
| 21  | ESPARTO_ALEXA_NAME   |                                    | 1            |
| 22  | ESPARTO_BOOT_REASON  | id of last reboot cause            | 0            |
| 23  | ESPARTO_CHIP_ID      |                                    | 0            |
| 24  | ESPARTO_DEVICE_NAME  |                                    | 2            |
| 25  | ESPARTO_FAIL_CODE    | last likely fatal reboot reason    | 0            |
| 26  | ESPARTO_PSK          |                                    | 2            |
| 27  | ESPARTO_ROOTWEB      | /ws.htm                            | 1            |
| 28  | ESPARTO_UNUSED_28    |                                    | 0            |
| 29  | ESPARTO_SSID         |                                    | 2            |
| 30  | ESPARTO_CMD_HASH     | /cmd/#                             | 1            |
| 31  | ESPARTO_TXT_HTM      | text/html                          | 1            |
| 32  | ESPARTO_CFG_FILE     | /cfg                               | 1            |
| 33  | ESPARTO_MEM_SIZE     |                                    | 0            |
| 34  | ESPARTO_MQTT_IP      |                                    | 1            |
| 35  | ESPARTO_MQTT_PORT    |                                    | 1            |
| 36  | ESPARTO_PRETTY_BOARD |                                    | 0            |
| 37  | ESPARTO_DUINO_BOARD  |                                    | 0            |



***
© 2019 Phil Bowles
* philbowles2012@gmail.com
* http://www.github.com/philbowles
* https://8266iot.blogspot.com
