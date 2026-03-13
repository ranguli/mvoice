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

#include <sstream>
#include <iomanip>
#include <cstring>

#include <alsa/asoundlib.h>

#include "SettingsController.h"
#include "Configure.h"

SettingsController::SettingsController(CAppCore *core, QObject *parent)
	: QObject(parent)
	, m_core(core)
	, m_latRegEx("^[+-]?(90|([1-8]?[0-9](\\.[0-9]*)?))$", std::regex::extended)
	, m_longRegEx("^[+-]?(180|(((1[0-7])|[1-9]?)[0-9](\\.[0-9]*)?))$", std::regex::extended)
{
	loadFromConfig();
}

bool SettingsController::hasDht() const
{
#ifndef NO_DHT
	return true;
#else
	return false;
#endif
}

void SettingsController::setSettingsCallsign(const QString &s)
{
	std::string str = s.toStdString();
	if (m_core->ToUpper(str)) {
		QString q = QString::fromStdString(str);
		if (q != m_settingsCallsign) {
			m_settingsCallsign = q;
			emit settingsCallsignChanged();
		}
	} else if (s != m_settingsCallsign) {
		m_settingsCallsign = s;
		emit settingsCallsignChanged();
	}
	m_settingsCallsignValid = std::regex_match(m_settingsCallsign.toStdString(), m_core->M17CallRegEx);
	emit settingsCallsignValidChanged();
	updateCanApply();
}

void SettingsController::setSettingsModuleIndex(int i)
{
	if (i < 0 || i > 25) return;
	if (m_settingsModuleIndex != i) {
		m_settingsModuleIndex = i;
		emit settingsModuleIndexChanged();
	}
}

void SettingsController::setSettingsVoiceOnly(bool v)
{
	if (m_settingsVoiceOnly != v) {
		m_settingsVoiceOnly = v;
		emit settingsVoiceOnlyChanged();
	}
}

void SettingsController::setSettingsLatitude(const QString &s)
{
	if (m_settingsLatitude != s) {
		m_settingsLatitude = s;
		emit settingsLatitudeChanged();
	}
	m_settingsLatitudeValid = std::regex_match(m_settingsLatitude.toStdString(), m_latRegEx);
	emit settingsLatitudeValidChanged();
	updateCanApply();
}

void SettingsController::setSettingsLongitude(const QString &s)
{
	if (m_settingsLongitude != s) {
		m_settingsLongitude = s;
		emit settingsLongitudeChanged();
	}
	m_settingsLongitudeValid = std::regex_match(m_settingsLongitude.toStdString(), m_longRegEx);
	emit settingsLongitudeValidChanged();
	updateCanApply();
}

void SettingsController::setSettingsMessage(const QString &s)
{
	QString truncated = s.left(52);
	if (m_settingsMessage != truncated) {
		m_settingsMessage = truncated;
		emit settingsMessageChanged();
	}
}

void SettingsController::setSettingsNetTypeIndex(int i)
{
	if (i < 0 || i > 2) return;
	if (m_settingsNetTypeIndex != i) {
		m_settingsNetTypeIndex = i;
		emit settingsNetTypeIndexChanged();
	}
}

#ifndef NO_DHT
void SettingsController::setSettingsBootstrap(const QString &s)
{
	if (m_settingsBootstrap != s) {
		m_settingsBootstrap = s;
		emit settingsBootstrapChanged();
	}
}
#endif

void SettingsController::setAudioInputIndex(int i)
{
	if (i < 0 || i >= m_audioInputNames.size()) return;
	if (m_audioInputIndex != i) {
		m_audioInputIndex = i;
		emit audioInputIndexChanged();
	}
	std::string key = m_audioInputNames.at(i).toStdString();
	auto it = m_audioInMap.find(key);
	if (it != m_audioInMap.end())
		m_audioInputDescription = QString::fromStdString(it->second.second);
	else
		m_audioInputDescription = m_audioInputNames.at(i) + tr(" not found");
	emit audioInputDescriptionChanged();
}

void SettingsController::setAudioOutputIndex(int i)
{
	if (i < 0 || i >= m_audioOutputNames.size()) return;
	if (m_audioOutputIndex != i) {
		m_audioOutputIndex = i;
		emit audioOutputIndexChanged();
	}
	std::string key = m_audioOutputNames.at(i).toStdString();
	auto it = m_audioOutMap.find(key);
	if (it != m_audioOutMap.end())
		m_audioOutputDescription = QString::fromStdString(it->second.second);
	else
		m_audioOutputDescription = m_audioOutputNames.at(i) + tr(" not found");
	emit audioOutputDescriptionChanged();
}

void SettingsController::updateCanApply()
{
	bool ok = m_settingsCallsignValid && m_settingsLatitudeValid && m_settingsLongitudeValid;
	if (ok != m_settingsCanApply) {
		m_settingsCanApply = ok;
		emit settingsCanApplyChanged();
	}
}

void SettingsController::loadFromConfig()
{
	const CFGDATA *d = m_core->GetConfig().GetData();
	if (!d) return;

	m_settingsCallsign = QString::fromStdString(d->sM17SourceCallsign);
	m_settingsCallsignValid = std::regex_match(d->sM17SourceCallsign, m_core->M17CallRegEx);

	m_settingsModuleIndex = d->cModule - 'A';
	if (m_settingsModuleIndex < 0) m_settingsModuleIndex = 0;
	if (m_settingsModuleIndex > 25) m_settingsModuleIndex = 25;

	m_settingsVoiceOnly = d->bVoiceOnlyEnable;

	std::ostringstream ss;
	ss << std::fixed << std::setprecision(5) << d->dLatitude;
	m_settingsLatitude = QString::fromStdString(ss.str());
	ss.str("");
	ss << std::fixed << std::setprecision(5) << d->dLongitude;
	m_settingsLongitude = QString::fromStdString(ss.str());
	m_settingsLatitudeValid = std::regex_match(m_settingsLatitude.toStdString(), m_latRegEx);
	m_settingsLongitudeValid = std::regex_match(m_settingsLongitude.toStdString(), m_longRegEx);

	m_settingsMessage = QString::fromStdString(d->sMessage);

	switch (d->eNetType) {
		case EInternetType::ipv6only:  m_settingsNetTypeIndex = 1; break;
		case EInternetType::dualstack: m_settingsNetTypeIndex = 2; break;
		default:                       m_settingsNetTypeIndex = 0; break;
	}

#ifndef NO_DHT
	m_settingsBootstrap = QString::fromStdString(d->sBootstrap);
#endif

	updateCanApply();
	rescanAudio();

	// After rescan, try to restore selected audio devices by internal name
	for (int i = 0; i < m_audioInputNames.size(); i++) {
		std::string key = m_audioInputNames.at(i).toStdString();
		auto it = m_audioInMap.find(key);
		if (it != m_audioInMap.end() && it->second.first == d->sAudioIn) {
			m_audioInputIndex = i;
			m_audioInputDescription = QString::fromStdString(it->second.second);
			break;
		}
	}
	for (int i = 0; i < m_audioOutputNames.size(); i++) {
		std::string key = m_audioOutputNames.at(i).toStdString();
		auto it = m_audioOutMap.find(key);
		if (it != m_audioOutMap.end() && it->second.first == d->sAudioOut) {
			m_audioOutputIndex = i;
			m_audioOutputDescription = QString::fromStdString(it->second.second);
			break;
		}
	}
	emit settingsCallsignChanged();
	emit settingsModuleIndexChanged();
	emit settingsVoiceOnlyChanged();
	emit settingsLatitudeChanged();
	emit settingsLongitudeChanged();
	emit settingsMessageChanged();
	emit settingsNetTypeIndexChanged();
#ifndef NO_DHT
	emit settingsBootstrapChanged();
#endif
	emit settingsCallsignValidChanged();
	emit settingsLatitudeValidChanged();
	emit settingsLongitudeValidChanged();
	emit settingsCanApplyChanged();
	emit audioInputIndexChanged();
	emit audioOutputIndexChanged();
	emit audioInputDescriptionChanged();
	emit audioOutputDescriptionChanged();
}

void SettingsController::rescanAudio()
{
	int inChoice = m_audioInputIndex;
	int outChoice = m_audioOutputIndex;
	if (inChoice < 0) inChoice = 0;
	if (outChoice < 0) outChoice = 0;

	m_audioInputNames.clear();
	m_audioOutputNames.clear();
	m_audioInMap.clear();
	m_audioOutMap.clear();

	void **hints;
	if (snd_device_name_hint(-1, "pcm", &hints) < 0) {
		emit audioInputNamesChanged();
		emit audioOutputNamesChanged();
		return;
	}

	for (void **n = hints; *n != nullptr; n++) {
		char *name = snd_device_name_get_hint(*n, "NAME");
		if (!name) continue;
		char *desc = snd_device_name_get_hint(*n, "DESC");
		if (!desc) { free(name); continue; }

		if ((strcmp(name, "default") == 0 || strstr(name, "plughw")) && !strstr(desc, "without any conversions")) {
			char *io = snd_device_name_get_hint(*n, "IOID");
			bool is_input = true, is_output = true;
			if (io) {
				if (strcasecmp(io, "Input") == 0) is_output = false;
				else if (strcasecmp(io, "Output") == 0) is_input = false;
				free(io);
			}

			std::string short_name(name);
			auto pos = short_name.find("plughw:CARD=");
			if (pos != std::string::npos) {
				short_name = short_name.replace(pos, 12, "");
				auto dpos = short_name.find(",DEV=0");
				if (dpos != std::string::npos)
					short_name = short_name.replace(dpos, 6, "");
				if (short_name.empty())
					short_name.assign(name);
			}

			if (is_input) {
				snd_pcm_t *handle;
				if (snd_pcm_open(&handle, name, SND_PCM_STREAM_CAPTURE, 0) == 0) {
					m_audioInMap[short_name] = {name, desc};
					snd_pcm_close(handle);
					m_audioInputNames.append(QString::fromStdString(short_name));
				}
			}
			if (is_output) {
				snd_pcm_t *handle;
				if (snd_pcm_open(&handle, name, SND_PCM_STREAM_PLAYBACK, 0) == 0) {
					m_audioOutMap[short_name] = {name, desc};
					snd_pcm_close(handle);
					m_audioOutputNames.append(QString::fromStdString(short_name));
				}
			}
		}
		free(name);
		free(desc);
	}
	snd_device_name_free_hint(hints);

	if (inChoice >= m_audioInputNames.size()) inChoice = 0;
	if (outChoice >= m_audioOutputNames.size()) outChoice = 0;
	m_audioInputIndex = inChoice;
	m_audioOutputIndex = outChoice;

	if (!m_audioInputNames.isEmpty() && inChoice < m_audioInputNames.size()) {
		std::string key = m_audioInputNames.at(inChoice).toStdString();
		auto it = m_audioInMap.find(key);
		m_audioInputDescription = (it != m_audioInMap.end()) ? QString::fromStdString(it->second.second) : QString();
	}
	if (!m_audioOutputNames.isEmpty() && outChoice < m_audioOutputNames.size()) {
		std::string key = m_audioOutputNames.at(outChoice).toStdString();
		auto it = m_audioOutMap.find(key);
		m_audioOutputDescription = (it != m_audioOutMap.end()) ? QString::fromStdString(it->second.second) : QString();
	}

	emit audioInputNamesChanged();
	emit audioOutputNamesChanged();
	emit audioInputIndexChanged();
	emit audioOutputIndexChanged();
	emit audioInputDescriptionChanged();
	emit audioOutputDescriptionChanged();
}

void SettingsController::refreshSettings()
{
	loadFromConfig();
}

bool SettingsController::applySettings()
{
	if (!m_settingsCanApply) return false;

	CFGDATA newstate;
	newstate.sM17SourceCallsign = m_settingsCallsign.toStdString();
	newstate.bVoiceOnlyEnable = m_settingsVoiceOnly;
	newstate.cModule = 'A' + m_settingsModuleIndex;
	newstate.dLatitude = m_settingsLatitude.toDouble();
	newstate.dLongitude = m_settingsLongitude.toDouble();
	newstate.sMessage = m_settingsMessage.toStdString();

	switch (m_settingsNetTypeIndex) {
		case 1: newstate.eNetType = EInternetType::ipv6only; break;
		case 2: newstate.eNetType = EInternetType::dualstack; break;
		default: newstate.eNetType = EInternetType::ipv4only; break;
	}

	if (!m_audioInputNames.isEmpty() && m_audioInputIndex >= 0 && m_audioInputIndex < m_audioInputNames.size()) {
		std::string key = m_audioInputNames.at(m_audioInputIndex).toStdString();
		auto it = m_audioInMap.find(key);
		if (it != m_audioInMap.end())
			newstate.sAudioIn = it->second.first;
		else
			newstate.sAudioIn = "default";
	} else {
		newstate.sAudioIn = "default";
	}
	if (!m_audioOutputNames.isEmpty() && m_audioOutputIndex >= 0 && m_audioOutputIndex < m_audioOutputNames.size()) {
		std::string key = m_audioOutputNames.at(m_audioOutputIndex).toStdString();
		auto it = m_audioOutMap.find(key);
		if (it != m_audioOutMap.end())
			newstate.sAudioOut = it->second.first;
		else
			newstate.sAudioOut = "default";
	} else {
		newstate.sAudioOut = "default";
	}

#ifndef NO_DHT
	newstate.sBootstrap = m_settingsBootstrap.toStdString();
#endif

	m_core->GetConfig().CopyFrom(newstate);
	m_core->GetConfig().WriteData();
	m_core->ApplyNewSettings(&newstate);
	emit settingsApplied();
	return true;
}
