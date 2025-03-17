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

import Meta from 'gi://Meta';
import Gio from 'gi://Gio';
import GLib from 'gi://GLib';
import Shell from 'gi://Shell';
import { Extension } from 'resource:///org/gnome/shell/extensions/extension.js';

class ActiveIE {
    constructor(rect, uid, pid, scale) {
        if (rect == null) {
            this.visible = false;
        }
        else {
            if (isNaN(scale)) {
                scale = 1;
            }
            this.visible = true;
            this.uid = uid;
            this.pid = pid;
            this.x = rect.x;
            this.y = rect.y;
            this.width = rect.width;
            this.height = rect.height;
        }
    }

    /** @param {ActiveIE} other */
    isEqual(other) {
        if (other == null) {
            return false;
        }
        return this.pid === other.pid &&
            this.uid === other.uid &&
            this.visible === other.visible &&
            this.x === other.x &&
            this.y === other.y &&
            this.width === other.width &&
            this.height === other.height;
    }

    toString() {
        return `[ActiveIE uid:${this.uid} pid:${this.pid} visible:${this.visible} ` +
            `origin:${this.x},${this.y} size:${this.width},${this.height}]`;
    }
}

export default class ShijimaExtension extends Extension {
    _isValidWindowType(window) {
        return window != null && window.get_window_type() === Meta.WindowType.NORMAL;
    }

    _initWindow(window) {
        if (!this._isValidWindowType(window)) {
            return;
        }
        if (window._shijimaSignals == null) {
            window._shijimaSignals = [
                window.connect('focus', this._focusChanged.bind(this)),
                window.connect('position-changed', this._windowPositionChanged.bind(this)),
                window.connect('unmanaged', this._windowClosed.bind(this)),
                window.connect('size-changed', this._windowSizeChanged.bind(this)),
                window.connect('notify::minimized', this._windowMinimized.bind(this))
            ];
        }
    }

    _deinitWindow(window) {
        if (window == null) {
            return;
        }
        if (window._shijimaSignals != null) {
            for (const signal of window._shijimaSignals) {
                window.disconnect(signal);
            }
            delete window._shijimaSignals;
        }
    }

    _getActiveIE(ignoreHidden = false) {
        const focused = this._activeWindow;
        if (focused == null) {
            return new ActiveIE();
        }
        if (!focused.is_alive || (!ignoreHidden && focused.is_hidden()) || focused.minimized) {
            return new ActiveIE();
        }
        if (focused.get_workspace() !== global.workspaceManager.get_active_workspace()) {
            return new ActiveIE();
        }
        const rect = focused.get_frame_rect();
        const scale = focused.get_display().get_monitor_scale(focused.get_monitor());
        return new ActiveIE(rect, focused.get_id(), focused.get_pid(), scale);
    }

    _sendShijimaMessage() {
        let variant;
        if (this._activeIE != null && this._activeIE.visible) {
            variant = new GLib.Variant('(sidddd)', [
                this._activeIE.uid.toString(),
                this._activeIE.pid,
                this._activeIE.x,
                this._activeIE.y,
                this._activeIE.width,
                this._activeIE.height
            ]);
        }
        else {
            variant = new GLib.Variant('(sidddd)', [ "", -1, -1, -1, -1, -1 ])
        }
        Gio.DBus.session.call(
            'com.pixelomer.ShijimaQt',
            '/',
            'com.pixelomer.ShijimaQt',
            'updateActiveWindow',
            variant,
            null,
            Gio.DBusCallFlags.NONE,
            -1,
            null).then(()=>{}).catch((err)=>{ /* console.log(err) */ });
    }

    _notifyShijima(ignoreHidden = false) {
        const activeIE = this._getActiveIE(ignoreHidden);
        if (this._activeIE == null || !this._activeIE.isEqual(activeIE)) {
            this._activeIE = activeIE;
            this._sendShijimaMessage();
        }
    }
    
    _focusChanged(metaWindow) {
        this._activeWindow = metaWindow;
        this._notifyShijima();
    }

    _windowMinimized(metaWindow) {
        //console.log("minimized changed: " + metaWindow.toString());
        this._notifyShijima(true);
    }

    _windowSizeChanged(metaWindow) {
        if (this._activeWindow === metaWindow) {
            this._notifyShijima();
        }
    }

    _windowPositionChanged(metaWindow) {
        if (this._activeWindow === metaWindow) {
            this._notifyShijima();
        }
    }

    _windowCreated(metaWorkspace, metaWindow) {
        this._initWindow(metaWindow);
    }

    _windowClosed(metaWindow) {
        if (metaWindow === this._activeWindow) {
            this._activeWindow = null;
            this._activeIE = null;
            this._sendShijimaMessage(); // send null message
            this._notifyShijima(); // find new active window
        }
    }

    _workspaceChanged(workspaceManager) {
        this._notifyShijima();
    }

    enable() {
        this._shijimaWindowSignals = [
            global.display.connect('window-created', this._windowCreated.bind(this)),
        ];
        this._shijimaWorkspaceSignals = [
            global.workspaceManager.connect('active-workspace-changed',
                this._workspaceChanged.bind(this)),
        ];
        for (const actor of global.get_window_actors()) {
            if (actor.is_destroyed()) {
                continue;
            }
            const window = actor.get_meta_window();
            this._initWindow(window);
        }
    }

    disable() {
        if (this._shijimaWindowSignals != null) {
            for (const signal of this._shijimaWindowSignals) {
                global.display.disconnect(signal);
            }
            delete this._shijimaWindowSignals;
        }
        if (this._shijimaWorkspaceSignals != null) {
            for (const signal of this._shijimaWorkspaceSignals) {
                global.workspaceManager.disconnect(signal);
            }
            delete this._shijimaWorkspaceSignals;
        }
        for (const actor of global.get_window_actors()) {
            if (actor.is_destroyed()) {
                continue;
            }
            const window = actor.get_meta_window();
            this._deinitWindow(window);
        }
    }
}
