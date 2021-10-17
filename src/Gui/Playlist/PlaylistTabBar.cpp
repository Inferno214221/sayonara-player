/* PlaylistTabBar.cpp */

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

#include "PlaylistTabBar.h"
#include "PlaylistTabMenu.h"

#include "Gui/Utils/InputDialog/LineInputDialog.h"
#include "Gui/Utils/MimeData/MimeDataUtils.h"
#include "Gui/Utils/Shortcuts/Shortcut.h"
#include "Gui/Utils/Shortcuts/ShortcutHandler.h"
#include "Gui/Utils/Widgets/DirectoryChooser.h"

#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Language/Language.h"
#include "Utils/Message/Message.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Utils.h"

#include <QShortcut>
#include <QInputDialog>
#include <QFileDialog>
#include <QMimeData>
#include <QMouseEvent>
#include <QDir>

using Playlist::TabBar;
using Playlist::TabMenu;

struct TabBar::Private
{
	QString lastDir;
	TabMenu* menu = nullptr;
	int tabBeforeDragDrop;
	int dragOriginTab;
	bool dragFromPlaylist;

	Private(QWidget* parent) :
		lastDir {QDir::homePath()},
		menu(new TabMenu(parent)),
		tabBeforeDragDrop(-1),
		dragOriginTab(-1),
		dragFromPlaylist(false) {}
};

TabBar::TabBar(QWidget* parent) :
	QTabBar(parent)
{
	m = Pimpl::make<Private>(this);

	this->setDrawBase(false);
	this->setAcceptDrops(true);
	this->setFocusPolicy(Qt::NoFocus);

	initShortcuts();

	connect(m->menu, &TabMenu::sigOpenFileClicked, this, &TabBar::openFilePressed);
	connect(m->menu, &TabMenu::sigOpenDirClicked, this, &TabBar::openDirPressed);
	connect(m->menu, &TabMenu::sigRenameClicked, this, &TabBar::renamePressed);
	connect(m->menu, &TabMenu::sigResetClicked, this, &TabBar::resetPressed);
	connect(m->menu, &TabMenu::sigSaveClicked, this, &TabBar::savePressed);
	connect(m->menu, &TabMenu::sigSaveAsClicked, this, &TabBar::saveAsPressed);
	connect(m->menu, &TabMenu::sigSaveToFileClicked, this, &TabBar::saveToFilePressed);
	connect(m->menu, &TabMenu::sigClearClicked, this, &TabBar::clearPressed);
	connect(m->menu, &TabMenu::sigDeleteClicked, this, &TabBar::deletePressed);
	connect(m->menu, &TabMenu::sigCloseClicked, this, &TabBar::closePressed);
	connect(m->menu, &TabMenu::sigCloseOthersClicked, this, &TabBar::closeOthersPressed);
}

TabBar::~TabBar() = default;

void TabBar::savePressed()
{
	emit sigTabSave(currentIndex());
}

void TabBar::saveAsPressed()
{
	const auto dialogText = QString("%1: %2")
		.arg(tabText(currentIndex()))
		.arg(Lang::get(Lang::SaveAs));

	const auto oldName = tabText(currentIndex());
	Gui::LineInputDialog dialog(Lang::get(Lang::SaveAs).triplePt(), dialogText, oldName, this);
	dialog.exec();
	
	if(dialog.returnValue() == Gui::LineInputDialog::ReturnValue::Ok)
	{
		const auto newName = dialog.text();
		emit sigTabSaveAs(currentIndex(), newName);
	}
}

void TabBar::saveToFilePressed()
{
	const auto fileName =
		QFileDialog::getSaveFileName(this, Lang::get(Lang::SaveAs), m->lastDir, "*.m3u");

	if(!fileName.isEmpty())
	{
		const auto answer = Message::question_yn(tr("Do you want relative file paths in your playlist?"));

		m->lastDir = Util::File::getParentDirectory(fileName);
		emit sigTabSaveToFile(this->currentIndex(), fileName, (answer == Message::Answer::Yes));
	}
}

void TabBar::openFilePressed()
{
	const auto filter = Util::getFileFilter(
		Util::Extensions(Util::Extension::Soundfile | Util::Extension::Playlist),
		tr("Media files"));

	const auto files = QFileDialog::getOpenFileNames(this, tr("Open Media files"), QDir::homePath(), filter);
	if(!files.isEmpty())
	{
		emit sigOpenFile(currentIndex(), files);
	}
}

void TabBar::openDirPressed()
{
	const auto dir = Gui::DirectoryChooser::getDirectory(Lang::get(Lang::OpenDir), QDir::homePath(), true, this);
	if(!dir.isEmpty())
	{
		emit sigOpenDir(currentIndex(), dir);
	}
}

void TabBar::clearPressed()
{
	emit sigTabClear(currentIndex());
}

void TabBar::deletePressed()
{
	const auto answer = Message::question_yn(
		Lang::get(Lang::Really).question(),
		Lang::get(Lang::Delete)
	);

	if(answer == Message::Answer::Yes)
	{
		emit sigTabDelete(currentIndex());
	}
}

void TabBar::closePressed()
{
	emit tabCloseRequested(currentIndex());
}

void TabBar::resetPressed()
{
	emit sigTabReset(currentIndex());
}

void TabBar::renamePressed()
{
	const auto oldName = tabText(currentIndex());

	Gui::LineInputDialog dialog(Lang::get(Lang::Rename), Lang::get(Lang::Rename), oldName, this);
	dialog.exec();

	const auto newName = dialog.text();

	if((dialog.returnValue() == Gui::LineInputDialog::ReturnValue::Ok) &&
	   (!newName.isEmpty()) &&
	   (newName != oldName))
	{
		emit sigTabRename(currentIndex(), newName);
	}
}

void TabBar::closeOthersPressed()
{
	const auto tabIndex = currentIndex();
	auto i = 0;
	while(count() > 2)
	{
		if(i < tabIndex)
		{
			emit tabCloseRequested(0);
		}

		else if(i != tabIndex)
		{
			emit tabCloseRequested(1);
		}

		i++;
	}
}

void TabBar::mousePressEvent(QMouseEvent* e)
{
	const auto tabIndex = this->tabAt(e->pos());
	if(tabIndex == this->count() - 1)
	{
		emit sigAddTabClicked();
		return;
	}

	else
	{
		this->setCurrentIndex(tabIndex);
	}

	if(e->button() == Qt::RightButton)
	{
		emit sigContextMenuRequested(this->currentIndex(), e->globalPos());
	}

	else if(e->button() == Qt::MiddleButton)
	{
		if(this->count() > 2)
		{
			emit tabCloseRequested(tabIndex);
		}
	}
}

void TabBar::wheelEvent(QWheelEvent* e)
{
	QTabBar::wheelEvent(e);
	if((count() > 1) && (currentIndex() == count() - 1))
	{
		this->setCurrentIndex(count() - 2);
	}
}

static QShortcut* initShortcut(QWidget* parent, QKeySequence key)
{
	auto* shortcut = new QShortcut(parent);
	shortcut->setKey(key);
	shortcut->setContext(Qt::WidgetWithChildrenShortcut);
	return shortcut;
}

void TabBar::initShortcuts()
{
	auto* shortcutHandler = ShortcutHandler::instance();
	shortcutHandler->shortcut(ShortcutIdentifier::AddTab).connect(this, this, SIGNAL(sigAddTabClicked()));
	shortcutHandler->shortcut(ShortcutIdentifier::CloseTab).connect(this, this, SLOT(closePressed()));

	auto* shortcut = initShortcut(this->parentWidget(), QKeySequence::Save);
	connect(shortcut, &QShortcut::activated, this, &TabBar::savePressed);

	shortcut = initShortcut(this->parentWidget(), QKeySequence::SaveAs);
	connect(shortcut, &QShortcut::activated, this, &TabBar::saveAsPressed);

	shortcut = initShortcut(this->parentWidget(), QKeySequence("F2"));
	connect(shortcut, &QShortcut::activated, this, &TabBar::renamePressed);

	shortcut = initShortcut(this->parentWidget(), QKeySequence::Open);
	connect(shortcut, &QShortcut::activated, this, &TabBar::openFilePressed);

	const auto keySequence = QKeySequence(QKeySequence::Open);
	shortcut = initShortcut(this->parentWidget(), QKeySequence("Shift+" + keySequence.toString()));
	connect(shortcut, &QShortcut::activated, this, &TabBar::openDirPressed);
}

void TabBar::showMenuItems(Playlist::MenuEntries entries, const QPoint& position)
{
	m->menu->showMenuItems(entries);
	m->menu->exec(position);
}

void TabBar::setTabsClosable(bool b)
{
	QTabBar::setTabsClosable(b);
	m->menu->showClose(b);
}

bool TabBar::wasDragFromPlaylist() const
{
	return m->dragFromPlaylist;
}

int TabBar::getDragOriginTab() const
{
	return m->dragOriginTab;
}

void TabBar::dragEnterEvent(QDragEnterEvent* e)
{
	auto source = e->source();
	const auto objectName = (source) ? source->objectName() : QString();

	m->dragOriginTab = -1;
	m->dragFromPlaylist = objectName.contains("playlist_view");

	if(!m->dragFromPlaylist)
	{
		m->tabBeforeDragDrop = -1;
	}

	else if(m->tabBeforeDragDrop < 0)
	{
		m->tabBeforeDragDrop = currentIndex();
	}

	e->accept();

	const auto tabIndex = tabAt(e->pos());
	setCurrentIndex(tabIndex);
}

void TabBar::dragMoveEvent(QDragMoveEvent* e)
{
	e->accept();

	const auto tabIndex = tabAt(e->pos());
	setCurrentIndex(tabIndex);
}

void TabBar::dragLeaveEvent(QDragLeaveEvent* e)
{
	bool isLastTab = (currentIndex() == count() - 1);
	if((m->tabBeforeDragDrop >= 0) && isLastTab)
	{
		setCurrentIndex(m->tabBeforeDragDrop);
		m->tabBeforeDragDrop = -1;
	}

	e->accept();
}

void TabBar::dropEvent(QDropEvent* e)
{
	e->accept();

	m->dragOriginTab = m->tabBeforeDragDrop;

	bool isLastTab = (currentIndex() == count() - 1);
	if((m->tabBeforeDragDrop >= 0) && isLastTab)
	{
		this->setCurrentIndex(m->tabBeforeDragDrop);
	}

	m->tabBeforeDragDrop = -1;

	auto* mimeData = e->mimeData();
	if(!mimeData)
	{
		return;
	}

	const auto tabIndex = this->tabAt(e->pos());
	if(Gui::MimeData::hasMetadata(mimeData))
	{
		const auto tracks {Gui::MimeData::metadata(mimeData)};
		emit sigMetadataDropped(tabIndex, tracks);
	}

	else if(mimeData->hasUrls())
	{
		QStringList files;
		Util::Algorithm::transform(mimeData->urls(), files, [](const auto& url) {
			return url.toLocalFile();
		});

		emit sigFilesDropped(tabIndex, files);
	}
}
