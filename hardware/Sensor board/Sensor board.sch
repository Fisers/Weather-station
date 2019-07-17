EESchema Schematic File Version 4
LIBS:Sensor board-cache
EELAYER 29 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Meteo~Station:HIH8130 U1
U 1 1 5D2E2E9A
P 3400 3550
F 0 "U1" V 2833 3550 50  0000 C CNN
F 1 "HIH8130" V 2924 3550 50  0000 C CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 3400 3100 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/uc3842.pdf" H 3700 4250 50  0001 C CNN
	1    3400 3550
	0    1    1    0   
$EndComp
$Comp
L Connector:Conn_01x07_Female J1
U 1 1 5D2E4808
P 1800 3550
F 0 "J1" H 1692 4035 50  0000 C CNN
F 1 "Conn_01x07_Female" H 1692 3944 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x07_P2.54mm_Vertical" H 1800 3550 50  0001 C CNN
F 3 "~" H 1800 3550 50  0001 C CNN
	1    1800 3550
	-1   0    0    -1  
$EndComp
$Comp
L Device:C_Small C2
U 1 1 5D2EBE64
P 3000 3350
F 0 "C2" H 2908 3304 50  0000 R CNN
F 1 "C_Small" H 2908 3395 50  0000 R CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 3000 3350 50  0001 C CNN
F 3 "~" H 3000 3350 50  0001 C CNN
	1    3000 3350
	1    0    0    1   
$EndComp
Text GLabel 2000 3350 2    50   Input ~ 0
VCC
Text GLabel 3800 3250 2    50   Input ~ 0
VCC
Connection ~ 3000 3450
Wire Wire Line
	2500 3650 2500 3250
Wire Wire Line
	2500 3250 2000 3250
$Comp
L Device:C_Small C1
U 1 1 5D302AFD
P 2750 3350
F 0 "C1" H 2658 3304 50  0000 R CNN
F 1 "C_Small" H 2658 3395 50  0000 R CNN
F 2 "Capacitor_SMD:C_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 2750 3350 50  0001 C CNN
F 3 "~" H 2750 3350 50  0001 C CNN
	1    2750 3350
	1    0    0    1   
$EndComp
Wire Wire Line
	2750 3450 3000 3450
Text GLabel 2750 3250 1    50   Input ~ 0
VCC
Wire Wire Line
	2000 3550 2400 3550
Wire Wire Line
	2400 3550 2400 3850
Wire Wire Line
	2400 3850 3000 3850
Wire Wire Line
	3800 3850 4200 3850
Wire Wire Line
	4200 3850 4200 3650
Wire Wire Line
	4200 3650 4550 3650
Wire Wire Line
	2000 4550 4550 4550
Wire Wire Line
	2000 3750 2200 3750
Wire Wire Line
	2200 4400 4400 4400
Wire Wire Line
	4400 3750 4550 3750
Text Label 3300 4400 0    50   ~ 0
MOSI
Text Label 3200 4550 0    50   ~ 0
BMP280_CS
Wire Wire Line
	2000 3650 2300 3650
Wire Wire Line
	2300 4250 4200 4250
Connection ~ 4200 3850
Text Label 3300 4250 0    50   ~ 0
MISO
Text GLabel 4550 3350 0    50   Input ~ 0
VCC
Text GLabel 4550 3450 0    50   Input ~ 0
GND
Text GLabel 2000 3450 2    50   Input ~ 0
GND
Text GLabel 2750 3450 0    50   Input ~ 0
GND
Wire Wire Line
	4200 3850 4200 4250
Wire Wire Line
	4400 3750 4400 4400
Wire Wire Line
	4550 3850 4550 4550
Wire Wire Line
	2000 3850 2000 4550
Wire Wire Line
	2200 3750 2200 4400
Wire Wire Line
	2300 3650 2300 4250
Wire Wire Line
	2400 3850 2400 4100
Wire Wire Line
	2400 4100 4000 4100
Wire Wire Line
	4000 4100 4000 3550
Wire Wire Line
	4000 3550 4550 3550
Connection ~ 2400 3850
Text Label 3300 4100 0    50   ~ 0
CLK
$Comp
L Connector_Generic:Conn_01x07 J2
U 1 1 5D30A9DC
P 4750 3550
F 0 "J2" H 4830 3592 50  0000 L CNN
F 1 "bmp280" H 4830 3501 50  0000 L CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x07_P2.54mm_Vertical" H 4750 3550 50  0001 C CNN
F 3 "~" H 4750 3550 50  0001 C CNN
	1    4750 3550
	1    0    0    -1  
$EndComp
Wire Wire Line
	2500 3650 3000 3650
NoConn ~ 3800 3450
NoConn ~ 3800 3650
NoConn ~ 4550 3250
$Comp
L Device:R_Small R1
U 1 1 5D30D206
P 2500 3150
F 0 "R1" H 2559 3196 50  0000 L CNN
F 1 "R_Small" H 2559 3105 50  0000 L CNN
F 2 "Resistor_SMD:R_0805_2012Metric_Pad1.15x1.40mm_HandSolder" H 2500 3150 50  0001 C CNN
F 3 "~" H 2500 3150 50  0001 C CNN
	1    2500 3150
	1    0    0    -1  
$EndComp
Connection ~ 2500 3250
Text GLabel 2500 3050 1    50   Input ~ 0
VCC
$EndSCHEMATC
