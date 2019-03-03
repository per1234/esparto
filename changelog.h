#define ESPARTO_VN "3.0.1"
/*
	Authors:
						PMB	Phil Bowles		<esparto8266@gmail.com>
	TODO:
		fix pin designations in Pins Tab
		either prohibit or allow m viewers
	Changelog:
		03/03/2019  3.0.1	BUGFIX Hide internal configs $10=>0
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
