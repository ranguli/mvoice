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

#include "UiController.h"

#include <QTimer>
#include <QDesktopServices>
#include <QUrl>

#include "Utilities.h"

UiController::UiController(CAppCore *core, QObject *parent)
    : QObject(parent),
      m_core(core)
{
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &UiController::pollLogQueue);
    timer->start(200);

    auto stateTimer = new QTimer(this);
    connect(stateTimer, &QTimer::timeout, this, &UiController::pollLinkState);
    stateTimer->start(500);

    m_lastLinkState = linkState();
    m_pttQuickKeyEnabled = canPTT();

    refreshTargets();
}

QString UiController::statusText() const
{
    if (!m_core) {
        return tr("Core not initialised");
    }

    if (m_core->IsConfigOkay()) {
        return tr("Configuration is OK");
    }

    return tr("Configuration is NOT OK");
}

void UiController::refreshStatus()
{
    emit statusTextChanged();
}

void UiController::setTargetCallsign(const QString &cs)
{
    if (m_targetCallsign == cs)
        return;
    m_targetCallsign = cs;
    emit targetCallsignChanged();
    emit stateChanged();
}

void UiController::setSelectedModuleIndex(int index)
{
    if (index < 0) index = 0;
    if (index > 25) index = 25;
    if (m_selectedModuleIndex == index)
        return;
    m_selectedModuleIndex = index;
    emit selectedModuleIndexChanged();
    emit stateChanged();
}

void UiController::setTargetIp(const QString &ip)
{
    if (m_targetIp == ip)
        return;
    m_targetIp = ip;
    emit targetIpChanged();
    emit stateChanged();
}

void UiController::setTargetPort(const QString &port)
{
    if (m_targetPort == port)
        return;
    m_targetPort = port;
    emit targetPortChanged();
    emit stateChanged();
}

void UiController::setDestinationCallsign(const QString &cs)
{
    if (m_destinationCallsign == cs)
        return;
    m_destinationCallsign = cs;
    emit destinationCallsignChanged();
    emit stateChanged();
}

void UiController::setIsLegacy(bool legacy)
{
    if (m_isLegacy == legacy)
        return;
    m_isLegacy = legacy;
    emit isLegacyChanged();
    emit stateChanged();
}

bool UiController::canConnect() const
{
    if (!m_core)
        return false;
    const auto &cfgdata = m_core->GetConfigData();
    if (cfgdata.sM17SourceCallsign.empty())
        return false;

    std::string target = m_targetCallsign.toStdString();
    bool bTargetCS = std::regex_match(target, m_core->ReflTarRegEx);

    std::string ipText = m_targetIp.toStdString();
    bool bIP4 = std::regex_match(ipText, m_core->IPv4RegEx);
    bool bIP6 = std::regex_match(ipText, m_core->IPv6RegEx);
    bool bTargetIP = false;
    switch (cfgdata.eNetType) {
    case EInternetType::ipv4only: bTargetIP = bIP4; break;
    case EInternetType::ipv6only: bTargetIP = bIP6; break;
    default: bTargetIP = (bIP4 || bIP6); break;
    }

    bool ok = false;
    int port = m_targetPort.toInt(&ok);
    bool bTargetPort = ok && (port > 1023 && port < 49000);

    return (m_core->GetLinkState() == ELinkState::unlinked) && bTargetCS && bTargetIP && bTargetPort;
}

bool UiController::canDisconnect() const
{
    if (!m_core)
        return false;
    return m_core->GetLinkState() == ELinkState::linked;
}

bool UiController::canPTT() const
{
    if (!m_core)
        return false;
    const auto &cfgdata = m_core->GetConfigData();
    if (cfgdata.sM17SourceCallsign.empty())
        return false;
    // Same condition as "target inputs greyed out": we are linked. Only then can we PTT.
    if (m_core->GetLinkState() != ELinkState::linked)
        return false;
    if (!m_core->IsTransmitOK())
        return false;
    std::string dest = m_destinationCallsign.toStdString();
    bool bDestCS = dest == "@ALL" || std::regex_match(dest, m_core->ReflDstRegEx) || dest == "#PARROT" || std::regex_match(dest, m_core->M17CallRegEx);
    return bDestCS;
}

bool UiController::canQuickKey() const
{
    return canPTT();
}

bool UiController::canOpenDashboard() const
{
    if (!m_core)
        return false;
    auto cs = m_targetCallsign.toStdString();
    auto host = m_core->GetRouteMap().Find(cs);
    return host && !host->url.empty();
}

int UiController::linkState() const
{
    if (!m_core)
        return static_cast<int>(ELinkState::unlinked);
    return static_cast<int>(m_core->GetLinkState());
}

bool UiController::targetInputsEnabled() const
{
    if (!m_core)
        return false;
    const auto &cfgdata = m_core->GetConfigData();
    if (cfgdata.sM17SourceCallsign.empty())
        return false;

    const auto linkState = m_core->GetLinkState();
    switch (linkState) {
    case ELinkState::unlinked:
        return true;
    case ELinkState::linking:
        // Original QWidget UI left target fields enabled while linking
        return true;
    case ELinkState::linked:
        return false;
    }
    return true;
}

bool UiController::destinationEditable() const
{
    if (!m_core)
        return false;
    const auto &cfgdata = m_core->GetConfigData();
    if (cfgdata.sM17SourceCallsign.empty())
        return false;

    const auto linkState = m_core->GetLinkState();
    const bool isLegacy = m_isLegacy;
    switch (linkState) {
    case ELinkState::unlinked:
        return true;
    case ELinkState::linking:
        // In legacy mode, destination is disabled while linking
        return !isLegacy;
    case ELinkState::linked:
        // In legacy mode, destination is disabled while linked; otherwise editable
        return !isLegacy;
    }
    return true;
}

void UiController::link()
{
    if (!m_core || !canConnect())
        return;

    auto &cfgdata = m_core->GetConfigData();
    if (cfgdata.sM17SourceCallsign.empty()) {
        return;
    }

    std::string cmd("M17L");
    std::string cs = m_targetCallsign.toStdString();
    std::string ip = m_targetIp.toStdString();

    uint16_t port = static_cast<uint16_t>(m_targetPort.toUShort());
    m_core->SetDestAddress(ip, port);
    if (cs.compare(0, 4, "M17-") == 0 || cs.compare(0, 3, "URF") == 0) {
        cs.resize(8, ' ');
        cs.append(1, static_cast<char>('A' + m_selectedModuleIndex));
    }
    cmd.append(cs);
    m_core->Link(cmd);
    emit stateChanged();
}

void UiController::unlink()
{
    if (!m_core)
        return;
    std::string cmd("M17U");
    m_core->Link(cmd);
    m_destinationCallsign = QStringLiteral("@ALL");
    emit destinationCallsignChanged();
    emit stateChanged();
}

void UiController::pttToggle()
{
    if (!m_core)
        return;

    if (!m_pttActive) {
        if (m_core->TryLockGateway()) {
            const std::string cs = m_destinationCallsign.toStdString();
            m_core->RecordMic(E_PTT_Type::m17, cs);
            m_pttActive = true;
        }
    } else {
        m_core->KeyOff();
        m_core->ReleaseGatewayLock();
        m_pttActive = false;
    }
    emit stateChanged();
}

void UiController::echoTest()
{
    if (!m_core)
        return;

    if (m_core->IsTransmitOK()) {
        m_core->SetTransmitOK(false);
        m_core->RecordMic(E_PTT_Type::echo, "ECHOTEST");
    } else {
        m_core->PlayEchoData();
        m_core->SetTransmitOK(true);
    }
    emit stateChanged();
}

void UiController::quickKey()
{
    if (!m_core)
        return;
    std::string cs = m_destinationCallsign.toStdString();
    m_core->QuickKey(cs, m_core->GetConfigData().sM17SourceCallsign);
}

void UiController::sendSms(const QString &dst, const QString &message)
{
    if (!m_core)
        return;

    std::string dest = dst.toStdString();
    if (m_core->ToUpper(dest)) {
        // destination normalization only; no need to feed it back
    }

    bool bDestCS = dest == "@ALL" || dest == "#PARROT" || std::regex_match(dest, m_core->M17CallRegEx);
    if (!bDestCS)
        return;

    std::string msg = message.toStdString();
    trim(msg);
    if (msg.empty())
        return;

    m_core->SendMessage(dest, msg);
}

void UiController::openDashboard()
{
    if (!m_core)
        return;
    auto cs = m_targetCallsign.toStdString();
    auto host = m_core->GetRouteMap().Find(cs);
    if (host && !host->url.empty()) {
        QDesktopServices::openUrl(QUrl(QString::fromStdString(host->url)));
    }
}

void UiController::refreshTargets()
{
    m_targetItems.clear();
    if (!m_core)
    {
        emit targetItemsChanged();
        return;
    }

    auto &cfgdata = m_core->GetConfigData();
    for (const auto &cs : m_core->GetRouteMap().GetKeys())
    {
        auto host = m_core->GetRouteMap().Find(cs);
        if (!host) continue;

        bool show = false;
        switch (cfgdata.eNetType) {
            case EInternetType::ipv6only: show = !host->ip6addr.empty(); break;
            case EInternetType::ipv4only: show = !host->ip4addr.empty(); break;
            default: show = true; break;
        }
        if (!show) continue;

        m_targetItems.append(QString::fromStdString(cs));
    }
    emit targetItemsChanged();
}

void UiController::chooseTarget(const QString &cs)
{
    if (!m_core)
        return;

    std::string dest = cs.toStdString();
    m_targetCallsign = cs;
    emit targetCallsignChanged();

    auto &cfgdata = m_core->GetConfigData();
    auto host = m_core->GetRouteMap().Find(dest);
    if (host)
    {
        if (EInternetType::ipv4only != cfgdata.eNetType && !host->ip6addr.empty())
            m_targetIp = QString::fromStdString(host->ip6addr);
        else
            m_targetIp = QString::fromStdString(host->ip4addr);
        emit targetIpChanged();

        m_isLegacy = host->is_legacy;
        emit isLegacyChanged();

        m_targetPort = QString::number(host->port);
        emit targetPortChanged();
    }

    emit stateChanged();
}

void UiController::pollLogQueue()
{
    if (!m_core)
        return;

    bool changed = false;
    std::string msg;
    while (m_core->logQueue.TryPop(msg)) {
        m_logLines.append(QString::fromStdString(msg));
        changed = true;
    }
    if (changed)
        emit logLinesChanged();
}

void UiController::pollLinkState()
{
    if (!m_core)
        return;

    const int current = linkState();
    if (m_lastLinkState != current) {
        m_lastLinkState = current;
        emit stateChanged();
    }

    const bool pttOk = canPTT();
    if (m_pttQuickKeyEnabled != pttOk) {
        m_pttQuickKeyEnabled = pttOk;
        emit pttQuickKeyEnabledChanged();
    }
}

