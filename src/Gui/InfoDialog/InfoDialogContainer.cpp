/* InfoDialogContainer.cpp */

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

#include "InfoDialogContainer.h"
#include "Components/Directories/MetaDataScanner.h"

#include "GUI_InfoDialog.h"
#include "Gui/Utils/GuiUtils.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Utils.h"

#include <QMainWindow>
#include <QThread>


struct InfoDialogContainer::Private
{
	GUI_InfoDialog*	info_dialog=nullptr;
	InfoDialogContainerAsyncHandler* async_helper=nullptr;
};

InfoDialogContainer::InfoDialogContainer()
{
	m = Pimpl::make<Private>();
}

InfoDialogContainer::~InfoDialogContainer()
{
	if(m->async_helper){
		delete m->async_helper;
	}
}

void InfoDialogContainer::info_dialog_closed() {}

void InfoDialogContainer::show_info()
{
	if(init_dialog(OpenMode::Info))
	{
		m->info_dialog->show(GUI_InfoDialog::Tab::Info);
	}
}

void InfoDialogContainer::show_lyrics()
{
	if(init_dialog(OpenMode::Lyrics))
	{
		m->info_dialog->show(GUI_InfoDialog::Tab::Lyrics);
	}
}

void InfoDialogContainer::show_edit()
{
	if(init_dialog(OpenMode::Edit))
	{
		m->info_dialog->show(GUI_InfoDialog::Tab::Edit);
	}
}

void InfoDialogContainer::show_cover_edit()
{
	if(init_dialog(OpenMode::Cover))
	{
		m->info_dialog->show_cover_edit_tab();
	}
}

bool InfoDialogContainer::init_dialog(OpenMode mode)
{
	if(!m->info_dialog)
	{
		m->info_dialog = new GUI_InfoDialog(this, Gui::Util::main_window());
	}

	if(!has_metadata())
	{
		if(!m->async_helper)
		{
			m->async_helper = new InfoDialogContainerAsyncHandler(this, mode);
		}

		if(m->async_helper->is_running())
		{
			return false;
		}

		m->info_dialog->set_metadata(MetaDataList(), metadata_interpretation());

		bool started = m->async_helper->start();
		if(started)
		{
			m->info_dialog->show(GUI_InfoDialog::Tab::Info);
		}

		return started;
	}

	m->info_dialog->set_busy(false);
	m->info_dialog->set_metadata(info_dialog_data(), metadata_interpretation());
	return m->info_dialog->has_metadata();
}

void InfoDialogContainer::go(OpenMode mode, const MetaDataList& v_md)
{
	m->info_dialog->set_busy(false);

	if(v_md.isEmpty()) {
		m->info_dialog->close();
		return;
	}

	m->info_dialog->set_metadata(v_md, metadata_interpretation());

	switch(mode)
	{
		case OpenMode::Info:
			m->info_dialog->show(GUI_InfoDialog::Tab::Info);
			break;
		case OpenMode::Lyrics:
			m->info_dialog->show(GUI_InfoDialog::Tab::Lyrics);
			break;

		case OpenMode::Edit:
			m->info_dialog->show(GUI_InfoDialog::Tab::Edit);
			break;

		case OpenMode::Cover:
			m->info_dialog->show_cover_edit_tab();
			break;
	}
}

bool InfoDialogContainer::has_metadata() const
{
	return true;
}

QStringList InfoDialogContainer::pathlist() const
{
	return QStringList();
}


struct InfoDialogContainerAsyncHandler::Private
{
	InfoDialogContainer* container=nullptr;
	OpenMode mode;
	bool is_running;

	Private(InfoDialogContainer* container, OpenMode mode) :
		container(container),
		mode(mode),
		is_running(false)
	{}
};

InfoDialogContainerAsyncHandler::InfoDialogContainerAsyncHandler(InfoDialogContainer* container, OpenMode mode) :
	QObject()
{
	m = Pimpl::make<Private>(container, mode);
}

InfoDialogContainerAsyncHandler::~InfoDialogContainerAsyncHandler() = default;

bool InfoDialogContainerAsyncHandler::start()
{
	using Directory::MetaDataScanner;

	QStringList files = m->container->pathlist();
	if(files.isEmpty()){
		return false;
	}

	m->is_running = true;
	auto* scanner = new MetaDataScanner(files, true);
	auto* t = new QThread();
	t->setObjectName(QString("InfoDialogContainer%1").arg(Util::random_string(8)));

	scanner->moveToThread(t);

	connect(t, &QThread::started, scanner, &MetaDataScanner::start);
	connect(t, &QThread::finished, t, &QObject::deleteLater);
	connect(scanner, &MetaDataScanner::sig_finished, this, &InfoDialogContainerAsyncHandler::scanner_finished);
	connect(scanner, &MetaDataScanner::sig_finished, t, &QThread::quit);

	t->start();

	return true;
}

bool InfoDialogContainerAsyncHandler::is_running() const
{
	return m->is_running;
}

void InfoDialogContainerAsyncHandler::scanner_finished()
{
	auto* scanner = static_cast<Directory::MetaDataScanner*>(sender());

	m->is_running = false;
	m->container->go(m->mode, scanner->metadata());

	scanner->deleteLater();
}
