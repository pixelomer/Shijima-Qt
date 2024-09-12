#include "ShijimaManager.hpp"
#include <iostream>
#include <QVBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QTextStream>
#include <QGuiApplication>
#include <QFile>
#include <QScreen>
#include <QRandomGenerator>
#include "ShijimaWidget.hpp"
#include "MascotFinder.hpp"

using namespace shijima;

static ShijimaManager *m_defaultManager = nullptr;

ShijimaManager *ShijimaManager::defaultManager() {
    if (m_defaultManager == nullptr) {
        m_defaultManager = new ShijimaManager;
    }
    return m_defaultManager;
}

void ShijimaManager::finalize() {
    if (m_defaultManager != nullptr) {
        delete m_defaultManager;
        m_defaultManager = nullptr;
    }
}

void ShijimaManager::killAll() {
    for (auto mascot : m_mascots) {
        mascot->markForDeletion();
    }
}

void ShijimaManager::killAllButOne(ShijimaWidget *widget) {
    for (auto mascot : m_mascots) {
        if (widget == mascot) {
            continue;
        }
        mascot->markForDeletion();
    }
}

ShijimaManager::ShijimaManager(QWidget *parent): QMainWindow(parent) {
    QVBoxLayout *layout = new QVBoxLayout;
    QPushButton *spawnButton = new QPushButton("Spawn");
    
    MascotFinder mascotFinder;
    int found = mascotFinder.findAll(m_factory);
    if (found == 0) {
        throw std::runtime_error("Could not find any mascots");
    }
    auto &allTemplates = m_factory.get_all_templates();
    for (auto &pair : allTemplates) {
        std::cout << "Loaded mascot: " << pair.first << std::endl;
    }

    m_env = m_factory.env = std::make_shared<mascot::environment>();
    connect(spawnButton, &QPushButton::clicked, this, &ShijimaManager::spawnClicked);
    layout->addWidget(spawnButton);
    QWidget *widget = new QWidget;
    widget->setLayout(layout);
    setCentralWidget(widget);
    m_mascotTimer = startTimer(40);
    if (m_windowObserver.tickFrequency() > 0) {
        m_windowObserverTimer = startTimer(m_windowObserver.tickFrequency());
    }
}

void ShijimaManager::timerEvent(QTimerEvent *event) {
    int timerId = event->timerId();
    if (timerId == m_mascotTimer) {
        tick();
    }
    else if (timerId == m_windowObserverTimer) {
        m_windowObserver.tick();
    }
}

void ShijimaManager::updateEnvironment() {
    m_currentWindow = m_windowObserver.getActiveWindow();
    auto cursor = QCursor::pos();
    auto screen = QGuiApplication::primaryScreen();
    auto geometry = screen->geometry();
    auto available = screen->availableGeometry();
    int taskbarHeight = geometry.height() - available.height() - available.y();
    if (taskbarHeight < 0) {
        taskbarHeight = 0;
    }
    double gwidth = (double)geometry.width();
    double gheight = (double)geometry.height();
    m_env->screen = { 0, gwidth, gheight, 0 };
    m_env->floor = { gheight - taskbarHeight, 0, gwidth };
    m_env->work_area = { 0, gwidth, gheight - taskbarHeight, 0 };
    m_env->ceiling = { 0, 0, gwidth };
    if (m_currentWindow.available && m_currentWindow.x != 0
        && m_currentWindow.y != 0)
    {
        m_env->active_ie = { m_currentWindow.y,
            m_currentWindow.x + m_currentWindow.width,
            m_currentWindow.y + m_currentWindow.height,
            m_currentWindow.x };
        if (m_previousWindow.available &&
            m_previousWindow.uid == m_currentWindow.uid)
        {
            m_env->active_ie.dy = m_currentWindow.y - m_previousWindow.y;
            m_env->active_ie.dx = m_currentWindow.x - m_previousWindow.x;
        }
    }
    else {
        m_env->active_ie = { -50, -50, -50, -50 };
    }
    int x = cursor.x(), y = cursor.y();
    m_env->cursor = { (double)x, (double)y, x - m_env->cursor.x, y - m_env->cursor.y };
    m_previousWindow = m_currentWindow;
}

void ShijimaManager::tick() {
    if (m_mascots.size() == 0) {
        return;
    }

    updateEnvironment();

    for (int i=m_mascots.size()-1; i>=0; --i) {
        ShijimaWidget *shimeji = m_mascots[i];
        if (!shimeji->isVisible()) {
            delete shimeji;
            m_mascots.erase(m_mascots.begin() + i);
            break;
        }
        shimeji->tick();
        auto &mascot = shimeji->mascot();
        auto &breedRequest = mascot.state->breed_request;
        if (breedRequest.available) {
            if (breedRequest.name == "") {
                breedRequest.name = shimeji->mascotName();
            }
            auto product = m_factory.spawn(breedRequest);
            ShijimaWidget *shimeji = new ShijimaWidget(product.tmpl->name,
                product.tmpl->path, std::move(product.manager));
            shimeji->show();
            m_mascots.push_back(shimeji);
            breedRequest.available = false;
        }
    }
}

ShijimaWidget *ShijimaManager::hitTest(QPoint const& screenPos) {
    for (auto mascot : m_mascots) {
        QPoint localPos = { screenPos.x() - mascot->x(),
            screenPos.y() - mascot->y() };
        if (mascot->pointInside(localPos)) {
            return mascot;
        }
    }
    return nullptr;
}

void ShijimaManager::spawn(std::string const& name) {
    updateEnvironment();
    auto product = m_factory.spawn(name, {});
    product.manager->reset_position();
    ShijimaWidget *shimeji = new ShijimaWidget(name,
        product.tmpl->path, std::move(product.manager));
    shimeji->show();
    m_mascots.push_back(shimeji);
}

void ShijimaManager::spawnClicked() {
    auto &allTemplates = m_factory.get_all_templates();
    int target = QRandomGenerator::global()->bounded((int)allTemplates.size());
    int i = 0;
    for (auto &pair : allTemplates) {
        if (i++ != target) continue;
        std::cout << "Spawning: " << pair.first << std::endl;
        spawn(pair.first);
        break;
    }
}