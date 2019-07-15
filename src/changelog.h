#define ESPARTO_VN "3.3.0"
/*
	Authors:
						PMB	Phil Bowles		<esparto8266@gmail.com>
	TODO:
		MAKE MQTT COMPILE-TIME (MAINLY SECURITY TO PREVENT USE OF FNS THAT ONLY WORK IF rtc REVCD)
		safeThing stop h/w + restart durign spiff uploads

	Changelog:
					3.3.0
							concept of the "thing" default buttons
							ground-up rewrite of scheduler + locking - mutex to avoid wdt resets
							ground-up rewrite of flashe routines ti cure meme leak +
							pattern is now 1010101 arbitrary,
							morse takes ... --- with proper Farnsworth Timing
							morseTEXT (compile-time option) takes ("abc etc") => ... --- + Farnsworth Timing => 1010101000111 etc
							realtime capability.
							new functions upTime(), clockTime
							new sched: at("hh:mm:ss"... ) => "once" with "hh:mm:ss:" rolls to +24hrs if T < now
							new sched: daily("hh:mm:ss") => "every" with "hh:mm:ss:". Will repeate at fixed time of day
							spool / plumbing / removed
							dyn pins / compile time if required...experimental wll be fixed soon						
							
					3.2.0	Alexa handling chnaged. No longer listens 100%. "a" button on WiFI screen to listen
							Alexa name change + device/SSDI decoupled.. new cmd/rename echo/listen echo/rename
							many arcane vars returned to #define hardcoding
							new MQTT tab + mqtt rename + pubstats / pubvars
							tabs now dynamic
							tabs optional (e.g. no dynp for SONOFFs) saves heap
							some functionality e.g. adc moved to config.h #define ti include (saves heap)
							graphs dynamic. new user graph(...) function
							web OTA + prgress bar
							spooler structure full rewrite. Logs fixed. new icons (tree + plumbing)
							new functions:
								finish(ESPARTO_TASK_PTR t);
								repeatWhile (when / whenever removed)
								aqf removed + chain handling unified
								addAllOutputDestination(uint32_t plan);
								addOutputDestination(uint32_t plan,ESPARTO_INPUT src=ESPARTO_INPUT_USER);
								removeAllOutputDestination(uint32_t plan);
								removeOutputDestination(uint32_t plan,ESPARTO_INPUT src=ESPARTO_INPUT_USER);	
								
							all functions now take optional spooler pointer
							
							require ./tools/boards.txt.py --nofloat --boardsgen to reduce heap
						
	                3.1.0	std3stage deprecated use DefaultInput
							Polled determines adc automatically from pin no, adc param now ignored
							med / long parameterised at $nn and $mm
							video sketches added to examples
							ThreeStage now prduces phases > 2 for multiples of long press 
							web auth!
							
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
