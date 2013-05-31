kilncontroler
=============

Arduino Based Kiln Controller
==============================================
In accordance with the contagion theory of Open Source licensing, this code is released under the BSD license.

Load this with info on how the buttons will work and how the software calculates Firing Profiles.

More info at https://256.makerslocal.org/wiki/Ceramic_Kiln/Controller


This is the current pin diagram from the wiki. Trust the wiki, have faith in the wiki.
Pin   Group 	Use
A5  MAX31855 Thermister 	CLK
A4 	MAX31855 Thermister 	CS
A3 	MAX31855 Thermister 	d0
A2 	
A1 	
A0 	6 Buttons	pin
D13 	SD Card Shield 	SCK
D12 	SD Card Shield 	MISO
D11 	SD Card Shield 	MOSI
D10 	SD Card Shield 	CS
D09 	LCD 	RS
D08 	LCD 	Enable
D07 	LCD 	d0
D06 	LCD 	d1
D05 	LCD 	d2
D04 	LCD 	d3
D03 	MAX31855 Thermister 	CLK
D02 	MAX31855 Thermister 	CS
D01 	MAX31855 Thermister 	d0
D00 	Kiln Relay 	pin
