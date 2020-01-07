/* GUI_EnginePreferences.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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

struct GUI_EnginePreferences::Private
{
	QString alsa_buffer;
};

GUI_EnginePreferences::GUI_EnginePreferences(const QString& identifier) :
	Preferences::Base(identifier)
{
	m = Pimpl::make<Private>();
}

GUI_EnginePreferences::~GUI_EnginePreferences()
{
	if(ui){
		delete ui; ui = nullptr;
	}
}

QString GUI_EnginePreferences::action_name() const
{
	return tr("Audio");
}

bool GUI_EnginePreferences::commit()
{
	if(ui->rb_pulse->isChecked()){
		SetSetting(Set::Engine_Sink, QString("pulse"));
	}

	else if(ui->rb_alsa->isChecked())
	{
		QString card = ui->combo_alsa_devices->currentData().toString();
		SetSetting(Set::Engine_AlsaDevice, card);
		SetSetting(Set::Engine_Sink, QString("alsa"));

		SetSetting(Set::Engine_CrossFaderActive, false);

		Playlist::Mode plm = GetSetting(Set::PL_Mode);
		plm.setGapless(false, false);
		SetSetting(Set::PL_Mode, plm);
	}

	else
	{
		SetSetting(Set::Engine_Sink, QString("auto"));
	}

	return true;
}

void GUI_EnginePreferences::revert()
{
	QString engine_name = GetSetting(Set::Engine_Sink);
	if(engine_name == "pulse"){
		ui->rb_pulse->setChecked(true);
	}

	else if(engine_name == "alsa"){
		ui->rb_alsa->setChecked(true);
	}

	else{
		ui->rb_auto->setChecked(true);
	}
}

void GUI_EnginePreferences::init_ui()
{
	if(ui){
		return;
	}

	setup_parent(this, &ui);

	connect(ui->rb_alsa, &QRadioButton::toggled, this, &GUI_EnginePreferences::radio_button_changed);
	connect(ui->rb_pulse, &QRadioButton::toggled, this, &GUI_EnginePreferences::radio_button_changed);

	revert();

	radio_button_changed(ui->rb_alsa->isChecked());
	ui->combo_alsa_devices->setVisible(false);

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

void GUI_EnginePreferences::retranslate_ui()
{
	ui->retranslateUi(this);

	ui->rb_auto->setText(Lang::get(Lang::Automatic));
}

void GUI_EnginePreferences::radio_button_changed(bool b)
{
	Q_UNUSED(b)
	ui->combo_alsa_devices->setVisible(false);
}

struct SubDevice
{
	QString name;
	int id=0;
};


void GUI_EnginePreferences::alsa_process_finished(int exit_code, QProcess::ExitStatus exit_status)
{
	Q_UNUSED(exit_code)
	Q_UNUSED(exit_status)

	auto* process = static_cast<QProcess*>(sender());
	m->alsa_buffer.append
	(
		QString::fromLocal8Bit(process->readAllStandardOutput())
	);

	ui->combo_alsa_devices->clear();

	if(exit_code != 0){
		return;
	}

	QMap <int, QList<SubDevice>> device_map;

	const QStringList splitted = m->alsa_buffer.split("\n");
	for(const QString& line : splitted)
	{
		QRegExp re("card ([0-9]+): (.+device ([0-9]+).+)");
		int idx = re.indexIn(line);
		if(idx < 0){
			continue;
		}

		int device_id = re.cap(1).toInt();
		int subdevice_id = re.cap(3).toInt();
		QString name = re.cap(2);

		SubDevice sd;
				sd.id = subdevice_id;
				sd.name = name;

		if(!device_map.contains(device_id))
		{
			device_map.insert(device_id, QList<SubDevice>{sd});
		}

		else
		{
			QList<SubDevice> subdevices = device_map[device_id];
			subdevices << sd;
			device_map[device_id] = subdevices;
		}
	}

	for(auto it=device_map.begin(); it != device_map.end(); it++)
	{
		for(const SubDevice& subdevice : it.value())
		{
			QString device_identifier = QString("hw:%1").arg(it.key());
			if(it.value().size() > 1){
				device_identifier += QString(",%1").arg(subdevice.id);
			}

			ui->combo_alsa_devices->addItem(subdevice.name, device_identifier);
		}
	}
}

void GUI_EnginePreferences::alsa_process_error_occured(QProcess::ProcessError error)
{
	Q_UNUSED(error)
}

void GUI_EnginePreferences::alsa_stdout_written()
{
	auto* process = static_cast<QProcess*>(sender());

	m->alsa_buffer.append
	(
		QString::fromLocal8Bit(process->readAllStandardOutput())
	);
}

