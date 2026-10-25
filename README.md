# ROSVis

**Status: proof-of-concept prototype.** This project is for experimentation and
learning. Features are incomplete, interfaces may change, and it is not
production-ready.

Qt 6/QML desktop prototype for exploring ROS 2 topics through ROSAdapter,
with an experimental Vulkan rendering view.

## Current functionality

- Connect to ROSAdapter over TCP and request the available topics.
- Display topic names in a scrollable list with hover and selection styling.
- Receive topic updates through the C++ UI adapter and QML signals.
- Integrate a custom Vulkan renderer into the Qt Quick window.

The data-table and scene/mesh functionality are scaffolding. Subscription
controls, continuous telemetry display, and general ROS message visualization
are not complete.

## Related projects

- [ROSAdapter](../ROSAdapter/README.md): ROS 2 bridge and TCP server.
- [ROSData](../ROSData/README.md): shared protocol definitions.

Keep the repositories alongside one another; the application builds include
`../ROSData` directly:

```text
ROS2Visualizer/
├── ROSAdapter/
├── ROSData/
└── ROSVis/
```

## Dependencies and build

- A C++23 compiler and CMake compatible with Qt 6.12 (the project declares 3.16).
- Qt 6.12 or later: Core, Gui, Qml, Quick, and the Quick Controls/Layout QML modules.
- Vulkan loader, suitable graphics driver, and `glslc` shader compiler.
- Boost.System with Asio coroutine and experimental channel support, plus threads.
- Protocol Buffers compiler (`protoc`) and C++ development library.
- Assimp headers/library and a meshoptimizer source checkout, currently required
  by CMake even though model loading is unfinished.

ROS 2 itself is required by ROSAdapter, not by this desktop application.

From this repository, select your Qt installation and dependency paths:

```sh
cmake -S . -B build/local -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_PREFIX_PATH=/path/to/Qt/6.12.0/gcc_64 \
  -DASSIMP_ROOT=/path/to/assimp \
  -DMESHOPTIMIZER_ROOT=/path/to/meshoptimizer
cmake --build build/local --parallel
```

The build defaults to third-party paths under `/home/f2tek/SwDev`. Override
those paths for your machine and make the Vulkan SDK discoverable to CMake.
Shaders are compiled to SPIR-V during the build.

## Run

Start ROSAdapter first. Then launch ROSVis from its build directory:

```sh
cd build/local
./rosvis
```

The working directory matters: shaders are loaded from `./shader/`.
The current connection defaults to **127.0.0.1:8080**, defined in
`ros2/rosadapterconfig.h`; configuration loading is not implemented yet.

## Source layout

- `ui/`: QML views and the custom Vulkan view item.
- `ros2/`: TCP client, command construction, response handling, and UI adapter.
- `renderer/`: experimental Vulkan renderer and scene/mesh scaffolding.
- `shader/`: GLSL source files.

## Prototype limitations

The connection is intended for one client and one adapter. Automatic reconnect,
request lifecycle management, UI data modeling, and rendering are still being
developed. Qt deployment rules exist, but shader packaging currently depends
on the working directory. No automated test suite is provided.
