/*
    GitHubDownloadCount shows download count for repository using the GitHub API
    Copyright (C) 2016  Błażej Szczygieł

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "MainWindow.hpp"

#include <QJsonDocument>
#include <QNetworkReply>
#include <QJsonObject>
#include <QMessageBox>
#include <QJsonArray>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_settings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::applicationName()),
    m_returnPressed(false)
{
    ui.setupUi(this);

    ui.userName->setText(m_settings.value("UserName").toString());
    ui.repoName->setText(m_settings.value("RepoName").toString());

    ui.dwnInfo->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui.dwnInfo->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    QAction *act = new QAction(this);
    act->setShortcut(QKeySequence("F5"));
    connect(act, &QAction::triggered, this, &MainWindow::getData);
    addAction(act);

    connect(&m_net, &QNetworkAccessManager::finished, this, &MainWindow::netFinished);

    restoreGeometry(m_settings.value("Geometry").toByteArray());

    getData();
}
MainWindow::~MainWindow()
{
    m_settings.setValue("UserName", ui.userName->text());
    m_settings.setValue("RepoName", ui.repoName->text());
    m_settings.setValue("Geometry", saveGeometry());
}

void MainWindow::getData()
{
    const QString userName = ui.userName->text().simplified();
    const QString repoName = ui.repoName->text().simplified();

    QObject *senderObj = sender();

     // Force download on return key or on action (F5 shourcut only)
    const bool forceDownload = m_returnPressed || qobject_cast<QAction *>(senderObj);
    m_returnPressed = false;

    if (!userName.isEmpty() && !repoName.isEmpty() && (forceDownload || userName != m_userName || repoName != m_repoName))
    {
        QWidget *focused = focusWidget();
        if (senderObj && focused != senderObj && qobject_cast<QLineEdit *>(focused))
        {
            // Do nothing when changing focus between line edits
            return;
        }

        for (auto &&obj : m_net.children())
        {
            // Terminate current connections
            if (auto reply = qobject_cast<QNetworkReply *>(obj))
                reply->deleteLater();
        }

        QNetworkReply *reply = m_net.get(QNetworkRequest("https://api.github.com/repos/" + userName + "/" + repoName + "/releases"));
        reply->ignoreSslErrors();

        ui.statusBar->showMessage("Downloading data...");

        m_userName = userName;
        m_repoName = repoName;
    }
}

void MainWindow::returnPressed()
{
    m_returnPressed = true;
}

void MainWindow::netFinished(QNetworkReply *reply)
{
    const QString error = (reply->error() == QNetworkReply::NoError) ? QString() : reply->errorString();
    const QByteArray json = reply->readAll();
    reply->deleteLater();
    parseData(json, error);
}

void MainWindow::parseData(const QByteArray &json, const QString &errorStr)
{
    const QJsonDocument jsonDoc = QJsonDocument::fromJson(json);

    ui.dwnInfo->clear();

    if (!errorStr.isEmpty())
    {
        const QString message = jsonDoc.object()["message"].toString();
        ui.statusBar->showMessage("Error");
        QMessageBox::warning(this, "GitHub connection error", "<p><b>" + errorStr + "</b></p>" + (message.isEmpty() ? QString() : "<p>" + message + "</p>"));
        return;
    }

    int allDownloads = 0;

    for (auto &&releaseElement : jsonDoc.array())
    {
        const QJsonObject release = releaseElement.toObject();

        QTreeWidgetItem *item = new QTreeWidgetItem;
        int overallDownloadCount = 0;

        for (auto &&assetElement : release["assets"].toArray())
        {
            const QJsonObject asset = assetElement.toObject();
            const int downloadCount = asset["download_count"].toInt();
            const QStringList columns = {
                asset["name"].toString(),
                QString::number(downloadCount)
            };
            new QTreeWidgetItem(item, columns);
            overallDownloadCount += downloadCount;
        }

        const QString nameTag = release["tag_name"].toString();
        const QString name = release["name"].toString();

        item->setText(0, name + (nameTag.isEmpty() ? QString() : " (" + nameTag + ")"));
        if (item->childCount() > 0)
            item->setText(1, QString::number(overallDownloadCount));

        if (!item->text(0).isEmpty())
            ui.dwnInfo->addTopLevelItem(item);
        else
            delete item;

        allDownloads += overallDownloadCount;
    }

    if (QTreeWidgetItem *topItem = ui.dwnInfo->topLevelItem(0))
    {
        ui.dwnInfo->expandItem(topItem);
        ui.dwnInfo->setCurrentItem(topItem);
    }

    ui.statusBar->showMessage(QString("Items: %1, downloads: %2").arg(ui.dwnInfo->topLevelItemCount()).arg(allDownloads));
}
