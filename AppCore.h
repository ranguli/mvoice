/*
 *   Copyright (c) 2021-2025 Thomas A. Early N7TAE
 *   Copyright (c) 2026 Joshua Murphy VO1RFX
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

#include <regex>
#include <future>
#include <atomic>
#include <functional>
#include <string>
#include <memory>

#include "Configure.h"
#include "M17Gateway.h"
#include "M17RouteMap.h"
#include "AudioManager.h"
#include "TemplateClasses.h"

#ifndef NO_DHT
struct SDHTState;
#endif

class CAppCore
{
public:
	CAppCore();
	~CAppCore();

	bool Init();
	void Shutdown();

	void SetState();
	void ApplyNewSettings(CFGDATA *newdata);
	void OnReceive(bool is_rx);

	bool SendMessage(const std::string &dst, const std::string &message);
	void SetDestAddress(const std::string &ip, uint16_t port);
	void StopM17();

	void RecordMic(E_PTT_Type for_who, const std::string &urcall);
	void PlayEchoData();
	void KeyOff();
	void QuickKey(const std::string &dest, const std::string &sour);
	void Link(const std::string &linkcmd);
	void BuildMetaBlocks();
	bool TryLockGateway();
	void ReleaseGatewayLock();

	ELinkState GetLinkState() const;
	bool IsTransmitOK() const { return bTransOK; }
	void SetTransmitOK(bool ok) { bTransOK = ok; }
	CM17RouteMap &GetRouteMap() { return routeMap; }
	CConfigure &GetConfig() { return cfg; }
	const CFGDATA &GetConfigData() const { return cfgdata; }
	const SVolStats &GetVolStats() const;
	bool IsConfigOkay();

	bool ToUpper(std::string &s);
	std::string FormatAudioSummary(const char *title);

	std::regex IPv4RegEx, IPv6RegEx, M17CallRegEx, ReflTarRegEx, ReflDstRegEx;

	CTQueue<std::string> logQueue;

	std::function<void(bool)> onReceiveStateChanged;

#ifndef NO_DHT
	bool InitDHT();
	void ShutdownDHT();
	void Get(const std::string &cs);
#endif

private:
	CConfigure cfg;
	CAudioManager audioManager;
	CM17Gateway gateM17;
	CM17RouteMap routeMap;
	CFGDATA cfgdata;

#ifndef NO_DHT
	std::unique_ptr<SDHTState> dht;
#endif

	CUnixDgramReader M172AM;

	std::future<void> futM17;
	std::future<void> futReadThread;
	std::atomic<bool> keep_running;

	bool bTransOK;

	void RunM17();
	void ReadThread();
	void CloseAll();
};
