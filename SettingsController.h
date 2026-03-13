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
 *   QML Settings controller
 *
 *   Exposes configuration get/set and ALSA device lists for the Settings dialog.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <map>
#include <regex>

#include "Configure.h"
#include "AppCore.h"

class SettingsController : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString settingsCallsign READ settingsCallsign WRITE setSettingsCallsign NOTIFY settingsCallsignChanged)
	Q_PROPERTY(int settingsModuleIndex READ settingsModuleIndex WRITE setSettingsModuleIndex NOTIFY settingsModuleIndexChanged)
	Q_PROPERTY(bool settingsVoiceOnly READ settingsVoiceOnly WRITE setSettingsVoiceOnly NOTIFY settingsVoiceOnlyChanged)
	Q_PROPERTY(QString settingsLatitude READ settingsLatitude WRITE setSettingsLatitude NOTIFY settingsLatitudeChanged)
	Q_PROPERTY(QString settingsLongitude READ settingsLongitude WRITE setSettingsLongitude NOTIFY settingsLongitudeChanged)
	Q_PROPERTY(QString settingsMessage READ settingsMessage WRITE setSettingsMessage NOTIFY settingsMessageChanged)
	Q_PROPERTY(int settingsNetTypeIndex READ settingsNetTypeIndex WRITE setSettingsNetTypeIndex NOTIFY settingsNetTypeIndexChanged)
	Q_PROPERTY(bool hasDht READ hasDht CONSTANT)
#ifndef NO_DHT
	Q_PROPERTY(QString settingsBootstrap READ settingsBootstrap WRITE setSettingsBootstrap NOTIFY settingsBootstrapChanged)
#endif
	Q_PROPERTY(QStringList audioInputNames READ audioInputNames NOTIFY audioInputNamesChanged)
	Q_PROPERTY(QStringList audioOutputNames READ audioOutputNames NOTIFY audioOutputNamesChanged)
	Q_PROPERTY(QString audioInputDescription READ audioInputDescription NOTIFY audioInputDescriptionChanged)
	Q_PROPERTY(QString audioOutputDescription READ audioOutputDescription NOTIFY audioOutputDescriptionChanged)
	Q_PROPERTY(int audioInputIndex READ audioInputIndex WRITE setAudioInputIndex NOTIFY audioInputIndexChanged)
	Q_PROPERTY(int audioOutputIndex READ audioOutputIndex WRITE setAudioOutputIndex NOTIFY audioOutputIndexChanged)
	Q_PROPERTY(bool settingsCallsignValid READ settingsCallsignValid NOTIFY settingsCallsignValidChanged)
	Q_PROPERTY(bool settingsLatitudeValid READ settingsLatitudeValid NOTIFY settingsLatitudeValidChanged)
	Q_PROPERTY(bool settingsLongitudeValid READ settingsLongitudeValid NOTIFY settingsLongitudeValidChanged)
	Q_PROPERTY(bool settingsCanApply READ settingsCanApply NOTIFY settingsCanApplyChanged)

public:
	explicit SettingsController(CAppCore *core, QObject *parent = nullptr);

	QString settingsCallsign() const { return m_settingsCallsign; }
	int settingsModuleIndex() const { return m_settingsModuleIndex; }
	bool settingsVoiceOnly() const { return m_settingsVoiceOnly; }
	QString settingsLatitude() const { return m_settingsLatitude; }
	QString settingsLongitude() const { return m_settingsLongitude; }
	QString settingsMessage() const { return m_settingsMessage; }
	int settingsNetTypeIndex() const { return m_settingsNetTypeIndex; }
	bool hasDht() const;
#ifndef NO_DHT
	QString settingsBootstrap() const { return m_settingsBootstrap; }
#endif
	QStringList audioInputNames() const { return m_audioInputNames; }
	QStringList audioOutputNames() const { return m_audioOutputNames; }
	QString audioInputDescription() const { return m_audioInputDescription; }
	QString audioOutputDescription() const { return m_audioOutputDescription; }
	int audioInputIndex() const { return m_audioInputIndex; }
	int audioOutputIndex() const { return m_audioOutputIndex; }
	bool settingsCallsignValid() const { return m_settingsCallsignValid; }
	bool settingsLatitudeValid() const { return m_settingsLatitudeValid; }
	bool settingsLongitudeValid() const { return m_settingsLongitudeValid; }
	bool settingsCanApply() const { return m_settingsCanApply; }

public slots:
	void setSettingsCallsign(const QString &s);
	void setSettingsModuleIndex(int i);
	void setSettingsVoiceOnly(bool v);
	void setSettingsLatitude(const QString &s);
	void setSettingsLongitude(const QString &s);
	void setSettingsMessage(const QString &s);
	void setSettingsNetTypeIndex(int i);
#ifndef NO_DHT
	void setSettingsBootstrap(const QString &s);
#endif
	void setAudioInputIndex(int i);
	void setAudioOutputIndex(int i);
	void refreshSettings();
	void rescanAudio();
	bool applySettings();

signals:
	void settingsCallsignChanged();
	void settingsModuleIndexChanged();
	void settingsVoiceOnlyChanged();
	void settingsLatitudeChanged();
	void settingsLongitudeChanged();
	void settingsMessageChanged();
	void settingsNetTypeIndexChanged();
#ifndef NO_DHT
	void settingsBootstrapChanged();
#endif
	void audioInputNamesChanged();
	void audioOutputNamesChanged();
	void audioInputDescriptionChanged();
	void audioOutputDescriptionChanged();
	void audioInputIndexChanged();
	void audioOutputIndexChanged();
	void settingsCallsignValidChanged();
	void settingsLatitudeValidChanged();
	void settingsLongitudeValidChanged();
	void settingsCanApplyChanged();
	void settingsApplied();

private:
	void updateCanApply();
	void loadFromConfig();

	CAppCore *m_core;
	QString m_settingsCallsign;
	int m_settingsModuleIndex{0};
	bool m_settingsVoiceOnly{true};
	QString m_settingsLatitude;
	QString m_settingsLongitude;
	QString m_settingsMessage;
	int m_settingsNetTypeIndex{0};
#ifndef NO_DHT
	QString m_settingsBootstrap;
#endif
	QStringList m_audioInputNames;
	QStringList m_audioOutputNames;
	QString m_audioInputDescription;
	QString m_audioOutputDescription;
	int m_audioInputIndex{0};
	int m_audioOutputIndex{0};
	bool m_settingsCallsignValid{false};
	bool m_settingsLatitudeValid{false};
	bool m_settingsLongitudeValid{false};
	bool m_settingsCanApply{false};

	std::map<std::string, std::pair<std::string, std::string>> m_audioInMap;
	std::map<std::string, std::pair<std::string, std::string>> m_audioOutMap;
	std::regex m_latRegEx;
	std::regex m_longRegEx;
};
