EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Multi Dedecoder Coder"
Date "2020-05-20"
Rev "v1.0"
Comp "pierrotm777@gmail.com"
Comment1 ""
Comment2 "- PPM SBUS IBUS output "
Comment3 "- 16 PWM outputs"
Comment4 "- PPM SBUS SUMD SRXL IBUS JETI Input"
$EndDescr
Text Label 6000 2900 1    60   ~ 0
RAW
Text Label 5300 4600 3    60   ~ 0
A4
Text Label 5200 4600 3    60   ~ 0
A5
Text Label 5600 4600 3    60   ~ 0
A7
Text Label 5700 4600 3    60   ~ 0
A6
$Comp
L Connector_Generic:Conn_01x02 P3
U 1 1 56D74FB3
P 5200 4400
F 0 "P3" H 5200 4550 50  0000 C CNN
F 1 "ADC" V 5300 4400 50  0000 C CNN
F 2 "Socket_Arduino_Pro_Mini:Socket_Strip_Arduino_1x02" H 5200 4400 50  0001 C CNN
F 3 "" H 5200 4400 50  0000 C CNN
	1    5200 4400
	0    -1   -1   0   
$EndComp
$Comp
L Connector_Generic:Conn_01x02 P5
U 1 1 56D7505C
P 5600 4400
F 0 "P5" H 5600 4550 50  0000 C CNN
F 1 "ADC" V 5700 4400 50  0000 C CNN
F 2 "Socket_Arduino_Pro_Mini:Socket_Strip_Arduino_1x02" H 5600 4400 50  0001 C CNN
F 3 "" H 5600 4400 50  0000 C CNN
	1    5600 4400
	0    -1   -1   0   
$EndComp
$Comp
L Connector_Generic:Conn_01x12 P1
U 1 1 56D754D1
P 5200 3500
F 0 "P1" H 5200 4150 50  0000 C CNN
F 1 "Digital" V 5300 3500 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x12_P2.54mm_Vertical" H 5200 3500 50  0001 C CNN
F 3 "" H 5200 3500 50  0000 C CNN
	1    5200 3500
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x12 P4
U 1 1 56D755F3
P 5700 3500
F 0 "P4" H 5700 4150 50  0000 C CNN
F 1 "Analog" V 5800 3500 50  0000 C CNN
F 2 "Connector_PinSocket_2.54mm:PinSocket_1x12_P2.54mm_Vertical" H 5700 3500 50  0001 C CNN
F 3 "" H 5700 3500 50  0000 C CNN
	1    5700 3500
	-1   0    0    -1  
$EndComp
$Comp
L power:GND #PWR03
U 1 1 56D756B8
P 4900 3300
F 0 "#PWR03" H 4900 3050 50  0001 C CNN
F 1 "GND" H 4900 3150 50  0000 C CNN
F 2 "" H 4900 3300 50  0000 C CNN
F 3 "" H 4900 3300 50  0000 C CNN
	1    4900 3300
	0    1    1    0   
$EndComp
Wire Wire Line
	5000 3300 4900 3300
Wire Wire Line
	4300 3400 5000 3400
Wire Wire Line
	5000 3500 4300 3500
Wire Wire Line
	5000 3600 4300 3600
Wire Wire Line
	4300 3700 5000 3700
Wire Wire Line
	5000 3800 4300 3800
Wire Wire Line
	4300 3900 5000 3900
Wire Wire Line
	5000 4000 4300 4000
Wire Wire Line
	5000 4100 4300 4100
Wire Wire Line
	6000 2900 6000 3000
Wire Wire Line
	6000 3000 5900 3000
$Comp
L power:GND #PWR04
U 1 1 56D75A03
P 6050 3200
F 0 "#PWR04" H 6050 2950 50  0001 C CNN
F 1 "GND" H 6050 3050 50  0000 C CNN
F 2 "" H 6050 3200 50  0000 C CNN
F 3 "" H 6050 3200 50  0000 C CNN
	1    6050 3200
	0    -1   -1   0   
$EndComp
Wire Wire Line
	5900 3300 6100 3300
Text Notes 5300 3000 0    60   ~ 0
1
$Comp
L Connector_Generic:Conn_01x08 J1
U 1 1 5EC5F20D
P 4100 3700
F 0 "J1" H 4100 4200 50  0000 C CNN
F 1 "Ch1_8" H 4100 4100 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x08_P2.54mm_Vertical" H 4100 3700 50  0001 C CNN
F 3 "~" H 4100 3700 50  0001 C CNN
	1    4100 3700
	-1   0    0    -1  
$EndComp
Text Label 4400 3400 0    50   ~ 0
D2
Text Label 4400 3500 0    50   ~ 0
D3
Text Label 4400 3600 0    50   ~ 0
D4
Text Label 4400 3700 0    50   ~ 0
D5
Text Label 4400 3800 0    50   ~ 0
D6
Text Label 4400 3900 0    50   ~ 0
D7
Text Label 4400 4000 0    50   ~ 0
D8
Text Label 4400 4100 0    50   ~ 0
D9
$Comp
L Connector_Generic:Conn_01x08 J4
U 1 1 5EC649B4
P 6700 3700
F 0 "J4" H 6650 4200 50  0000 L CNN
F 1 "Ch9_16" H 6550 4100 50  0000 L CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x08_P2.54mm_Vertical" H 6700 3700 50  0001 C CNN
F 3 "~" H 6700 3700 50  0001 C CNN
	1    6700 3700
	1    0    0    -1  
$EndComp
Wire Wire Line
	5900 4100 6500 4100
Wire Wire Line
	5900 4000 6500 4000
Wire Wire Line
	5900 3900 6500 3900
Wire Wire Line
	5900 3800 6500 3800
Wire Wire Line
	5900 3700 6500 3700
Wire Wire Line
	5900 3600 6500 3600
Wire Wire Line
	5900 3500 6500 3500
Wire Wire Line
	5900 3400 6500 3400
Text Label 6250 4100 0    50   ~ 0
D10
Text Label 6250 4000 0    50   ~ 0
D11
Text Label 6250 3900 0    50   ~ 0
D12
Text Label 6250 3800 0    50   ~ 0
D13
Text Label 6250 3700 0    50   ~ 0
A0
Text Label 6250 3600 0    50   ~ 0
A1
Text Label 6250 3500 0    50   ~ 0
A2
Text Label 6250 3400 0    50   ~ 0
A3
Wire Wire Line
	5900 3200 6050 3200
$Comp
L Connector_Generic:Conn_01x08 J2
U 1 1 5EC6F553
P 3700 3700
F 0 "J2" H 3700 4200 50  0000 C CNN
F 1 "Ch1_8V+" H 3700 4100 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x08_P2.54mm_Vertical" H 3700 3700 50  0001 C CNN
F 3 "~" H 3700 3700 50  0001 C CNN
	1    3700 3700
	-1   0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x08 J3
U 1 1 5EC7016E
P 3300 3700
F 0 "J3" H 3300 4200 50  0000 C CNN
F 1 "Ch1_8V-" H 3300 4100 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x08_P2.54mm_Vertical" H 3300 3700 50  0001 C CNN
F 3 "~" H 3300 3700 50  0001 C CNN
	1    3300 3700
	-1   0    0    -1  
$EndComp
Wire Wire Line
	3900 4100 3950 4100
Wire Wire Line
	3950 4100 3950 4000
Wire Wire Line
	3900 3400 3950 3400
Connection ~ 3950 3400
Wire Wire Line
	3950 3400 3950 3200
Wire Wire Line
	3900 3500 3950 3500
Connection ~ 3950 3500
Wire Wire Line
	3950 3500 3950 3400
Wire Wire Line
	3900 3600 3950 3600
Connection ~ 3950 3600
Wire Wire Line
	3950 3600 3950 3500
Wire Wire Line
	3900 3700 3950 3700
Connection ~ 3950 3700
Wire Wire Line
	3950 3700 3950 3600
Wire Wire Line
	3900 3800 3950 3800
Connection ~ 3950 3800
Wire Wire Line
	3950 3800 3950 3700
Wire Wire Line
	3900 3900 3950 3900
Connection ~ 3950 3900
Wire Wire Line
	3950 3900 3950 3800
Wire Wire Line
	3900 4000 3950 4000
Connection ~ 3950 4000
Wire Wire Line
	3950 4000 3950 3900
$Comp
L power:+5V #PWR0101
U 1 1 5EC75270
P 3950 3200
F 0 "#PWR0101" H 3950 3050 50  0001 C CNN
F 1 "+5V" H 3965 3373 50  0000 C CNN
F 2 "" H 3950 3200 50  0001 C CNN
F 3 "" H 3950 3200 50  0001 C CNN
	1    3950 3200
	1    0    0    -1  
$EndComp
Wire Wire Line
	3500 3400 3550 3400
Wire Wire Line
	3550 3400 3550 3500
$Comp
L power:GND #PWR0102
U 1 1 5EC762E9
P 3550 4250
F 0 "#PWR0102" H 3550 4000 50  0001 C CNN
F 1 "GND" H 3550 4100 50  0000 C CNN
F 2 "" H 3550 4250 50  0000 C CNN
F 3 "" H 3550 4250 50  0000 C CNN
	1    3550 4250
	1    0    0    -1  
$EndComp
Wire Wire Line
	3500 4100 3550 4100
Connection ~ 3550 4100
Wire Wire Line
	3550 4100 3550 4250
Wire Wire Line
	3500 4000 3550 4000
Connection ~ 3550 4000
Wire Wire Line
	3550 4000 3550 4100
Wire Wire Line
	3500 3900 3550 3900
Connection ~ 3550 3900
Wire Wire Line
	3550 3900 3550 4000
Wire Wire Line
	3500 3800 3550 3800
Connection ~ 3550 3800
Wire Wire Line
	3550 3800 3550 3900
Wire Wire Line
	3500 3700 3550 3700
Connection ~ 3550 3700
Wire Wire Line
	3550 3700 3550 3800
Wire Wire Line
	3500 3600 3550 3600
Connection ~ 3550 3600
Wire Wire Line
	3550 3600 3550 3700
Wire Wire Line
	3500 3500 3550 3500
Connection ~ 3550 3500
Wire Wire Line
	3550 3500 3550 3600
$Comp
L Connector_Generic:Conn_01x08 J5
U 1 1 5EC7CFB8
P 7150 3700
F 0 "J5" H 7150 4200 50  0000 C CNN
F 1 "Ch9_16V+" H 7150 4100 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x08_P2.54mm_Vertical" H 7150 3700 50  0001 C CNN
F 3 "~" H 7150 3700 50  0001 C CNN
	1    7150 3700
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x08 J6
U 1 1 5EC7CFC2
P 7550 3700
F 0 "J6" H 7550 4200 50  0000 C CNN
F 1 "Ch9_16V-" H 7550 4100 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x08_P2.54mm_Vertical" H 7550 3700 50  0001 C CNN
F 3 "~" H 7550 3700 50  0001 C CNN
	1    7550 3700
	1    0    0    -1  
$EndComp
Wire Wire Line
	6950 4100 6900 4100
Wire Wire Line
	6900 4100 6900 4000
Wire Wire Line
	6950 3400 6900 3400
Connection ~ 6900 3400
Wire Wire Line
	6900 3400 6900 3200
Wire Wire Line
	6950 3500 6900 3500
Connection ~ 6900 3500
Wire Wire Line
	6900 3500 6900 3400
Wire Wire Line
	6950 3600 6900 3600
Connection ~ 6900 3600
Wire Wire Line
	6900 3600 6900 3500
Wire Wire Line
	6950 3700 6900 3700
Connection ~ 6900 3700
Wire Wire Line
	6900 3700 6900 3600
Wire Wire Line
	6950 3800 6900 3800
Connection ~ 6900 3800
Wire Wire Line
	6900 3800 6900 3700
Wire Wire Line
	6950 3900 6900 3900
Connection ~ 6900 3900
Wire Wire Line
	6900 3900 6900 3800
Wire Wire Line
	6950 4000 6900 4000
Connection ~ 6900 4000
Wire Wire Line
	6900 4000 6900 3900
$Comp
L power:+5V #PWR0103
U 1 1 5EC7CFE3
P 6900 3200
F 0 "#PWR0103" H 6900 3050 50  0001 C CNN
F 1 "+5V" H 6915 3373 50  0000 C CNN
F 2 "" H 6900 3200 50  0001 C CNN
F 3 "" H 6900 3200 50  0001 C CNN
	1    6900 3200
	-1   0    0    -1  
$EndComp
Wire Wire Line
	7350 3400 7300 3400
Wire Wire Line
	7300 3400 7300 3500
$Comp
L power:GND #PWR0104
U 1 1 5EC7CFEF
P 7300 4250
F 0 "#PWR0104" H 7300 4000 50  0001 C CNN
F 1 "GND" H 7300 4100 50  0000 C CNN
F 2 "" H 7300 4250 50  0000 C CNN
F 3 "" H 7300 4250 50  0000 C CNN
	1    7300 4250
	-1   0    0    -1  
$EndComp
Wire Wire Line
	7350 4100 7300 4100
Connection ~ 7300 4100
Wire Wire Line
	7300 4100 7300 4250
Wire Wire Line
	7350 4000 7300 4000
Connection ~ 7300 4000
Wire Wire Line
	7300 4000 7300 4100
Wire Wire Line
	7350 3900 7300 3900
Connection ~ 7300 3900
Wire Wire Line
	7300 3900 7300 4000
Wire Wire Line
	7350 3800 7300 3800
Connection ~ 7300 3800
Wire Wire Line
	7300 3800 7300 3900
Wire Wire Line
	7350 3700 7300 3700
Connection ~ 7300 3700
Wire Wire Line
	7300 3700 7300 3800
Wire Wire Line
	7350 3600 7300 3600
Connection ~ 7300 3600
Wire Wire Line
	7300 3600 7300 3700
Wire Wire Line
	7350 3500 7300 3500
Connection ~ 7300 3500
Wire Wire Line
	7300 3500 7300 3600
$Comp
L Connector_Generic:Conn_01x03 J7
U 1 1 5EC91016
P 3300 2750
F 0 "J7" H 3218 3067 50  0000 C CNN
F 1 "Conn_01x03" H 3218 2976 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical" H 3300 2750 50  0001 C CNN
F 3 "~" H 3300 2750 50  0001 C CNN
	1    3300 2750
	-1   0    0    -1  
$EndComp
Wire Wire Line
	3500 2650 4300 2650
Wire Wire Line
	4300 3100 5000 3100
Wire Wire Line
	4300 2650 4300 3100
Text Label 4400 3100 0    50   ~ 0
D0(RX)
$Comp
L power:+5V #PWR0105
U 1 1 5EC99E74
P 3700 2550
F 0 "#PWR0105" H 3700 2400 50  0001 C CNN
F 1 "+5V" H 3715 2723 50  0000 C CNN
F 2 "" H 3700 2550 50  0001 C CNN
F 3 "" H 3700 2550 50  0001 C CNN
	1    3700 2550
	1    0    0    -1  
$EndComp
Wire Wire Line
	3500 2750 3700 2750
Wire Wire Line
	3700 2750 3700 2550
Wire Wire Line
	3500 2850 3700 2850
Wire Wire Line
	3700 2850 3700 2950
$Comp
L power:GND #PWR0106
U 1 1 5EC9F1B4
P 3700 2950
F 0 "#PWR0106" H 3700 2700 50  0001 C CNN
F 1 "GND" H 3700 2800 50  0000 C CNN
F 2 "" H 3700 2950 50  0000 C CNN
F 3 "" H 3700 2950 50  0000 C CNN
	1    3700 2950
	1    0    0    -1  
$EndComp
$EndSCHEMATC
