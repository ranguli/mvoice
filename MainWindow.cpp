/*
 *   Copyright (c) 2019-2021 by Thomas A. Early N7TAE
 *   Copyright (c) 2026 by Joshua Murphy VO1RFX
 *
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

#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "AppCore.h"
#include "UiController.h"
#include "SettingsController.h"
#include "AppIconProvider.h"

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
	QGuiApplication app(argc, argv);

	auto home = getenv("HOME");
	if (home) {
		if (chdir(home) != 0) {
			std::cerr << "ERROR: Can't cd to '" << home << "': " << strerror(errno) << std::endl;
			return EXIT_FAILURE;
		}
		mkpath(CFGDIR);
	} else {
		std::cerr << "ERROR: HOME environment variable not found" << std::endl;
		return EXIT_FAILURE;
	}

	CAppCore core;
	if (core.Init()) {
		return 1;
	}
	core.SetState();

#ifndef NO_DHT
	core.InitDHT();
#endif

	UiController uiController(&core);
	SettingsController settingsController(&core);
	QObject::connect(&settingsController, &SettingsController::settingsApplied,
	                 &uiController, [&uiController]() { uiController.refreshTargets(); });

	QQmlApplicationEngine engine;
	engine.addImageProvider(QStringLiteral("appicon"), new AppIconProvider);
	engine.rootContext()->setContextProperty(QStringLiteral("uiController"), &uiController);
	engine.rootContext()->setContextProperty(QStringLiteral("settingsController"), &settingsController);
	engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
	if (engine.rootObjects().isEmpty())
		return EXIT_FAILURE;
	return app.exec();
}
