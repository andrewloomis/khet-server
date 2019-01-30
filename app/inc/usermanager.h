#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QFile>

class UserManager
{
public:
    UserManager();
    bool addNewUser(const QString& username, const QString& salt, const QString& hash);
    bool authUser(const QString& username, const QString& hash);
    QString getSaltFromUser(const QString& username);
    QList<QString> getOnlinePlayers(const QString& username);
    void playerNotOnline(const QString& username);
    void addGameData(const QString& username, bool winner);
    QJsonObject getRankingsData();

private:
    QFile userData{".vault"};
    QList<QString> onlinePlayers;

    bool getJSONFromVault(QJsonObject& json);

};

#endif // USERMANAGER_H
