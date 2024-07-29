- [Overview](#overview)
- [Features](#features)
- [Getting Started](#getting-started)
  - [Installation](#installation)
  - [Setup and Initialization](#setup-and-initialization)
  - [Folder Structure](#folder-structure)
  - [Examples (TODO)](#examples-todo)
- [License](#license)

# Overview

LoRaLite is a lightweight library for building a LoRa-based communication network using ESP32 devices with LoRa transceivers, without needing a LoRa Gateway. It supports managing peer devices (auto-pairing), handling file transfers (chunked transfers), and configuring devices as either master or slave.

This one-to-many architecture allows only the master to initiate communication, ensuring collision management and network congestion control due to LoRa transceiver limitations.

# Features

- **Master**: Manages communication between devices.
- **Slave**: Responds and sends files to the master in send or sync mode.
- **Peers**: The master can add, remove, and manage peer devices, storing details like MAC address, device name, last communication time, and signal strength.
- **Auto-pairing**: Slave devices automatically send pairing messages to the master when they wake up, allowing registration and polling by the master.
- **File Transfer**: Facilitates sending and receiving files between devices. The master can request single file transfers or use append mode to sync with a slave folder.
- **Customizable Handlers and Schedule**:
  - **Handlers**: Handle different message types such as pairing requests, file transfers, and polling acknowledgments.
  - **Scheduling**: Periodic tasks on the master device like polling, time synchronization, and configuration synchronization.

# Getting Started

## Installation

```sh
git clone https://github.com/qiweimao/LoRaLite.git
cd LoRaLite
```

## Setup and Initialization

1. **Include** `LoRaLite.h`
2. **Edit** `lora_user_settings` with necessary parameters:
   - Customized wiring pin designations for `LORA_RST`, `DIO0`, `LORA_SCK`, `LORA_MISO`, `LORA_MOSI`, `LORA_SS`
   - Customized message types in `enum UserMessageType`
   - Networking parameters: `CHUNK_SIZE`, `ACK_TIMEOUT`, `POLL_TIMEOUT`
3. **Handler Registration**: Register handlers for each customized message type defined in `enum UserMessageType`. Handlers should exit quickly to avoid blocking incoming packet processing. Use tasks for longer operations.
4. **Schedule Registration**: Register functions to send out scheduled polling requests at set intervals, allowing the master to poll the slave.

## Folder Structure

The master automatically creates a `node` folder at the root of the SD card. For each new peer (slave), a folder using its device name is created inside `node`. Handlers can request data or files from the slave, storing them in the respective folder on the master device's SD card.

## Examples (TODO)

Refer to the `examples` directory for sample code demonstrating the setup and usage of different components in the LoRa network.

# License

This project is licensed under the MIT License.