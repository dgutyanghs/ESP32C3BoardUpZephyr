# ESP32-C3 Standalone Zephyr RTOS Project Template

A lightweight, optimized, and application-centric template for developing **ESP32-C3** applications using **Zephyr RTOS**. 

By leveraging West's workspace inheritance configuration, this project setup bypasses downloading gigabytes of unneeded vendor HALs (like Nordic, NXP, or STM32), fetching **only** the core Zephyr kernel, Espressif HAL binaries, and the LVGL graphics library.

---

## 🛠️ Prerequisites & Environment Setup

Before cloning or working with this repository, ensure your system has the required cross-compilation toolchains.

### 1. Install the Zephyr SDK
The ESP32-C3 utilizes the RISC-V architecture. Download the Zephyr SDK (v0.16.0 or newer recommended):

1. Download the toolchain package from the [Zephyr SDK GitHub Releases](https://github.com).
2. Extract the SDK and run the setup script to register the RISC-V platform toolchain:
   ```bash
   cd /path/to/zephyr-sdk-<version>
   ./setup.sh -t riscv64-zephyr-elf -c
   ```

### 2. Prepare the Python Virtual Environment
Navigate to your desired workspace root location, isolate your environment, and install `west`:

```bash
mkdir esp32c3-zephyr-workspace
cd esp32c3-zephyr-workspace

python3 -m venv .venv
source .venv/bin/activate
pip install west
```

---

## 📦 Workspace Initialization

Instead of pulling down the entire massive Zephyr upstream ecosystem, initialization is handled via this application-centric folder structure.

### 1. Download Modules via Lean Manifest
Initialize `west` pointing to this repository's local `west.yml` configuration. The manifest is pre-configured with a custom `name-allowlist` to download only **`hal_espressif`** and **`lvgl`**:

```bash
# Initialize the workspace using this folder's manifest
west init --local my_esp32_app

# Fetch only the targeted Zephyr core + Espressif HAL + LVGL 
west update
```

### 2. Fetch Binary Blobs & Python Requirements
Complete the initialization by pulling Espressif's proprietary binary components (Wi-Fi, Bluetooth, and ROM hooks), installing specific script modules, and exporting CMake paths:

```bash
# Fetch required hardware bootloaders and binary blobs
west blobs fetch hal_espressif

# Install module-specific Python dependencies (including LVGL dependency scripts)
west packages pip --install

# Register the workspace into the local CMake package registry
west zephyr-export
```

---

## 🏗️ Project Architecture & Layout

Once initialization is fully generated, your workspace framework maps out like this:

```text
esp32c3-zephyr-workspace/
├── .west/             # West meta-tool definitions
├── modules/           # Isolated downstream repositories (hal_espressif, lvgl)
├── zephyr/            # Core Zephyr RTOS system layer
└── my_esp32_app/      # Active development folder (This Repository)
    ├── CMakeLists.txt # Main build framework links
    ├── prj.conf       # Kconfig configuration adjustments
    ├── app.overlay    # Devicetree hardware-mapping modifiers
    ├── west.yml       # Workspace download filtering limits
    └── src/
        └── main.c     # Application runtime code logic
```

---

## 🔧 Hardware Core Configuration Details

### Native USB Serial/JTAG Routing
The ESP32-C3 features a dedicated hardware **USB Serial/JTAG Controller** built into the silicon. Instead of loading heavy software USB stacks (`CONFIG_USB_DEVICE_STACK=y`) which will cause Kconfig abortion crashes, output data is routed natively.

The included **`app.overlay`** remaps standard input/output channels directly into the serial controller block:
```dts
/ {
	chosen {
		zephyr,console = &usb_serial;
		zephyr,shell-uart = &usb_serial;
	};
};

&usb_serial {
	status = "okay";
};
```

Ensure your **`prj.conf`** keeps standard serial infrastructure enabled:
```ini
CONFIG_CONSOLE=y
CONFIG_UART_CONSOLE=y
CONFIG_GPIO=y
```

---

## 🚀 Compiling and Flashing

Ensure your Python virtual environment remains active (`source .venv/bin/activate`) and run all commands from the top-level workspace folder.

### 1. Build the Binary
Compile for the target board blueprint (the base Espressif RISC-V Development Kit footprint is `esp32c3_devkitm`):
```bash
west build -b esp32c3_devkitm my_esp32_app --pristine
```

### 2. Flash to the Hardware
Deploy the firmware over the direct USB interface port (e.g., `/dev/ttyACM0` on Linux systems):
```bash
west flash --esp-device /dev/ttyACM0
```
*Note: If the terminal throws a `Port is busy` error, verify that any active external serial monitoring utility (like CuteCom or Minicom) has dropped or minimized its active connection port hook.*

### 3. Monitoring Log Output
Open your serial monitor tool at a **`115200`** baud rate. Cycle power or hit the physical **EN/RST** toggle pin on your development board to observe your data iterations streaming smoothly:

```text
Hello from ESP32-C3 running on Zephyr RTOS!
Running loop iteration: 0
Running loop iteration: 1
```

---

## 💡 Troubleshooting & FAQ

#### Q: The boot log displays `SHA-256 comparison failed` warning message?
**A:** This is standard, safe behavior during development. Zephyr builds the app binary payload but does not append a cryptographic signature hash verification into the metadata padding layout by default. The internal ROM bootloader notes the empty field, flags the warning, prints `Attempting to boot anyway...`, and successfully mounts the code execution environment.

#### Q: How do I expand the `CMakeLists.txt` file as my project expands?
**A:** Avoid standard executable definitions like `add_executable()`. Zephyr automatically implements an internal target layer labeled `app`. Append your dependencies using target modifiers:
```cmake
target_include_directories(app PRIVATE src/include)
target_sources(app PRIVATE 
    src/main.c
    src/ui_layer.c
)
```
