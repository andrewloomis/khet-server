#include "usermanager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

UserManager::UserManager()
{

}

bool UserManager::authUser(const QString &username, const QString &hash)
{
    QJsonObject json;
    if (getJSONFromVault(json) && !json.isEmpty() && json.contains(username))
    {
        auto user = json.value(username).toObject();
        if (user["hash"] == hash)
        {
            return true;
        }
    }
    return false;
}

QString UserManager::getSaltFromUser(const QString &username)
{
    QJsonObject json;
    qDebug() << json.contains(username);
    if (getJSONFromVault(json) && !json.isEmpty() && json.contains(username))
    {
        auto user = json.value(username).toObject();
        return user["salt"].toString();
    }
    return "";
}

void UserManager::playerNotOnline(const QString &username)
{
    onlinePlayers.removeOne(username);
}

QList<QString> UserManager::getOnlinePlayers(const QString &username)
{
    QList<QString> otherOnlinePlayers;
    otherOnlinePlayers = onlinePlayers;
    if (!onlinePlayers.contains(username))
    {
        onlinePlayers << username;
        return otherOnlinePlayers;
    }
    else {
        otherOnlinePlayers.removeOne(username);
    }
    return otherOnlinePlayers;
}

bool UserManager::addNewUser(const QString &username, const QString &salt, const QString &hash)
{
    if(userData.open(QFile::ReadWrite))
    {
        auto contents = userData.readAll();
        auto json = QJsonDocument::fromJson(contents).object();
        if (json.isEmpty())
        {
            QJsonObject users, user;
            user["salt"] = salt;
            user["hash"] = hash;
            users[username] = user;
            json["users"] = users;
            userData.write(QJsonDocument(json).toJson());
            userData.close();
            return true;
        }
        else
        {
            QFile backup{".vault.backup"};
            if (backup.open(QFile::ReadWrite))
            {
                backup.resize(0);
                backup.write(contents);
                backup.close();
            }
            else {
                qDebug() << "cannot open backup file";
            }

            auto users = json["users"].toObject();
            if (users.contains(username))
            {
                userData.close();
                return false;
            }
            else {
                QJsonObject user;
                user["salt"] = salt;
                user["hash"] = hash;
                users[username] = user;
                json["users"] = users;
                userData.resize(0);
                userData.write(QJsonDocument(json).toJson());
                userData.close();
                return true;
            }
        }
    }
    else {
        qDebug() << "Unable to open user data file";
        return false;
    }
}

bool UserManager::getJSONFromVault(QJsonObject& json)
{
    if(userData.open(QFile::ReadOnly))
    {
        json = QJsonDocument::fromJson(userData.readAll()).object();
        json = json["users"].toObject();
        userData.close();
        return true;
    }
    else {
        qDebug() << "cannot open json vault file";
    }
    userData.close();
    return false;
}
