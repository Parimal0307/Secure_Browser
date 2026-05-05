const { app, BrowserWindow, ipcMain, globalShortcut } = require('electron');
const path = require('path');

const addon = require('../native-addon/build/Release/addon.node');

let mainWindow;
let isQuitting = false;

function createWindow() {
    mainWindow = new BrowserWindow({
        width: 1200,
        height: 800,

        fullscreen: true,
        kiosk: true,
        alwaysOnTop: true,

        minimizable: false,
        maximizable: false,
        closable: false,

        webPreferences: {
            preload: path.join(__dirname, 'preload.js')
        }
    });

    const filePath = path.join(
        __dirname,
        'secure-browser-renderer',
        'dist',
        'index.html'
    );

    // mainWindow.loadFile(filePath);
    mainWindow.loadFile('index.html');

    // ----------------------------
    // 🔥 WIN-08 / WIN-10: BLOCK CLOSE
    // ----------------------------
    mainWindow.on('close', (e) => {
        if (!isQuitting) {
            e.preventDefault();
            console.log("From Electron: CLOSE_BLOCKED");
            mainWindow.webContents.send('security-event', 'CLOSE_BLOCKED');
        }
    });

    // ----------------------------
    // 🔥 WIN-09: BLOCK MINIMIZE
    // ----------------------------
    mainWindow.on('minimize', (e) => {
        e.preventDefault();
        console.log("From Electron: MINIMIZE_BLOCKED");
        mainWindow.webContents.send('security-event', 'MINIMIZE_BLOCKED');

        // force restore
        setTimeout(() => {
            mainWindow.restore();
            mainWindow.focus();
        }, 100);
    });

    // ----------------------------
    // 🔥 WIN-11: FOCUS LOSS DETECTION
    // ----------------------------
    mainWindow.on('blur', () => {
        if (isQuitting) return;
        console.log("From Electron: FOCUS_LOST");
        mainWindow.webContents.send('security-event', 'FOCUS_LOST');

        // force focus back
        setTimeout(() => {
            mainWindow.focus();
        }, 100);
    });

    mainWindow.on('focus', () => {
        console.log("From Electron: FOCUS_GAINED");
        mainWindow.webContents.send('security-event', 'FOCUS_GAINED');
    });

    // Register shortcut (example: Ctrl + Alt + Q)
    globalShortcut.register('Control+Alt+Q', () => {
        console.log('Exit shortcut pressed');
        isQuitting = true;

        BrowserWindow.getAllWindows().forEach(win => {
            win.destroy(); // bypass close prevention
        });

        app.quit();
    });

    // ----------------------------
    // 🔥 C++ KEY EVENTS
    // ----------------------------
    addon.onKeyEvent((event) => {
        console.log("From C++:", event);
        mainWindow.webContents.send('key-event', event);
    });

    addon.startHook();
}

// ----------------------------
// ALLOW SAFE EXIT (ADMIN)
// ----------------------------
ipcMain.on('force-exit', () => {
    isQuitting = true;
    app.quit();
});

app.whenReady().then(createWindow);

app.on('will-quit', () => {
    globalShortcut.unregisterAll();
});