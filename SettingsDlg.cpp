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

#include <iostream>
#include <sstream>
#include <iomanip>
#include <alsa/asoundlib.h>

#include <QTabWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QRadioButton>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QButtonGroup>
#include <QMessageBox>

#include "SettingsDlg.h"
#include "MainWindow.h"

CSettingsDlg::CSettingsDlg(CMainWindow *pMain, QWidget *parent)
	: QDialog(parent), bM17Source(false), bLatitude(false), bLongitude(false), pMainWindow(pMain)
{
	LatRegEx = std::regex("^[+-]?(90|([1-8]?[0-9](\\.[0-9]*)?))$", std::regex::extended);
	LongRegEx = std::regex("^[+-]?(180|(((1[0-7])|[1-9]?)[0-9](\\.[0-9]*)?))$", std::regex::extended);

	setWindowTitle(tr("Settings"));
	setModal(true);
	resize(450, 350);

	auto *mainLayout = new QVBoxLayout(this);

	pTabs = new QTabWidget;
	mainLayout->addWidget(pTabs);

	// ---- Station Tab ----
	auto *stationWidget = new QWidget;
	auto *stationLayout = new QVBoxLayout(stationWidget);

	auto *callRow = new QHBoxLayout;
	callRow->addWidget(new QLabel(tr("My Callsign:")));
	pSourceCallsignInput = new QLineEdit;
	pSourceCallsignInput->setToolTip(tr("Input your callsign, up to 8 characters"));
	pSourceCallsignInput->setMaxLength(8);
	callRow->addWidget(pSourceCallsignInput);

	callRow->addWidget(new QLabel(tr("Module:")));
	pModuleChoice = new QComboBox;
	pModuleChoice->setToolTip(tr("Assign the transceiver module"));
	for (char c = 'A'; c <= 'Z'; c++)
		pModuleChoice->addItem(QString(QChar(c)));
	callRow->addWidget(pModuleChoice);
	stationLayout->addLayout(callRow);

	auto *codecGroup = new QGroupBox(tr("Codec"));
	auto *codecLayout = new QHBoxLayout(codecGroup);
	pVoiceOnlyRadioButton = new QRadioButton(tr("Voice-only"));
	pVoiceOnlyRadioButton->setToolTip(tr("This is the higher quality, 3200 bits/s codec"));
	pVoiceDataRadioButton = new QRadioButton(tr("Voice+Data"));
	pVoiceDataRadioButton->setToolTip(tr("This is the 1600 bits/s codec"));
	codecLayout->addWidget(pVoiceOnlyRadioButton);
	codecLayout->addWidget(pVoiceDataRadioButton);
	stationLayout->addWidget(codecGroup);

	auto *coordRow = new QHBoxLayout;
	coordRow->addWidget(new QLabel(tr("Latitude:")));
	pLatitudeInput = new QLineEdit("0.0");
	pLatitudeInput->setToolTip(tr("North is +, South is -"));
	coordRow->addWidget(pLatitudeInput);
	coordRow->addWidget(new QLabel(tr("Longitude:")));
	pLongitudeInput = new QLineEdit("0.0");
	pLongitudeInput->setToolTip(tr("East is +, West is -"));
	coordRow->addWidget(pLongitudeInput);
	stationLayout->addLayout(coordRow);

	auto *msgRow = new QHBoxLayout;
	msgRow->addWidget(new QLabel(tr("Message:")));
	pTextMessageInput = new QLineEdit;
	pTextMessageInput->setToolTip(tr("Up to a 52 character text message"));
	pTextMessageInput->setMaxLength(52);
	msgRow->addWidget(pTextMessageInput);
	stationLayout->addLayout(msgRow);

	stationLayout->addStretch();
	pTabs->addTab(stationWidget, tr("Station"));

	// ---- Network Tab ----
	auto *netWidget = new QWidget;
	auto *netLayout = new QVBoxLayout(netWidget);
	netLayout->addSpacing(20);
	pIPv4RadioButton = new QRadioButton(tr("IPv4 Only"));
	pIPv6RadioButton = new QRadioButton(tr("IPv6 Only"));
	pDualStackRadioButton = new QRadioButton(tr("IPv4 && IPv6"));
	netLayout->addWidget(pIPv4RadioButton);
	netLayout->addWidget(pIPv6RadioButton);
	netLayout->addWidget(pDualStackRadioButton);
	netLayout->addStretch();
	pTabs->addTab(netWidget, tr("Network"));

	// ---- DHT Tab ----
#ifndef NO_DHT
	auto *dhtWidget = new QWidget;
	auto *dhtLayout = new QFormLayout(dhtWidget);
	pBootstrapInput = new QLineEdit;
	pBootstrapInput->setToolTip(tr("An existing node on the DHT Network"));
	dhtLayout->addRow(tr("DHT Bootstrap:"), pBootstrapInput);
	pTabs->addTab(dhtWidget, tr("DHT"));
#endif

	// ---- Audio Tab ----
	auto *audioWidget = new QWidget;
	auto *audioLayout = new QVBoxLayout(audioWidget);

	auto *inRow = new QHBoxLayout;
	inRow->addWidget(new QLabel(tr("Input:")));
	pAudioInputChoice = new QComboBox;
	pAudioInputChoice->setToolTip(tr("Select your audio input device, usually \"default\""));
	inRow->addWidget(pAudioInputChoice);
	audioLayout->addLayout(inRow);

	pAudioInputDescBox = new QLabel(tr("input description"));
	pAudioInputDescBox->setAlignment(Qt::AlignCenter);
	audioLayout->addWidget(pAudioInputDescBox);

	auto *outRow = new QHBoxLayout;
	outRow->addWidget(new QLabel(tr("Output:")));
	pAudioOutputChoice = new QComboBox;
	pAudioOutputChoice->setToolTip(tr("Select the audio output device, usually \"default\""));
	outRow->addWidget(pAudioOutputChoice);
	audioLayout->addLayout(outRow);

	pAudioOutputDescBox = new QLabel(tr("output description"));
	pAudioOutputDescBox->setAlignment(Qt::AlignCenter);
	audioLayout->addWidget(pAudioOutputDescBox);

	pAudioRescanButton = new QPushButton(tr("Rescan"));
	pAudioRescanButton->setToolTip(tr("Rescan for new audio devices"));
	audioLayout->addWidget(pAudioRescanButton, 0, Qt::AlignCenter);

	audioLayout->addStretch();
	pTabs->addTab(audioWidget, tr("Audio"));

	// ---- Update button ----
	pOkayButton = new QPushButton(tr("Update"));
	pOkayButton->setDefault(true);
	mainLayout->addWidget(pOkayButton, 0, Qt::AlignRight);

	// Connections
	connect(pSourceCallsignInput, &QLineEdit::textChanged, this, &CSettingsDlg::SourceCallsignInput);
	connect(pLatitudeInput, &QLineEdit::textChanged, this, &CSettingsDlg::LatitudeInput);
	connect(pLongitudeInput, &QLineEdit::textChanged, this, &CSettingsDlg::LongitudeInput);
	connect(pTextMessageInput, &QLineEdit::textChanged, this, &CSettingsDlg::TextMessageInput);
	connect(pModuleChoice, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CSettingsDlg::ModuleChoice);
	connect(pAudioInputChoice, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CSettingsDlg::AudioInputChoice);
	connect(pAudioOutputChoice, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CSettingsDlg::AudioOutputChoice);
	connect(pAudioRescanButton, &QPushButton::clicked, this, &CSettingsDlg::AudioRescanButton);
	connect(pOkayButton, &QPushButton::clicked, this, &CSettingsDlg::UpdateButton);
}

void CSettingsDlg::Refresh()
{
	pMainWindow->core.GetConfig().CopyTo(data);
	SetWidgetStates(data);
	pTabs->setCurrentIndex(0);
	AudioRescanButton();
}

void CSettingsDlg::UpdateButton()
{
	hide();
	CFGDATA newstate;						// the user clicked okay, time to look at what's changed
	SaveWidgetStates(newstate);				// newstate is now the current contents of the Settings Dialog
#ifndef NO_DHT
	if (newstate.sBootstrap.compare(pMainWindow->core.GetConfig().GetData()->sBootstrap))
	{
		QMessageBox::information(this, tr("DHT"), tr("Please restart to use new bootstrap"));
	}
#endif
	pMainWindow->core.GetConfig().CopyFrom(newstate);
	pMainWindow->core.GetConfig().WriteData();
	pMainWindow->core.GetConfig().CopyTo(data);
	pMainWindow->NewSettings(&newstate);
}

void CSettingsDlg::SaveWidgetStates(CFGDATA &d)
{
	d.sM17SourceCallsign = pSourceCallsignInput->text().toStdString();
	d.bVoiceOnlyEnable = pVoiceOnlyRadioButton->isChecked();
	d.cModule = data.cModule;
	d.dLatitude = pLatitudeInput->text().toDouble();
	d.dLongitude = pLongitudeInput->text().toDouble();
	d.sMessage = pTextMessageInput->text().toStdString();

	if (pIPv4RadioButton->isChecked())
		d.eNetType = EInternetType::ipv4only;
	else if (pIPv6RadioButton->isChecked())
		d.eNetType = EInternetType::ipv6only;
	else
		d.eNetType = EInternetType::dualstack;

	auto inText = pAudioInputChoice->currentText().toStdString();
	auto itin = AudioInMap.find(inText);
	if (itin != AudioInMap.end())
		d.sAudioIn = itin->second.first;

	auto outText = pAudioOutputChoice->currentText().toStdString();
	auto itout = AudioOutMap.find(outText);
	if (itout != AudioOutMap.end())
		d.sAudioOut = itout->second.first;

#ifndef NO_DHT
	d.sBootstrap = pBootstrapInput->text().toStdString();
#endif
}

void CSettingsDlg::SetWidgetStates(const CFGDATA &d)
{
	if (d.bVoiceOnlyEnable)
		pVoiceOnlyRadioButton->setChecked(true);
	else
		pVoiceDataRadioButton->setChecked(true);

	pSourceCallsignInput->setText(QString::fromStdString(d.sM17SourceCallsign));
	SourceCallsignInput();
	LatitudeInput();
	LongitudeInput();
	TextMessageInput();
	pModuleChoice->setCurrentIndex(d.cModule - 'A');

	std::stringstream ss;
	ss << std::fixed << std::setprecision(5) << d.dLatitude;
	pLatitudeInput->setText(QString::fromStdString(ss.str()));
	ss.str("");
	ss << std::fixed << std::setprecision(5) << d.dLongitude;
	pLongitudeInput->setText(QString::fromStdString(ss.str()));

	pTextMessageInput->setText(QString::fromStdString(d.sMessage));

#ifndef NO_DHT
	pBootstrapInput->setText(QString::fromStdString(d.sBootstrap));
#endif

	switch (d.eNetType) {
		case EInternetType::ipv6only:  pIPv6RadioButton->setChecked(true); break;
		case EInternetType::dualstack: pDualStackRadioButton->setChecked(true); break;
		default:                       pIPv4RadioButton->setChecked(true); break;
	}
}

void CSettingsDlg::ModuleChoice(int index)
{
	data.cModule = 'A' + index;
}

void CSettingsDlg::SourceCallsignInput()
{
	auto pos = pSourceCallsignInput->cursorPosition();
	std::string s = pSourceCallsignInput->text().toStdString();
	if (pMainWindow->core.ToUpper(s))
	{
		pSourceCallsignInput->setText(QString::fromStdString(s));
		pSourceCallsignInput->setCursorPosition(pos);
	}
	bM17Source = std::regex_match(s, pMainWindow->core.M17CallRegEx);

	QPalette pal = pSourceCallsignInput->palette();
	pal.setColor(QPalette::Base, bM17Source ? QColor(Qt::green) : QColor(Qt::red));
	pSourceCallsignInput->setPalette(pal);
	SetOkayButton();
}

void CSettingsDlg::SetOkayButton()
{
	pOkayButton->setEnabled(bM17Source && bLatitude && bLongitude);
}

void CSettingsDlg::AudioInputChoice(int)
{
	auto selected = pAudioInputChoice->currentText().toStdString();
	auto it = AudioInMap.find(selected);
	if (it == AudioInMap.end())
	{
		data.sAudioIn = "ERROR";
		pAudioInputDescBox->setText(QString::fromStdString(selected + tr(" not found").toStdString()));
	}
	else
	{
		data.sAudioIn = it->second.first;
		pAudioInputDescBox->setText(QString::fromStdString(it->second.second));
	}
}

void CSettingsDlg::LatitudeInput()
{
	std::string str = pLatitudeInput->text().toStdString();
	bLatitude = std::regex_match(str, LatRegEx);

	QPalette pal = pLatitudeInput->palette();
	pal.setColor(QPalette::Base, bLatitude ? QColor(Qt::green) : QColor(Qt::red));
	pLatitudeInput->setPalette(pal);
	SetOkayButton();
}

void CSettingsDlg::LongitudeInput()
{
	std::string str = pLongitudeInput->text().toStdString();
	bLongitude = std::regex_match(str, LongRegEx);

	QPalette pal = pLongitudeInput->palette();
	pal.setColor(QPalette::Base, bLongitude ? QColor(Qt::green) : QColor(Qt::red));
	pLongitudeInput->setPalette(pal);
	SetOkayButton();
}

void CSettingsDlg::AudioOutputChoice(int)
{
	auto selected = pAudioOutputChoice->currentText().toStdString();
	auto it = AudioOutMap.find(selected);
	if (it == AudioOutMap.end())
	{
		data.sAudioOut = "ERROR";
		pAudioOutputDescBox->setText(QString::fromStdString(selected + tr(" not found").toStdString()));
	}
	else
	{
		data.sAudioOut = it->second.first;
		pAudioOutputDescBox->setText(QString::fromStdString(it->second.second));
	}
}

void CSettingsDlg::TextMessageInput()
{
	// Max length enforced by QLineEdit::setMaxLength
}

void CSettingsDlg::AudioRescanButton()
{
	auto inchoice = pAudioInputChoice->currentIndex();
	if (inchoice < 0) inchoice = 0;
	auto outchoice = pAudioOutputChoice->currentIndex();
	if (outchoice < 0) outchoice = 0;

	void **hints;
	if (snd_device_name_hint(-1, "pcm", &hints) < 0)
		return;

	pAudioInputChoice->clear();
	pAudioOutputChoice->clear();
	AudioInMap.clear();
	AudioOutMap.clear();

	for (void **n = hints; *n != nullptr; n++)
	{
		char *name = snd_device_name_get_hint(*n, "NAME");
		if (!name) continue;
		char *desc = snd_device_name_get_hint(*n, "DESC");
		if (!desc) { free(name); continue; }

		if ((strcmp(name, "default") == 0 || strstr(name, "plughw")) && !strstr(desc, "without any conversions"))
		{
			char *io = snd_device_name_get_hint(*n, "IOID");
			bool is_input = true, is_output = true;
			if (io)
			{
				if (strcasecmp(io, "Input") == 0) is_output = false;
				else if (strcasecmp(io, "Output") == 0) is_input = false;
				free(io);
			}

			std::string short_name(name);
			auto pos = short_name.find("plughw:CARD=");
			if (pos != std::string::npos)
			{
				short_name = short_name.replace(pos, 12, "");
				auto dpos = short_name.find(",DEV=0");
				if (dpos != std::string::npos)
					short_name = short_name.replace(dpos, 6, "");
				if (short_name.empty())
					short_name.assign(name);
			}

			if (is_input)
			{
				snd_pcm_t *handle;
				if (snd_pcm_open(&handle, name, SND_PCM_STREAM_CAPTURE, 0) == 0)
				{
					AudioInMap[short_name] = {name, desc};
					snd_pcm_close(handle);
					pAudioInputChoice->addItem(QString::fromStdString(short_name));
				}
			}
			if (is_output)
			{
				snd_pcm_t *handle;
				if (snd_pcm_open(&handle, name, SND_PCM_STREAM_PLAYBACK, 0) == 0)
				{
					AudioOutMap[short_name] = {name, desc};
					snd_pcm_close(handle);
					pAudioOutputChoice->addItem(QString::fromStdString(short_name));
				}
			}
		}

		free(name);
		free(desc);
	}
	snd_device_name_free_hint(hints);

	pAudioInputChoice->setCurrentIndex(inchoice);
	AudioInputChoice(inchoice);
	pAudioOutputChoice->setCurrentIndex(outchoice);
	AudioOutputChoice(outchoice);
}
