#include "MascotBackendWindowed.hpp"
#include "ShijimaManager.hpp"
#include "WindowedShimeji.hpp"
#include <QWidget>
#include <QMouseEvent>

MascotBackendWindowed::MascotBackendWindowed(ShijimaManager *manager):
    QObject(), MascotBackend(manager)
{
    m_env = std::make_shared<shijima::mascot::environment>();
    m_sandbox = new QWidget { manager, Qt::Window };
    m_sandbox->installEventFilter(this);
    m_sandbox->setAttribute(Qt::WA_StyledBackground, true);
    m_sandbox->resize(640, 480);
    m_sandbox->setObjectName("sandboxWindow");
    m_sandbox->show();
}

MascotBackendWindowed::~MascotBackendWindowed() {
    delete m_sandbox;
}

ActiveMascot *MascotBackendWindowed::spawn(MascotData *mascotData,
    std::unique_ptr<shijima::mascot::manager> mascot,
    ActiveMascot *parent, int mascotId, bool resetPosition)
{
    mascot->state->env = m_env;
    if (resetPosition) {
        preTick();
        mascot->reset_position();
        postTick();
    }
    auto shimeji = new WindowedShimeji { this, mascotData,
        std::move(mascot), mascotId, m_sandbox };
    shimeji->installEventFilter(this);
    return shimeji;
}

ActiveMascot *MascotBackendWindowed::migrate(ActiveMascot &old) {
    old.mascot().state->env = m_env;
    preTick();
    old.mascot().reset_position();
    postTick();
    auto shimeji = new WindowedShimeji { this,
        old, m_sandbox };
    shimeji->installEventFilter(this);
    return shimeji;
}

void MascotBackendWindowed::preTick() {
    auto &env = *m_env;
    env.screen = { 0, (double)m_sandbox->width(),
        (double)m_sandbox->height(), 0 };
    env.work_area = env.screen;
    env.floor = { env.screen.bottom, env.screen.left, env.screen.right };
    env.ceiling = { env.screen.top, env.screen.left, env.screen.right };
    auto manager = this->manager();
    env.subtick_count = manager->subtickCount();
    env.set_scale(1.0 / std::sqrt(manager->userScale()));
}

void MascotBackendWindowed::postTick() {
    m_env->reset_scale();
    m_env->cursor.dx = m_env->cursor.dy = 0;
}

void MascotBackendWindowed::updateEnvironments(
    std::function<void(shijima::mascot::environment &)> cb)
{
    cb(*m_env);
}

bool MascotBackendWindowed::eventFilter(QObject *object, QEvent *event) {
    QWidget *widget = (QWidget *)object;
    if (event->type() == QEvent::MouseMove || event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseMove = (QMouseEvent *)event;
        auto pos = mouseMove->pos();
        if (widget != m_sandbox) {
            pos = widget->mapToParent(pos);
        }
        m_env->cursor.move({ (double)pos.x(), (double)pos.y() });
    }
    return false;
}
