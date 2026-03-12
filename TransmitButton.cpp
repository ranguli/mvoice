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

#include "TransmitButton.h"

CTransmitButton::CTransmitButton(const QString &label, QWidget *parent)
	: QPushButton(label, parent), defaultLabel(label)
{
	setCheckable(true);
}

void CTransmitButton::toggle()
{
	if (isChecked())
	{
		timer.start();
	}
	UpdateLabel();
}

void CTransmitButton::UpdateLabel()
{
	if (isChecked())
	{
		auto t = int(timer.time());
		auto s = t % 60;
		auto m = t / 60;
		setText(QString("%1:%2").arg(m).arg(s, 2, 10, QChar('0')));
	}
	else
	{
		setText(defaultLabel);
	}
}
