#include <shijima/mascot/factory.hpp>
#include <QString>
#include <QDir>
#include <optional>

class MascotFinder {
private:
    static void findXMLs(QDir conf, std::optional<QString> &behaviors,
        std::optional<QString> &actions, bool &isJapanese);
    static void findXMLs(QDir conf, std::optional<QString> &behaviors,
        std::optional<QString> &actions);
public:
    MascotFinder() {}
    int findAll(shijima::mascot::factory &factory);
};