// 
// Shijima-Qt - Cross-platform shimeji simulation app for desktop
// Copyright (C) 2025 pixelomer
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 

(function(){
    const allWindows = new Map();
    const serviceName = "com.pixelomer.ShijimaQt";
    const interfaceName = serviceName;
    const methodPath = "/";
    const methodName = "updateActiveWindow";
    const getFrame = (window) => {
        if (window == null) {
            return null;
        }
        const frame = {
            x: window?.pos?.x,
            y: window?.pos?.y,
            height: window?.size?.height,
            width: window?.size?.width
        };
        for (const key in frame) {
            if (frame[key] == null) {
                return null;
            }
        }
        return frame;
    };
    const notifyClient = () => {
        const window = workspace.activeWindow;
        const frame = getFrame(window);
        if (frame != null) {
            callDBus(serviceName, methodPath,
                interfaceName, methodName,
                window.internalId.toString(), window.pid, frame.x, frame.y, frame.width, frame.height,
                ()=>{});
        }
        else {
            callDBus(serviceName, methodPath,
                interfaceName, methodName,
                "", window?.pid ?? -1, -1, -1, -1, -1, ()=>{});
        }
    };
    const geometryChangedCallback = (window) => {
        if (window == workspace.activeWindow) {
            notifyClient();
        }
    };
    const registerWindow = (window) => {
        if (window == null) return;
        const id = window.internalId.toString();
        if (allWindows.has(id)) return;
        const cb = geometryChangedCallback.bind(undefined, window);
        allWindows.set(id, cb);
        window.visibleGeometryChanged.connect(cb);
    };
    const deregisterWindow = (window) => {
        if (window == null) return;
        const id = window.internalId.toString();
        if (!allWindows.has(id)) return;
        const cb = allWindows.get(id);
        window.visibleGeometryChanged.disconnect(cb);
        allWindows.delete(id);
    };
    workspace.windowRemoved.connect((window) => {
        deregisterWindow(window);
        notifyClient();
    });
    workspace.windowActivated.connect((window) => {
        registerWindow(window);
        notifyClient();
    });
    const activeWindow = workspace.activeWindow;
    if (activeWindow != null) {
        registerWindow(activeWindow);
    }
    notifyClient();
})();
