#include "ShijimaManager.hpp"
#include <iostream>
#include <QVBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QGuiApplication>
#include <QFile>
#include <QScreen>
#include "ShijimaWidget.hpp"

using namespace shijima;

static ShijimaManager *m_defaultManager;

static QString readFile(QString const& file) {
    QFile f { file };
    if (!f.open(QFile::ReadOnly | QFile::Text)) return "";
    QTextStream in(&f);
    return in.readAll(); 
}

ShijimaManager *ShijimaManager::defaultManager() {
    if (m_defaultManager == nullptr) {
        m_defaultManager = new ShijimaManager;
    }
    return m_defaultManager;
}

ShijimaManager::ShijimaManager(QWidget *parent): QMainWindow(parent) {
    QVBoxLayout *layout = new QVBoxLayout;
    QPushButton *spawnButton = new QPushButton("Spawn");
    
    mascot::factory::tmpl tmpl;
    tmpl.actions_xml = readFile("test/actions.xml").toStdString();
    tmpl.behaviors_xml = readFile("test/behaviors.xml").toStdString();
    tmpl.name = "test";
    tmpl.path = "test";
    m_factory.register_template(tmpl);
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
    m_activeWindow = m_windowObserver.getActiveWindow();
    auto cursor = QCursor::pos();
    auto screen = QGuiApplication::primaryScreen();
    auto geometry = screen->geometry();
    auto available = screen->availableGeometry();
    int taskbarHeight = geometry.height() - available.height() - available.y();
    if (taskbarHeight < 0) {
        taskbarHeight = 0;
    }
    m_env->screen = { 0, geometry.width(), geometry.height(), 0 };
    m_env->floor = { geometry.height() - taskbarHeight, 0, geometry.width() };
    m_env->work_area = { 0, geometry.width(), geometry.height() - taskbarHeight, 0 };
    m_env->ceiling = { 0, 0, geometry.width() };
    if (m_activeWindow.available && m_activeWindow.x != 0
        && m_activeWindow.y != 0)
    {
        m_env->active_ie = { static_cast<int>(m_activeWindow.y),
            static_cast<int>(m_activeWindow.x + m_activeWindow.width),
            static_cast<int>(m_activeWindow.y + m_activeWindow.height),
            static_cast<int>(m_activeWindow.x) };
    }
    else {
        m_env->active_ie = { -50, -50, -50, -50 };
    }
    int x = cursor.x(), y = cursor.y();
    m_env->cursor = { (double)x, (double)y, x - m_env->cursor.x, y - m_env->cursor.y };
}

void ShijimaManager::tick() {
    if (m_mascots.size() == 0) {
        return;
    }

    updateEnvironment();

    std::vector<ShijimaWidget *> newShimeji;
    for (ShijimaWidget *shimeji : m_mascots) {
        shimeji->tick();
        auto &mascot = shimeji->mascot();
        auto &breedRequest = mascot.state->breed_request;
        if (breedRequest.available) {
            auto product = m_factory.spawn("test", breedRequest);
            ShijimaWidget *shimeji = new ShijimaWidget(std::move(product.manager));
            shimeji->show();
            newShimeji.push_back(shimeji);
            breedRequest.available = false;
        }
    }
    for (ShijimaWidget *spawned : newShimeji) {
        m_mascots.push_back(spawned);
    }
}

void ShijimaManager::spawnClicked() {
    updateEnvironment();
    auto product = m_factory.spawn("test", {});
    product.manager->reset_position();
    ShijimaWidget *shimeji = new ShijimaWidget(std::move(product.manager));
    shimeji->show();
    m_mascots.push_back(shimeji);
}