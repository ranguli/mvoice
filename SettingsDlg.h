/*
 *   Copyright (c) 2019-2025 by Thomas A. Early N7TAE
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

#pragma once

#include <QDialog>
#include <map>
#include <regex>

#include "Configure.h"

class QTabWidget;
class QLineEdit;
class QComboBox;
class QRadioButton;
class QPushButton;
class QLabel;
class CMainWindow;

class CSettingsDlg : public QDialog
{
	Q_OBJECT
public:
	explicit CSettingsDlg(CMainWindow *pMain, QWidget *parent = nullptr);
	void Refresh();

private slots:
	void SourceCallsignInput();
	void AudioRescanButton();
	void AudioInputChoice(int index);
	void AudioOutputChoice(int index);
	void ModuleChoice(int index);
	void UpdateButton();
	void LatitudeInput();
	void LongitudeInput();
	void TextMessageInput();

private:
	std::map<std::string, std::pair<std::string, std::string>> AudioInMap, AudioOutMap;
	void SaveWidgetStates(CFGDATA &d);
	void SetWidgetStates(const CFGDATA &d);
	void SetOkayButton();

	std::regex LatRegEx, LongRegEx;
	CFGDATA data;
	bool bM17Source, bLatitude, bLongitude;

	CMainWindow *pMainWindow;

	QTabWidget *pTabs;
	QPushButton *pOkayButton;
	QPushButton *pAudioRescanButton;
	QComboBox *pAudioInputChoice, *pAudioOutputChoice, *pModuleChoice;
	QLineEdit *pSourceCallsignInput, *pTextMessageInput;
	QLineEdit *pLatitudeInput, *pLongitudeInput;
#ifndef NO_DHT
	QLineEdit *pBootstrapInput;
#endif
	QLabel *pAudioInputDescBox, *pAudioOutputDescBox;
	QRadioButton *pVoiceOnlyRadioButton, *pVoiceDataRadioButton;
	QRadioButton *pIPv4RadioButton, *pIPv6RadioButton, *pDualStackRadioButton;
};
