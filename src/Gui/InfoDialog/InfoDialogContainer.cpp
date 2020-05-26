/* InfoDialogContainer.cpp */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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
	GUI_InfoDialog* infoDialog = nullptr;
	InfoDialogContainerAsyncHandler* asyncHelper = nullptr;
};

InfoDialogContainer::InfoDialogContainer()
{
	m = Pimpl::make<Private>();
}

InfoDialogContainer::~InfoDialogContainer()
{
	delete m->asyncHelper;
}

void InfoDialogContainer::infoDialogClosed() {}

void InfoDialogContainer::showInfo()
{
	if(initDialog(OpenMode::Info))
	{
		m->infoDialog->show(GUI_InfoDialog::Tab::Info);
	}
}

void InfoDialogContainer::showLyrics()
{
	if(initDialog(OpenMode::Lyrics))
	{
		m->infoDialog->show(GUI_InfoDialog::Tab::Lyrics);
	}
}

void InfoDialogContainer::showEdit()
{
	if(initDialog(OpenMode::Edit))
	{
		m->infoDialog->show(GUI_InfoDialog::Tab::Edit);
	}
}

void InfoDialogContainer::showCoverEdit()
{
	if(initDialog(OpenMode::Cover))
	{
		m->infoDialog->showCoverEditTab();
	}
}

bool InfoDialogContainer::initDialog(OpenMode mode)
{
	if(!m->infoDialog)
	{
		m->infoDialog = new GUI_InfoDialog(this, Gui::Util::mainWindow());
	}

	if(!hasMetadata())
	{
		if(!m->asyncHelper)
		{
			m->asyncHelper = new InfoDialogContainerAsyncHandler(this, mode);
		}

		if(m->asyncHelper->isRunning())
		{
			return false;
		}

		m->infoDialog->setMetadata(MetaDataList(), metadataInterpretation());

		bool started = m->asyncHelper->start();
		if(started)
		{
			m->infoDialog->show(GUI_InfoDialog::Tab::Info);
		}

		return started;
	}

	m->infoDialog->setBusy(false);
	m->infoDialog->setMetadata(infoDialogData(), metadataInterpretation());
	return m->infoDialog->hasMetadata();
}

void InfoDialogContainer::go(OpenMode mode, const MetaDataList& v_md)
{
	m->infoDialog->setBusy(false);

	if(v_md.isEmpty())
	{
		m->infoDialog->close();
		return;
	}

	m->infoDialog->setMetadata(v_md, metadataInterpretation());

	switch(mode)
	{
		case OpenMode::Info:
			m->infoDialog->show(GUI_InfoDialog::Tab::Info);
			break;
		case OpenMode::Lyrics:
			m->infoDialog->show(GUI_InfoDialog::Tab::Lyrics);
			break;

		case OpenMode::Edit:
			m->infoDialog->show(GUI_InfoDialog::Tab::Edit);
			break;

		case OpenMode::Cover:
			m->infoDialog->showCoverEditTab();
			break;
	}
}

bool InfoDialogContainer::hasMetadata() const
{
	return true;
}

QStringList InfoDialogContainer::pathlist() const
{
	return QStringList();
}

struct InfoDialogContainerAsyncHandler::Private
{
	InfoDialogContainer* container = nullptr;
	OpenMode mode;
	bool is_running;

	Private(InfoDialogContainer* container, OpenMode mode) :
		container(container),
		mode(mode),
		is_running(false) {}
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
	if(files.isEmpty())
	{
		return false;
	}

	m->is_running = true;
	auto* scanner = new MetaDataScanner(files, true);
	auto* t = new QThread();

	scanner->moveToThread(t);

	connect(t, &QThread::started, scanner, &MetaDataScanner::start);
	connect(t, &QThread::finished, t, &QObject::deleteLater);
	connect(scanner, &MetaDataScanner::sigFinished, this, &InfoDialogContainerAsyncHandler::scannerFinished);
	connect(scanner, &MetaDataScanner::sigFinished, t, &QThread::quit);

	t->start();

	return true;
}

bool InfoDialogContainerAsyncHandler::isRunning() const
{
	return m->is_running;
}

void InfoDialogContainerAsyncHandler::scannerFinished()
{
	auto* scanner = dynamic_cast<Directory::MetaDataScanner*>(sender());

	m->is_running = false;
	m->container->go(m->mode, scanner->metadata());

	scanner->deleteLater();
}
