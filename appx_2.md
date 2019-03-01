![Esparto Logo](/assets/espartotitle.png)
# APPENDIX 2: Sample sketch / API function cross-reference

| Example Sketch                                                                                                        | API Functions called                                                |
|-----------------------------------------------------------------------------------------------------------------------|---------------------------------------------------------------------|
| [A_Utilities ](../master/examples/basics/A_Utilities/A_Utilities.ino)                                                 | [join](../master/api_utils.md#join)                                 |
|                                                                                                                       | [readSPIFFS](../master/api_utils.md#readSPIFFS)                     |
|                                                                                                                       | [replaceBetween](../master/api_utils.md#replaceBetween)             |
|                                                                                                                       | [split](../master/api_utils.md#split)                               |
|                                                                                                                       | [stringFromBuff](../master/api_utils.md#stringFromBuff)             |
|                                                                                                                       | [stringFromBuff](../master/api_utils.md#stringFromBuff)             |
|                                                                                                                       | [stringFromInt](../master/api_utils.md#stringFromInt)               |
|                                                                                                                       | [stringFromInt](../master/api_utils.md#stringFromInt)               |
|                                                                                                                       | [writeSPIFFS](../master/api_utils.md#writeSPIFFS)                   |
| [BareMinimum ](../master/examples/zz_fun/BareMinimum/BareMinimum.ino)                                                 | [DefaultOutput](../master/api_gpio.md#DefaultOutput)                |
|                                                                                                                       | [std3StageButton](../master/api_gpio.md#std3StageButton)            |
| [BareMinimum_SONOFF_BASIC ](../master/examples/zz_fun/BareMinimum_SONOFF_BASIC/BareMinimum_SONOFF_BASIC.ino)          | [DefaultOutput](../master/api_gpio.md#DefaultOutput)                |
|                                                                                                                       | [digitalWrite](../master/api_gpio.md#digitalWrite)                  |
|                                                                                                                       | [Output](../master/api_gpio.md#Output)                              |
|                                                                                                                       | [std3StageButton](../master/api_gpio.md#std3StageButton)            |
| [BareMinimum_wifi ](../master/examples/zz_fun/BareMinimum_wifi/BareMinimum_wifi.ino)                                  | [DefaultOutput](../master/api_gpio.md#DefaultOutput)                |
|                                                                                                                       | [std3StageButton](../master/api_gpio.md#std3StageButton)            |
| [BareMinimum_wifiMQTT ](../master/examples/zz_fun/BareMinimum_wifiMQTT/BareMinimum_wifiMQTT.ino)                      | [DefaultOutput](../master/api_gpio.md#DefaultOutput)                |
|                                                                                                                       | [std3StageButton](../master/api_gpio.md#std3StageButton)            |
| [Basic_Features ](../master/examples/core/Basic_Features/Basic_Features.ino)                                          | [onFactoryReset](../master/api_cycle.md#onFactoryReset)             |
|                                                                                                                       | [onReboot](../master/api_cycle.md#onReboot)                         |
|                                                                                                                       | [Output](../master/api_gpio.md#Output)                              |
|                                                                                                                       | [std3StageButton](../master/api_gpio.md#std3StageButton)            |
| [Blinky ](../master/examples/basics/Blinky/Blinky.ino)                                                                | [flashLED](../master/api_flash.md#flashLED)                         |
|                                                                                                                       | [Output](../master/api_gpio.md#Output)                              |
| [Blinky_Pattern ](../master/examples/basics/Blinky_Pattern/Blinky_Pattern.ino)                                        | [flashPattern](../master/api_flash.md#flashPattern)                 |
|                                                                                                                       | [Output](../master/api_gpio.md#Output)                              |
| [Blinky_PWM ](../master/examples/basics/Blinky_PWM/Blinky_PWM.ino)                                                    | [flashPWM](../master/api_flash.md#flashPWM)                         |
|                                                                                                                       | [Output](../master/api_gpio.md#Output)                              |
| [Blinky_Xmas_Tree ](../master/examples/basics/Blinky_Xmas_Tree/Blinky_Xmas_Tree.ino)                                  | [flashLED](../master/api_flash.md#flashLED)                         |
|                                                                                                                       | [flashPattern](../master/api_flash.md#flashPattern)                 |
|                                                                                                                       | [flashPWM](../master/api_flash.md#flashPWM)                         |
|                                                                                                                       | [Output](../master/api_gpio.md#Output)                              |
| [Config ](../master/examples/core/Config/Config.ino)                                                                  | [addConfig](../master/api_cycle.md#addConfig)                       |
|                                                                                                                       | [decConfigInt](../master/api_timer.md#decConfigInt)                 |
|                                                                                                                       | [getConfig](../master/api_timer.md#getConfig)                       |
|                                                                                                                       | [getConfigInt](../master/api_timer.md#getConfigInt)                 |
|                                                                                                                       | [getConfigString](../master/api_timer.md#getConfigString)           |
|                                                                                                                       | [getConfigString](../master/api_timer.md#getConfigString)           |
|                                                                                                                       | [incConfigInt](../master/api_timer.md#incConfigInt)                 |
|                                                                                                                       | [minusEqualsConfigInt](../master/api_timer.md#minusEqualsConfigInt) |
|                                                                                                                       | [onConfigItemChange](../master/api_cycle.md#onConfigItemChange)     |
|                                                                                                                       | [Output](../master/api_gpio.md#Output)                              |
|                                                                                                                       | [plusEqualsConfigInt](../master/api_timer.md#plusEqualsConfigInt)   |
|                                                                                                                       | [setConfig](../master/api_timer.md#setConfig)                       |
|                                                                                                                       | [setConfigInt](../master/api_timer.md#setConfigInt)                 |
|                                                                                                                       | [setConfigString](../master/api_timer.md#setConfigString)           |
|                                                                                                                       | [setConfigString](../master/api_timer.md#setConfigString)           |
|                                                                                                                       | [std3StageButton](../master/api_gpio.md#std3StageButton)            |
| [EncoderAuto_Variable_Blinky ](../master/examples/zz_fun/EncoderAuto_Variable_Blinky/EncoderAuto_Variable_Blinky.ino) | [EncoderAuto](../master/api_gpio.md#EncoderAuto)                    |
|                                                                                                                       | [flashLED](../master/api_flash.md#flashLED)                         |
|                                                                                                                       | [Output](../master/api_gpio.md#Output)                              |
| [MQTT_Blinky ](../master/examples/wifi_mqtt/MQTT_Blinky/MQTT_Blinky.ino)                                              | [addConfig](../master/api_cycle.md#addConfig)                       |
|                                                                                                                       | [flashLED](../master/api_flash.md#flashLED)                         |
|                                                                                                                       | [isFlashing](../master/api_flash.md#isFlashing)                     |
|                                                                                                                       | [onAlexaCommand](../master/api_cycle.md#onAlexaCommand)             |
|                                                                                                                       | [onConfigItemChange](../master/api_cycle.md#onConfigItemChange)     |
|                                                                                                                       | [onMqttConnect](../master/api_cycle.md#onMqttConnect)               |
|                                                                                                                       | [onWiFiConnect](../master/api_cycle.md#onWiFiConnect)               |
|                                                                                                                       | [onWiFiDisconnect](../master/api_cycle.md#onWiFiDisconnect)         |
|                                                                                                                       | [Output](../master/api_gpio.md#Output)                              |
|                                                                                                                       | [setAlexaDeviceName](../master/api_cycle.md#setAlexaDeviceName)     |
|                                                                                                                       | [std3StageButton](../master/api_gpio.md#std3StageButton)            |
|                                                                                                                       | [stopLED](../master/api_flash.md#stopLED)                           |
|                                                                                                                       | [subscribe](../master/api_mqtt.md#subscribe)                        |
| [MQTT_DefaultOutput ](../master/examples/wifi_mqtt/MQTT_DefaultOutput/MQTT_DefaultOutput.ino)                         | [DefaultOutput](../master/api_gpio.md#DefaultOutput)                |
|                                                                                                                       | [onMqttConnect](../master/api_cycle.md#onMqttConnect)               |
|                                                                                                                       | [onMqttDisconnect](../master/api_cycle.md#onMqttDisconnect)         |
|                                                                                                                       | [onWiFiConnect](../master/api_cycle.md#onWiFiConnect)               |
|                                                                                                                       | [setAlexaDeviceName](../master/api_cycle.md#setAlexaDeviceName)     |
|                                                                                                                       | [std3StageButton](../master/api_gpio.md#std3StageButton)            |
| [MQTT_Wildcards ](../master/examples/wifi_mqtt/MQTT_Wildcards/MQTT_Wildcards.ino)                                     | [addConfig](../master/api_cycle.md#addConfig)                       |
|                                                                                                                       | [flashLED](../master/api_flash.md#flashLED)                         |
|                                                                                                                       | [getConfigInt](../master/api_timer.md#getConfigInt)                 |
|                                                                                                                       | [isFlashing](../master/api_flash.md#isFlashing)                     |
|                                                                                                                       | [onAlexaCommand](../master/api_cycle.md#onAlexaCommand)             |
|                                                                                                                       | [onConfigItemChange](../master/api_cycle.md#onConfigItemChange)     |
|                                                                                                                       | [onMqttConnect](../master/api_cycle.md#onMqttConnect)               |
|                                                                                                                       | [Output](../master/api_gpio.md#Output)                              |
|                                                                                                                       | [publish](../master/api_mqtt.md#publish)                            |
|                                                                                                                       | [setAlexaDeviceName](../master/api_cycle.md#setAlexaDeviceName)     |
|                                                                                                                       | [std3StageButton](../master/api_gpio.md#std3StageButton)            |
|                                                                                                                       | [stopLED](../master/api_flash.md#stopLED)                           |
|                                                                                                                       | [subscribe](../master/api_mqtt.md#subscribe)                        |
| [Pins0_digital_vs_logical ](../master/examples/gpio/Pins0_digital_vs_logical/Pins0_digital_vs_logical.ino)            | [digitalWrite](../master/api_gpio.md#digitalWrite)                  |
|                                                                                                                       | [every](../master/api_timer.md#every)                               |
|                                                                                                                       | [logicalWrite](../master/api_gpio.md#logicalWrite)                  |
|                                                                                                                       | [Output](../master/api_gpio.md#Output)                              |
| [Pins1_Raw ](../master/examples/gpio/Pins1_Raw/Pins1_Raw.ino)                                                         | [Raw](../master/api_gpio.md#Raw)                                    |
| [Pins10_Encoder ](../master/examples/gpio/Pins10_Encoder/Pins10_Encoder.ino)                                          | [Encoder](../master/api_gpio.md#Encoder)                            |
|                                                                                                                       | [every](../master/api_timer.md#every)                               |
| [Pins11_EncoderBound ](../master/examples/gpio/Pins11_EncoderBound/Pins11_EncoderBound.ino)                           | [Encoder](../master/api_gpio.md#Encoder)                            |
|                                                                                                                       | [every](../master/api_timer.md#every)                               |
| [Pins12_EncoderAuto ](../master/examples/gpio/Pins12_EncoderAuto/Pins12_EncoderAuto.ino)                              | [Debounced](../master/api_gpio.md#Debounced)                        |
|                                                                                                                       | [EncoderAuto::center](../master/api_gpio.md#EncoderAuto)            |
|                                                                                                                       | [EncoderAuto::getValue](../master/api_gpio.md#EncoderAuto)          |
|                                                                                                                       | [EncoderAuto::reconfigure](../master/api_gpio.md#EncoderAuto)       |
|                                                                                                                       | [EncoderAuto::setPercent](../master/api_gpio.md#EncoderAuto)        |
|                                                                                                                       | [EncoderAuto::setValue](../master/api_gpio.md#EncoderAuto)          |
|                                                                                                                       | [EncoderAuto](../master/api_gpio.md#EncoderAuto)                    |
|                                                                                                                       | [everyRandom](../master/api_timer.md#everyRandom)                   |
|                                                                                                                       | [once](../master/api_timer.md#once)                                 |
| [Pins13_EncoderAutoBound ](../master/examples/gpio/Pins13_EncoderAutoBound/Pins13_EncoderAutoBound.ino)               | [Debounced](../master/api_gpio.md#Debounced)                        |
|                                                                                                                       | [EncoderAuto::center](../master/api_gpio.md#EncoderAuto)            |
|                                                                                                                       | [EncoderAuto::reconfigure](../master/api_gpio.md#EncoderAuto)       |
|                                                                                                                       | [EncoderAuto::setPercent](../master/api_gpio.md#EncoderAuto)        |
|                                                                                                                       | [EncoderAuto](../master/api_gpio.md#EncoderAuto)                    |
|                                                                                                                       | [everyRandom](../master/api_timer.md#everyRandom)                   |
| [Pins14_Throttling ](../master/examples/gpio/Pins14_Throttling/Pins14_Throttling.ino)                                 | [Debounced](../master/api_gpio.md#Debounced)                        |
|                                                                                                                       | [EncoderAuto](../master/api_gpio.md#EncoderAuto)                    |
|                                                                                                                       | [every](../master/api_timer.md#every)                               |
|                                                                                                                       | [Output](../master/api_gpio.md#Output)                              |
|                                                                                                                       | [Raw](../master/api_gpio.md#Raw)                                    |
|                                                                                                                       | [throttlePin](../master/api_gpio.md#throttlePin)                    |
| [Pins15_DefaultOutput ](../master/examples/gpio/Pins15_DefaultOutput/Pins15_DefaultOutput.ino)                        | [DefaultOutput](../master/api_gpio.md#DefaultOutput)                |
|                                                                                                                       | [std3StageButton](../master/api_gpio.md#std3StageButton)            |
| [Pins2_Filtered ](../master/examples/gpio/Pins2_Filtered/Pins2_Filtered.ino)                                          | [Filtered](../master/api_gpio.md#Filtered)                          |
|                                                                                                                       | [once](../master/api_timer.md#once)                                 |
|                                                                                                                       | [onPinConfigChange](../master/api_cycle.md#onPinConfigChange)       |
|                                                                                                                       | [reconfigurePin](../master/api_gpio.md#reconfigurePin)              |
| [Pins3_Polled ](../master/examples/gpio/Pins3_Polled/Pins3_Polled.ino)                                                | [onPinConfigChange](../master/api_cycle.md#onPinConfigChange)       |
|                                                                                                                       | [Polled](../master/api_gpio.md#Polled)                              |
|                                                                                                                       | [reconfigurePin](../master/api_gpio.md#reconfigurePin)              |
| [Pins4_Retriggering ](../master/examples/gpio/Pins4_Retriggering/Pins4_Retriggering.ino)                              | [flashLED](../master/api_flash.md#flashLED)                         |
|                                                                                                                       | [onceRandom](../master/api_timer.md#onceRandom)                     |
|                                                                                                                       | [onPinConfigChange](../master/api_cycle.md#onPinConfigChange)       |
|                                                                                                                       | [Output](../master/api_gpio.md#Output)                              |
|                                                                                                                       | [reconfigurePin](../master/api_gpio.md#reconfigurePin)              |
|                                                                                                                       | [Retriggering](../master/api_gpio.md#Retriggering)                  |
|                                                                                                                       | [stopLED](../master/api_flash.md#stopLED)                           |
| [Pins5_Debounced ](../master/examples/gpio/Pins5_Debounced/Pins5_Debounced.ino)                                       | [Debounced](../master/api_gpio.md#Debounced)                        |
|                                                                                                                       | [flashLED](../master/api_flash.md#flashLED)                         |
|                                                                                                                       | [onceRandom](../master/api_timer.md#onceRandom)                     |
|                                                                                                                       | [onPinConfigChange](../master/api_cycle.md#onPinConfigChange)       |
|                                                                                                                       | [Output](../master/api_gpio.md#Output)                              |
|                                                                                                                       | [reconfigurePin](../master/api_gpio.md#reconfigurePin)              |
|                                                                                                                       | [stopLED](../master/api_flash.md#stopLED)                           |
| [Pins6_Latching ](../master/examples/gpio/Pins6_Latching/Pins6_Latching.ino)                                          | [flashLED](../master/api_flash.md#flashLED)                         |
|                                                                                                                       | [Latching](../master/api_gpio.md#Latching)                          |
|                                                                                                                       | [onceRandom](../master/api_timer.md#onceRandom)                     |
|                                                                                                                       | [onPinConfigChange](../master/api_cycle.md#onPinConfigChange)       |
|                                                                                                                       | [Output](../master/api_gpio.md#Output)                              |
|                                                                                                                       | [reconfigurePin](../master/api_gpio.md#reconfigurePin)              |
|                                                                                                                       | [stopLED](../master/api_flash.md#stopLED)                           |
| [Pins7_Timed ](../master/examples/gpio/Pins7_Timed/Pins7_Timed.ino)                                                   | [onceRandom](../master/api_timer.md#onceRandom)                     |
|                                                                                                                       | [onPinConfigChange](../master/api_cycle.md#onPinConfigChange)       |
|                                                                                                                       | [reconfigurePin](../master/api_gpio.md#reconfigurePin)              |
|                                                                                                                       | [Timed](../master/api_gpio.md#Timed)                                |
| [Pins8_Reporting ](../master/examples/gpio/Pins8_Reporting/Pins8_Reporting.ino)                                       | [onceRandom](../master/api_timer.md#onceRandom)                     |
|                                                                                                                       | [onPinConfigChange](../master/api_cycle.md#onPinConfigChange)       |
|                                                                                                                       | [reconfigurePin](../master/api_gpio.md#reconfigurePin)              |
|                                                                                                                       | [Reporting](../master/api_gpio.md#Reporting)                        |
| [Pins9_ThreeStage ](../master/examples/gpio/Pins9_ThreeStage/Pins9_ThreeStage.ino)                                    | [flashLED](../master/api_flash.md#flashLED)                         |
|                                                                                                                       | [isFlashing](../master/api_flash.md#isFlashing)                     |
|                                                                                                                       | [onceRandom](../master/api_timer.md#onceRandom)                     |
|                                                                                                                       | [onPinConfigChange](../master/api_cycle.md#onPinConfigChange)       |
|                                                                                                                       | [Output](../master/api_gpio.md#Output)                              |
|                                                                                                                       | [pulseLED](../master/api_flash.md#pulseLED)                         |
|                                                                                                                       | [reconfigurePin](../master/api_gpio.md#reconfigurePin)              |
|                                                                                                                       | [stopLED](../master/api_flash.md#stopLED)                           |
|                                                                                                                       | [ThreeStage](../master/api_gpio.md#ThreeStage)                      |
| [SONOFF_BASIC_Firmware ](../master/examples/wifi_mqtt/SONOFF_BASIC_Firmware/SONOFF_BASIC_Firmware.ino)                | [DefaultOutput](../master/api_gpio.md#DefaultOutput)                |
|                                                                                                                       | [digitalWrite](../master/api_gpio.md#digitalWrite)                  |
|                                                                                                                       | [Output](../master/api_gpio.md#Output)                              |
|                                                                                                                       | [publish](../master/api_mqtt.md#publish)                            |
|                                                                                                                       | [std3StageButton](../master/api_gpio.md#std3StageButton)            |
| [Tasks_Spoolers ](../master/examples/xpert/Tasks_Spoolers/Tasks_Spoolers.ino)                                         | [addConfig](../master/api_cycle.md#addConfig)                       |
|                                                                                                                       | [every](../master/api_timer.md#every)                               |
|                                                                                                                       | [flashLED](../master/api_flash.md#flashLED)                         |
|                                                                                                                       | [getConfigInt](../master/api_timer.md#getConfigInt)                 |
|                                                                                                                       | [getTask](../master/api_gpio.md#getTask)                            |
|                                                                                                                       | [getTaskName](../master/api_gpio.md#getTaskName)                    |
|                                                                                                                       | [getTaskSource](../master/api_.md#getTaskSource)                    |
|                                                                                                                       | [isFlashing](../master/api_flash.md#isFlashing)                     |
|                                                                                                                       | [onAlexaCommand](../master/api_cycle.md#onAlexaCommand)             |
|                                                                                                                       | [once](../master/api_timer.md#once)                                 |
|                                                                                                                       | [onConfigItemChange](../master/api_cycle.md#onConfigItemChange)     |
|                                                                                                                       | [onMqttConnect](../master/api_cycle.md#onMqttConnect)               |
|                                                                                                                       | [onWiFiConnect](../master/api_cycle.md#onWiFiConnect)               |
|                                                                                                                       | [onWiFiDisconnect](../master/api_cycle.md#onWiFiDisconnect)         |
|                                                                                                                       | [Output](../master/api_gpio.md#Output)                              |
|                                                                                                                       | [runWithSpooler](../master/api_.md#runWithSpooler)                  |
|                                                                                                                       | [setAlexaDeviceName](../master/api_cycle.md#setAlexaDeviceName)     |
|                                                                                                                       | [setAllSpoolDestination](../master/api_.md#setAllSpoolDestination)  |
|                                                                                                                       | [setSrcSpoolDestination](../master/api_.md#setSrcSpoolDestination)  |
|                                                                                                                       | [std3StageButton](../master/api_gpio.md#std3StageButton)            |
|                                                                                                                       | [stopLED](../master/api_flash.md#stopLED)                           |
|                                                                                                                       | [subscribe](../master/api_mqtt.md#subscribe)                        |
| [Timers1_simple ](../master/examples/core/Timers1_simple/Timers1_simple.ino)                                          | [cancel](../master/api_timer.md#cancel)                             |
|                                                                                                                       | [every](../master/api_timer.md#every)                               |
|                                                                                                                       | [flashLED](../master/api_flash.md#flashLED)                         |
|                                                                                                                       | [onceRandom](../master/api_timer.md#onceRandom)                     |
|                                                                                                                       | [Output](../master/api_gpio.md#Output)                              |
| [Timers2_lambda ](../master/examples/core/Timers2_lambda/Timers2_lambda.ino)                                          | [cancel](../master/api_timer.md#cancel)                             |
|                                                                                                                       | [every](../master/api_timer.md#every)                               |
|                                                                                                                       | [flashLED](../master/api_flash.md#flashLED)                         |
|                                                                                                                       | [onceRandom](../master/api_timer.md#onceRandom)                     |
|                                                                                                                       | [Output](../master/api_gpio.md#Output)                              |
| [Timers3_classy ](../master/examples/core/Timers3_classy/Timers3_classy.ino)                                          | [everyRandom](../master/api_timer.md#everyRandom)                   |
|                                                                                                                       | [flashLED](../master/api_flash.md#flashLED)                         |
|                                                                                                                       | [Output](../master/api_gpio.md#Output)                              |
|                                                                                                                       | [queueFunction](../master/api_timer.md#queueFunction)               |
| [Timers4_chaining ](../master/examples/core/Timers4_chaining/Timers4_chaining.ino)                                    | [nTimes](../master/api_timer.md#nTimes)                             |
|                                                                                                                       | [once](../master/api_timer.md#once)                                 |
|                                                                                                                       | [onceRandom](../master/api_timer.md#onceRandom)                     |
|                                                                                                                       | [Output](../master/api_gpio.md#Output)                              |
|                                                                                                                       | [pulseLED](../master/api_flash.md#pulseLED)                         |
| [Timers5_advanced ](../master/examples/core/Timers5_advanced/Timers5_advanced.ino)                                    | [cancel](../master/api_timer.md#cancel)                             |
|                                                                                                                       | [cancelAll](../master/api_timer.md#cancelAll)                       |
|                                                                                                                       | [every](../master/api_timer.md#every)                               |
|                                                                                                                       | [everyRandom](../master/api_timer.md#everyRandom)                   |
|                                                                                                                       | [nTimes](../master/api_timer.md#nTimes)                             |
|                                                                                                                       | [once](../master/api_timer.md#once)                                 |
|                                                                                                                       | [onceRandom](../master/api_timer.md#onceRandom)                     |
|                                                                                                                       | [queueFunction](../master/api_timer.md#queueFunction)               |
| [Timers6_whenever ](../master/examples/core/Timers6_whenever/Timers6_whenever.ino)                                    | [cancel](../master/api_timer.md#cancel)                             |
|                                                                                                                       | [cancelAll](../master/api_timer.md#cancelAll)                       |
|                                                                                                                       | [every](../master/api_timer.md#every)                               |
|                                                                                                                       | [everyRandom](../master/api_timer.md#everyRandom)                   |
|                                                                                                                       | [nTimes](../master/api_timer.md#nTimes)                             |
|                                                                                                                       | [once](../master/api_timer.md#once)                                 |
|                                                                                                                       | [onceRandom](../master/api_timer.md#onceRandom)                     |
|                                                                                                                       | [Output](../master/api_gpio.md#Output)                              |
|                                                                                                                       | [queueFunction](../master/api_timer.md#queueFunction)               |
|                                                                                                                       | [when](../master/api_timer.md#when)                                 |
|                                                                                                                       | [whenever](../master/api_timer.md#whenever)                         |
| [Timers7_mayhem ](../master/examples/core/Timers7_mayhem/Timers7_mayhem.ino)                                          | [nTimes](../master/api_timer.md#nTimes)                             |
|                                                                                                                       | [nTimesRandom](../master/api_timer.md#nTimesRandom)                 |
|                                                                                                                       | [once](../master/api_timer.md#once)                                 |
|                                                                                                                       | [Output](../master/api_gpio.md#Output)                              |
|                                                                                                                       | [pulseLED](../master/api_flash.md#pulseLED)                         |
|                                                                                                                       | [randomTimes](../master/api_timer.md#randomTimes)                   |
|                                                                                                                       | [randomTimesRandom](../master/api_timer.md#randomTimesRandom)       |
| [VeryUselessMeter_1 ](../master/examples/zz_fun/VeryUselessMeter_1/VeryUselessMeter_1.ino)                            | [Raw](../master/api_gpio.md#Raw)                                    |
| [VeryUselessMeter_2 ](../master/examples/zz_fun/VeryUselessMeter_2/VeryUselessMeter_2.ino)                            | [Output](../master/api_gpio.md#Output)                              |
| [VeryUselessMeter_2_Variable ](../master/examples/zz_fun/VeryUselessMeter_2_Variable/VeryUselessMeter_2_Variable.ino) | [EncoderAuto](../master/api_gpio.md#EncoderAuto)                    |
|                                                                                                                       | [Output](../master/api_gpio.md#Output)                              |
|                                                                                                                       | [Raw](../master/api_gpio.md#Raw)                                    |
| [VeryUselessMeter_3 ](../master/examples/zz_fun/VeryUselessMeter_3/VeryUselessMeter_3.ino)                            | [pulseLED](../master/api_flash.md#pulseLED)                         |
| [WiFi_Blinky ](../master/examples/wifi/WiFi_Blinky/WiFi_Blinky.ino)                                                   | [addCmd](../master/api_.md#addCmd)                                  |
|                                                                                                                       | [addConfig](../master/api_cycle.md#addConfig)                       |
|                                                                                                                       | [flashLED](../master/api_flash.md#flashLED)                         |
|                                                                                                                       | [getConfigInt](../master/api_timer.md#getConfigInt)                 |
|                                                                                                                       | [invokeCmd](../master/api_.md#invokeCmd)                            |
|                                                                                                                       | [isFlashing](../master/api_flash.md#isFlashing)                     |
|                                                                                                                       | [onAlexaCommand](../master/api_cycle.md#onAlexaCommand)             |
|                                                                                                                       | [onConfigItemChange](../master/api_cycle.md#onConfigItemChange)     |
|                                                                                                                       | [onWiFiConnect](../master/api_cycle.md#onWiFiConnect)               |
|                                                                                                                       | [onWiFiDisconnect](../master/api_cycle.md#onWiFiDisconnect)         |
|                                                                                                                       | [Output](../master/api_gpio.md#Output)                              |
|                                                                                                                       | [setAlexaDeviceName](../master/api_cycle.md#setAlexaDeviceName)     |
|                                                                                                                       | [std3StageButton](../master/api_gpio.md#std3StageButton)            |
|                                                                                                                       | [stopLED](../master/api_flash.md#stopLED)                           |
| [WiFi_DefaultOutput ](../master/examples/wifi/WiFi_DefaultOutput/WiFi_DefaultOutput.ino)                              | [DefaultOutput](../master/api_gpio.md#DefaultOutput)                |
|                                                                                                                       | [onWiFiConnect](../master/api_cycle.md#onWiFiConnect)               |
|                                                                                                                       | [setAlexaDeviceName](../master/api_cycle.md#setAlexaDeviceName)     |
|                                                                                                                       | [std3StageButton](../master/api_gpio.md#std3StageButton)            |
| [WiFi_Warning ](../master/examples/wifi/WiFi_Warning/WiFi_Warning.ino)                                                | [flashLED](../master/api_flash.md#flashLED)                         |
|                                                                                                                       | [flashPattern](../master/api_flash.md#flashPattern)                 |
|                                                                                                                       | [onWiFiConnect](../master/api_cycle.md#onWiFiConnect)               |
|                                                                                                                       | [onWiFiDisconnect](../master/api_cycle.md#onWiFiDisconnect)         |
|                                                                                                                       | [Output](../master/api_gpio.md#Output)                              |
|                                                                                                                       | [wifiConnected](../master/api_.md#wifiConnected)                    |   
***

© 2019 Phil Bowles
* esparto8266@gmail.com
* [Youtube channel (instructional videos)](https://www.youtube.com/channel/UCYi-Ko76_3p9hBUtleZRY6g)
* [Blog](https://8266iot.blogspot.com)
* [Facebook Support / Discussion Group](https://www.facebook.com/groups/esparto8266/)
* [Support me on Patreon](https://patreon.com/esparto)
