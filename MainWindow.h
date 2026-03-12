/*
 *   Copyright (c) 2019-2022 by Thomas A. Early N7TAE
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

#include <QMainWindow>

#include "AppCore.h"

class QLineEdit;
class QCheckBox;
class QRadioButton;
class QGroupBox;
class QPushButton;
class QMenuBar;
class QMenu;
class QTextEdit;
class QTimer;
class QIntValidator;

class CTransmitButton;
class CSMSDlg;
class CSettingsDlg;
class CAboutDlg;

class CMainWindow : public QMainWindow
{
	Q_OBJECT
public:
	CMainWindow(QWidget *parent = nullptr);
	~CMainWindow();

	CAppCore core;

	bool Init();
	void NewSettings(CFGDATA *newdata);

private slots:
	void UpdateGUI();
	void Quit();
	void ShowSMSDialog();
	void ShowSettingsDialog();
	void ShowAboutDialog();
	void EchoButton();
	void PTTButton();
	void QuickKeyButton();
	void TargetCSInput();
	void TargetIPInput();
	void TargetPortInput();
	void DestinationCSInput();
	void ActionButton();
	void LinkButton();
	void UnlinkButton();
	void DashboardButton();

private:
	CSMSDlg *pSMSDlg;
	CSettingsDlg *pSettingsDlg;

	CTransmitButton *pPTTButton, *pEchoTestButton;
	QPushButton *pQuickKeyButton, *pActionButton, *pConnectButton, *pDisconnectButton, *pDashboardButton;
	QCheckBox *pIsLegacyCheck;
	QLineEdit *pTargetCSInput, *pTargetIpInput;
	QLineEdit *pDSTCallsignInput;
	QLineEdit *pTargetPortInput;
	QIntValidator *pPortValidator;
	QGroupBox *pModuleGroup;
	QRadioButton *pModuleRadioButton[26];
	QMenu *pTargetMenu;
	QTextEdit *pTextDisplay;
	QTimer *pUpdateTimer;

	bool bDestCS, bTargetCS, bTargetIP, bTargetPort;

	void BuildTargetMenuButton();
	void FixTargetMenuButton();
	void SetTargetMenuButton(const QString &label = QString());
	void TransmitterButtonControl();
	char GetTargetModule();
	void SetTargetAddress(std::string &cs);
	void ActivateModules(const std::string &modules = "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	void insertLogText(const char *line);
	void AudioSummary(const char *title);
	void DrainLogQueue();

	void closeEvent(QCloseEvent *event) override;
};
