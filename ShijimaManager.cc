#include "ShijimaManager.hpp"
#include <exception>
#include <iostream>
#include <QVBoxLayout>
#include <QWidget>
#include <QCloseEvent>
#include <QPushButton>
#include <QWindow>
#include <QTextStream>
#include <QGuiApplication>
#include <QFile>
#include <QScreen>
#include <QRandomGenerator>
#include "ShijimaWidget.hpp"
#include <QDirIterator>
#include <shijima/mascot/factory.hpp>
#include <shimejifinder/analyze.hpp>
#include <QStandardPaths>
#include "ForcedProgressDialog.hpp"
#include <QtConcurrent>
#include <QMessageBox>
#include <string>

using namespace shijima;

// https://stackoverflow.com/questions/34135624/-/54029758#54029758
static void dispatchToMainThread(std::function<void()> callback) {
    QTimer *timer = new QTimer;
    timer->moveToThread(qApp->thread());
    timer->setSingleShot(true);
    QObject::connect(timer, &QTimer::timeout, [timer, callback]() {
        callback();
        timer->deleteLater();
    });
    QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection, Q_ARG(int, 0));
}

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

void ShijimaManager::killAll(QString const& name) {
    std::string stdName = name.toStdString();
    for (auto mascot : m_mascots) {
        if (mascot->mascotName() == stdName) {
            mascot->markForDeletion();
        }
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

void ShijimaManager::killAllButOne(QString const& name) {
    bool foundOne = false;
    std::string stdName = name.toStdString();
    for (auto mascot : m_mascots) {
        if (mascot->mascotName() == stdName) {
            if (!foundOne) {
                foundOne = true;
                continue;
            }
            mascot->markForDeletion();
        }
    }
}

void ShijimaManager::reloadMascot(QString const& name) {
    MascotData data;
    try {
        data = { m_mascotsPath + QDir::separator() + name + ".mascot" };
    }
    catch (std::exception &ex) {
        std::cerr << "couldn't load mascot: " << name.toStdString() << std::endl;
        std::cerr << ex.what() << std::endl;
    }
    if (m_loadedMascots.contains(name)) {
        m_factory.deregister_template(name.toStdString());
        m_loadedMascots[name].unloadCache();
        killAll(name);
        m_loadedMascots.remove(name);
        std::cout << "Unloaded mascot: " << name.toStdString() << std::endl;
    }
    if (data.valid()) {
        shijima::mascot::factory::tmpl tmpl;
        tmpl.actions_xml = data.actionsXML().toStdString();
        tmpl.behaviors_xml = data.behaviorsXML().toStdString();
        tmpl.name = name.toStdString();
        tmpl.path = data.path().toStdString();
        m_factory.register_template(tmpl);
        m_loadedMascots.insert(name, data);
        std::cout << "Loaded mascot: " << name.toStdString() << std::endl;
    }
}

void ShijimaManager::loadAllMascots() {
    QDirIterator iter { m_mascotsPath, QDir::Dirs | QDir::NoDotAndDotDot,
        QDirIterator::NoIteratorFlags };
    while (iter.hasNext()) {
        auto name = iter.nextFileInfo().fileName();
        if (!name.endsWith(".mascot") || name.length() <= 7) {
            continue;
        }
        reloadMascot(name.sliced(0, name.length() - 7));
    }
}

void ShijimaManager::reloadMascots(std::set<std::string> const& mascots) {
    for (auto &mascot : mascots) {
        reloadMascot(QString::fromStdString(mascot));
    }
}

std::set<std::string> ShijimaManager::import(QString const& path) noexcept {
    try {
        auto ar = shimejifinder::analyze(path.toStdString());
        ar->extract(m_mascotsPath.toStdString());
        return ar->shimejis();
    }
    catch (std::exception &ex) {
        std::cerr << "import failed: " << ex.what() << std::endl;
        return {};
    }
}

void ShijimaManager::importWithDialog(QString const& path) {
    ForcedProgressDialog *dialog = new ForcedProgressDialog { this };
    dialog->setRange(0, 0);
    QPushButton *cancelButton = new QPushButton;
    cancelButton->setEnabled(false);
    cancelButton->setText("Cancel");
    dialog->setModal(true);
    dialog->setCancelButton(cancelButton);
    dialog->setLabelText("Importing shimeji...");
    dialog->show();
    //hide();
    QtConcurrent::run([this, path](){
        return import(path);
    }).then([this, dialog](std::set<std::string> changed){
        dispatchToMainThread([this, changed, dialog](){
            reloadMascots(changed);
            this->show();
            dialog->close();
            QString msg;
            QMessageBox::Icon icon;
            if (changed.size() > 0) {
                msg = QString::fromStdString("Imported " + std::to_string(changed.size()) +
                    " mascot" + (changed.size() == 1 ? "" : "s") + ".");
                icon = QMessageBox::Icon::Information;
            }
            else {
                msg = "Could not import any mascots from the specified archive.";
                icon = QMessageBox::Icon::Warning;
            }
            QMessageBox msgBox { icon, "Import", msg,
                QMessageBox::StandardButton::Ok, this };
            msgBox.exec();
        });
    });
}

void ShijimaManager::showEvent(QShowEvent *event) {
    if (!m_importOnShowPath.isEmpty()) {
        QString path = m_importOnShowPath;
        m_importOnShowPath = {};
        importWithDialog(path);
    }
}

void ShijimaManager::importOnShow(QString const& path) {
    m_importOnShowPath = path;
}

ShijimaManager::ShijimaManager(QWidget *parent): QMainWindow(parent) {
    QVBoxLayout *layout = new QVBoxLayout;
    QPushButton *spawnButton = new QPushButton("Spawn");

    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QString mascotsPath = QDir::cleanPath(dataPath + QDir::separator() + "mascots");
    QDir mascotsDir(mascotsPath);
    if (!mascotsDir.exists()) {
        mascotsDir.mkpath(mascotsPath);
    }
    m_mascotsPath = mascotsPath;
    std::cout << "Mascots path: " << m_mascotsPath.toStdString() << std::endl;
    
    loadAllMascots();

    m_env = m_factory.env = std::make_shared<mascot::environment>();
    connect(spawnButton, &QPushButton::clicked, this, &ShijimaManager::spawnClicked);
    layout->addWidget(spawnButton);
    QWidget *widget = new QWidget;
    widget->setLayout(layout);
    setCentralWidget(widget);
    m_mascotTimer = startTimer(10);
    if (m_windowObserver.tickFrequency() > 0) {
        m_windowObserverTimer = startTimer(m_windowObserver.tickFrequency());
    }
    setWindowFlags((windowFlags() | Qt::CustomizeWindowHint | Qt::MaximizeUsingFullscreenGeometryHint |
        Qt::WindowMinimizeButtonHint) & ~Qt::WindowMaximizeButtonHint);
    setManagerVisible(true);
}

void ShijimaManager::closeEvent(QCloseEvent *event) {
    if (!m_allowClose) {
        event->ignore();
        askClose();
        return;
    }
    event->accept();
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
    m_env->subtick_count = 4;
    m_previousWindow = m_currentWindow;

    m_env->set_scale(1.0);
}

void ShijimaManager::askClose() {
    setManagerVisible(true);
    QMessageBox msgBox { this };
    msgBox.setWindowTitle("Close Shijima-Qt");
    msgBox.setIcon(QMessageBox::Icon::Question);
    msgBox.setStandardButtons(QMessageBox::StandardButton::Close |
        QMessageBox::StandardButton::Cancel);
    msgBox.setText("Do you want to close Shijima-Qt?");
    int ret = msgBox.exec();
    if (ret == QMessageBox::Button::Close) {
        m_allowClose = true;
        close();
    }
}

std::string ShijimaManager::imgRootForTemplatePath(std::string const& path) {
    return QDir::cleanPath(QString::fromStdString(path)
        + QDir::separator() + "img").toStdString();
}

void ShijimaManager::setManagerVisible(bool visible) {
    auto screen = QGuiApplication::primaryScreen();
    auto geometry = screen->geometry();
    if (visible) {
        setFixedSize(400, 300);
        move(geometry.width() / 2 - 200, geometry.height() / 2 - 150);
        m_wasVisible = true;
    }
    else if (m_mascots.size() == 0) {
        askClose();
    }
    else {
        setFixedSize(1, 1);
        move(geometry.width() * 10, geometry.height() * 10);
        clearFocus();
        m_wasVisible = false;
    }
}

void ShijimaManager::tick() {
    if (isMinimized()) {
        setWindowState((windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
        setManagerVisible(!m_wasVisible);
    }
    else if (isMaximized()) {
        setManagerVisible(true);
    }

    if (m_mascots.size() == 0) {
        return;
    }

    updateEnvironment();

    for (int i=m_mascots.size()-1; i>=0; --i) {
        ShijimaWidget *shimeji = m_mascots[i];
        if (!shimeji->isVisible()) {
            delete shimeji;
            m_mascots.erase(m_mascots.begin() + i);
            continue;
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
                imgRootForTemplatePath(product.tmpl->path),
                std::move(product.manager), this);
            shimeji->show();
            m_mascots.push_back(shimeji);
            breedRequest.available = false;
        }
    }
    
    m_env->reset_scale();
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
        imgRootForTemplatePath(product.tmpl->path),
        std::move(product.manager), this);
    shimeji->show();
    m_mascots.push_back(shimeji);
    m_env->reset_scale();
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