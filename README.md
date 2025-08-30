# COSMO Mobilus shutters x Home Assistant (MQTT) Adapter
This repository provides all the necessary information—both software and hardware—to build your own adapter that connects COSMO Mobilus-controlled shutters to Home Assistant via MQTT.

The hardware setup is based on an actual COSMO Mobilus remote control and requires basic soldering skills to complete.

A high-level schematic of the end-to-end solution is shown in the diagram below.

![Adapter overview](/diagrams/img/architecture-overview.png)

## Motivation
There is no native support for COSMO protocol in Home Assistant.

## Features
- [x] Each shutter is automatically discovered by Home Assistant.
- [x] Position control is supported for each shutter, where 100 represents fully open and 0 fully closed.
- [x] Shutter positions are stored in memory to prevent re-synchronization after restarts.
- [x] The current position of the remote control unit is also saved in memory.

## Supported Device
- Mobilus COSMO HM
- Mobilus COSMO HM1
- Mobilus COSMO HM3+
- Mobilus COSMO G3+ Light (*)
- Mobilus COSMO WM (*)
- Mobilus COSMO L5 (*)
- Mobilus COSMO L1 (*)

(*) - These devices have not been tested; however, given that they use mechanical switches, this approach is expected to work with them as well.