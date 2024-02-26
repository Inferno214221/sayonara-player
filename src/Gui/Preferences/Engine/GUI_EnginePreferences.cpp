/* GUI_EnginePreferences.cpp */

/* Copyright (C) 2011-2024 Michael Lugmair (Lucio Carreras)
 *
 * This file is part of sayonara player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "GUI_EnginePreferences.h"
#include "Gui/Preferences/ui_GUI_EnginePreferences.h"

#include "Utils/Utils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language/Language.h"
#include "Utils/Playlist/PlaylistMode.h"

#include <QProcess>
#include <QMap>

namespace
{
	struct SubDevice
	{
		QString name;
		int id {0};
	};

	using Devices = std::map<int, QList<SubDevice>>;

	Devices addSubdevice(const QString& name, const int deviceId, const int subdeviceId, Devices devices)
	{
		const auto subDevice = SubDevice {name, subdeviceId};

		if(const auto it = devices.find(deviceId); it != devices.end())
		{
			devices.insert({deviceId, {subDevice}});
		}

		else
		{
			devices[deviceId] << subDevice;
		}

		return devices;
	}

	Devices createDevicesFromAlsaBuffer(const QString& alsaInfo)
	{
		Devices devices;

		const auto splitted = alsaInfo.split("\n");
		for(const auto& line: splitted)
		{
			auto re = QRegExp("card ([0-9]+): (.+device ([0-9]+).+)");
			if(const auto idx = re.indexIn(line); idx >= 0)
			{
				devices = addSubdevice(re.cap(2), re.cap(1).toInt(), re.cap(3).toInt(), std::move(devices));
			}
		}

		return devices;
	}

	void applyDevicesToCombobox(const Devices& devices, QComboBox* comboBox)
	{
		for(const auto& [deviceId, subdevices]: devices)
		{
			for(const auto& subdevice: subdevices)
			{
				auto deviceIdentifier = QString("hw:%1").arg(deviceId);
				if(subdevices.size() > 1)
				{
					deviceIdentifier += QString(",%1").arg(subdevice.id);
				}

				comboBox->addItem(subdevice.name, deviceIdentifier);
			}
		}
	}
}

struct GUI_EnginePreferences::Private
{
	QString alsaBuffer;
};

GUI_EnginePreferences::GUI_EnginePreferences(const QString& identifier) :
	Preferences::Base(identifier),
	m {Pimpl::make<Private>()} {}

GUI_EnginePreferences::~GUI_EnginePreferences() = default;

QString GUI_EnginePreferences::actionName() const { return tr("Audio"); }

bool GUI_EnginePreferences::commit()
{
	if(ui->rbPulse->isChecked())
	{
		SetSetting(Set::Engine_Sink, QString("pulse"));
	}

	else if(ui->rbAlsa->isChecked())
	{
		const auto card = ui->comboAlsaDevices->currentData().toString();
		SetSetting(Set::Engine_AlsaDevice, card);
		SetSetting(Set::Engine_Sink, QString("alsa"));
		SetSetting(Set::Engine_CrossFaderActive, false);

		Playlist::Mode playlistMode = GetSetting(Set::PL_Mode);
		playlistMode.setGapless(false, false);
		SetSetting(Set::PL_Mode, playlistMode);
	}

	else
	{
		SetSetting(Set::Engine_Sink, QString("auto"));
	}

	return true;
}

void GUI_EnginePreferences::revert()
{
	const auto engineName = GetSetting(Set::Engine_Sink);
	if(engineName == "pulse")
	{
		ui->rbPulse->setChecked(true);
	}

	else if(engineName == "alsa")
	{
		ui->rbAlsa->setChecked(true);
	}

	else
	{
		ui->rbAuto->setChecked(true);
	}
}

void GUI_EnginePreferences::initUi()
{
	ui = std::make_shared<Ui::GUI_EnginePreferences>();
	ui->setupUi(this);

	connect(ui->rbAlsa, &QRadioButton::toggled, this, &GUI_EnginePreferences::radioButtonChanged);
	connect(ui->rbPulse, &QRadioButton::toggled, this, &GUI_EnginePreferences::radioButtonChanged);

	radioButtonChanged(ui->rbAlsa->isChecked());
	ui->comboAlsaDevices->setVisible(false);

/*
	auto* process = new QProcess(this);
	m->alsa_buffer.clear();

	Util::set_environment("LANGUAGE", "en_US");
	connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &GUI_EnginePreferences::alsa_process_finished);
	connect(process, &QProcess::readyReadStandardOutput, this, &GUI_EnginePreferences::alsa_stdout_written);

	QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
	env.insert("LANGUAGE", "en_US");
	env.insert("LANG", "en_US.UTF-8");
	env.insert("LC_ALL", "en_US.UTF-8");
	process->setProcessEnvironment(env);
	process->start("aplay", QStringList{"-l"});
*/
}

void GUI_EnginePreferences::retranslate()
{
	ui->retranslateUi(this);
	ui->rbAuto->setText(Lang::get(Lang::Automatic));
}

void GUI_EnginePreferences::radioButtonChanged(const bool /*b*/)
{
	ui->comboAlsaDevices->setVisible(false);
}

void GUI_EnginePreferences::alsaProcessFinished(const int exitCode, const QProcess::ExitStatus /*exitStatus*/)
{
	auto* process = dynamic_cast<QProcess*>(sender());
	m->alsaBuffer.append(
		QString::fromLocal8Bit(process->readAllStandardOutput()));

	ui->comboAlsaDevices->clear();

	if(exitCode == 0)
	{
		const auto devices = createDevicesFromAlsaBuffer(m->alsaBuffer);
		applyDevicesToCombobox(devices, ui->comboAlsaDevices);
	}
}

void GUI_EnginePreferences::alsaProcessErrorOccured(QProcess::ProcessError /*error*/) {}

void GUI_EnginePreferences::alsaStdoutWritten()
{
	auto* process = dynamic_cast<QProcess*>(sender());

	m->alsaBuffer.append(
		QString::fromLocal8Bit(process->readAllStandardOutput()));
}

