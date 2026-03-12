/*
 *   Copyright (c) 2022 by Thomas A. Early N7TAE
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

#include <QLabel>
#include <QVBoxLayout>
#include <QImage>
#include <QPixmap>
#include "AboutDlg.h"
#include "IconData.h"

#define VERSION "1.4.1"

CAboutDlg::CAboutDlg(QWidget *parent)
	: QDialog(parent)
{
	setWindowTitle(tr("About MVoice"));
	setFixedSize(400, 200);

	auto *layout = new QVBoxLayout(this);
	layout->setAlignment(Qt::AlignCenter);

	QImage img(icon_image.pixel_data, icon_image.width, icon_image.height, QImage::Format_RGBA8888);
	auto *iconLabel = new QLabel;
	iconLabel->setPixmap(QPixmap::fromImage(img));
	iconLabel->setAlignment(Qt::AlignCenter);
	layout->addWidget(iconLabel);

	auto *versionLabel = new QLabel(tr("MVoice version # %1").arg(VERSION));
	versionLabel->setAlignment(Qt::AlignCenter);
	layout->addWidget(versionLabel);

	auto *copyrightLabel = new QLabel(tr("Copyright (c) 2025 by Thomas A. Early N7TAE"));
	copyrightLabel->setAlignment(Qt::AlignCenter);
	layout->addWidget(copyrightLabel);
}
