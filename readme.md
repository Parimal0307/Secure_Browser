# Secure Browser Demo (Electron + Win32 Native Addon)

## Overview

This project is a **hybrid desktop application** combining:

* **Electron (JavaScript)** → UI + application control
* **Win32 (C++ via Node-API)** → system-level enforcement

The current implementation includes:

* Global keyboard hook (`WH_KEYBOARD_LL`)
* Blocking of restricted shortcuts:

  * Alt+Tab
  * Alt+Esc
  * Ctrl+Esc
  * Windows key
  * Alt+F4
  * Ctrl+Shift+Esc
* Event communication from C++ → Electron via **ThreadSafeFunction (TSFN)**

---

## Folder Structure

```
secure-browser-demo/
│
├── electron-app/              # Electron application (UI + control)
│   ├── main.js               # Main process (entry point)
│   ├── preload.js            # Secure bridge (IPC exposure)
│   ├── renderer.js           # UI logic
│   ├── index.html            # UI layout
│   ├── secure-browser-renderer/ (optional React build)
│   ├── package.json
│
├── native-addon/             # Win32 + N-API layer
│   ├── addon.cpp             # Core native logic (hooks + blocking)
│   ├── binding.gyp           # Build config for node-gyp
│   ├── build/                # Compiled output (.node file)
│   ├── package.json
│
└── README.md
```

---

## How Components Interact

### High-Level Flow

```
User Input → Win32 Hook → C++ Addon → TSFN → Electron Main → Renderer UI
```

---

### Detailed Interaction

#### 1. `addon.cpp` (Native Layer)

* Installs global keyboard hook
* Detects restricted key combinations
* Blocks them using `return 1`
* Sends event using:

  * `ThreadSafeFunction` → safe cross-thread call

---

#### 2. `main.js` (Electron Main)

* Loads native addon:

  ```js
  const addon = require('../native-addon/build/Release/addon.node');
  ```
* Registers callback:

  ```js
  addon.onKeyEvent(...)
  ```
* Starts monitoring:

  ```js
  addon.startHook();
  ```
* Sends events to renderer:

  ```js
  mainWindow.webContents.send(...)
  ```

---

#### 3. `preload.js`

* Exposes safe IPC API to frontend using `contextBridge`

---

#### 4. `renderer.js`

* Listens for events from main process
* Updates UI (e.g., display blocked key events)

---

## Setup & Installation

### Prerequisites

Install:

* Node.js (LTS)
* Visual Studio (with **Desktop Development with C++**)
* Python (for node-gyp)

---

### Step 1 — Build Native Addon

```bash
cd native-addon
node-gyp configure
node-gyp build
```

Output:

```
native-addon/build/Release/addon.node
```

---

### Step 2 — Rebuild for Electron

```bash
cd ../electron-app
npm install
npx electron-rebuild
```

---

### Step 3 — Run Application

```bash
npm start
```

---

## Tasks Completed (Your Work)

### WIN-01 to WIN-07

Implemented in:

```
native-addon/addon.cpp
```

Includes:

* Global keyboard hook
* Blocking:

  * Alt+Tab
  * Alt+Esc
  * Ctrl+Esc
  * Windows key
  * Alt+F4
  * Ctrl+Shift+Esc

---

### WIN-12 (Clipboard) — To Be Done

### WIN-13 (Integration) — To Be Done

You will extend:

```
addon.cpp
main.js
preload.js
```

---

## Tasks for Teammate (WIN-08 to WIN-11)

Your teammate will implement:

### WIN-08 → Intercept minimize/close

### WIN-09 → Prevent minimize

### WIN-10 → Prevent window close

### WIN-11 → Detect focus loss

---

## Where Your Teammate Should Work

### 1. Electron Main Layer

File:

```
electron-app/main.js
```

Responsibilities:

* Control window behavior
* Modify BrowserWindow options:

  * `fullscreen`
  * `kiosk`
  * `closable: false`
* Handle window events:

  ```js
  win.on('close', ...)
  win.on('minimize', ...)
  ```

---

### 2. Renderer Layer

File:

```
renderer.js
```

Responsibilities:

* Display warnings (focus loss, minimize attempts)
* Handle UI reactions

---

### 3. Native Layer (Optional for Focus Detection)

File:

```
native-addon/addon.cpp
```

If implementing OS-level focus detection:

* Use Win32 APIs:

  * `GetForegroundWindow()`
  * Compare with app window handle

---

## Key Design Constraints

* Do NOT call JS directly from C++ threads
  → Always use `ThreadSafeFunction`

* Do NOT block Node.js main thread

* Maintain separation:

  * C++ → enforcement
  * Electron → UI + orchestration

---

## Common Issues & Fixes

### 1. `cl.exe not found`

→ Install Visual Studio C++ workload

---

### 2. Native module not loading

```bash
npx electron-rebuild
```

---

### 3. Crash on key press

→ Ensure TSFN is used (not direct callback)

---

## Notes for Collaboration

* Native logic → only in `addon.cpp`
* UI + app behavior → only in Electron
* Avoid mixing responsibilities

---

## Summary

This project demonstrates:

* Win32 system-level input control
* Safe native ↔ JS communication
* Electron as a UI wrapper over native enforcement

You now have a working base for:

* Secure exam browser features
* Further system-level controls

---
