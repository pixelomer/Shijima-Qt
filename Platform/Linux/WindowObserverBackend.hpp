#pragma once

namespace Platform {

class WindowObserverBackend {
public:
    explicit WindowObserverBackend() {}
    virtual bool alive() = 0;
    virtual ~WindowObserverBackend() {};
};

}