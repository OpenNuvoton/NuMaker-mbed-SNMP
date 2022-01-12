# Example for lwIP SNMP Agent on Nuvoton's Mbed Enabled boards

## Introduction

[Simple Network Management Protocol (SNMP)](https://en.wikipedia.org/wiki/Simple_Network_Management_Protocol) is an Internet Standard protocol for network management.
Its transport binding is usually UDP.
In SNMP managed network, there are largely two types of SNMP entities:
-   Manager: Manage or monitor the network through Agents
-   Agent: Access management instrumentation and respond to Manager

Management information is viewed as a collection of managed objects, residing in a virtual information store,
termed the Management Information Base (MIB) ([RFC2578](https://datatracker.ietf.org/doc/html/rfc2578)).
Collections of related objects are defined in MIB modules.
In MIB, objects are organized in tree-like structure and each has a unique object identifier (OID).
For example, the MIB module MIB-2 ([RFC1213](https://datatracker.ietf.org/doc/html/rfc1213)) is administratively assigned .1.3.6.1.2.1, standing for .iso(1).org(3).dod(6).internet(1).mgmt(2).mib-2(1).

### lwIP SNMP on Mbed-OS

This example shows lwIP provided SNMP Agent running on Nuvoton's Mbed Enabled boards.
It relies on the following software modules:

-   [Mbed OS](https://github.com/ARMmbed/mbed-os):
    Is an open source embedded operating system designed specifically for the "things" in the Internet of Things.
-   [Lightweight TCP/IP stack](https://savannah.nongnu.org/projects/lwip/):
    Is a small independent implementation of the TCP/IP protocol suite.

With the cornerstone below, the example can realize SNMP Agent on Mbed OS without introducing excess integration complexity:
-   Mbed OS integrated lwIP core
-   lwIP decoupled SNMP from its core

For demostration, the example involves two MIB modules:
-   MIB-2 ([RFC1213](https://datatracker.ietf.org/doc/html/rfc1213)), standing for Management Information Base for Network Management of TCP/IP-based internets: MIB-II

    This is lwIP's implementation of MIB-2. It relies on:
    -   lwIP stats enabled (`LWIP_STATS=1`)
    -   lwIP stack really works with network interface say Ethernet attached.

    > **_NOTE:_** MIB-2 OID starts with .1.3.6.1.2.1, standing for .iso(1).org(3).dod(6).internet(1).mgmt(2).mib-2(1)

-   Private gpio-perif MIB

    This is intended for showing writing one simple private MIB for access to GPIO peripherals, using lwIP provided MIB parsing framework.
    For more details like access to lwIP stats, refer to the lwIP `snmp_mib2*` code.

    > **_NOTE:_** gpio-perif MIB OID starts with .1.3.6.1.4.1.**`{vendor-enterprise}`**.1, standing for .iso(1).org(3).dod(6).internet(1).private(4).enterprises(1).**`{vendor-enterprise}`**.gpio-perif(1),
    where **`{vendor-enterprise}`** is IANA assigned enterprise ID.
    OID following **`{vendor-enterprise}`** are enterprise-specific.

## Support targets

Platform                                                                    |  Connectivity       
----------------------------------------------------------------------------|-------------------
[NuMaker-PFM-M487](https://developer.mbed.org/platforms/NUMAKER-PFM-M487/)  | Ethernet          
[NuMaker-IoT-M487](https://os.mbed.com/platforms/NUMAKER-IOT-M487/)         | Wi-Fi ESP8266     

> **_NOTE:_** For network interface, Etherent is mandatory. Non-Ethernet is for experimental.

> **_NOTE:_** For non-Ethernet, lwIP stack is enabled but without network interface attached, and lwIP MIB-2 implementation will become trivial due to no real network traffic.

## Support development tools

-   [Arm's Mbed Studio](https://os.mbed.com/docs/mbed-os/v6.15/build-tools/mbed-studio.html)
-   [Arm's Mbed CLI 2](https://os.mbed.com/docs/mbed-os/v6.15/build-tools/mbed-cli-2.html)
-   [Arm's Mbed CLI 1](https://os.mbed.com/docs/mbed-os/v6.15/tools/developing-mbed-cli.html)

## Developer guide

This section is intended for developers to get started, import the example application, compile with Mbed CLI 1, and get it running as SNMP Agent on target board.

In the following, we take NuMaker-PFM-M487 as example board to show this example.

### Hardware requirements

-   [NuMaker-PFM-M487](https://developer.mbed.org/platforms/NUMAKER-PFM-M487/)
-   Host OS: Windows or others
-   Ethernet cable (RJ45)

### Software requirements

-   [Arm's Mbed CLI 1](https://os.mbed.com/docs/mbed-os/v6.15/tools/developing-mbed-cli.html)
-   [SNMPSoft Tools](https://ezfive.com/snmpsoft-tools/) or equivalent

    > **_NOTE:_** SNMPSoft Tools runs on Windows. It is fine to replace with another SNMP test tool running on another host OS say Linux or Mac OS as long as the alternative can issue equivalent SNMP test command later.

    > **_NOTE:_** SNMPSoft Tools is free for non-commercial. Please check its license terms for further uses.

### Hardware setup

-   Switch target board

    NuMaker-PFM-M487's Nu-Link: TX/RX/VCOM to ON, MSG to also ON

-   Connect target board to host through USB

    You should see Mbed USB drive shows up in File Browser.

-   Connect target board to LAN through Ethernet cable

### Network setup

Dependent on your network environment, connect host and target board to the same [LAN](https://en.wikipedia.org/wiki/Local_area_network).

### Compile with Mbed CLI 1

1.  Clone the example and navigate into it
    ```
    $ git clone https://github.com/OpenNuvoton/NuMaker-mbed-lwIP-SNMP-Agent-example
    $ cd NuMaker-mbed-lwIP-SNMP-Agent-example
    ```
1.  Deploy necessary libraries
    ```
    $ mbed deploy
    ```
1.  Configure network interface
    In `mbed_app.json`, the network interface has been Ethernet. No need for further configuration.
    ```json
        "target.network-default-interface-type" : "ETHERNET",
    ```

1.  In `app_snmp/config/snmp_agent_config.h`, change the SNMP defines to meet your requirement.
    For development, you could just leave them unchanged.

1.  Build the example on **ARM** toolchain
    ```
    $ mbed compile -m NUMAKER_PFM_M487 -t ARM
    ```

### Flash the image

Just drag-n-drop `NuMaker-mbed-lwIP-SNMP-Agent-example.bin` onto NuMaker-PFM-M487 board.

> **_NOTE:_** The operation above requires NuMaker-PFM-M487 board's Nu-Link swiched to **MASS** mode (MSG to ON).

### Verify SNMP communication

To verify SNMP communication, make sure the following host environments have set up:

-   Configure the path for SNMPSoft Tools so that they are availabe in command-line program say Windows Command Prompt
-   Configure host terminal program with **115200/8-N-1**

First, reboot target board and you should see log like below.
Make a note of **`target-board-IP`**.

<pre>
[INFO][Main]: Mbed OS version: 6.15.1
[INFO][Main]: Connecting to the network...
[INFO][Main]: MAC: <b>${target-board-MAC}</b>
[INFO][Main]: IP: <b>${target-board-IP}</b>
[INFO][Main]: Connection Success
[INFO][Main]: SNMP community: public
[INFO][Main]: SNMP community for write-access: private
[INFO][Main]: SNMP community for sending traps: public
[INFO][lwIP]: SNMP Agent thread started
</pre>

Confirm host and target board have connected to the same LAN:
<pre>
$ ping <b>${target-board-IP}</b>

Pinging <b>${target-board-IP}</b> with 32 bytes of data:
Reply from <b>${target-board-IP}</b>: bytes=32 time=21ms TTL=255
Reply from <b>${target-board-IP}</b>: bytes=32 time=1ms TTL=255
Reply from <b>${target-board-IP}</b>: bytes=32 time=4ms TTL=255
Reply from <b>${target-board-IP}</b>: bytes=32 time=6ms TTL=255
</pre>

In the following, we will access MIB by sending SNMP request message from host and expecting to receive SNMP response message replied from target board.
In every successful to and fro, you should see target board log like below:
<pre>
[INFO][lwIP]: Receive SNMP request (n-bytes) over UDP from: <b>${host-IP}</b>:<b>${host-port}</b>
[INFO][lwIP]: Send SNMP response (n-bytes) over UDP to: <b>${host-IP}</b>:<b>${host-port}</b>
</pre>

> **_NOTE:_** Without change to `MYSNMPAGENT_COMMUNITY`/`MYSNMPAGENT_COMMUNITY_WRITE`, default community `public` for read operation and `private` for write operation.

> **_NOTE:_** Not familiar with SNMPSoft Tools, run `SnmpGet.exe -h`/`SnmpSet.exe -h`/`SnmpWalk.exe -h` for help.

#### Operate on mib-2

**_Example:_** Read mib-2.system.sysDescr

<pre>
$ SnmpGet.exe -r:<b>${target-board-IP}</b> -v:2c -c:"public" -o:.1.3.6.1.2.1.1.1.0

OID=.1.3.6.1.2.1.1.1.0
Type=OctetString
Value=SNMP Agent on Nuvoton Mbed platform
</pre>

**_Example:_** Read/write mib-2.system.sysName

First, read current value:
<pre>
$ SnmpGet.exe -r:<b>${target-board-IP}</b> -v:2c -c:"public" -o:.1.3.6.1.2.1.1.5.0

OID=.1.3.6.1.2.1.1.5.0
Type=OctetString
Value=
</pre>

Write to a different value say M487 of string type (-tp:str):
<pre>
$ SnmpSet.exe -r:<b>${target-board-IP}</b> -v:2c -c:"private" -o:.1.3.6.1.2.1.1.5.0 -val:M487 -tp:str

OK
</pre>

Re-read to check if the above write takes effect:
<pre>
$ SnmpGet.exe -r:<b>${target-board-IP}</b> -v:2c -c:"public" -o:.1.3.6.1.2.1.1.5.0

OID=.1.3.6.1.2.1.1.5.0
Type=OctetString
Value=M487
</pre>

**_Example:_** Traverse mib-2.system group (delimited by -os and -op)

<pre>
$ SnmpWalk.exe -r:<b>${target-board-IP}</b> -v:2c -c:"public" -os:.1.3.6.1.2.1.1 -op:.1.3.6.1.2.1.2

OID=.1.3.6.1.2.1.1.1.0, Type=OctetString, Value=SNMP Agent on Nuvoton Mbed platform
OID=.1.3.6.1.2.1.1.2.0, Type=OID, Value=1.3.6.1.4.1.26381
OID=.1.3.6.1.2.1.1.3.0, Type=TimeTicks, Value=0:10:44.61
OID=.1.3.6.1.2.1.1.4.0, Type=OctetString, Value=foo@example.com
OID=.1.3.6.1.2.1.1.5.0, Type=OctetString, Value=M487
OID=.1.3.6.1.2.1.1.6.0, Type=OctetString, Value=
OID=.1.3.6.1.2.1.1.7.0, Type=Integer, Value=72
Total: 7
</pre>

**_Example:_** Traverse mib-2.ip group (delimited by -os and -op)

Take notice of the first row **`ipForwarding=2`**, target board not running as gateway.
<pre>
$ SnmpWalk.exe -r:<b>${target-board-IP}</b> -v:2c -c:"public" -os:.1.3.6.1.2.1.4 -op:.1.3.6.1.2.1.5

<b>OID=.1.3.6.1.2.1.4.1.0, Type=Integer, Value=2</b>
OID=.1.3.6.1.2.1.4.2.0, Type=Integer, Value=255
OID=.1.3.6.1.2.1.4.3.0, Type=Counter32, Value=1417
......
Total: 54
</pre>

> **_NOTE:_** About mib-2.ip.ipForwarding,1 for forwarding, acting as gateway, 2 for not-forwarding, not acting as gateway.

**_Example:_** Traverse mib-2.udp group (delimited by -os and -op)

Take notice of the first and fourth rows **`udpInDatagrams=10`**/**`udpOutDatagrams=10`**, total number of UDP datagrams delivered to/sent from target board.
<pre>
$ SnmpWalk.exe -r:<b>${target-board-IP}</b> -v:2c -c:"public" -os:.1.3.6.1.2.1.7 -op:.1.3.6.1.2.1.8

<b>OID=.1.3.6.1.2.1.7.1.0, Type=Counter32, Value=10</b>
OID=.1.3.6.1.2.1.7.2.0, Type=Counter32, Value=0
OID=.1.3.6.1.2.1.7.3.0, Type=Counter32, Value=0
<b>OID=.1.3.6.1.2.1.7.4.0, Type=Counter32, Value=10</b>
......
Total: 12
</pre>

**_Example:_** Traverse mib-2.snmp group (delimited by -os and -op)

Take notice of the first two rows **`snmpInPkts=7`**/**`snmpOutPkts=6`**, total number of messages delivered to/passed from target board.
<pre>
$ SnmpWalk.exe -r:<b>${target-board-IP}</b> -v:2c -c:"public" -os:.1.3.6.1.2.1.11 -op:.1.3.6.1.2.1.12

<b>OID=.1.3.6.1.2.1.11.1.0, Type=Counter32, Value=9</b>
<b>OID=.1.3.6.1.2.1.11.2.0, Type=Counter32, Value=8</b>
OID=.1.3.6.1.2.1.11.3.0, Type=Counter32, Value=0
OID=.1.3.6.1.2.1.11.4.0, Type=Counter32, Value=0

......
Total: 30
</pre>

**_Example:_** Generate trap by write to mib-2.system.sysName with invalid community name

<pre>
$ SnmpSet.exe -r:<b>${target-board-IP}</b> -v:2c -c:"invalid_community" -o:.1.3.6.1.2.1.1.5.0 -val:M487 -tp:str

%Failed to set value to SNMP variable. Timeout.
</pre>

Different than normal case, target board responds with SNMP trap message and destination port changes to officially assigned **`162`** for trap.

<pre>
[INFO][lwIP]: Receive SNMP request (56) over UDP from: <b>${host-IP}</b>:<b>${host-port}</b>
[WARN][lwIP]: Trap destination IP is dummy. Change to the SNMP request source: <b>${host-IP}</b>
[INFO][lwIP]: Send SNMP trap (42) over UDP to: <b>${host-IP}</b>:<b>162</b>
</pre>

> **_NOTE:_** Without change to `MYSNMPAGENT_COMMUNITY_TRAP`, default community `public` for sending traps.

> **_NOTE:_** Without change to `MYSNMPAGENT_TRAP_DST_IP`, default trap destination IP is dummy and changes to SNMP request source.

#### Operate on private gpio-perif MIB

> **_NOTE:_** Without change to `MYSNMPAGENT_VENDOR_ENTERPRISE_OID`, use default lwIP enterprise OID **`26381`** for demo.

**_Example:_** Toggle gpio-perif.leds.led2

Run the write commands alternately (0/1 of integer type (-tp:int)) and check if on-board LED changes accordingly:
<pre>
SnmpSet.exe -r:<b>${target-board-IP}</b> -v:2c -c:"private" -o:.1.3.6.1.4.1.26381.1.2.2.0 -val:0 -tp:int
SnmpSet.exe -r:<b>${target-board-IP}</b> -v:2c -c:"private" -o:.1.3.6.1.4.1.26381.1.2.2.0 -val:1 -tp:int
</pre>

**_Example:_** Traverse gpio-perif MIB (delimited by -os and -op)

<pre>
$ SnmpWalk.exe -r:<b>${target-board-IP}</b> -v:2c -c:"public" -os:.1.3.6.1.4.1.26381.1 -op:.1.3.6.1.4.1.26381.2

OID=.1.3.6.1.4.1.26381.1.1.1.0, Type=Integer, Value=1
OID=.1.3.6.1.4.1.26381.1.1.2.0, Type=Integer, Value=1
OID=.1.3.6.1.4.1.26381.1.2.1.0, Type=Integer, Value=0
OID=.1.3.6.1.4.1.26381.1.2.2.0, Type=Integer, Value=0
Total: 4
</pre>

**_Example:_** Traverse gpio-perif.buttons (delimited by -os and -op)

<pre>
$ SnmpWalk.exe -r:<b>${target-board-IP}</b> -v:2c -c:"public" -os:.1.3.6.1.4.1.26381.1.1 -op:.1.3.6.1.4.1.26381.1.2

OID=.1.3.6.1.4.1.26381.1.1.1.0, Type=Integer, Value=1
OID=.1.3.6.1.4.1.26381.1.1.2.0, Type=Integer, Value=1
Total: 2
</pre>

**_Example:_** Traverse gpio-perif.leds group (delimited by -os and -op)

<pre>
SnmpWalk.exe -r:<b>${target-board-IP}</b> -v:2c -c:"public" -os:.1.3.6.1.4.1.26381.1.2 -op:.1.3.6.1.4.1.26381.1.3

OID=.1.3.6.1.4.1.26381.1.2.1.0, Type=Integer, Value=0
OID=.1.3.6.1.4.1.26381.1.2.2.0, Type=Integer, Value=0
Total: 2
</pre>

### Walk through source code

#### SNMP Agent application on Mbed OS (`app_snmp`)

Based on lwIP SNMP code, the main SNMP Agent application code on Mbed OS is placed here.

-   `config/`:  Define SNMP Agent parameters

    Largely, customizable SNMP Agent parameters are centralized in `snmp_agent_config.h`.
    For development, just leave it unchanged.

-   `mib/`: Show SNMP private gpio-perif MIB, accessing buttons/leds on the target board

    > **_NOTE:_** Configurations with GPIO pin names of buttons/leds are defined in `mbed_app.json`.

    ```json
    "config": {
        "gpio-perif-button1": {
            "help"      : "PinName for button1, used for SNMP private GPIO peripheral MIB"
        },
        "gpio-perif-button2": {
            "help"      : "PinName for button2, used for SNMP private GPIO peripheral MIB"
        },
        "gpio-perif-led1": {
            "help"      : "PinName for led1, used for SNMP private GPIO peripheral MIB"
        },
        "gpio-perif-led2": {
            "help"      : "PinName for led2, used for SNMP private GPIO peripheral MIB"
        }
    }
    ```
-   `transport/`: Replace lwIP SNMP's transport with Mbed OS's `UDPSocket`.

#### lwIP SNMP (`lwip_snmp/`)

The original lwIP SNMP code is imported here without further modification.

> **_NOTE:_** The lwIP version is tagged `STABLE-2_1_3_RELEASE`.

> **_NOTE:_** snmp_netconn.c and snmp_raw.c are removed from build list.
They are default transports provided by lwIP SNMP code and are to replace with Mbed OS's `UDPSocket`.

#### Pre-main (`pre-main/`)

In Mbed OS boot sequence, `mbed_main()`, designed for user application override, is run before `main()`.
Here, it is used to run the following tasks:

1.  Set up event queue for dispatching host command (`host-stdin/`)

    Currently, press the following command:
    1.  `h` for printing heap statistics
        ```
        ** MBED HEAP STATS **
        **** current_size   : 10936
        **** max_size       : 11076
        **** reserved_size  : 107600
        *****************************
        ```

    1.  `s` for printing stack statistics
        ```
        ** MBED THREAD STACK STATS **
        Thread: 0x20006868, Stack size: 2048, Max stack: 464
        Thread: 0x2000cf10, Stack size: 512, Max stack: 64
        Thread: 0x2000d8f0, Stack size: 2400, Max stack: 536
        Thread: 0x2000cf98, Stack size: 4096, Max stack: 712
        Thread: 0x2000cf54, Stack size: 768, Max stack: 96
        Thread: 0x2000d95c, Stack size: 4096, Max stack: 384
        Thread: 0x20005ae8, Stack size: 1024, Max stack: 320
        *****************************
        ```

    1.  `r` for resetting target board
        ```
        System reset after 2 secs...
        ```

    > **_NOTE:_** This function is for development requirement. Remove `host-stdin/` directory completely for production.

> **_NOTE:_** Without pre-main requirement, remove `pre-main/` directory completely for production.

## Limitations

-   Support SNMP v1/v2c, no v3.
