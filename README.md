# EFM32-RN2483-Masterbuoy

The code is based on the [DRAMCO EFM32-RN2483-LoRa-Node example](https://github.com/DRAMCO/EFM32-RN2483-LoRa-Node) with functions from [dbprint](https://github.com/Fescron/dbprint) aswell. 

Added code is marked with ```/* BEGIN ADDED CODE ********/``` and ```/* END ADDED CODE ********/```. Code was added in the following files when starting from the DRAMCO example:
- ```my_lora_device.h``` (```LORAWAN_DEVICE_EUI```, ```LORAWAN_APPLICATION_EUI``` and ```LORAWAN_APPLICATION_KEY``` hidden)
- ```lpp.c```
- ```lpp.h```
- ```leuart.c```

 Added the following file (declaration of "public" variables):
 - ```main.h```  

The code is designed for use on the [EMF32 Happy Ghecko board](https://www.silabs.com/products/development-tools/mcu/32-bit/efm32-happy-gecko-starter-kit).

**Other useful links:**
- [The Things Network - Applications (Register LoRaWAN device)](https://console.thethingsnetwork.org/applications/)
- [Cayenne Dashboard](https://cayenne.mydevices.com/cayenne/dashboard/start)

## 1 - Documentation

This code receives messages over UART (**115200 baudrate**) using interrupts, parses the characters back to numbers, repacks it into a LPP (Cayenne Low Power Payload) packet and sends the data to the cloud using a LoRaWAN network.

### 1.1 - Structure of messages

### 1.1.1 - UART

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

### 1.1.2 - LPP

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


## 2 - Installation instructions (Simplicity Studo v4)

[This DRAMCO tutorial](https://dramco.be/tutorials/low-power-iot/technology-campus-ghent-2018/iot-development-environment/node) also has interesting info about setting everything up.

### 2.1 - Import the files on this REPO as an existing project

`File > Import > General > Existing Projects`

![instructions](https://dramco.be/tutorials/low-power-iot/technology-campus-ghent-2018/user/pages/03.iot-development-environment/02.Node/ezgif-3-c109845b4d-2.gif "instructions")

### 2.2 - Add "dbprint" to the project

See [dbprint GIT repo](https://github.com/Fescron/dbprint).

### 2.3 - Change settings for the specific LoRaWAN application

These settings are in the file `src/my_lora_device.h`.
