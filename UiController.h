/*
 *   Copyright (C) 2026 by Joshua Murphy VO1RFX
 *   Based on the mvoice project by Thomas A. Early N7TAE
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 *   MVoice QML UI controller
 *
 *   Exposes a thin, QML-friendly interface over CAppCore.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>

#include "AppCore.h"

class UiController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString targetCallsign READ targetCallsign WRITE setTargetCallsign NOTIFY targetCallsignChanged)
    Q_PROPERTY(int selectedModuleIndex READ selectedModuleIndex WRITE setSelectedModuleIndex NOTIFY selectedModuleIndexChanged)
    Q_PROPERTY(QString targetIp READ targetIp WRITE setTargetIp NOTIFY targetIpChanged)
    Q_PROPERTY(QString targetPort READ targetPort WRITE setTargetPort NOTIFY targetPortChanged)
    Q_PROPERTY(QString destinationCallsign READ destinationCallsign WRITE setDestinationCallsign NOTIFY destinationCallsignChanged)
    Q_PROPERTY(bool isLegacy READ isLegacy WRITE setIsLegacy NOTIFY isLegacyChanged)
    Q_PROPERTY(bool canConnect READ canConnect NOTIFY stateChanged)
    Q_PROPERTY(bool canDisconnect READ canDisconnect NOTIFY stateChanged)
    Q_PROPERTY(bool canPTT READ canPTT NOTIFY stateChanged)
    Q_PROPERTY(bool canQuickKey READ canQuickKey NOTIFY stateChanged)
    Q_PROPERTY(bool pttQuickKeyEnabled READ pttQuickKeyEnabled NOTIFY pttQuickKeyEnabledChanged)
    Q_PROPERTY(bool canOpenDashboard READ canOpenDashboard NOTIFY stateChanged)
    Q_PROPERTY(bool targetInputsEnabled READ targetInputsEnabled NOTIFY stateChanged)
    Q_PROPERTY(bool destinationEditable READ destinationEditable NOTIFY stateChanged)
    Q_PROPERTY(int linkState READ linkState NOTIFY stateChanged)
    Q_PROPERTY(QVariant logLines READ logLines NOTIFY logLinesChanged)
    Q_PROPERTY(QStringList targetItems READ targetItems NOTIFY targetItemsChanged)

public:
    explicit UiController(CAppCore *core, QObject *parent = nullptr);

    QString statusText() const;

public slots:
    void refreshStatus();

    void setTargetCallsign(const QString &cs);
    void setSelectedModuleIndex(int index);
    void setTargetIp(const QString &ip);
    void setTargetPort(const QString &port);
    void setDestinationCallsign(const QString &cs);
    void setIsLegacy(bool legacy);

    void link();
    void unlink();
    void pttToggle();
    void echoTest();
    void quickKey();
    void sendSms(const QString &dst, const QString &message);
    void openDashboard();

signals:
    void statusTextChanged();
    void targetCallsignChanged();
    void selectedModuleIndexChanged();
    void targetIpChanged();
    void targetPortChanged();
    void destinationCallsignChanged();
    void isLegacyChanged();
    void stateChanged();
    void pttQuickKeyEnabledChanged();
    void logLinesChanged();
    void targetItemsChanged();

private slots:
    void pollLogQueue();
    void pollLinkState();

private:
    CAppCore *m_core;
    QString m_targetCallsign;
    int m_selectedModuleIndex{0};
    QString m_targetIp;
    QString m_targetPort;
    QString m_destinationCallsign;
    bool m_isLegacy{false};
    bool m_pttActive{false};
    bool m_pttQuickKeyEnabled{false};
    int m_lastLinkState{-1};
    QStringList m_logLines;
    QStringList m_targetItems;

    QString targetCallsign() const { return m_targetCallsign; }
    int selectedModuleIndex() const { return m_selectedModuleIndex; }
    QString targetIp() const { return m_targetIp; }
    QString targetPort() const { return m_targetPort; }
    QString destinationCallsign() const { return m_destinationCallsign; }
    bool isLegacy() const { return m_isLegacy; }

    bool canConnect() const;
    bool canDisconnect() const;
    bool canPTT() const;
    bool canQuickKey() const;
    bool pttQuickKeyEnabled() const { return m_pttQuickKeyEnabled; }
    bool canOpenDashboard() const;
    int linkState() const;
    bool targetInputsEnabled() const;
    bool destinationEditable() const;
    QVariant logLines() const { return m_logLines; }
    QStringList targetItems() const { return m_targetItems; }

public:
    Q_INVOKABLE void refreshTargets();
    Q_INVOKABLE void chooseTarget(const QString &cs);
};

