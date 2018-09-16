#define ESPARTO_VERSION "2.0.1"
/*
	Authors:
						PMB	Phil Bowles		<esparto8266@gmail.com>
	TODO:

	Changelog:
		06/09/2018	2.0.1	allow username / pwd on mqtt server
							PMB	add _rawPublish, some code tidying
							increase 1M devices to 128k SPIFFS in boards.txt
							fix bug where device name not ready before v fast wifi start - caused double connect
							fix bug setupHardware causing events for which stuff i.e. above not yet ready
		27/08/2018	2.0.0	PMB total rewrite, full systems test and public release of Esparto 2.0	

*/