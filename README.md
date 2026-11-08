# ESP32-C3 Standalone Zephyr RTOS Project Template

A lightweight, optimized, and application-centric template for developing **ESP32-C3** applications using **Zephyr RTOS**. 

By leveraging West's workspace inheritance configuration, this project setup bypasses downloading gigabytes of unneeded vendor HALs (like Nordic, NXP, or STM32), fetching **only** the core Zephyr kernel, Espressif HAL binaries, and the LVGL graphics library directly through the manifest layout.

---

## 📦 Workspace Initialization & Environment Setup

### 1. Prepare the Python Virtual Environment
Navigate to your desired workspace root location, isolate your environment, and install `west`:

```bash
mkdir esp32c3-zephyr-workspace
cd esp32c3-zephyr-workspace

python3 -m venv .venv
source .venv/bin/activate
pip install west
```

### 2. Download and Instantiate the Workspace

You can initialize this project layout using one of two methods depending on whether you are initializing a local directory or pulling it directly from remote production:

* **Option A: Pulling directly from GitHub (Recommended for Team/Downstream Users)**
  If you are cloning this repository as a ready-to-use template, point `west init` directly to your remote repository URL using the `-m` flag. This clones the app folder and initializes the manifest registry in a single step:
  ```bash
  # Initialize using the remote project template repository
  west init -m https://github.com/dgutyanghs/ESP32C3BoardUpZephyr . 

  # Fetch only the targeted Zephyr core + Espressif HAL + LVGL
  west update
  ```

* **Option B: Initializing from a Local Directory (For Local Machine Setup)**
  If you already have this folder on your machine and want to instantiate the workspace locally around it:
  ```bash
  # Initialize the workspace using this folder's manifest configuration
  west init --local my_esp32_app

  # Fetch only the targeted Zephyr core + Espressif HAL + LVGL
  west update
  ```

### 3. Fetch Binary Blobs & Python Requirements
Complete the initialization sequence to pull Espressif's proprietary binary components (Wi-Fi, Bluetooth, and ROM hooks), install specific script modules, and export CMake paths:

```bash
# Move to the workspace root directory if not already there
# Fetch required hardware bootloaders and binary blobs
west blobs fetch hal_espressif

# Install module-specific Python dependencies (including requests, jsonschema, and LVGL dependency scripts)
west packages pip --install

# Register the workspace into the local CMake package registry
west zephyr-export
```

---

## 🏗️ Project Architecture & Layout

Once initialization is fully generated, your workspace framework maps out like this:

```text
esp32c3-zephyr-workspace/  # This folder name doesn't matter. 
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
