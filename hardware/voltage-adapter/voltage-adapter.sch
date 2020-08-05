EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A3 16535 11693
encoding utf-8
Sheet 1 1
Title "Energy Monitor Voltage Adapter"
Date "2020-08-05"
Rev "1.0"
Comp "Tinkerforge GmbH"
Comment1 "Licensed under CERN OHL v.1.1"
Comment2 "Copyright (©) 2020, Tim Schneidermann <tim@tinkerforge.com>"
Comment3 ""
Comment4 ""
$EndDescr
Text Notes 850  11000 0    39   ~ 0
Copyright Tinkerforge GmbH 2020.\nThis documentation describes Open Hardware and is licensed under the\nCERN OHL v. 1.1.\nYou may redistribute and modify this documentation under the terms of the\nCERN OHL v.1.1. (http://ohwr.org/cernohl). This documentation is distributed\nWITHOUT ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING OF\nMERCHANTABILITY, SATISFACTORY QUALITY AND FITNESS FOR A\nPARTICULAR PURPOSE. Please see the CERN OHL v.1.1 for applicable\nconditions
$Comp
L tinkerforge:FUSE F3
U 1 1 5F30D783
P 4250 5600
F 0 "F3" H 4250 5840 50  0000 C CNN
F 1 "MRF 50mA T" H 4250 5749 50  0000 C CNN
F 2 "kicad-libraries:Fuse-MRF" H 4250 5600 50  0001 C CNN
F 3 "" H 4250 5600 50  0000 C CNN
	1    4250 5600
	1    0    0    -1  
$EndComp
Wire Wire Line
	3550 5300 3550 5600
Wire Wire Line
	3550 5600 4000 5600
$Comp
L tinkerforge:FUSE F2
U 1 1 5F30EBA3
P 4250 5100
F 0 "F2" H 4250 5340 50  0000 C CNN
F 1 "MRF 50mA T" H 4250 5249 50  0000 C CNN
F 2 "kicad-libraries:Fuse-MRF" H 4250 5248 50  0001 C CNN
F 3 "" H 4250 5100 50  0000 C CNN
	1    4250 5100
	1    0    0    -1  
$EndComp
$Comp
L tinkerforge:FUSE F1
U 1 1 5F30F455
P 4250 4650
F 0 "F1" H 4250 4890 50  0000 C CNN
F 1 "MRF 50mA T" H 4250 4799 50  0000 C CNN
F 2 "kicad-libraries:Fuse-MRF" H 4250 4650 50  0001 C CNN
F 3 "" H 4250 4650 50  0000 C CNN
	1    4250 4650
	1    0    0    -1  
$EndComp
Wire Wire Line
	3550 4650 4000 4650
$Comp
L tinkerforge:Trafo-749118115 U1
U 1 1 5F31CE47
P 5950 4250
F 0 "U1" H 5975 4691 39  0000 C CNN
F 1 "Trafo-749118115" H 5975 4616 39  0000 C CNN
F 2 "kicad-libraries:749118115" H 6150 4250 39  0001 C CNN
F 3 "" H 6150 4250 39  0001 C CNN
	1    5950 4250
	1    0    0    -1  
$EndComp
$Comp
L tinkerforge:Trafo-749118115 U2
U 1 1 5F31E3A0
P 5950 5050
F 0 "U2" H 5975 5491 39  0000 C CNN
F 1 "Trafo-749118115" H 5975 5416 39  0000 C CNN
F 2 "kicad-libraries:749118115" H 6150 5050 39  0001 C CNN
F 3 "" H 6150 5050 39  0001 C CNN
	1    5950 5050
	1    0    0    -1  
$EndComp
$Comp
L tinkerforge:Trafo-749118115 U3
U 1 1 5F31ECF5
P 5950 5850
F 0 "U3" H 5975 6291 39  0000 C CNN
F 1 "Trafo-749118115" H 5975 6216 39  0000 C CNN
F 2 "kicad-libraries:749118115" H 6150 5850 39  0001 C CNN
F 3 "" H 6150 5850 39  0001 C CNN
	1    5950 5850
	1    0    0    -1  
$EndComp
Wire Wire Line
	4500 4650 4950 4650
Wire Wire Line
	4950 4650 4950 4450
Wire Wire Line
	4950 4450 5600 4450
Wire Wire Line
	5600 5250 4950 5250
Wire Wire Line
	4950 5250 4950 5100
Wire Wire Line
	4950 5100 4500 5100
Wire Wire Line
	5600 6050 4600 6050
Wire Wire Line
	4600 6050 4600 5600
Wire Wire Line
	4600 5600 4500 5600
$Comp
L tinkerforge:CONN_01X03 P3
U 1 1 5F32A16D
P 8300 4900
F 0 "P3" H 8378 4941 50  0000 L CNN
F 1 "CONN_01X06" H 8378 4850 50  0000 L CNN
F 2 "kicad-libraries:AKL_5_3" H 8300 4900 50  0001 C CNN
F 3 "" H 8300 4900 50  0000 C CNN
	1    8300 4900
	1    0    0    -1  
$EndComp
$Comp
L tinkerforge:CONN_01X03 P4
U 1 1 5F32A54E
P 8300 5200
F 0 "P4" H 8500 5250 50  0000 C CNN
F 1 "CONN_01X06" H 8650 5100 50  0000 C CNN
F 2 "kicad-libraries:AKL_5_3" H 8300 5200 50  0001 C CNN
F 3 "" H 8300 5200 50  0000 C CNN
	1    8300 5200
	1    0    0    -1  
$EndComp
Wire Wire Line
	6350 4050 7450 4050
Wire Wire Line
	7450 4050 7450 4800
Wire Wire Line
	7450 4800 8100 4800
Wire Wire Line
	8100 4900 7350 4900
Wire Wire Line
	7350 4900 7350 4450
Wire Wire Line
	7350 4450 6350 4450
Wire Wire Line
	6350 4850 7250 4850
Wire Wire Line
	7250 4850 7250 5000
Wire Wire Line
	7250 5000 8100 5000
Wire Wire Line
	8100 5100 7250 5100
Wire Wire Line
	7250 5100 7250 5250
Wire Wire Line
	7250 5250 6350 5250
Wire Wire Line
	6350 5650 7350 5650
Wire Wire Line
	7350 5650 7350 5200
Wire Wire Line
	7350 5200 8100 5200
Wire Wire Line
	8100 5300 7450 5300
Wire Wire Line
	7450 5300 7450 6050
Wire Wire Line
	7450 6050 6350 6050
$Comp
L tinkerforge:CONN_01X04 P1
U 1 1 5F33FAF9
P 2400 5050
F 0 "P1" H 2317 5415 50  0000 C CNN
F 1 "CONN_01X04" H 2317 5324 50  0000 C CNN
F 2 "kicad-libraries:AKL_5_4" H 2400 5050 50  0001 C CNN
F 3 "" H 2400 5050 50  0000 C CNN
	1    2400 5050
	-1   0    0    -1  
$EndComp
Wire Wire Line
	2800 5300 2800 5200
Wire Wire Line
	2800 5200 2600 5200
Wire Wire Line
	2800 5300 3550 5300
Wire Wire Line
	2600 5100 4000 5100
Wire Wire Line
	3550 4650 3550 5000
Wire Wire Line
	3550 5000 2600 5000
Wire Wire Line
	5600 4050 5250 4050
Wire Wire Line
	5250 4050 5250 4850
Wire Wire Line
	5250 4850 5600 4850
Wire Wire Line
	5250 4850 5250 5650
Wire Wire Line
	5250 5650 5600 5650
Connection ~ 5250 4850
Wire Wire Line
	3450 4900 3450 4050
Wire Wire Line
	3450 4050 5250 4050
Connection ~ 5250 4050
Wire Wire Line
	2600 4900 3450 4900
$EndSCHEMATC
