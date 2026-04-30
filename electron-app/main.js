const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');

// Load native addon
const addon = require('../native-addon/build/Release/addon.node');

let mainWindow;

function createWindow() {
    mainWindow = new BrowserWindow({
        width: 800,
        height: 600,
        webPreferences: {
            preload: path.join(__dirname, 'preload.js')
        }
    });
    
    const filePath = path.join(__dirname, 'secure-browser-renderer', 'dist', 'index.html')
    mainWindow.loadFile(filePath);

    // Receive events from C++
    addon.onKeyEvent((event) => {
        console.log("From C++:", event);
        mainWindow.webContents.send('key-event', event);
    });

    addon.startHook();
}

app.whenReady().then(createWindow);