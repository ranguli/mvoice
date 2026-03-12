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

#pragma once

#include <QDialog>

class QLineEdit;
class QPushButton;
class QTextEdit;
class CAppCore;

class CSMSDlg : public QDialog
{
	Q_OBJECT
public:
	explicit CSMSDlg(CAppCore *core, QWidget *parent = nullptr);
	void UpdateSMS(bool cansend);

private slots:
	void DestinationCSInput();
	void SendButton();
	void ClearButton();

private:
	CAppCore *pCore;
	QLineEdit *pDSTCallsignInput;
	QPushButton *pSendButton;
	QPushButton *pClearButton;
	QTextEdit *pMessage;
	bool bDestCS;
};
