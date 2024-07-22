(function(){
    const allWindows = new Map();
    const serviceName = "com.pixelomer.ShijimaQT";
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
                window.pid, frame.x, frame.y, frame.width, frame.height, ()=>{});
        }
        else {
            callDBus(serviceName, methodPath,
                interfaceName, methodName,
                window?.pid ?? -1, -1, -1, -1, -1, ()=>{});
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
