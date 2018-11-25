# EFM32-RN2483-Masterbuoy

The code is based on the [DRAMCO EFM32-RN2483-LoRa-Node example](https://github.com/DRAMCO/EFM32-RN2483-LoRa-Node) with functions from [dbprint](https://github.com/Fescron/dbprint) aswell and is designed for use on the [EMF32 Happy Ghecko board](https://www.silabs.com/products/development-tools/mcu/32-bit/efm32-happy-gecko-starter-kit).

Added code is marked with ```/* BEGIN ADDED CODE ********/``` and ```/* END ADDED CODE ********/```. Code was added in the following files when starting from the DRAMCO example:
- ```src/my_lora_device.h``` (```LORAWAN_DEVICE_EUI```, ```LORAWAN_APPLICATION_EUI``` and ```LORAWAN_APPLICATION_KEY``` hidden)
- ```lora/lpp.c```
- ```lora/lpp.h```
- ```system/leuart.c```

 Added the following file (declaration of "public" variables):
 - ```src/main.h```  

**Other useful links:**
- [The Things Network - Applications (Register LoRaWAN device)](https://console.thethingsnetwork.org/applications/)
- [Cayenne Dashboard](https://cayenne.mydevices.com/cayenne/dashboard/start)

------

## 1 - Documentation

This code receives messages over UART (**115200 baudrate**) using interrupts, parses the characters back to numbers, repacks it into a LPP (Cayenne Low Power Payload) packet and sends the data to the cloud using a LoRaWAN network.

### 1.1 - Code flow

![Flowchart](/latex-flowchart/flowchart.png?raw=true "Flowchart")

#### 1.1.1 - Extra notes on the flowchart
- **(\*1):** If there was a line received using interrupts on UART *AND* there is at least one of the three buffers free, copy the received data to one of those buffers.
- **(\*2):** Check if **one (of three)** or more buffers are filled *AND* at least one of the two data fields are free.
- **(\*3):** Parse the data out of a used buffer into a free data field.

#### 1.1.2 - Processes on the flowchart
- **INIT:**
  - Initialize system & chip (clocks, ...).
  - Initialize delay function.
  - Initialize interrupts (IRQ).
  - Initialize LED functions to give feedback when there is an error.
  - Initialize buttons and their interrupt functionality.
  - Initialize power management.
  - Initialize ADC for reading the battery voltage.
  - Initialize I²C for sensor readout.
  - Initialize UART using dbprint functionality for:
    - Printing debug messages.
    - Getting input using interrupts.
    - Parsing characters to values.
- **JOIN:**
  - Initialize LoRaWAN communications with *Over The Air Activation* (OTAA).
- **MEASURE:**
  - Read battery voltage.
  - Read relative humidity.
  - Read temperature.
    - **NOTE:** The sensor readouts are not really used since we have a limited amount of bytes we can send to the cloud.
- **SEND:**
  - See flowchart.
- **SLEEP:**
  - See flowchart.
  
------

### 1.2 - Structure of messages

#### 1.2.1 - UART

The Received UART lines need to have the following structure (the data is in **decimal notation**):

|     Data    | Bouy ID |  RSSI   |  VBAT   |
|:-----------:|:-------:|:-------:|:-------:|
| **Length**  | 3 chars | 2 chars | 3 chars |
| **Example** |   008   |   66    |   325   |

The example data corresponds with:
- **Bouy ID** = 8
- **RSSI** = (-)66
- **VBAT** = 3,25 V
  - **NOTE:** The VBAT data gets send to "the cloud" with a **bouy-ID-offset of "1"**!!

#### 1.2.2 - LPP

After parsing the UART characters back to numbers, the retransmitted [Cayenne Low Power Payload](https://github.com/myDevicesIoT/cayenne-docs/blob/master/docs/LORA.md) packet has the following structure (the data is in **hexadecimal notation**, *each column represents one byte*):

| LPP terminology | Data channel |   Data type   | Byte 0 | Data channel |  Data type   | Byte 0 | Byte 1 | 
|:---------------:|:------------:|:-------------:|:------:|:------------:|:------------:|:------:|:------:|
|    **Data**     |   Bouy ID    | Digital Input |        | Bouy ID + 1  | Analog Input |        |        |
|   **Example**   |      08      |      00       |   42   |      09      |      02      |   01   |   45   |

As noted above, the **RSSI value** is *disguised as* an **analog input** while the **VBAT data** is *disguised as* a **digital input**.

The example data corresponds with:
- **Bouy ID** = 0x08 = 8d
- **RSSI** = 0x42 = 66d
- **VBAT** = 0x0145 = 325d
  - **NOTE:** The VBAT data gets send to "the cloud" with a **bouy-ID-offset of "1"**!!

------

## 2 - Installation instructions (Simplicity Studo v4)

[This DRAMCO tutorial](https://dramco.be/tutorials/low-power-iot/technology-campus-ghent-2018/iot-development-environment/node) also has interesting info about setting everything up.

### 2.1 - Import the files on this REPO as an existing project

`File > Import > General > Existing Projects`

![Instructions](https://dramco.be/tutorials/low-power-iot/technology-campus-ghent-2018/user/pages/03.iot-development-environment/02.Node/ezgif-3-c109845b4d-2.gif "Instructions")

### 2.2 - Add "dbprint" to the project

See [dbprint GIT repo](https://github.com/Fescron/dbprint).

### 2.3 - Change settings for the specific LoRaWAN application

These settings are in the file `src/my_lora_device.h`.
