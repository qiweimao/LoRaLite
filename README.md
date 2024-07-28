### LoRaLite: A LoRa Network Based on Transceivers

**Overview**
LoRaLite is a lightweight library for building a LoRa-based communication network using ESP32 devices. This library provides functionalities for initializing LoRa communication, managing peer devices, handling file transfers, and operating both gateway and slave devices.

**Features**
- **Initialization**: Setup and configure the LoRa module.
- **Gateway**: Manage communication between devices.
- **Peers**: Add, remove, and manage peer devices.
- **File Transfer**: Send and receive files between devices.
- **Slave**: Implement slave functionalities within the network.
- **Handlers and Schedule**: 
  - **Handlers**: Functions to handle different types of messages such as pairing requests, file transfers, and polling acknowledgments.
  - **Scheduling**: Periodic tasks like polling, time synchronization, and configuration synchronization.

**Getting Started**
1. **Clone the repository**:
    ```sh
    git clone https://github.com/qiweimao/LoRaLite.git
    cd LoRaLite
    ```

2. **Setup and Initialization**:
   - Include `lora_init.h` and call `initLoRa()` to initialize the LoRa module with the necessary parameters.

3. **Gateway Configuration**:
   - Use `lora_gateway.h` to set up the gateway functions, including data reception and polling.

4. **Peer Management**:
   - Utilize `lora_peer.h` to manage peer devices by adding, removing, saving, and loading peers.

5. **File Transfer**:
   - Implement file transfer functionalities using `lora_file_transfer.h`.

6. **Slave Configuration**:
   - Configure slave devices with `lora_slave.h` to handle data reception and polling.

**Examples**
Refer to the `examples` directory for sample code demonstrating the setup and usage of different components in the LoRa network.

**Contributing**
Contributions are welcome! Please fork the repository and submit a pull request.

**License**
This project is licensed under the MIT License.

For more information, visit the [GitHub repository](https://github.com/qiweimao/LoRaLite).
