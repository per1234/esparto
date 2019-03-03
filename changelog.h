#define ESPARTO_VN "3.0.3"
/*
	Authors:
						PMB	Phil Bowles		<esparto8266@gmail.com>
	TODO:
		IDE 1.8.8 and 2.5.0!
		fix pin designations in Pins Tab
		either prohibit > 1 or allow m viewers
		prohibit webUI in low heap / throttle?
	Changelog:
		03/03/2019 	3.0.3 	BUGFIX - type in Esparto.h (Thanks uherting)
		03/03/2019 	3.0.2 	BUGFIX - missing Wemos D1 R1 pin definitions added (Thanks Peter Peters!)
		03/03/2019  3.0.1	BUGFIX Hide internal configs $10=>0 (Thanks Adam Sharp!)
		26/02/2019	3.0.0	dynamic pin configuration
							combine sub-projects, MAHOOSIVE code tidy/rewrite
							major changes to task, name, ptr etc - spooler concept
							"rest"-style web to replicate cmd functions
							lots of diags, synth tasks, q buster etc
		06/09/2018	2.0.2	add cache-control to reduce server overload when serving static
		06/09/2018	2.0.1	allow username / pwd on mqtt server
							PMB	add _rawPublish, some code tidying
							increase 1M devices to 128k SPIFFS in boards.txt
							fix bug where device name not ready before v fast wifi start - caused double connect
							fix bug setupHardware causing events for which stuff i.e. above not yet ready
		27/08/2018	2.0.0	PMB total rewrite, full systems test and public release of Esparto 2.0	
*/
