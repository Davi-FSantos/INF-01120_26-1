## Prerequisites

Before building the project, ensure you have the following tools installed.

### C++ Compiler (MSVC)
`xmake` requires a modern C++ compiler. We tested with **Visual Studio 2022 Build Tools**.

If you don't have it, it can be installed easily using `winget`:
```powershell
winget install -e --id Microsoft.VisualStudio.2022.BuildTools --override "--passive --wait --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"
```

### XMake
XMake is the build system used for this project. If you don't have it installed:
```powershell
winget install -e --id Xmake-io.Xmake
```

### Qt6 SDK
The GUI backend of this application is built using Qt6. You can install it either through the official GUI installer or via the command line using `aqtinstall`.

#### Installing via CLI (Recommended & Fast)
1. Install the `aqtinstall` tool using `winget`:
   ```powershell
   winget install -e --id miurahr.aqtinstall
   ```
2. Install the Qt 6.8.0 MSVC toolchain:
   ```powershell
   aqt install-qt windows desktop 6.8.0 win64_msvc2022_64
   ```

---

## Configuring and Building

Once the prerequisites are installed, you can configure and build the application.

### Configure XMake
Configure `xmake` to point to your compiler and Qt installation path. From your repository root folder, run:

```powershell
xmake f
```

> [!NOTE]
> **Dependency Troubleshooting:**
> XMake will automatically download dependencies like `fluidsynth`, `libremidi`, and `glib`.

### Compile the Project
Build the binary executable:
```powershell
xmake
```

### Deploy and Run the Project
Because the project dynamically links to Qt6 and FluidSynth, running the raw executable in `bin/` directly will fail with a `STATUS_DLL_NOT_FOUND` error (since Windows cannot find the required `.dll` files in its search path).

To deploy the executable along with all its required dynamic libraries:

1. **Deploy the files:**
   ```powershell
   xmake install -o build/install
   ```
   *This packages the executable into `build/install/bin/` and automatically pulls in the necessary Qt6 and FluidSynth DLLs.*

2. **Run the application:**
   Launch the deployed executable from your project root:
   ```powershell
   & "build\install\bin\Music_Machine.exe"
   ```
