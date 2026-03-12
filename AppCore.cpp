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

#include <sys/select.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <cstring>
#include <thread>
#include <chrono>

#include "AppCore.h"
#include "FrameType.h"
#include "Callsign.h"

#ifndef NO_DHT
#include <opendht.h>
#include "dht-values.h"

struct SDHTState
{
	dht::DhtRunner node;
	dht::Value nodevalue;
	const std::string exportNodeFilename{"/exNodes.bin"};
};
#endif

CAppCore::CAppCore() :
	bTransOK(true)
{
	cfg.CopyTo(cfgdata);
	IPv4RegEx = std::regex("^((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])\\.){3,3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9]){1,1}$", std::regex::extended);
	IPv6RegEx = std::regex("^(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}(:[0-9a-fA-F]{1,4}){1,1}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|([0-9a-fA-F]{1,4}:){1,1}(:[0-9a-fA-F]{1,4}){1,6}|:((:[0-9a-fA-F]{1,4}){1,7}|:))$", std::regex::extended);
	ReflTarRegEx = std::regex("^(M17-|URF)[A-Z0-9]{3,3}$", std::regex::extended);
	ReflDstRegEx = std::regex("^(M17-[A-Z0-9]{3,3} [A-Z])|(URF[A-Z0-9]{3,3}  [A-Z])$", std::regex::extended);
	M17CallRegEx = std::regex("^[0-9]?[A-Z]{1,2}[0-9]{1,2}[A-Z]{1,4}([-/\\.][A-Z0-9]{1,2})? *[A-Z]?$", std::regex::extended);
}

CAppCore::~CAppCore()
{
	Shutdown();
}

void CAppCore::Shutdown()
{
#ifndef NO_DHT
	ShutdownDHT();
#endif

	if (futReadThread.valid())
	{
		keep_running = false;
		futReadThread.get();
	}
	StopM17();
}

bool CAppCore::Init()
{
	keep_running = true;
	futReadThread = std::async(std::launch::async, &CAppCore::ReadThread, this);

	if (M172AM.Open("m172am")) {
		CloseAll();
		return true;
	}

	CBase::SetLogQueue(&logQueue);

	if (audioManager.Init(
		[this]() -> const CFGDATA* { return cfg.GetData(); },
		[this](bool is_rx) { OnReceive(is_rx); }
	)) {
		CloseAll();
		return true;
	}
	audioManager.BuildMetaBlocks();

	routeMap.ReadAll();

	return false;
}

void CAppCore::CloseAll()
{
	M172AM.Close();
}

void CAppCore::RunM17()
{
	std::cout << "Starting M17 Gateway..." << std::endl;
	if (!gateM17.Init(cfgdata))
		gateM17.Process();
	std::cout << "M17 Gateway has stopped." << std::endl;
}

void CAppCore::StopM17()
{
	if (gateM17.keep_running) {
		gateM17.keep_running = false;
		futM17.get();
	}
}

void CAppCore::SetState()
{
	if (cfg.IsOkay() && !gateM17.keep_running)
		futM17 = std::async(std::launch::async, &CAppCore::RunM17, this);
}

void CAppCore::ApplyNewSettings(CFGDATA *newdata)
{
	if (newdata) {
		if (newdata->sM17SourceCallsign.compare(cfgdata.sM17SourceCallsign) || (newdata->eNetType != cfgdata.eNetType)) {
			StopM17();
		}
		bool updateMetaBlock = false;
		if ((newdata->dLatitude != cfgdata.dLatitude) || (newdata->dLongitude != cfgdata.dLongitude) || (newdata->sMessage.compare(cfgdata.sM17SourceCallsign))) {
			updateMetaBlock = true;
		}
		cfg.CopyTo(cfgdata);
		if (updateMetaBlock)
			audioManager.BuildMetaBlocks();
	}
	SetState();
}

void CAppCore::OnReceive(bool is_rx)
{
	bTransOK = !is_rx;
	if (bTransOK && audioManager.volStats.count) {
		logQueue.Push(FormatAudioSummary("RX Audio"));
	}
	if (onReceiveStateChanged)
		onReceiveStateChanged(is_rx);
}

bool CAppCore::SendMessage(const std::string &dst, const std::string &message)
{
	auto l = gateM17.TryLock();
	if (l) {
		CPacket pack;
		pack.Initialize(38u + message.length(), false);
		CCallsign cs(dst);
		cs.CodeOut(pack.GetDstAddress());
		cs.CSIn(cfgdata.sM17SourceCallsign);
		cs.CodeOut(pack.GetSrcAddress());
		CFrameType frameType(0);
		if (cfgdata.dLatitude || cfgdata.dLongitude) {
			frameType.SetMetaDataType(EMetaDatType::gnss);
			CGNSS gnss;
			gnss.SetDataStationTypes(EGnssSourceType::Client, EGnssStationType::Fixed);
			gnss.Set(cfgdata.dLatitude, cfgdata.dLongitude);
			memcpy(pack.GetMetaData(), gnss.GetData(), 14);
		}
		pack.SetFrameType(frameType.GetFrameType(EVersionType::legacy));
		pack.GetData()[34] = 0x5u;
		auto len = message.length();
		if (len > (MAX_PACKET_SIZE - 38u)) {
			logQueue.Push("Message is too long, it will be truncated.\n");
			len = MAX_PACKET_SIZE - 38u;
		}
		memcpy(pack.GetData() + 35, message.c_str(), len);
		pack.CalcCRC();
		gateM17.SendMessage(pack);
		gateM17.ReleaseLock();
		std::stringstream ss;
		ss << "Sent an SMS text msg to " << dst << ":\n" << message << "\n";
		logQueue.Push(ss.str());
	} else {
		logQueue.Push("Could not send the message because the gateway was locked!\n");
	}
	return l;
}

void CAppCore::SetDestAddress(const std::string &ip, uint16_t port)
{
	gateM17.SetDestAddress(ip, port);
}

void CAppCore::RecordMic(E_PTT_Type for_who, const std::string &urcall)
{
	audioManager.RecordMicThread(for_who, urcall);
}

void CAppCore::PlayEchoData()
{
	audioManager.PlayEchoDataThread();
}

void CAppCore::KeyOff()
{
	audioManager.KeyOff();
}

void CAppCore::QuickKey(const std::string &dest, const std::string &sour)
{
	audioManager.QuickKey(dest, sour);
}

void CAppCore::Link(const std::string &linkcmd)
{
	audioManager.Link(linkcmd);
}

void CAppCore::BuildMetaBlocks()
{
	audioManager.BuildMetaBlocks();
}

bool CAppCore::TryLockGateway()
{
	return gateM17.TryLock();
}

void CAppCore::ReleaseGatewayLock()
{
	gateM17.ReleaseLock();
}

ELinkState CAppCore::GetLinkState() const
{
	return gateM17.GetLinkState();
}

const SVolStats &CAppCore::GetVolStats() const
{
	return audioManager.volStats;
}

bool CAppCore::IsConfigOkay()
{
	return cfg.IsOkay();
}

bool CAppCore::ToUpper(std::string &s)
{
	bool rval = false;
	for (auto it = s.begin(); it != s.end(); it++) {
		if (islower(*it)) {
			rval = true;
			*it = toupper(*it);
		}
	}
	return rval;
}

std::string CAppCore::FormatAudioSummary(const char *title)
{
	char line[64];
	double t = audioManager.volStats.count * 0.000125;
	double d = 20.0 * log10(sqrt(audioManager.volStats.ss / (0.5 * audioManager.volStats.count))) - 65.0;
	double c = 100.0 * audioManager.volStats.clip / audioManager.volStats.count;
	snprintf(line, 64, "%s Time=%.1fs Vol=%.0fdB Clip=%.0f%%\n", title, t, d, c);
	return std::string(line);
}

void CAppCore::ReadThread()
{
	while (keep_running)
	{
		auto gatefd = M172AM.GetFD();
		fd_set fdset;
		FD_ZERO(&fdset);
		FD_SET(gatefd, &fdset);
		timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 100000;

		auto ret = select(gatefd + 1, &fdset, 0, 0, &tv);
		if (ret < 0)
		{
			std::cout << "M17Relay select() error - " << strerror(errno) << std::endl;
		}
		else if (ret > 0)
		{
			if (FD_ISSET(gatefd, &fdset))
			{
				CPacket pack;
				M172AM.Read(pack.GetData(), MAX_PACKET_SIZE);
				if (0 == memcmp(pack.GetCData(), "M17 ", 4))
				{
					pack.Initialize(54u, true);
					audioManager.M17_2AudioMgr(pack);
				}
			}
		}
	}
}

#ifndef NO_DHT
bool CAppCore::InitDHT()
{
	dht = std::make_unique<SDHTState>();

	std::string idstr(cfgdata.sM17SourceCallsign);
	if (idstr.empty()) {
		idstr.assign("MyNode");
		idstr.append(std::to_string(getpid()));
		std::cout << "Using " << idstr << " for identity" << std::endl;
	}
	try {
		dht->node.run(17171, dht::crypto::generateIdentity(idstr), true, 59973);
	} catch (const std::exception &e) {
		std::cout << "MVoice could not start the Ham-network! " << e.what() << std::endl;
		return true;
	}

	std::string path(CFGDIR);
	path.append(dht->exportNodeFilename);
	std::ifstream myfile(path, std::ios::binary | std::ios::ate);
	if (myfile.is_open()) {
		msgpack::unpacker pac;
		auto size = myfile.tellg();
		myfile.seekg(0, std::ios::beg);
		pac.reserve_buffer(size);
		myfile.read(pac.buffer(), size);
		pac.buffer_consumed(size);
		msgpack::object_handle oh;
		while (pac.next(oh)) {
			auto imported_nodes = oh.get().as<std::vector<dht::NodeExport>>();
			std::cout << "Importing " << imported_nodes.size() << " ham-dht nodes from " << path << std::endl;
			dht->node.bootstrap(imported_nodes);
		}
		myfile.close();
	} else if (cfgdata.sBootstrap.length()) {
		std::cout << "Bootstrapping from " << cfgdata.sBootstrap << std::endl;
		dht->node.bootstrap(cfgdata.sBootstrap, "17171");
	} else {
		std::cout << "ERROR: MVoice did not bootstrap the Ham-DHT network!" << std::endl;
	}

	return false;
}

void CAppCore::ShutdownDHT()
{
	if (!dht) return;

	auto exnodes = dht->node.exportNodes();
	if (exnodes.size() > 1) {
		std::string path(CFGDIR);
		path.append(dht->exportNodeFilename);
		std::ofstream myfile(path, std::ios::binary | std::ios::trunc);
		if (myfile.is_open()) {
			std::cout << "Saving " << exnodes.size() << " nodes to " << path << std::endl;
			msgpack::pack(myfile, exnodes);
			myfile.close();
		} else
			std::cerr << "Trouble opening " << path << std::endl;
	}
	dht->node.join();
}

void CAppCore::Get(const std::string &cs)
{
	if (!dht) return;

	static std::time_t ts;
	ts = 0;
	dht::Where w;
	if (0 == cs.compare(0, 4, "M17-"))
		w.id(toUType(EMrefdValueID::Config));
	else if (0 == cs.compare(0, 3, "URF"))
		w.id(toUType(EUrfdValueID::Config));
	else
		return;

	dht->node.get(
		dht::InfoHash::get(cs),
		[this](const std::shared_ptr<dht::Value> &v) {
			if (0 == v->user_type.compare(MREFD_CONFIG_1))
			{
				auto rdat = dht::Value::unpack<SMrefdConfig1>(*v);
				if (rdat.timestamp > ts)
				{
					ts = rdat.timestamp;
					routeMap.Update(EFrom::dht, rdat.callsign, '0'==rdat.version[0], "", rdat.ipv4addr, rdat.ipv6addr, rdat.modules, rdat.encryptedmods, rdat.port, rdat.url);
				}
			}
			else if (0 == v->user_type.compare(URFD_CONFIG_1))
			{
				auto rdat = dht::Value::unpack<SUrfdConfig1>(*v);
				if (rdat.timestamp > ts)
				{
					ts = rdat.timestamp;
					routeMap.Update(EFrom::dht, rdat.callsign, true, "", rdat.ipv4addr, rdat.ipv6addr, rdat.modules, rdat.transcodedmods, rdat.port[toUType(EUrfdPorts::m17)], rdat.url);
				}
			}
			else
			{
				std::cerr << "Found the data, but it has an unknown user_type: " << v->user_type << std::endl;
			}
			return true;
		},
		[](bool success) {
			if (! success)
				std::cout << "node.get() was unsuccessful!" << std::endl;
		},
		{},
		w
	);
}
#endif
