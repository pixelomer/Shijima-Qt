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

#include "cli.hpp"
#include <QString>
#include <QVariant>
#include <QMap>
#include <httplib.h>
#include <QJsonObject>
#include <QByteArray>
#include <QJsonArray>
#include <fstream>
#include <QJsonDocument>
#include <QRandomGenerator>

#ifdef _WIN32
#include <windows.h>
#include <sstream>
#else
#include <iostream>
#endif

static std::ostream *coutPt;
static std::ostream *cerrPt;

#define cout (*coutPt)
#define cerr (*cerrPt)

class Argument {
private:
    QString m_name;
    QString m_description;
    QMetaType::Type m_type;
    QVariant *m_variant;
    bool m_fulfilled;
public:
    QString const& name() const { return m_name; }
    QMetaType::Type type() const { return m_type; }
    QString const& description() const { return m_description; }
    bool fulfilled() const { return m_fulfilled; }
    Argument() {}
    Argument(QString const& name, QString const& description, QVariant *variant,
        QMetaType::Type type, bool required): m_name(name),
        m_description(description), m_type(type), m_variant(variant),
        m_fulfilled(!required) {}
    bool parse(int argc, char **argv, int &i) {
        if (m_type == QMetaType::Bool) {
            m_variant->setValue(true);
            return m_fulfilled = true;
        }
        if (i == (argc - 1)) {
            return m_fulfilled = false;
        }
        QString value = argv[++i];
        switch (m_type) {
            case QMetaType::QString:
                m_variant->setValue(value);
                m_fulfilled = true;
                break;
            case QMetaType::QStringList:
                if (m_variant->isNull()) {
                    m_variant->setValue(QStringList {});
                }
                static_cast<QStringList *>(m_variant->data())->append(value);
                m_fulfilled = true;
                break;
            case QMetaType::Int:
                m_variant->setValue(value.toInt(&m_fulfilled));
                break;
            case QMetaType::Double:
                m_variant->setValue(value.toDouble(&m_fulfilled));
                break;
            default:
                m_fulfilled = false;
                break;
        }
        return m_fulfilled;
    }
};

class ArgumentList {
private:
    QMap<QString, Argument> m_arguments;
    QList<QString> m_orderedArguments;
public:
    ArgumentList(std::initializer_list<Argument> list) {
        for (auto &arg : list) {
            m_arguments[arg.name()] = arg;
            m_orderedArguments.append(arg.name());
        }
    }
    QMap<QString, Argument> const& arguments() {
        return m_arguments;
    }
    bool parse(int argc, char **argv) {
        for (int i=2; i<argc; ++i) {
            QString fullName = argv[i];
            if (!fullName.startsWith("--")) {
                return false;
            }
            auto name = fullName.sliced(2);
            if (!m_arguments.contains(name)) {
                return false;
            }
            auto &argument = m_arguments[name];
            if (!argument.parse(argc, argv, i)) {
                return false;
            }
        }
        for (auto &argument : m_arguments) {
            if (!argument.fulfilled()) {
                return false;
            }
        }
        return true;
    }
    void printUsage(int, char **argv) {
        cerr << "Usage: " << argv[0] << " " << argv[1] << " [options...]"
            << std::endl;
        for (auto &argName : m_orderedArguments) {
            auto &arg = m_arguments[argName];
            const char *typeName;
            switch (arg.type()) {
                case QMetaType::QString:
                case QMetaType::QStringList:
                    typeName = " (string)";
                    break;
                case QMetaType::Int:
                    typeName = " (int)";
                    break;
                case QMetaType::Double:
                    typeName = " (double)";
                    break;
                default:
                    typeName = "";
                    break;
            }
            cerr << "  --" << (arg.name() + typeName).leftJustified(20)
                .toStdString() << " " << arg.description().toStdString()
                << std::endl;
        }
    }
};

static bool parseOptions(int argc, char **argv, ArgumentList &&arguments) {
    bool success = arguments.parse(argc, argv);
    if (!success) {
        arguments.printUsage(argc, argv);
    }
    return success;
}

static bool parseOptions(int argc, char **argv, ArgumentList &arguments) {
    bool success = arguments.parse(argc, argv);
    if (!success) {
        arguments.printUsage(argc, argv);
    }
    return success;
}

static bool parseAPIResult(httplib::Result const& res, QJsonObject &object) {
    QByteArray bytes { &res->body[0], (qsizetype)res->body.size() };
    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(bytes, &error);
    if (error.error != QJsonParseError::NoError) {
        cerr << "ERROR: Failed to parse response: " <<
            error.errorString().toStdString() << std::endl;
        return false;
    }
    else if (!doc.isObject()) {
        cerr << "ERROR: Response JSON does not contain an object" << std::endl;
        return false;
    }
    object = doc.object();
    if (object.contains("error")) {
        auto value = object["error"];
        cerr << "ERROR: API request failed: ";
        if (value.isString()) {
            cerr << value.toString().toStdString();
        }
        else {
            cerr << "Unknown error";
        }
        cerr << std::endl;
        return false;
    }
    return true;
}

static bool parseAPIResult(httplib::Result const& res) {
    QJsonObject obj;
    return parseAPIResult(res, obj);
}

static int notRunning() {
    cerr << "Request failed. Is Shijima-Qt running?" << std::endl;
    return EXIT_FAILURE;
}

static bool parseShimejiAttributes(QJsonObject &object, QVariant const& behaviors,
    QVariant const& x, QVariant const& y)
{
    if (x.isNull() != y.isNull()) {
        cerr << "ERROR: X and Y must be specified together" << std::endl;
        return false;
    }
    else if (!x.isNull()) {
        QJsonObject anchor;
        anchor["x"] = x.toDouble();
        anchor["y"] = y.toDouble();
        object["anchor"] = anchor;
    }
    if (!behaviors.isNull()) {
        auto &list = *static_cast<const QStringList *>(behaviors.data());
        object["behavior"] = list[QRandomGenerator::global()->bounded(0,
            list.size())];
    }
    return true;
}

static void printMascot(QJsonObject const& object) {
    cout << "[" << object["id"].toInt() << "] " <<
        object["name"].toString().toStdString() << std::endl;
    cout << "  Data ID: " << object["data_id"].toInt() << std::endl;
    cout << "  Active behavior: " <<
        object["active_behavior"].toString().toStdString() << std::endl;
    cout << "  Anchor: {" <<
        object["anchor"].toObject()["x"].toDouble() << ", " <<
        object["anchor"].toObject()["y"].toDouble() << "}" << std::endl;
}

static bool getMascotIDAtIndex(QJsonArray const& array, QVariant &variant,
    qsizetype index)
{
    if (array.size() <= index) {
        return false;
    }
    auto value = array[index];
    if (!value.isObject()) {
        return false;
    }
    value = value["id"];
    if (!value.isDouble()) {
        return false;
    }
    variant = (int)value.toDouble();
    return true;
}

static int getMascotID(httplib::Client &client, QVariant &idVariant,
    QVariant const& selectorsVariant)
{
    if (idVariant.typeId() == QMetaType::Int) {
        // already an ID
        return EXIT_SUCCESS;
    }
    else if (idVariant.typeId() != QMetaType::QString) {
        // this function expects a string
        throw std::runtime_error("idVariant must be a QString");
    }

    QStringList selectors;
    if (selectorsVariant.typeId() == QMetaType::QStringList) {
        selectors = selectorsVariant.toStringList();
    }
    else if (selectorsVariant.typeId() == QMetaType::QString) {
        selectors = { selectorsVariant.toString() };
    }
    else {
        selectors = { "" };
    }

    bool ok;
    if (int convertedId = idVariant.toInt(&ok); ok) {
        idVariant = convertedId;
        if (selectors[0] != "") {
            cerr << "ERROR: You can't specify a numeric ID and a selector at" <<
                " the same time" << std::endl;
            return EXIT_FAILURE;
        }
        if (convertedId >= 0) {
            return EXIT_SUCCESS;
        }
        else {
            cerr << "ERROR: ID must be greater than or equal to 0" << std::endl;
            return EXIT_FAILURE;
        }
    }

    auto str = idVariant.toString();
    static const QMap<QString, std::function<bool(QJsonArray const&,
        QVariant &)>> parsers =
    {
        { "oldest", [](QJsonArray const& array, QVariant &variant){
            return getMascotIDAtIndex(array, variant, 0);
        } },
        { "newest", [](QJsonArray const& array, QVariant &variant){
            return getMascotIDAtIndex(array, variant, array.size()-1);
        } },
        { "random", [](QJsonArray const& array, QVariant &variant){
            return getMascotIDAtIndex(array, variant, 
                QRandomGenerator::global()->bounded(0, array.size()));
        } },
    };
    if (!parsers.contains(str)) {
        cerr << "Invalid auto ID, expected: " << std::endl;
        auto keys = parsers.keys();
        cerr << keys.join(", ").toStdString() << std::endl;
        return EXIT_FAILURE;
    }
    for (auto &selector : selectors) {
        httplib::Params params;
        if (!selector.isEmpty()) {
            params.insert({ "selector", selector.toStdString() });
        }
        if (auto res = client.Get("/shijima/api/v1/mascots", params,
            {}))
        {
            QJsonObject object;
            if (!parseAPIResult(res, object)) {
                return EXIT_FAILURE;
            }
            auto mascots = object["mascots"];
            if (!mascots.isArray()) {
                cerr << "ERROR: Malformed response" << std::endl;
                return EXIT_FAILURE;
            }
            auto array = mascots.toArray();
            if (!array.empty() && parsers[str](array, idVariant) &&
                idVariant.typeId() == QMetaType::Int)
            {
                return EXIT_SUCCESS;
            }
        }
        else {
            return notRunning();
        }
    }
    cerr << "ERROR: Failed to determine ID (are any mascots spawned?)"
        << std::endl;
    return EXIT_FAILURE;
}

static int cliMain(int argc, char **argv) {
    std::string action = argv[1];
    httplib::Client client { "http://127.0.0.1:32456" };
    if (action == "list") {
        QVariant selector, json { false };
        if (!parseOptions(argc, argv, {
            { "json", "Print the API response as JSON", &json, QMetaType::Bool, false },
            { "selector", "JavaScript code for filtering shimeji", &selector, QMetaType::QString, false }
        })) {
            return EXIT_FAILURE;
        }
        httplib::Params params;
        if (selector.typeId() == QMetaType::QString) {
            params.insert({ "selector", selector.toString().toStdString() });
        }
        if (auto res = client.Get("/shijima/api/v1/mascots",
            params, {}))
        {
            QJsonObject object;
            if (!parseAPIResult(res, object)) {
                if (json.toBool() && object.contains("error")) {
                    cout << res->body << std::endl;
                }
                return EXIT_FAILURE;
            }
            if (json.toBool()) {
                cout << res->body << std::endl;
            }
            else {
                auto mascotsValue = object["mascots"];
                if (!mascotsValue.isArray()) {
                    cerr << "ERROR: Malformed response" << std::endl;
                    return EXIT_FAILURE;
                }
                auto mascots = mascotsValue.toArray();
                for (auto mascot : mascots) {
                    if (!mascot.isObject()) {
                        continue;
                    }
                    auto object = mascot.toObject();
                    printMascot(object);
                }
            }
            return EXIT_SUCCESS;
        }
        else {
            return notRunning();
        }
    }
    else if (action == "list-loaded") {
        QVariant json { false }, sortById { false };
        if (!parseOptions(argc, argv, {
            { "json", "Print the API response as JSON", &json, QMetaType::Bool, false },
            { "sort-by-id", "Sort results by ID", &sortById, QMetaType::Bool, false }
        })) {
            return EXIT_FAILURE;
        }
        if (json.toBool() && sortById.toBool()) {
            cerr << "ERROR: --json and --sort-by-id cannot be used together." <<
                std::endl;
            return EXIT_FAILURE;
        }
        if (auto res = client.Get("/shijima/api/v1/loadedMascots")) {
            QJsonObject object;
            if (!parseAPIResult(res, object)) {
                if (json.toBool() && object.contains("error")) {
                    cout << res->body << std::endl;
                }
                return EXIT_FAILURE;
            }
            if (json.toBool()) {
                cout << res->body << std::endl;
            }
            else {
                auto loadedValue = object["loaded_mascots"];
                if (!loadedValue.isArray()) {
                    cerr << "ERROR: Malformed response" << std::endl;
                    return EXIT_FAILURE;
                }
                QJsonArray loaded = loadedValue.toArray();
                std::vector<std::pair<int, std::string>> sorted;
                for (auto mascotValue : loaded) {
                    if (!mascotValue.isObject()) {
                        continue;
                    }
                    auto mascot = mascotValue.toObject();
                    sorted.push_back({ mascot["id"].toInt(), 
                        mascot["name"].toString().toStdString() });
                }
                if (sortById.toBool()) {
                    std::sort(sorted.begin(), sorted.end(),
                        [](auto &a, auto &b)
                    {
                        return a.first < b.first;
                    });
                }
                for (auto &item : sorted) {
                    cout << "[" << item.first << "] "
                        << item.second << std::endl;
                }
            }
            return EXIT_SUCCESS;
        }
        else {
            return notRunning();
        }
    }
    else if (action == "spawn") {
        QVariant name, dataId, behaviors, x, y, printJson { false };
        ArgumentList args = {
            { "data-id", "Data ID of the shimeji to spawn", &dataId, QMetaType::Int, false },
            { "name", "Name of the shimeji to spawn", &name, QMetaType::QString, false },
            { "behavior", "Initial behavior for the shimeji", &behaviors, QMetaType::QStringList, false },
            { "x", "Initial X position for the shimeji", &x, QMetaType::Double, false },
            { "y", "Initial Y position for the shimeji", &y, QMetaType::Double, false },
            { "json", "Print the API response as JSON", &printJson, QMetaType::Bool, false }
        };
        if (!parseOptions(argc, argv, args)) {
            return EXIT_FAILURE;
        }
        QJsonObject object;
        if (dataId.isNull() == name.isNull()) {
            cerr << "ERROR: You must specify one of name or data-id." << std::endl;
            args.printUsage(argc, argv);
            return EXIT_FAILURE;
        }
        if (!dataId.isNull()) {
            object["data_id"] = dataId.toInt();
        }
        else {
            object["name"] = name.toString();
        }
        if (!parseShimejiAttributes(object, behaviors, x, y)) {
            return EXIT_FAILURE;
        }
        QJsonDocument doc { object };
        auto json = doc.toJson(QJsonDocument::Compact);
        if (auto res = client.Post("/shijima/api/v1/mascots",
            std::string { &json[0], (size_t)json.size() }, "application/json"))
        {
            QJsonObject object;
            int ret = EXIT_SUCCESS;
            if (parseAPIResult(res, object)) {
                if (!printJson.toBool()) {
                    auto mascot = object["mascot"];
                    if (!mascot.isObject()) {
                        cerr << "ERROR: Malformed response" << std::endl;
                        ret = EXIT_FAILURE;
                    }
                    else {
                        printMascot(mascot.toObject());
                    }
                }
            }
            else {
                ret = EXIT_FAILURE;
            }
            if (printJson.toBool()) {
                cout << res->body << std::endl;
            }
            return ret;
        }
        else {
            return notRunning();
        }
    }
    else if (action == "alter") {
        QVariant id, selectors, behaviors, x, y, printJson { false };
        if (!parseOptions(argc, argv, {
            { "id", "ID of the shimeji to alter", &id, QMetaType::QString, true },
            { "selector", "JavaScript code for filtering shimeji", &selectors, QMetaType::QStringList, false },
            { "behavior", "New behavior for the shimeji", &behaviors, QMetaType::QStringList, false },
            { "x", "New X position for the shimeji", &x, QMetaType::Double, false },
            { "y", "New Y position for the shimeji", &y, QMetaType::Double, false },
            { "json", "Print the API response as JSON", &printJson, QMetaType::Bool, false }
        })) {
            return EXIT_FAILURE;
        }
        if (int ret = getMascotID(client, id, selectors); ret != EXIT_SUCCESS) {
            return ret;
        }
        QJsonObject object;
        if (!parseShimejiAttributes(object, behaviors, x, y)) {
            return EXIT_FAILURE;
        }
        QJsonDocument doc { object };
        auto json = doc.toJson(QJsonDocument::Compact);
        if (auto res = client.Put("/shijima/api/v1/mascots/"
            + std::to_string(id.toInt()),
            std::string { &json[0], (size_t)json.size() }, "application/json"))
        {
            QJsonObject object;
            int ret = EXIT_SUCCESS;
            if (parseAPIResult(res, object)) {
                if (!printJson.toBool()) {
                    auto mascot = object["mascot"];
                    if (!mascot.isObject()) {
                        cerr << "ERROR: Malformed response" << std::endl;
                        ret = EXIT_FAILURE;
                    }
                    else {
                        printMascot(mascot.toObject());
                    }
                }
            }
            else {
                ret = EXIT_FAILURE;
            }
            if (printJson.toBool()) {
                cout << res->body << std::endl;
            }
            return ret;
        }
        else {
            return notRunning();
        }
    }
    else if (action == "dismiss") {
        QVariant id, selector;
        if (!parseOptions(argc, argv, {
            { "id", "ID of the shimeji to dismiss", &id, QMetaType::QString, true },
            { "selector", "JavaScript code for filtering shimeji", &selector, QMetaType::QString, false }
        })) {
            return EXIT_FAILURE;
        }
        if (int ret = getMascotID(client, id, selector); ret != EXIT_SUCCESS) {
            return ret;
        }
        if (auto res = client.Delete("/shijima/api/v1/mascots/"
            + std::to_string(id.toInt())))
        {
            if (parseAPIResult(res)) {
                return EXIT_SUCCESS;
            }
            else {
                return EXIT_FAILURE;
            }
        }
        else {
            return notRunning();
        }
    }
    else if (action == "dismiss-all") {
        QVariant selector;
        if (!parseOptions(argc, argv, {
            { "selector", "JavaScript code for filtering shimeji", &selector, QMetaType::QString, false }
        })) {
            return EXIT_FAILURE;
        }
        QJsonObject obj;
        if (selector.typeId() == QMetaType::QString) {
            obj["selector"] = selector.toString();
        }
        QJsonDocument doc { obj };
        auto bytes = doc.toJson();
        if (auto res = client.Delete("/shijima/api/v1/mascots",
            std::string { &bytes[0], (size_t)bytes.size() },
            "application/json" ))
        {
            if (parseAPIResult(res)) {
                return EXIT_SUCCESS;
            }
            else {
                return EXIT_FAILURE;
            }
        }
        else {
            return notRunning();
        }
    }
    else {
        cerr << "Usage: " << argv[0] << " [--quiet] <command> [options...]"
            << std::endl;
        cerr << "   Possible commands are: list, list-loaded, spawn, "
            "alter, dismiss, dismiss-all" << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

#undef cout
#undef cerr

int shijimaRunCli(int argc, char **argv) {
    #ifdef _WIN32
        const char *argv0 = argv[0];
    #endif
    bool quiet = false;
    if (argv[1] != NULL && argv[2] != NULL &&
        strcmp(argv[1], "--quiet") == 0)
    {
        quiet = true;
        argv[1] = argv[0];
        ++argv;
        --argc;
    }
    if (quiet) {
        auto stream = new std::ofstream;
        #ifdef _WIN32
            stream->open("NUL");
        #else
            stream->open("/dev/null");
        #endif
        coutPt = stream;
        cerrPt = stream;
    }
    else {
        #ifdef _WIN32
            auto stream = new std::stringstream;
            coutPt = stream;
            cerrPt = stream;
        #else
            coutPt = &std::cout;
            cerrPt = &std::cerr;
        #endif
    }
    int ret = cliMain(argc, argv);
    #ifdef _WIN32
        if (!quiet) {
            auto stream = static_cast<std::stringstream *>(coutPt);
            std::string output = stream->str();
            if (!output.empty()) {
                MessageBoxA(NULL, output.c_str(), argv0, MB_OK);
            }
        }
        delete coutPt;
    #else
        if (quiet) {
            delete coutPt;
        }
    #endif
    return ret;
}
