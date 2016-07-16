QT += core gui widgets network

CONFIG += c++11

TARGET = GitHubDownloadCount
DESTDIR = ..

TEMPLATE = app

SOURCES += main.cpp MainWindow.cpp
HEADERS +=          MainWindow.hpp
FORMS   +=          MainWindow.ui
