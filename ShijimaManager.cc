#include "ShijimaManager.hpp"
#include <cmath>
#include <exception>
#include <filesystem>
#include <iostream>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>
#include <QCloseEvent>
#include <QMenuBar>
#include <QFileDialog>
#include <QPushButton>
#include <QWindow>
#include <QTextStream>
#include <QGuiApplication>
#include <QFile>
#include <QDesktopServices>
#include <QScreen>
#include <QRandomGenerator>
#include "PlatformWidget.hpp"
#include "ShijimaLicensesDialog.hpp"
#include "ShijimaWidget.hpp"
#include <QDirIterator>
#include <shijima/mascot/factory.hpp>
#include <shimejifinder/analyze.hpp>
#include <QStandardPaths>
#include "ForcedProgressDialog.hpp"
#include <QAbstractItemModel>
#include <QAction>
#include <QCoreApplication>
#include <QCursor>
#include <QDesktopServices>
#include <QFileDialog>
#include <QItemSelectionModel>
#include <QKeySequence>
#include <QListWidget>
#include <QMessageBox>
#include <QUrl>
#include <QtConcurrent>
#include <string>
#include <QLabel>
#include <QFormLayout>

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
    for (auto mascot : m_mascots) {
        if (mascot->mascotName() == name) {
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
    for (auto mascot : m_mascots) {
        if (mascot->mascotName() == name) {
            if (!foundOne) {
                foundOne = true;
                continue;
            }
            mascot->markForDeletion();
        }
    }
}

void ShijimaManager::loadData(MascotData *data) {
    if (data != nullptr && data->valid()) {
        shijima::mascot::factory::tmpl tmpl;
        tmpl.actions_xml = data->actionsXML().toStdString();
        tmpl.behaviors_xml = data->behaviorsXML().toStdString();
        tmpl.name = data->name().toStdString();
        tmpl.path = data->path().toStdString();
        m_factory.register_template(tmpl);
        m_loadedMascots.insert(data->name(), data);
        m_loadedMascotsById.insert(data->id(), data);
        std::cout << "Loaded mascot: " << data->name().toStdString() << std::endl;
    }
    else {
        throw std::runtime_error("loadData() called with invalid data");
    }
}

void ShijimaManager::loadDefaultMascot() {
    auto data = new MascotData { "@", m_idCounter++ };
    loadData(data);
}

QMap<QString, MascotData *> const& ShijimaManager::loadedMascots() {
    return m_loadedMascots;
}

QMap<int, MascotData *> const& ShijimaManager::loadedMascotsById() {
    return m_loadedMascotsById;
}

std::vector<ShijimaWidget *> const& ShijimaManager::mascots() {
    return m_mascots;
}

std::map<int, ShijimaWidget *> const& ShijimaManager::mascotsById() {
    return m_mascotsById;
}


void ShijimaManager::reloadMascot(QString const& name) {
    if (m_loadedMascots.contains(name) && !m_loadedMascots[name]->deletable()) {
        std::cout << "Refusing to unload mascot: " << name.toStdString()
            << std::endl;
        return;
    }
    MascotData *data = nullptr;
    try {
        data = new MascotData { m_mascotsPath + QDir::separator() + name + ".mascot",
            m_idCounter++ };
    }
    catch (std::exception &ex) {
        std::cerr << "couldn't load mascot: " << name.toStdString() << std::endl;
        std::cerr << ex.what() << std::endl;
    }
    if (m_loadedMascots.contains(name)) {
        MascotData *data = m_loadedMascots[name];
        m_factory.deregister_template(name.toStdString());
        data->unloadCache();
        killAll(name);
        m_loadedMascots.remove(name);
        m_loadedMascotsById.remove(data->id());
        delete data;
        std::cout << "Unloaded mascot: " << name.toStdString() << std::endl;
    }
    if (data != nullptr) {
        if (data->name() != name) {
            throw std::runtime_error("Impossible condition: New mascot name is incorrect");
        }
        loadData(data);
    }
    m_listItemsToRefresh.insert(name);
}

void ShijimaManager::importAction() {
    auto paths = QFileDialog::getOpenFileNames(this, "Choose shimeji archive...");
    if (paths.isEmpty()) {
        return;
    }
    importWithDialog(paths);
}

void ShijimaManager::quitAction() {
    m_allowClose = true;
    close();
}

void ShijimaManager::deleteAction() {
    if (m_loadedMascots.size() == 0) {
        return;
    }
    auto selected = m_listWidget.selectedItems();
    for (long i=(long)selected.size()-1; i>=0; --i) {
        auto mascotData = m_loadedMascots[selected[i]->text()];
        if (!mascotData->deletable()) {
            selected.remove(i);
        }
    }
    if (selected.size() == 0) {
        return;
    }
    QString msg = "Are you sure you want to delete these shimeji?";
    for (long i=0; i<selected.size() && i<5; ++i) {
        msg += "\n* " + selected[i]->text();
    }
    if (selected.size() > 5) {
        msg += "\n... and " + QString::number(selected.size() - 5) + " other(s)";
    }
    QMessageBox msgBox { this };
    msgBox.setWindowTitle("Delete shimeji");
    msgBox.setText(msg);
    msgBox.setStandardButtons(QMessageBox::StandardButton::Yes |
        QMessageBox::StandardButton::No);
    msgBox.setIcon(QMessageBox::Icon::Question);
    int ret = msgBox.exec();
    if (ret == QMessageBox::StandardButton::Yes) {
        for (auto item : selected) {
            auto mascotData = m_loadedMascots[item->text()];
            if (!mascotData->deletable()) {
                continue;
            }
            std::filesystem::path path = mascotData->path().toStdString();
            std::cout << "Deleting mascot: " << item->text().toStdString() << std::endl;
            try {
                // remove_all(path) could be dangerous
                std::filesystem::remove_all(path / "img");
                std::filesystem::remove_all(path / "sound");
                std::filesystem::remove(path / "actions.xml");
                std::filesystem::remove(path / "behaviors.xml");
                std::filesystem::remove(path);
            }
            catch (std::exception &ex) {
                std::cerr << "failed to delete mascot: " << path.string()
                    << ": " << ex.what() << std::endl;
            }
            reloadMascot(item->text());
        }
        refreshListWidget();
    }
}

std::unique_lock<std::mutex> ShijimaManager::acquireLock() {
    return std::unique_lock<std::mutex> { m_mutex };
}

void ShijimaManager::buildToolbar() {
    QAction *action;
    QMenu *menu;
    QMenu *submenu;
    
    menu = menuBar()->addMenu("File");
    {
        action = menu->addAction("Import shimeji...");
        connect(action, &QAction::triggered, this, &ShijimaManager::importAction);

        action = menu->addAction("Quit");
        connect(action, &QAction::triggered, this, &ShijimaManager::quitAction);
    }

    menu = menuBar()->addMenu("Edit");
    {
        action = menu->addAction("Delete shimeji", QKeySequence::StandardKey::Delete);
        connect(action, &QAction::triggered, this, &ShijimaManager::deleteAction);
    }

    menu = menuBar()->addMenu("Settings");
    {
        {
            static const QString key = "multiplicationEnabled";
            bool initial = m_settings.value(key, 
                QVariant::fromValue(true)).toBool();

            action = menu->addAction("Enable multiplication");
            action->setCheckable(true);
            action->setChecked(initial);
            for (auto &env : m_env) {
                env->allows_breeding = initial;
            }
            connect(action, &QAction::triggered, [this](bool checked){
                for (auto &env : m_env) {
                    env->allows_breeding = checked;
                }
                m_settings.setValue(key, QVariant::fromValue(checked));
            });
        }

        submenu = menu->addMenu("Scale");
        {
            static const QString key = "userScale";
            m_userScale = m_settings.value(key,
                QVariant::fromValue(1.0)).toDouble();
            
            auto makeScaleText = [](double scale){
                return QString::asprintf("%.3lfx", scale);
            };

            auto makeCustomActionText = [this, makeScaleText]() {
                return QString { "Custom... (" } +
                    makeScaleText(m_userScale) + ")";
            };
            QAction *customAction = submenu->addAction(makeCustomActionText());

            #define addPreset(scale) do { \
                action = submenu->addAction(#scale "x"); \
                action->setCheckable(true); \
                action->setChecked(std::fabs(m_userScale - scale) < 0.01); \
                connect(action, &QAction::triggered, [this, customAction, \
                    makeCustomActionText, action, submenu]() \
                { \
                    for (auto neighbour : submenu->actions()) { \
                        neighbour->setChecked(false); \
                    } \
                    m_userScale = scale; \
                    m_settings.setValue(key, QVariant::fromValue(scale)); \
                    action->setChecked(true); \
                    customAction->setText(makeCustomActionText()); \
                }); \
            } while (0)
            
            addPreset(0.25);
            addPreset(0.50);
            addPreset(0.75);
            addPreset(1.00);
            addPreset(1.25);
            addPreset(1.50);
            addPreset(1.75);
            addPreset(2.00);

            #undef addPreset

            connect(customAction, &QAction::triggered, [this,
                customAction, makeCustomActionText, submenu, makeScaleText]()
            {
                QDialog dialog { this };
                QFormLayout layout;
                dialog.setLayout(&layout);
                QSlider slider { Qt::Horizontal };
                QLabel label;
                QPushButton button;
                button.setText("Save");
                label.setMinimumWidth(80);
                slider.setMinimumWidth(300);
                layout.addRow(&label, &slider);
                layout.addRow(&button);
                label.setText(makeScaleText(m_userScale));
                slider.setMinimum(100);
                slider.setMaximum(10000);
                slider.setValue(static_cast<int>(m_userScale * 1000.0));
                connect(&slider, &QSlider::valueChanged,
                    [this, &label, makeScaleText](int value)
                {
                    m_userScale = value / 1000.0;
                    label.setText(makeScaleText(m_userScale));
                });
                connect(&button, &QPushButton::clicked,
                    [&dialog]()
                {
                    dialog.close();
                });
                dialog.exec();
                for (auto neighbour : submenu->actions()) {
                    //double value = neighbour->text().sliced(0, 4).toDouble();
                    //std::cout << std::fabs(m_userScale - value) << std::endl;
                    //neighbour->setChecked(std::fabs(m_userScale - value) < 0.01);
                    neighbour->setChecked(false);
                }
                customAction->setText(makeCustomActionText());
                m_settings.setValue(key, QVariant::fromValue(m_userScale));
            });
        }
    }

    menu = menuBar()->addMenu("Help");
    {
        action = menu->addAction("View Licenses");
        connect(action, &QAction::triggered, [this](){
            ShijimaLicensesDialog dialog { this };
            dialog.exec();
        });

        action = menu->addAction("Visit Shijima Homepage");
        connect(action, &QAction::triggered, [](){
            QDesktopServices::openUrl(QUrl { "https://getshijima.app" });
        });

        action = menu->addAction("Report Issue");
        connect(action, &QAction::triggered, [](){
            QDesktopServices::openUrl(QUrl { "https://github.com/pixelomer/Shijima-Qt-releases/issues" });
        });
    }
}

void ShijimaManager::refreshListWidget() {
    //FIXME: refresh only changed items
    m_listWidget.clear();
    auto names = m_loadedMascots.keys();
    names.sort(Qt::CaseInsensitive);
    for (auto &name : names) {
        auto item = new QListWidgetItem;
        item->setText(name);
        item->setIcon(m_loadedMascots[name]->preview());
        m_listWidget.addItem(item);
    }
    m_listItemsToRefresh.clear();
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
    refreshListWidget();
}

void ShijimaManager::reloadMascots(std::set<std::string> const& mascots) {
    for (auto &mascot : mascots) {
        reloadMascot(QString::fromStdString(mascot));
    }
    refreshListWidget();
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

void ShijimaManager::importWithDialog(QList<QString> const& paths) {
    ForcedProgressDialog *dialog = new ForcedProgressDialog { this };
    dialog->setRange(0, 0);
    QPushButton *cancelButton = new QPushButton;
    cancelButton->setEnabled(false);
    cancelButton->setText("Cancel");
    dialog->setModal(true);
    dialog->setCancelButton(cancelButton);
    dialog->setLabelText("Importing shimeji...");
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
    //hide();
    QtConcurrent::run([this, paths](){
        std::set<std::string> changed;
        for (auto &path : paths) {
            auto newChanged = import(path);
            changed.insert(newChanged.begin(), newChanged.end());
        }
        return changed;
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
                msg = "Could not import any mascots from the specified archive(s).";
                icon = QMessageBox::Icon::Warning;
            }
            QMessageBox msgBox { icon, "Import", msg,
                QMessageBox::StandardButton::Ok, this };
            msgBox.exec();
        });
    });
}

void ShijimaManager::showEvent(QShowEvent *event) {
    PlatformWidget::showEvent(event);
    if (!m_firstShow) {
        return;
    }
    m_firstShow = false;
    if (!m_importOnShowPath.isEmpty()) {
        QString path = m_importOnShowPath;
        m_importOnShowPath = {};
        importWithDialog({ path });
    }
    else {
        QMessageBox msgBox { this };
        msgBox.setText("This is an early alpha version of Shijima-Qt. Please "
            "report any issues you encounter by pressing Help > Report Issue. "
            "Your feedback is highly appreciated.");
        msgBox.addButton(QMessageBox::StandardButton::Ok);
        msgBox.exec();

        if (m_loadedMascots.size() == 1) {
            auto msgBox = new QMessageBox { this };
            msgBox->setText("Welcome to Shijima! Get started by dragging and dropping a "
                "shimeji archive to the manager window. You can also import archives "
                "by selecting File > Import.");
            msgBox->addButton(QMessageBox::StandardButton::Ok);
            msgBox->setAttribute(Qt::WA_DeleteOnClose);
            msgBox->show();
        }
    }
}

void ShijimaManager::importOnShow(QString const& path) {
    m_importOnShowPath = path;
}

void ShijimaManager::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void ShijimaManager::dropEvent(QDropEvent *event) {
    QList<QString> paths;
    for (auto &url : event->mimeData()->urls()) {
        paths.append(url.toLocalFile());
    }
    importWithDialog(paths);
}

void ShijimaManager::screenAdded(QScreen *screen) {
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

void ShijimaManager::screenRemoved(QScreen *screen) {
    if (m_env.contains(screen)) {
        auto primary = QGuiApplication::primaryScreen();
        for (auto &mascot : m_mascots) {
            mascot->setEnv(m_env[primary]);
            mascot->mascot().reset_position();
        }
        m_reverseEnv.remove(m_env[primary].get());
        m_env.remove(screen);
    }
}

ShijimaManager::~ShijimaManager() {
    disconnect(qApp, &QGuiApplication::screenAdded,
        this, &ShijimaManager::screenAdded);
    disconnect(qApp, &QGuiApplication::screenRemoved,
        this, &ShijimaManager::screenRemoved);
}

void ShijimaManager::onTickSync(std::function<void(ShijimaManager *)> callback) {
    auto lock = acquireLock();
    m_hasTickCallbacks = true;
    m_tickCallbacks.push_back(callback);
    m_tickCallbackCompletion.wait(lock);
}

ShijimaManager::ShijimaManager(QWidget *parent):
    PlatformWidget(parent, PlatformWidget::ShowOnAllDesktops),
    m_settings("pixelomer", "Shijima-Qt"),
    m_idCounter(0), m_httpApi(this),
    m_hasTickCallbacks(false)
{
    for (auto screen : QGuiApplication::screens()) {
        screenAdded(screen);
    }

    connect(qApp, &QGuiApplication::screenAdded,
        this, &ShijimaManager::screenAdded);
    connect(qApp, &QGuiApplication::screenRemoved,
        this, &ShijimaManager::screenRemoved);

    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QString mascotsPath = QDir::cleanPath(dataPath + QDir::separator() + "mascots");
    QDir mascotsDir(mascotsPath);
    if (!mascotsDir.exists()) {
        mascotsDir.mkpath(mascotsPath);
    }
    m_mascotsPath = mascotsPath;
    std::cout << "Mascots path: " << m_mascotsPath.toStdString() << std::endl;
    
    loadDefaultMascot();
    loadAllMascots();
    setAcceptDrops(true);

    m_mascotTimer = startTimer(10);
    if (m_windowObserver.tickFrequency() > 0) {
        m_windowObserverTimer = startTimer(m_windowObserver.tickFrequency());
    }
    setWindowFlags((windowFlags() | Qt::CustomizeWindowHint | Qt::MaximizeUsingFullscreenGeometryHint |
        Qt::WindowMinimizeButtonHint) & ~Qt::WindowMaximizeButtonHint);
    setManagerVisible(true);

    connect(&m_listWidget, &QListWidget::itemDoubleClicked,
        this, &ShijimaManager::itemDoubleClicked);
    m_listWidget.setIconSize({ 64, 64 });
    m_listWidget.installEventFilter(this);
    m_listWidget.setSelectionMode(QListWidget::ExtendedSelection);
    setCentralWidget(&m_listWidget);
    buildToolbar();

    m_httpApi.start("127.0.0.1", 32456);
}

void ShijimaManager::itemDoubleClicked(QListWidgetItem *qItem) {
    spawn(qItem->text().toStdString());
}

void ShijimaManager::closeEvent(QCloseEvent *event) {
    #if !defined(__APPLE__)
    if (!m_allowClose) {
        event->ignore();
        #if defined(_WIN32)
        if (m_mascots.size() == 0) {
            askClose();
        }
        else {
            setManagerVisible(false);
        }
        #else
        askClose();
        #endif
        return;
    }
    event->accept();
    #else
    event->ignore();
    setManagerVisible(false);
    #endif
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

void ShijimaManager::updateEnvironment(QScreen *screen) {
    if (!m_env.contains(screen)) {
        return;
    }
    auto &env = m_env[screen];
    auto cursor = QCursor::pos();
    auto geometry = screen->geometry();
    auto available = screen->availableGeometry();
    int taskbarHeight = available.bottom() - geometry.bottom();
    int statusBarHeight = geometry.top() - available.top();
    if (taskbarHeight < 0) {
        taskbarHeight = 0;
    }
    if (statusBarHeight < 0) {
        statusBarHeight = 0;
    }
    env->screen = { (double)geometry.top() + statusBarHeight,
        (double)geometry.right(),
        (double)geometry.bottom(),
        (double)geometry.left() };
    env->floor = { (double)geometry.bottom() - taskbarHeight,
        (double)geometry.left(), (double)geometry.right() };
    env->work_area = { (double)geometry.top(),
        (double)geometry.right(),
        (double)geometry.bottom() - taskbarHeight,
        (double)geometry.left() };
    env->ceiling = { (double)geometry.top(), (double)geometry.left(),
        (double)geometry.right() };
    if (m_currentWindow.available && std::fabs(m_currentWindow.x) > 1
        && std::fabs(m_currentWindow.y) > 1)
    {
        env->active_ie = { m_currentWindow.y,
            m_currentWindow.x + m_currentWindow.width,
            m_currentWindow.y + m_currentWindow.height,
            m_currentWindow.x };
        if (m_previousWindow.available &&
            m_previousWindow.uid == m_currentWindow.uid)
        {
            env->active_ie.dy = m_currentWindow.y - m_previousWindow.y;
            if (env->active_ie.dy == 0) {
                env->active_ie.dy = m_currentWindow.height - m_previousWindow.height;
            }
            env->active_ie.dx = m_currentWindow.x - m_previousWindow.x;
            if (env->active_ie.dx == 0) {
                env->active_ie.dx = m_currentWindow.width - m_previousWindow.width;
            }
        }
    }
    else {
        env->active_ie = { -50, -50, -50, -50 };
    }
    int x = cursor.x(), y = cursor.y();
    env->cursor = { (double)x, (double)y, x - env->cursor.x, y - env->cursor.y };
    env->subtick_count = 4;
    m_previousWindow = m_currentWindow;

    env->set_scale(1.0 / std::sqrt(m_userScale));
}

void ShijimaManager::updateEnvironment() {
    m_currentWindow = m_windowObserver.getActiveWindow();
    for (auto screen : QGuiApplication::screens()) {
        updateEnvironment(screen);
    }
}

void ShijimaManager::askClose() {
    setManagerVisible(true);
    QMessageBox msgBox { this };
    msgBox.setWindowTitle("Close Shijima-Qt");
    msgBox.setIcon(QMessageBox::Icon::Question);
    msgBox.setStandardButtons(QMessageBox::StandardButton::Yes |
        QMessageBox::StandardButton::No);
    msgBox.setText("Do you want to close Shijima-Qt?");
    int ret = msgBox.exec();
    if (ret == QMessageBox::Button::Yes) {
        #if defined(__APPLE__)
        QCoreApplication::quit();
        #else
        m_allowClose = true;
        close();
        #endif
    }
}

void ShijimaManager::setManagerVisible(bool visible) {
    #if !defined(__APPLE__)
    auto screen = QGuiApplication::primaryScreen();
    auto geometry = screen->geometry();
    if (!m_wasVisible && visible) {
        if (window() != nullptr) {
            window()->activateWindow();
        }
        setMinimumSize(480, 320);
        setMaximumSize(999999, 999999);
        move(geometry.width() / 2 - 240, geometry.height() / 2 - 160);
        m_wasVisible = true;
    }
    else if (m_wasVisible && !visible) {
        setFixedSize(1, 1);
        move(geometry.width() * 10, geometry.height() * 10);
        clearFocus();
        if (window() != nullptr) {
            window()->activateWindow();
        }
        m_wasVisible = false;
    }
    #else
    if (visible) {
        show();
        m_wasVisible = true;
    }
    else if (m_mascots.size() == 0) {
        askClose();
    }
    else {
        hide();
        m_wasVisible = false;
    }
    #endif
}

void ShijimaManager::tick() {
    if (m_hasTickCallbacks) {
        auto lock = acquireLock();
        for (auto &callback : m_tickCallbacks) {
            callback(this);
        }
        m_tickCallbacks.clear();
        m_hasTickCallbacks = false;
        m_tickCallbackCompletion.notify_all();
    }

    #if !defined(__APPLE__)
    if (isMinimized()) {
        setWindowState(windowState() & ~Qt::WindowMinimized);
        setManagerVisible(!m_wasVisible);
    }
    else if (isMaximized()) {
        setManagerVisible(true);
    }
    #endif

    if (m_mascots.size() == 0) {
        #if !defined(__APPLE__)
        if (isMinimized() || !m_wasVisible) {
            setWindowState(windowState() & ~Qt::WindowMinimized);
            setManagerVisible(true);
        }
        #endif
        return;
    }

    updateEnvironment();

    for (int i=m_mascots.size()-1; i>=0; --i) {
        ShijimaWidget *shimeji = m_mascots[i];
        if (!shimeji->isVisible()) {
            int mascotId = shimeji->mascotId();
            delete shimeji;
            m_mascots.erase(m_mascots.begin() + i);
            m_mascotsById.erase(mascotId);
            continue;
        }
        shimeji->tick();
        auto &mascot = shimeji->mascot();
        auto &breedRequest = mascot.state->breed_request;
        if (mascot.state->dragging) {
            auto oldScreen = m_reverseEnv[mascot.state->env.get()];
            auto newScreen = QGuiApplication::screenAt(QPoint {
                (int)mascot.state->anchor.x, (int)mascot.state->anchor.y });
            if (newScreen != nullptr && oldScreen != newScreen) {
                mascot.state->env = m_env[newScreen];
            }
        }
        if (breedRequest.available) {
            if (breedRequest.name == "") {
                breedRequest.name = shimeji->mascotName().toStdString();
            }
            // only consider the last path component
            breedRequest.name = breedRequest.name.substr(breedRequest.name.rfind('\\')+1);
            breedRequest.name = breedRequest.name.substr(breedRequest.name.rfind('/')+1);
            std::optional<shijima::mascot::factory::product> product;
            try {
                product = m_factory.spawn(breedRequest);
            }
            catch (std::exception &ex) {
                std::cerr << "couldn't fulfill breed request for "
                    << breedRequest.name << std::endl;
                std::cerr << ex.what() << std::endl;
            }
            if (product.has_value()) {
                ShijimaWidget *child = new ShijimaWidget(
                    m_loadedMascots[QString::fromStdString(breedRequest.name)],
                    std::move(product->manager), m_idCounter++, this);
                child->setEnv(shimeji->env());
                child->show();
                m_mascots.push_back(child);
                m_mascotsById[child->mascotId()] = child;
            }
            breedRequest.available = false;
        }
    }
    
    for (auto &env : m_env) {
        env->reset_scale();
    }

    if (m_mascots.size() == 0) {
        // All mascots self-destructed, show manager
        setManagerVisible(true);
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

ShijimaWidget *ShijimaManager::spawn(std::string const& name) {
    QScreen *screen = this->screen();
    updateEnvironment(screen);
    auto &env = m_env[screen];
    auto product = m_factory.spawn(name, {});
    product.manager->state->env = env;
    product.manager->reset_position();
    ShijimaWidget *shimeji = new ShijimaWidget(
        m_loadedMascots[QString::fromStdString(name)],
        std::move(product.manager), m_idCounter++, this);
    shimeji->show();
    m_mascots.push_back(shimeji);
    m_mascotsById[shimeji->mascotId()] = shimeji;
    env->reset_scale();
    return shimeji;
}

bool ShijimaManager::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        auto key = keyEvent->key();
        if (key == Qt::Key::Key_Return || key == Qt::Key::Key_Enter) {
            for (auto item : m_listWidget.selectedItems()) {
                itemDoubleClicked(item);
            }
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
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