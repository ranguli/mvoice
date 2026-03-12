/*
 *   Copyright (c) 2025 by Thomas A. Early N7TAE
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

#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <regex>

#include "SMSDlg.h"
#include "AppCore.h"
#include "Utilities.h"

CSMSDlg::CSMSDlg(CAppCore *core, QWidget *parent)
	: QDialog(parent), pCore(core), bDestCS(false)
{
	setWindowTitle(tr("SMS Texting"));
	resize(600, 240);

	auto *mainLayout = new QVBoxLayout(this);

	auto *topRow = new QHBoxLayout;
	topRow->addWidget(new QLabel(tr("Destination Callsign:")));
	pDSTCallsignInput = new QLineEdit("@ALL");
	pDSTCallsignInput->setToolTip(tr("Packet Mode destination callsign"));
	pDSTCallsignInput->setMaxLength(10);
	topRow->addWidget(pDSTCallsignInput);

	pSendButton = new QPushButton(tr("Send"));
	pSendButton->setToolTip(tr("Send this message"));
	pSendButton->setEnabled(false);
	topRow->addWidget(pSendButton);

	pClearButton = new QPushButton(tr("Clear"));
	pClearButton->setToolTip(tr("Clear the outgoing message"));
	pClearButton->setEnabled(false);
	topRow->addWidget(pClearButton);

	mainLayout->addLayout(topRow);

	pMessage = new QTextEdit;
	pMessage->setToolTip(tr("Message to send"));
	mainLayout->addWidget(pMessage);

	connect(pDSTCallsignInput, &QLineEdit::textChanged, this, &CSMSDlg::DestinationCSInput);
	connect(pSendButton, &QPushButton::clicked, this, &CSMSDlg::SendButton);
	connect(pClearButton, &QPushButton::clicked, this, &CSMSDlg::ClearButton);
}

void CSMSDlg::DestinationCSInput()
{
	auto pos = pDSTCallsignInput->cursorPosition();
	std::string dest = pDSTCallsignInput->text().toStdString();
	if (pCore->ToUpper(dest))
	{
		pDSTCallsignInput->setText(QString::fromStdString(dest));
		pDSTCallsignInput->setCursorPosition(pos);
	}

	bDestCS = dest == "@ALL" || dest == "#PARROT" || std::regex_match(dest, pCore->M17CallRegEx);

	QPalette pal = pDSTCallsignInput->palette();
	pal.setColor(QPalette::Base, bDestCS ? QColor(Qt::green) : QColor(Qt::red));
	pDSTCallsignInput->setPalette(pal);
}

void CSMSDlg::SendButton()
{
	const std::string dst = pDSTCallsignInput->text().toStdString();
	std::string msg = pMessage->toPlainText().toStdString();
	trim(msg);
	if (pCore->SendMessage(dst, msg))
		ClearButton();
}

void CSMSDlg::ClearButton()
{
	pMessage->clear();
}

void CSMSDlg::UpdateSMS(bool cansend)
{
	DestinationCSInput();
	if (!pMessage->toPlainText().isEmpty())
	{
		pClearButton->setEnabled(true);
		pSendButton->setEnabled(bDestCS && cansend);
	}
	else
	{
		pClearButton->setEnabled(false);
		pSendButton->setEnabled(false);
	}
}
