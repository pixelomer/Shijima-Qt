#include "MascotBackendWidgets.hpp"
#include "ShijimaWidget.hpp"
#include "ShijimaManager.hpp"
#include <QGuiApplication>

MascotBackendWidgets::MascotBackendWidgets(ShijimaManager *manager):
    MascotBackend(manager)
{
    for (auto screen : QGuiApplication::screens()) {
        screenAdded(screen);
    }

    connect(qApp, &QGuiApplication::screenAdded,
        this, &MascotBackendWidgets::screenAdded);
    connect(qApp, &QGuiApplication::screenRemoved,
        this, &MascotBackendWidgets::screenRemoved);
}

MascotBackendWidgets::~MascotBackendWidgets() {
    disconnect(qApp, &QGuiApplication::screenAdded,
        this, &MascotBackendWidgets::screenAdded);
    disconnect(qApp, &QGuiApplication::screenRemoved,
        this, &MascotBackendWidgets::screenRemoved);
}

ActiveMascot *MascotBackendWidgets::spawn(MascotData *mascotData,
    std::unique_ptr<shijima::mascot::manager> mascot,
    int mascotId)
{
    QScreen *screen = spawnScreen();
    updateEnvironment(screen);
    auto &env = m_env[screen];
    mascot->state->env = env;
    mascot->reset_position();
    env->reset_scale();
    return new ShijimaWidget(this, mascotData, std::move(mascot),
        mascotId, manager());
}

std::shared_ptr<shijima::mascot::environment> MascotBackendWidgets::spawnEnv() {
    return m_env[spawnScreen()];
}

ActiveMascot *MascotBackendWidgets::migrate(ActiveMascot &old) {
    QScreen *screen = spawnScreen();
    updateEnvironment(screen);
    old.mascot().state->env = spawnEnv();
    old.mascot().reset_position();
    old.mascot().state->env->reset_scale();
    auto shimeji = new ShijimaWidget(this, old, manager());
    return shimeji;
}

void MascotBackendWidgets::preTick() {
    for (auto screen : m_env.keys()) {
        updateEnvironment(screen);
    }
}

void MascotBackendWidgets::postTick() {
    for (auto &env : m_env) {
        env->reset_scale();
        env->cursor.dx = env->cursor.dy = 0;
    }
}

void MascotBackendWidgets::screenAdded(QScreen *screen) {
    if (!m_env.contains(screen)) {
        auto env = std::make_shared<shijima::mascot::environment>();
        m_env[screen] = env;
        m_reverseEnv[env.get()] = screen;
        auto primary = QGuiApplication::primaryScreen();
        if (screen != primary && m_env.contains(primary)) {
            m_env[screen]->allows_breeding = m_env[primary]->allows_breeding;
        }
    }
}

void MascotBackendWidgets::screenRemoved(QScreen *screen) {
    if (m_env.contains(screen) && screen != nullptr) {
        auto primary = QGuiApplication::primaryScreen();
        /*for (auto &mascot : m_mascots) {
            mascot->setEnv(m_env[primary]);
            mascot->mascot().reset_position();
        }*/
        m_reverseEnv.remove(m_env[primary].get());
        m_env.remove(screen);
    }
}

void MascotBackendWidgets::updateEnvironments(
    std::function<void(shijima::mascot::environment &)> cb)
{
    for (auto &env : m_env) {
        cb(*env);
    }
}

QScreen *MascotBackendWidgets::spawnScreen() {
    QScreen *screen = manager()->screen();
    if (screen == nullptr) {
        screen = qApp->primaryScreen();
    }
    return screen;
}


void MascotBackendWidgets::updateEnvironment(QScreen *screen) {
    if (!m_env.contains(screen)) {
        return;
    }
    auto &env = *m_env[screen];
    QRect geometry, available;
    QPoint cursor;
    cursor = QCursor::pos();
    geometry = screen->geometry();
    available = screen->availableGeometry();
    int taskbarHeight = available.bottom() - geometry.bottom();
    int statusBarHeight = geometry.top() - available.top();
    if (taskbarHeight < 0) {
        taskbarHeight = 0;
    }
    if (statusBarHeight < 0) {
        statusBarHeight = 0;
    }
    env.screen = { (double)geometry.top() + statusBarHeight,
        (double)geometry.right(),
        (double)geometry.bottom(),
        (double)geometry.left() };
    env.floor = { (double)geometry.bottom() - taskbarHeight,
        (double)geometry.left(), (double)geometry.right() };
    env.work_area = { (double)geometry.top(),
        (double)geometry.right(),
        (double)geometry.bottom() - taskbarHeight,
        (double)geometry.left() };
    env.ceiling = { (double)geometry.top(), (double)geometry.left(),
        (double)geometry.right() };
    manager()->applyActiveIE(env);
    int x = cursor.x(), y = cursor.y();
    env.cursor = { (double)x, (double)y, x - env.cursor.x, y - env.cursor.y };
    env.subtick_count = manager()->subtickCount();
    //m_previousWindow = m_currentWindow;

    env.set_scale(1.0 / std::sqrt(manager()->userScale()));
}

QMap<QScreen *, std::shared_ptr<shijima::mascot::environment>> const&
    MascotBackendWidgets::env()
{
    return m_env;
}

QMap<shijima::mascot::environment *, QScreen *> const&
    MascotBackendWidgets::reverseEnv()
{
    return m_reverseEnv;
}
