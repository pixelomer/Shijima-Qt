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

#include "ShijimaHttpApi.hpp"
#include <httplib.h>
#include "ShijimaManager.hpp"
#include <thread>
#include <iostream>
#include <QJsonArray>
#include <QJsonDocument>
#include <QBuffer>
#include <QJsonObject>
#include <QPixmap>

using namespace httplib;

static QJsonObject vecToObject(shijima::math::vec2 vec) {
    QJsonObject obj;
    obj["x"] = vec.x;
    obj["y"] = vec.y;
    return obj;
}

static QJsonObject mascotToObject(ShijimaWidget *widget) {
    QJsonObject obj;
    obj["id"] = widget->mascotId();
    obj["data_id"] = widget->mascotData()->id();
    obj["name"] = widget->mascotData()->name();
    obj["anchor"] = vecToObject(widget->mascot().state->anchor);
    auto activeBehavior = widget->mascot().active_behavior();
    if (activeBehavior != nullptr) {
        obj["active_behavior"] = QString::fromStdString(activeBehavior->name);
    }
    else {
        obj["active_behavior"] = QJsonValue {};
    }
    return obj;
}

static QJsonObject mascotDataToObject(MascotData *data) {
    QJsonObject obj;
    obj["id"] = data->id();
    obj["name"] = data->name();
    return obj;
}

static shijima::math::vec2 valueToVec(QJsonValue const& value) {
    shijima::math::vec2 vec { NAN, NAN };
    if (value.isObject()) {
        auto object = value.toObject();
        auto xValue = object.take("x");
        auto yValue = object.take("y");
        if (xValue.isDouble() && yValue.isDouble()) {
            vec.x = xValue.toDouble();
            vec.y = yValue.toDouble();
        }
    }
    return vec;
}

static void applyObjectToWidget(QJsonObject &object, ShijimaWidget *widget) {
    if (auto anchor = valueToVec(object.take("anchor"));
        !std::isnan(anchor.x))
    {
        widget->mascot().state->anchor = anchor;
    }
    if (auto value = object.take("behavior"); value.isString()) {
        auto str = value.toString().toStdString();
        auto behavior = widget->mascot()
            .initial_behavior_list().find(str, false);
        if (behavior != nullptr) {
            widget->mascot().next_behavior(str);
        }
    }
}

static std::optional<QJsonObject> jsonForRequest(Request const& req) {
    if (req.get_header_value("content-type") != "application/json") {
        return {};
    }
    QByteArray bytes { req.body.c_str(), (qsizetype)req.body.size() };
    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(bytes, &error);
    if (error.error != QJsonParseError::NoError) {
        std::cerr << "JSON parse error: " << error.errorString().toStdString()
            << std::endl;
        return {};
    }
    else if (!doc.isObject()) {
        // request bodies must contain objects
        return {};
    }
    else {
        return doc.object();
    }
}

static void sendJson(Response &res, QJsonObject const& object) {
    QJsonDocument doc { object };
    auto bytes = doc.toJson(QJsonDocument::Compact);
    res.set_content(&bytes[0], bytes.size(), "application/json");
}

static void badRequest(Request const&, Response &res) {
    QJsonObject obj;
    obj["error"] = "400 Bad Request";
    res.status = 400;
    sendJson(res, obj);
}

static bool selectorEval(ShijimaWidget *mascot, std::string const& selector) {
    if (selector.empty()) {
        return true;
    }
    bool eval;
    try {
        mascot->mascot().script_ctx->state = mascot->mascot().state;
        eval = mascot->mascot().script_ctx->eval_bool(selector);
    }
    catch (std::exception &ex) {
        std::cerr << "selector eval failed: " << ex.what() << std::endl;
        eval = false;
    }
    return eval;
}

ShijimaHttpApi::ShijimaHttpApi(ShijimaManager *manager): m_server(new Server),
    m_thread(nullptr), m_manager(manager), m_host(""), m_port(-1)
{
    m_server->Get("/shijima/api/v1/mascots",
        [this](Request const& req, Response &res)
    {
        QJsonArray array;
        std::string selector;
        if (req.has_param("selector")) {
            selector = req.get_param_value("selector");
        }
        m_manager->onTickSync([&array, &selector](ShijimaManager *manager){
            auto &mascots = manager->mascots();
            for (auto mascot : mascots) {
                if (!selectorEval(mascot, selector)) {
                    continue;
                }
                array.append(mascotToObject(mascot));
            }
        });
        QJsonObject object;
        object["mascots"] = array;
        sendJson(res, object);
    });
    m_server->Post("/shijima/api/v1/mascots",
        [this](Request const& req, Response &res)
    {
        auto json = jsonForRequest(req);
        if (!json.has_value()) {
            badRequest(req, res);
            return;
        }
        auto nameValue = json->take("name");
        auto dataIdValue = json->take("data_id");
        int dataId = -1;
        if (!nameValue.isUndefined() && !dataIdValue.isUndefined()) {
            badRequest(req, res);
            return;
        }
        if (dataIdValue.isDouble()) {
            dataId = dataIdValue.toInt();
        }
        QJsonObject object;
        m_manager->onTickSync([&dataId, &nameValue, &res, &object, &json]
            (ShijimaManager *manager)
        {
            QString mascotName;
            if (dataId == -1) {
                auto name = nameValue.toString();
                if (manager->loadedMascots().contains(name)) {
                    mascotName = name;
                }
            }
            else {
                if (manager->loadedMascotsById().contains(dataId)) {
                    mascotName = manager->loadedMascotsById()[dataId]->name();
                }
            }
            if (mascotName.isEmpty()) {
                res.status = 400;
                object["error"] = "Invalid mascot name or data ID";
            }
            else {
                auto widget = manager->spawn(mascotName.toStdString());
                applyObjectToWidget(*json, widget);
                object["mascot"] = mascotToObject(widget);
            }
        });
        sendJson(res, object);
    });
    m_server->Put("/shijima/api/v1/mascots/([0-9]+)",
        [this](Request const& req, Response &res)
    {
        auto json = jsonForRequest(req);
        if (!json.has_value()) {
            badRequest(req, res);
            return;
        }
        auto id = std::stoi(req.matches[1].str());
        QJsonObject object;
        m_manager->onTickSync([&json, &object, &res, id]
            (ShijimaManager *manager)
        {
            if (manager->mascotsById().count(id) == 1) {
                auto widget = manager->mascotsById().at(id);
                applyObjectToWidget(*json, widget);
                object["mascot"] = mascotToObject(widget);
            }
            else {
                res.status = 404;
                object["error"] = "No such mascot";
            }
        });
        sendJson(res, object);
    });
    m_server->Get("/shijima/api/v1/mascots/([0-9]+)",
        [this](Request const& req, Response &res)
    {
        auto id = std::stoi(req.matches[1].str());
        QJsonObject object;
        m_manager->onTickSync([&object, &res, id](ShijimaManager *manager){
            if (manager->mascotsById().count(id) == 1) {
                object["mascot"] = mascotToObject(manager->mascotsById().at(id));
            }
            else {
                res.status = 404;
                object["mascot"] = QJsonValue {};
            }
        });
        sendJson(res, object);
    });
    m_server->Delete("/shijima/api/v1/mascots/([0-9]+)",
        [this](Request const& req, Response &res)
    {
        auto id = std::stoi(req.matches[1].str());
        QJsonObject object;
        m_manager->onTickSync([&object, &res, id](ShijimaManager *manager){
            if (manager->mascotsById().count(id) == 1) {
                auto mascot = manager->mascotsById().at(id);
                mascot->markForDeletion();
            }
            else {
                res.status = 404;
                object["error"] = "404 Not Found";
            }
        });
        sendJson(res, object);
    });
    m_server->Delete("/shijima/api/v1/mascots",
        [this](Request const& req, Response &res)
    {
        auto json = jsonForRequest(req);
        std::string selector;
        if (json.has_value() && json->contains("selector")) {
            auto value = json->take("selector");
            if (value.isString()) {
                selector = value.toString().toStdString();
            }
        }
        m_manager->onTickSync([&selector](ShijimaManager *manager){
            auto &mascots = manager->mascots();
            for (auto mascot : mascots) {
                if (!selectorEval(mascot, selector)) {
                    continue;
                }
                mascot->markForDeletion();
            }
        });
        sendJson(res, {});
    });
    m_server->Get("/shijima/api/v1/loadedMascots",
        [this](Request const&, Response &res)
    {
        QJsonArray array;
        m_manager->onTickSync([&array](ShijimaManager *manager){
            auto &mascots = manager->loadedMascots();
            for (auto mascot : mascots) {
                array.append(mascotDataToObject(mascot));
            }
        });
        QJsonObject object;
        object["loaded_mascots"] = array;
        sendJson(res, object);
    });
    m_server->Get("/shijima/api/v1/ping",
        [](Request const&, Response &res)
    {
        sendJson(res, {});
    });
    m_server->Get("/shijima/api/v1/loadedMascots/([0-9]+)",
        [this](Request const& req, Response &res)
    {
        auto id = std::stoi(req.matches[1].str());
        QJsonObject object;
        m_manager->onTickSync([&object, &res, id](ShijimaManager *manager){
            if (manager->loadedMascotsById().contains(id)) {
                object["loaded_mascot"] = mascotDataToObject(manager->loadedMascotsById()[id]);
            }
            else {
                res.status = 404;
                object["loaded_mascot"] = QJsonValue {};
            }
        });
        sendJson(res, object);
    });
    m_server->Get("/shijima/api/v1/loadedMascots/([0-9]+)/preview.png",
        [this](Request const& req, Response &res)
    {
        auto id = std::stoi(req.matches[1].str());
        m_manager->onTickSync([&res, id](ShijimaManager *manager){
            if (manager->loadedMascotsById().contains(id)) {
                auto data = manager->loadedMascotsById()[id];
                auto &preview = data->preview();
                auto pixmap = preview.pixmap(preview.availableSizes()[0]);
                QByteArray bytes {};
                QBuffer buf { &bytes };
                buf.open(QBuffer::WriteOnly);
                pixmap.save(&buf, "PNG");
                buf.close();
                res.set_content(&bytes[0], bytes.size(), "image/png");
            }
            else {
                res.status = 404;
                res.set_content("404 Not Found", "text/plain");
            }
        });
    });
    m_server->Get(".*", badRequest);
    m_server->Put(".*", badRequest);
    m_server->Post(".*", badRequest);
    m_server->Delete(".*", badRequest);
    m_server->Patch(".*", badRequest);
    m_server->set_logger([](const Request &req, const Response &) {
        std::cout << req.method << " " << req.path;
        for (auto it = req.params.begin(); it != req.params.end(); ++it) {
            if (it == req.params.begin()) {
                std::cout << "?";
            }
            else {
                std::cout << "&";
            }
            std::cout << it->first << "=" << it->second;
        }
        std::cout << std::endl;
    });
}

void ShijimaHttpApi::start(std::string const& host, int port) {
    stop();
    m_host = host;
    m_port = port;
    m_thread = new std::thread { [this, host, port](){
        m_server->listen(host, port);
    } };
}

bool ShijimaHttpApi::running() {
    return m_server->is_running();
}

int ShijimaHttpApi::port() {
    return m_port;
}

std::string const& ShijimaHttpApi::host() {
    return m_host;
}

void ShijimaHttpApi::stop() {
    if (m_server->is_running()) {
        m_server->stop();
    }
    if (m_thread != nullptr) {
        m_thread->join();
        delete m_thread;
        m_thread = nullptr;
    }
}

ShijimaHttpApi::~ShijimaHttpApi() {
    stop();
    delete m_server;
}
