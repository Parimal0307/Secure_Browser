const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('api', {
    onKeyEvent: (callback) => ipcRenderer.on('key-event', (_, data) => callback(data)),
    onSecurityEvent: (callback) => ipcRenderer.on('security-event', (_, data) => callback(data)),

    forceExit: () => ipcRenderer.send('force-exit')
});