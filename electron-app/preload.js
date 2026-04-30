const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('api', {
    onKeyEvent: (callback) => {
        ipcRenderer.on('key-event', (event, data) => {
            callback(data);
        });
    }
});