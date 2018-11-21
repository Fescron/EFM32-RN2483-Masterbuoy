# EFM32-RN2483-Masterbuoy

The code is based on the [DRAMCO EFM32-RN2483-LoRa-Node example](https://github.com/DRAMCO/EFM32-RN2483-LoRa-Node) with functions from [dbprint](https://github.com/Fescron/dbprint) aswell.

[This DRAMCO tutorial](https://dramco.be/tutorials/low-power-iot/technology-campus-ghent-2018/iot-development-environment/node) also has interesting info.

## 1- Installation instructions (Simplicity Studo v4)

### 1.1 - Import the files on this REPO as an existing project

`File > Import > General > Existing Projects`

![instructions](https://dramco.be/tutorials/low-power-iot/technology-campus-ghent-2018/user/pages/03.iot-development-environment/02.Node/ezgif-3-c109845b4d-2.gif "instructions")

### 1.2 - Add "dbprint" to the project

See [dbprint GIT repo](https://github.com/Fescron/dbprint).

### 1.3 - Change settings for the specific LoRaWAN application

These settings are in the file `src/my_lora_device.h`.
