/*
 *   Copyright (c) 2019-2021 by Thomas A. Early N7TAE
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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <cstring>

#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QLineEdit>
#include <QCheckBox>
#include <QRadioButton>
#include <QPushButton>
#include <QGroupBox>
#include <QTextEdit>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QIntValidator>
#include <QDesktopServices>
#include <QUrl>
#include <QLabel>
#include <QCloseEvent>
#include <QImage>
#include <QPixmap>
#include <QIcon>

#include "MainWindow.h"
#include "SMSDlg.h"
#include "SettingsDlg.h"
#include "AboutDlg.h"
#include "TransmitButton.h"
#include "Utilities.h"
#include "IconData.h"

CMainWindow::CMainWindow(QWidget *parent)
	: QMainWindow(parent),
	  pSMSDlg(nullptr),
	  pSettingsDlg(nullptr),
	  bTargetCS(false),
	  bTargetIP(false),
	  bTargetPort(false),
	  bDestCS(false)
{
}

CMainWindow::~CMainWindow()
{
}

bool CMainWindow::Init()
{
	if (core.Init())
		return true;

	core.onReceiveStateChanged = [this](bool) {
		QMetaObject::invokeMethod(this, [this]() {
			if (core.IsTransmitOK())
				pEchoTestButton->setEnabled(true);
			else
				pEchoTestButton->setEnabled(false);
			TransmitterButtonControl();
		}, Qt::QueuedConnection);
	};

	// Window icon from embedded RGBA data
	QImage img(icon_image.pixel_data, icon_image.width, icon_image.height, QImage::Format_RGBA8888);
	setWindowIcon(QIcon(QPixmap::fromImage(img)));
	setWindowTitle("MVoice");
	setMinimumSize(760, 440);
	resize(900, 600);

	// Menu bar
	auto *menuBar = this->menuBar();
	pTargetMenu = menuBar->addMenu(tr("Target"));
	menuBar->addAction(tr("Texting..."), this, &CMainWindow::ShowSMSDialog);
	menuBar->addAction(tr("Settings..."), this, &CMainWindow::ShowSettingsDialog);
	menuBar->addAction(tr("About..."), this, &CMainWindow::ShowAboutDialog);

	// Central widget
	auto *central = new QWidget;
	setCentralWidget(central);
	auto *mainLayout = new QVBoxLayout(central);

	// Log display
	pTextDisplay = new QTextEdit;
	pTextDisplay->setReadOnly(true);
	mainLayout->addWidget(pTextDisplay, 1);

	// Target callsign row
	auto *targetRow = new QHBoxLayout;
	targetRow->addWidget(new QLabel(tr("Target Callsign:")));
	pTargetCSInput = new QLineEdit;
	pTargetCSInput->setToolTip(tr("A reflector or user callsign"));
	targetRow->addWidget(pTargetCSInput);

	pIsLegacyCheck = new QCheckBox(tr("Is Legacy"));
	pIsLegacyCheck->setToolTip(tr("Is the M17 reflector version 0.x.y?"));
	targetRow->addWidget(pIsLegacyCheck);

	targetRow->addWidget(new QLabel(tr("IP:")));
	pTargetIpInput = new QLineEdit;
	pTargetIpInput->setToolTip(tr("The IP of the reflector or user"));
	targetRow->addWidget(pTargetIpInput);

	targetRow->addWidget(new QLabel(tr("Port:")));
	pTargetPortInput = new QLineEdit;
	pTargetPortInput->setToolTip(tr("The comm port of the reflector or user"));
	pPortValidator = new QIntValidator(1024, 49000, this);
	pTargetPortInput->setValidator(pPortValidator);
	pTargetPortInput->setMaximumWidth(80);
	targetRow->addWidget(pTargetPortInput);
	mainLayout->addLayout(targetRow);

	// Module selection
	pModuleGroup = new QGroupBox(tr("Module"));
	pModuleGroup->setToolTip(tr("Select a module for the reflector or repeater"));
	auto *modGrid = new QGridLayout(pModuleGroup);
	for (int i = 0; i < 26; i++)
	{
		pModuleRadioButton[i] = new QRadioButton(QString(QChar('A' + i)));
		modGrid->addWidget(pModuleRadioButton[i], i / 13, i % 13);
	}
	pModuleRadioButton[0]->setChecked(true);

	auto *moduleAndConnectRow = new QHBoxLayout;
	moduleAndConnectRow->addWidget(pModuleGroup, 1);

	auto *connectCol = new QVBoxLayout;
	pConnectButton = new QPushButton(tr("Connect"));
	pConnectButton->setToolTip(tr("Connect to an M17 Reflector"));
	pConnectButton->setEnabled(false);
	connectCol->addWidget(pConnectButton);

	pDisconnectButton = new QPushButton(tr("Disconnect"));
	pDisconnectButton->setToolTip(tr("Disconnect from an M17 reflector"));
	pDisconnectButton->setEnabled(false);
	connectCol->addWidget(pDisconnectButton);
	moduleAndConnectRow->addLayout(connectCol);
	mainLayout->addLayout(moduleAndConnectRow);

	// Action / Destination / Dashboard row
	auto *actionRow = new QHBoxLayout;
	pActionButton = new QPushButton(tr("Action"));
	pActionButton->setToolTip(tr("Update or delete an existing contact, or save a new contact"));
	pActionButton->setEnabled(false);
	actionRow->addWidget(pActionButton);

	actionRow->addWidget(new QLabel(tr("Destination Callsign:")));
	pDSTCallsignInput = new QLineEdit("@ALL");
	pDSTCallsignInput->setToolTip(tr("A destination callsign"));
	actionRow->addWidget(pDSTCallsignInput);

	pDashboardButton = new QPushButton(tr("Open Dashboard"));
	pDashboardButton->setToolTip(tr("Open a reflector dashboard, if available"));
	pDashboardButton->setEnabled(false);
	actionRow->addWidget(pDashboardButton);
	mainLayout->addLayout(actionRow);

	// PTT / Echo / Quick Key row
	auto *pttRow = new QHBoxLayout;
	pEchoTestButton = new CTransmitButton(tr("Echo Test"));
	pEchoTestButton->setToolTip(tr("Push to record a test that will be played back"));
	pttRow->addWidget(pEchoTestButton);

	pPTTButton = new CTransmitButton(tr("PTT"));
	pPTTButton->setToolTip(tr("Push to talk. This is actually a toggle button"));
	pPTTButton->setEnabled(false);
	auto font = pPTTButton->font();
	font.setPointSize(16);
	pPTTButton->setFont(font);
	pPTTButton->setMinimumHeight(50);
	pttRow->addWidget(pPTTButton, 1);

	pQuickKeyButton = new QPushButton(tr("Quick Key"));
	pQuickKeyButton->setToolTip(tr("Send a short, silent voice stream"));
	pQuickKeyButton->setEnabled(false);
	pttRow->addWidget(pQuickKeyButton);
	mainLayout->addLayout(pttRow);

	// Create dialogs
	pSMSDlg = new CSMSDlg(&core, this);
	pSettingsDlg = new CSettingsDlg(this, this);

	// Wire up signals
	connect(pTargetCSInput, &QLineEdit::textChanged, this, &CMainWindow::TargetCSInput);
	connect(pTargetIpInput, &QLineEdit::textChanged, this, &CMainWindow::TargetIPInput);
	connect(pTargetPortInput, &QLineEdit::textChanged, this, &CMainWindow::TargetPortInput);
	connect(pDSTCallsignInput, &QLineEdit::textChanged, this, &CMainWindow::DestinationCSInput);
	connect(pEchoTestButton, &QPushButton::clicked, this, &CMainWindow::EchoButton);
	connect(pPTTButton, &QPushButton::clicked, this, &CMainWindow::PTTButton);
	connect(pQuickKeyButton, &QPushButton::clicked, this, &CMainWindow::QuickKeyButton);
	connect(pActionButton, &QPushButton::clicked, this, &CMainWindow::ActionButton);
	connect(pConnectButton, &QPushButton::clicked, this, &CMainWindow::LinkButton);
	connect(pDisconnectButton, &QPushButton::clicked, this, &CMainWindow::UnlinkButton);
	connect(pDashboardButton, &QPushButton::clicked, this, &CMainWindow::DashboardButton);

	core.OnReceive(false);
	core.SetState();
	BuildTargetMenuButton();

	// Periodic GUI update
	pUpdateTimer = new QTimer(this);
	connect(pUpdateTimer, &QTimer::timeout, this, &CMainWindow::UpdateGUI);
	pUpdateTimer->start(1000);

#ifndef NO_DHT
	if (core.InitDHT())
		return true;
#endif

	return false;
}

void CMainWindow::closeEvent(QCloseEvent *event)
{
	Quit();
	event->accept();
}

void CMainWindow::Quit()
{
	if (pSMSDlg) pSMSDlg->hide();
	core.KeyOff();
	core.StopM17();
}

void CMainWindow::ShowSMSDialog()
{
	if (pIsLegacyCheck->isChecked())
	{
		insertLogText("Sorry! You can't send a message to a legacy reflector.\n");
	}
	else
	{
		pSMSDlg->show();
		pSMSDlg->raise();
	}
}

void CMainWindow::ShowSettingsDialog()
{
	pSettingsDlg->Refresh();
	pSettingsDlg->show();
	pSettingsDlg->raise();
}

void CMainWindow::ShowAboutDialog()
{
	auto *dlg = new CAboutDlg(this);
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->setModal(true);
	dlg->show();
}

void CMainWindow::NewSettings(CFGDATA *newdata)
{
	core.ApplyNewSettings(newdata);
	BuildTargetMenuButton();
}

void CMainWindow::BuildTargetMenuButton()
{
	pTargetMenu->clear();
	auto &cfgdata = core.GetConfigData();
	for (const auto &cs : core.GetRouteMap().GetKeys())
	{
		auto host = core.GetRouteMap().Find(cs);
		if (!host) continue;

		bool show = false;
		switch (cfgdata.eNetType) {
			case EInternetType::ipv6only: show = !host->ip6addr.empty(); break;
			case EInternetType::ipv4only: show = !host->ip4addr.empty(); break;
			default: show = true; break;
		}
		if (!show) continue;

		pTargetMenu->addAction(QString::fromStdString(cs), this, [this, cs]() {
			pTargetCSInput->setText(QString::fromStdString(cs));
			TargetCSInput();
			auto host = core.GetRouteMap().Find(cs);
			if (host) {
				auto &cfgdata = core.GetConfigData();
				if (EInternetType::ipv4only != cfgdata.eNetType && !host->ip6addr.empty())
					pTargetIpInput->setText(QString::fromStdString(host->ip6addr));
				else
					pTargetIpInput->setText(QString::fromStdString(host->ip4addr));
				TargetIPInput();
				pTargetPortInput->setText(QString::number(host->port));
				TargetPortInput();
			}
		});
	}
}

void CMainWindow::ActivateModules(const std::string &modules)
{
	for (int i = 0; i < 26; i++)
	{
		if (modules.find('A' + i) == std::string::npos)
			pModuleRadioButton[i]->setEnabled(false);
		else
			pModuleRadioButton[i]->setEnabled(true);
	}
}

void CMainWindow::ActionButton()
{
	static const QString saveStr = tr("Save");
	static const QString deleteStr = tr("Delete");
	static const QString updateStr = tr("Update");

	const QString label = pActionButton->text();
	auto cs = pTargetCSInput->text().toStdString();
	bool islegacy = pIsLegacyCheck->isChecked();

	if (label == saveStr) {
		const std::string a = pTargetIpInput->text().toStdString();
		auto p = uint16_t(pTargetPortInput->text().toUInt());
		if (a.find(':') == std::string::npos)
			core.GetRouteMap().Update(EFrom::user, cs, islegacy, "", a, "", "", "", p, "");
		else
			core.GetRouteMap().Update(EFrom::user, cs, islegacy, "", "", a, "", "", p, "");
		BuildTargetMenuButton();
	} else if (label == deleteStr) {
		core.GetRouteMap().Erase(cs);
		BuildTargetMenuButton();
	} else if (label == updateStr) {
		std::string a = pTargetIpInput->text().toStdString();
		const auto p = uint16_t(pTargetPortInput->text().toUInt());
		if (a.find(':') == std::string::npos)
			core.GetRouteMap().Update(EFrom::user, cs, islegacy, "", a, "", "", "", p, "");
		else
			core.GetRouteMap().Update(EFrom::user, cs, islegacy, "", "", a, "", "", p, "");
	}
	FixTargetMenuButton();
	core.GetRouteMap().Save();
}

void CMainWindow::AudioSummary(const char *title)
{
	auto summary = core.FormatAudioSummary(title);
	core.logQueue.Push(summary);
}

void CMainWindow::EchoButton()
{
	pEchoTestButton->toggle();
	if (pEchoTestButton->isChecked()) {
		core.SetTransmitOK(false);
		core.RecordMic(E_PTT_Type::echo, "ECHOTEST");
	} else {
		AudioSummary(tr("Echo").toUtf8().constData());
		core.PlayEchoData();
		core.SetTransmitOK(true);
	}
}

void CMainWindow::SetTargetAddress(std::string &cs)
{
	cs = pTargetCSInput->text().toStdString();
	const std::string ip = pTargetIpInput->text().toStdString();
	uint16_t port = pTargetPortInput->text().toUInt();
	core.SetDestAddress(ip, port);
	if (cs.compare(0, 4, "M17-") == 0 || cs.compare(0, 3, "URF") == 0) {
		cs.resize(8, ' ');
		cs.append(1, GetTargetModule());
	}
	if (pIsLegacyCheck->isChecked())
	{
		pDSTCallsignInput->setText(QString::fromStdString(cs));
	}
}

void CMainWindow::PTTButton()
{
	pPTTButton->toggle();
	if (pPTTButton->isChecked()) {
		if (core.TryLockGateway())
		{
			const std::string cs = pDSTCallsignInput->text().toStdString();
			core.RecordMic(E_PTT_Type::m17, cs);
		}
		else
		{
			pPTTButton->setChecked(false);
		}
	}
	else
	{
		core.KeyOff();
		AudioSummary(tr("PTT").toUtf8().constData());
		core.ReleaseGatewayLock();
	}
}

void CMainWindow::QuickKeyButton()
{
	std::string cs = pDSTCallsignInput->text().toStdString();
	core.QuickKey(cs, core.GetConfigData().sM17SourceCallsign);
}

void CMainWindow::DrainLogQueue()
{
	std::string msg;
	while (core.logQueue.TryPop(msg)) {
		pTextDisplay->moveCursor(QTextCursor::End);
		pTextDisplay->insertPlainText(QString::fromStdString(msg));
		pTextDisplay->moveCursor(QTextCursor::End);
	}
}

void CMainWindow::insertLogText(const char *line)
{
	core.logQueue.Push(std::string(line));
}

void CMainWindow::UpdateGUI()
{
	DrainLogQueue();
	auto &cfgdata = core.GetConfigData();
	if (cfgdata.sM17SourceCallsign.empty())
	{
		pPTTButton->setEnabled(false);
		pQuickKeyButton->setEnabled(false);
		pConnectButton->setEnabled(false);
	}
	else
	{
		bool isLegacy = pIsLegacyCheck->isChecked();
		std::string target = pTargetCSInput->text().toStdString();
		const auto linkState = core.GetLinkState();
		switch (linkState)
		{
			case ELinkState::unlinked:
				pDisconnectButton->setEnabled(false);
				pConnectButton->setEnabled(std::regex_match(target, core.ReflTarRegEx) && bTargetIP && bTargetPort);
				pTargetCSInput->setEnabled(true);
				pIsLegacyCheck->setEnabled(true);
				pTargetIpInput->setEnabled(true);
				pTargetPortInput->setEnabled(true);
				pDSTCallsignInput->setEnabled(true);
				break;
			case ELinkState::linking:
				pDisconnectButton->setEnabled(false);
				pConnectButton->setEnabled(false);
				if (isLegacy) pDSTCallsignInput->setEnabled(false);
				break;
			case ELinkState::linked:
				pDisconnectButton->setEnabled(true);
				pConnectButton->setEnabled(false);
				pTargetCSInput->setEnabled(false);
				pIsLegacyCheck->setEnabled(false);
				pTargetIpInput->setEnabled(false);
				pTargetPortInput->setEnabled(false);
				if (isLegacy)
				{
					pDSTCallsignInput->setEnabled(false);
					pSMSDlg->hide();
				}
				break;
		}
		pPTTButton->UpdateLabel();
		pEchoTestButton->UpdateLabel();
		TransmitterButtonControl();
		if (core.IsTransmitOK())
		{
			auto host = core.GetRouteMap().Find(target);
			if (host)
			{
				if (host->updated)
				{
					if (EInternetType::ipv4only != cfgdata.eNetType && !host->ip6addr.empty())
						pTargetIpInput->setText(QString::fromStdString(host->ip6addr));
					else
						pTargetIpInput->setText(QString::fromStdString(host->ip4addr));
					TargetIPInput();
					pIsLegacyCheck->setChecked(host->is_legacy);
					pTargetPortInput->setText(QString::number(host->port));
					TargetPortInput();
					host->updated = false;
				}

				if (ELinkState::unlinked != linkState)
				{
					ActivateModules("#"); // this will turnoff all modules
				}
				else
				{
					if (host->mods.size())
						ActivateModules(host->mods);
					else
						ActivateModules();
				}
			}
		}
	}
}

void CMainWindow::DestinationCSInput()
{
	auto pos = pDSTCallsignInput->cursorPosition();
	std::string dest = pDSTCallsignInput->text().toStdString();
	// Convert to uppercase
	if (core.ToUpper(dest))
	{
		pDSTCallsignInput->setText(QString::fromStdString(dest));
		pDSTCallsignInput->setCursorPosition(pos);
	}
	
	// the destination either has to be @ALL, PARROT or a legal callsign
	bDestCS = dest == "@ALL" || std::regex_match(dest, core.ReflDstRegEx) || dest == "#PARROT" || std::regex_match(dest, core.M17CallRegEx);

	QPalette pal = pDSTCallsignInput->palette();
	pal.setColor(QPalette::Base, bDestCS ? QColor(Qt::green) : QColor(Qt::red));
	pDSTCallsignInput->setPalette(pal);
}

void CMainWindow::TargetCSInput()
{
	auto pos = pTargetCSInput->cursorPosition();
	std::string dest = pTargetCSInput->text().toStdString();
	// Convert to uppercase
	if (core.ToUpper(dest))
	{
		pTargetCSInput->setText(QString::fromStdString(dest));
		pTargetCSInput->setCursorPosition(pos);
	}
	
	// the target either has to be a reflector or a legal callsign
	bTargetCS = std::regex_match(dest, core.M17CallRegEx) || std::regex_match(dest, core.ReflTarRegEx);

	if (bTargetCS)
	{
		auto &cfgdata = core.GetConfigData();
		auto host = core.GetRouteMap().Find(dest); // is it already in the routeMap?
		if (host)
		{
#ifndef NO_DHT
			core.Get(host->cs);
#endif
			// let's try to come up with a destination IP
			if (EInternetType::ipv4only != cfgdata.eNetType && !host->ip6addr.empty())
				// if we aren't in IPv4-only mode and there is an IPv6 address, use it
				pTargetIpInput->setText(QString::fromStdString(host->ip6addr));
			else if (EInternetType::ipv6only != cfgdata.eNetType && !host->ip4addr.empty())
				// otherwise, if there is an IPv4 adddress, use it
				pTargetIpInput->setText(QString::fromStdString(host->ip4addr));
			else if (!host->dn.empty()) // if there is a domain name specified
			{
				// then we'll try to resolve the domain name to a preferred IP address
				struct addrinfo *res, hints;
				memset(&hints, 0, sizeof hints);
				switch (cfgdata.eNetType) {
					case EInternetType::ipv4only: hints.ai_family = AF_INET; break;
					case EInternetType::ipv6only: hints.ai_family = AF_INET6; break;
					default: hints.ai_family = AF_UNSPEC; break;
				}
				hints.ai_socktype = SOCK_DGRAM;

				int status = getaddrinfo(host->dn.c_str(), std::to_string(host->port).c_str(), &hints, &res);
				if (status) {
					insertLogText(gai_strerror(status));
					insertLogText("\n");
				} else {
					if (res) {
						void *addr = nullptr;
						// get the pointer to the address itself
						// different fields in IPv4 and IPv6
						if (res->ai_family == AF_INET) {
							struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
							addr = &(ipv4->sin_addr);
						} else if (res->ai_family == AF_INET6) {
							struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)res->ai_addr;
							addr = &(ipv6->sin6_addr);
						}
						if (addr) {
							char ipstr[INET6_ADDRSTRLEN];
							// convert the IP to a string and print it:
							inet_ntop(res->ai_family, addr, ipstr, sizeof ipstr);
							pTargetIpInput->setText(QString(ipstr));
							std::string logmsg("Resolved domain name ");
							logmsg.append(host->dn + " to " + ipstr + "\n");
							insertLogText(logmsg.c_str());
						}
					}
					freeaddrinfo(res); // free the linked list
				}
			}

			pTargetPortInput->setText(QString::number(host->port));
			
			// activate the configure modules
			// if there aren't any confgured modules, activate all modules
			if (host->mods.size())
				ActivateModules(host->mods);
			else
				ActivateModules();

			if (!host->url.empty())
				pDashboardButton->setEnabled(true);
			else
				pDashboardButton->setEnabled(false);
		}
		else
		{
			ActivateModules();
#ifndef NO_DHT
			core.Get(dest);
#endif
			core.SetState();
			BuildTargetMenuButton();
		}
	}
	else
	{
		// bTargetCS is false
		pTargetIpInput->clear();
		pTargetPortInput->clear();
		pIsLegacyCheck->setChecked(false);
	}

	TargetIPInput();
	QPalette pal = pTargetCSInput->palette();
	pal.setColor(QPalette::Base, bTargetCS ? QColor(Qt::green) : QColor(Qt::red));
	pTargetCSInput->setPalette(pal);

	TargetPortInput();
}

void CMainWindow::TargetIPInput()
{
	auto &cfgdata = core.GetConfigData();
	auto ipText = pTargetIpInput->text().toStdString();
	auto bIP4 = std::regex_match(ipText, core.IPv4RegEx);
	auto bIP6 = std::regex_match(ipText, core.IPv6RegEx);
	switch (cfgdata.eNetType) {
		case EInternetType::ipv4only: bTargetIP = bIP4; break;
		case EInternetType::ipv6only: bTargetIP = bIP6; break;
		default: bTargetIP = (bIP4 || bIP6); break;
	}

	QPalette pal = pTargetIpInput->palette();
	pal.setColor(QPalette::Base, bTargetIP ? QColor(Qt::green) : QColor(Qt::red));
	pTargetIpInput->setPalette(pal);
	FixTargetMenuButton();
}

void CMainWindow::TargetPortInput()
{
	auto port = pTargetPortInput->text().toInt();
	bTargetPort = (port > 1023 && port < 49000);

	QPalette pal = pTargetPortInput->palette();
	pal.setColor(QPalette::Base, bTargetPort ? QColor(Qt::green) : QColor(Qt::red));
	pTargetPortInput->setPalette(pal);
	FixTargetMenuButton();
}

void CMainWindow::SetTargetMenuButton(const QString &label)
{
	if (!label.isEmpty())
		pActionButton->setEnabled(true);
	else
		pActionButton->setEnabled(false);
	pActionButton->setText(label.isEmpty() ? tr("Action") : label);
}

void CMainWindow::LinkButton()
{
	auto &cfgdata = core.GetConfigData();
	if (cfgdata.sM17SourceCallsign.empty()) {
		insertLogText(tr("ERROR: Your system is not yet configured!\n").toUtf8().constData());
		insertLogText(tr("Be sure to save your callsign and internet access in Settings\n").toUtf8().constData());
		insertLogText(tr("You can usually leave the audio devices as 'default'\n").toUtf8().constData());
	}
	else
	{
		std::string cmd("M17L");
		std::string cs;
		SetTargetAddress(cs);
		cmd.append(cs);
		core.Link(cmd);
	}
}

void CMainWindow::UnlinkButton()
{
	std::string cmd("M17U");
	core.Link(cmd);
	pDSTCallsignInput->setText("@ALL");
}

void CMainWindow::DashboardButton()
{
	auto cs = pTargetCSInput->text().toStdString();
	auto host = core.GetRouteMap().Find(cs);
	if (host && !host->url.empty()) {
		QDesktopServices::openUrl(QUrl(QString::fromStdString(host->url)));
	}
}

void CMainWindow::FixTargetMenuButton()
{
	static const QString saveStr = tr("Save");
	static const QString deleteStr = tr("Delete");
	static const QString updateStr = tr("Update");

	if (bTargetCS) { // is the destination c/s valid?
		const std::string cs = pTargetCSInput->text().toStdString();
		auto host = core.GetRouteMap().Find(cs); // look for it
		if (host) {
			// cs is found in map
			if (bTargetIP && bTargetPort && host->mods.empty()) { // is the IP and port okay and is this not from the csv file?
				const std::string ip = pTargetIpInput->text().toStdString();
				const std::string port = pTargetPortInput->text().toStdString();
				if ((ip != host->ip4addr && ip != host->ip6addr) || port != std::to_string(host->port) || pIsLegacyCheck->isChecked() != host->is_legacy) {
					// the ip in the IPEntry is different, or the port is different
					SetTargetMenuButton(updateStr);
				} else {
					// perfect match
					if (EFrom::user != host->from)
						SetTargetMenuButton();
					else
						SetTargetMenuButton(deleteStr);
				}
			} else {
				SetTargetMenuButton();
			}
		} else {
			// cs is not found in map
			if (bTargetIP && bTargetPort) { // is the IP okay and is the not from the csv file?
				SetTargetMenuButton(saveStr);
			}
			else {
				SetTargetMenuButton();
			}
		}
	} else {
		SetTargetMenuButton();
	}
	TransmitterButtonControl();
}

void CMainWindow::TransmitterButtonControl()
{
	DestinationCSInput();
	if (core.IsTransmitOK() && bDestCS && bTargetCS && bTargetIP && bTargetPort && !pConnectButton->isEnabled())
	{
		pPTTButton->setEnabled(true);
		pQuickKeyButton->setEnabled(true);
		pSMSDlg->UpdateSMS(true);
	}
	else
	{
		pPTTButton->setEnabled(false);
		pQuickKeyButton->setEnabled(false);
		pSMSDlg->UpdateSMS(false);
	}
}

char CMainWindow::GetTargetModule()
{
	for (int i = 0; i < 26; i++) {
		if (pModuleRadioButton[i]->isChecked())
			return 'A' + i;
	}
	return '!'; // ERROR!
}

#define MKDIR(PATH) ::mkdir(PATH, 0755)

static bool do_mkdir(const std::string &path)
{
	struct stat st;
	if (::stat(path.c_str(), &st) != 0) {
		if (MKDIR(path.c_str()) != 0 && errno != EEXIST)
			return false;
	} else if (!S_ISDIR(st.st_mode)) {
		errno = ENOTDIR;
		return false;
	}
	return true;
}

static void mkpath(std::string path)
{
	std::string build;
	for (size_t pos = 0; (pos = path.find('/')) != std::string::npos; ) {
		build += path.substr(0, pos + 1);
		do_mkdir(build);
		path.erase(0, pos + 1);
	}
	if (!path.empty()) {
		build += path;
		do_mkdir(build);
	}
}

int main(int argc, char **argv)
{
	QApplication app(argc, argv);
	
	// make the user's config directory
	auto home = getenv("HOME");
	if (home)
	{
		if (chdir(home))
		{
			std::cerr << "ERROR: Can't cd to '" << home << "': " << strerror(errno) << std::endl;
			return EXIT_FAILURE;
		}
		mkpath(CFGDIR);
	}
	else
	{
		std::cerr << "ERROR: HOME environmental variable not found" << std::endl;
		return EXIT_FAILURE;
	}

	CMainWindow mainWindow;

	if (mainWindow.Init()) {
		return 1;
	}

	mainWindow.show();
	return app.exec();
}
