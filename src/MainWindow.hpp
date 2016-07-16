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

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QNetworkAccessManager>
#include <QSettings>

#include "ui_MainWindow.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void getData();

private slots:
    void returnPressed();

    void netFinished(QNetworkReply *reply);

private:
    void parseData(const QByteArray &json, const QString &errorStr);

    Ui::MainWindow ui;

    QString m_userName, m_repoName;
    QNetworkAccessManager m_net;
    QSettings m_settings;
    bool m_returnPressed;
};

#endif // MAINWINDOW_HPP
