/*
 * GUI_PlayerMenubar.cpp
 *
 *  Created on: 10.10.2012
 *      Author: Michael Lugmair (Lucio Carreras)
 */

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

#include "GUI_PlayerMenubar.h"

#include "Gui/Shutdown/GUI_Shutdown.h"
#include "Components/Playlist/PlaylistHandler.h"
#include "Components/LibraryManagement/LibraryPluginHandler.h"
#include "Components/LibraryManagement/AbstractLibraryContainer.h"

#include "Gui/Utils/Shortcuts/ShortcutHandler.h"
#include "Gui/Utils/Shortcuts/Shortcut.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/Style.h"

#include "Gui/Plugins/PlayerPluginBase.h"
#include "Gui/Plugins/PlayerPluginHandler.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Utils.h"
#include "Utils/Language/Language.h"
#include "Utils/Message/Message.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QStringList>
#include <QMenu>
#include <QAction>
#include <QPalette>
#include <QDateTime>
#include <QHBoxLayout>
#include <QLayoutItem>
#include <QLabel>

struct Menubar::Private
{
	QMenu*			menuFile=nullptr;
	QMenu*			menuView=nullptr;
	QMenu*			menuPlugins=nullptr;
	QMenu*			menuHelp=nullptr;

	QAction*		menuHelpAction=nullptr;

	//file
	QAction*		actionOpenFile=nullptr;
	QAction*		actionOpenDir=nullptr;
	QAction*		sepAfterOpen=nullptr; // after open file and open dir
	QAction*		sepAfterPreferences=nullptr;
	QAction*		actionShutdown=nullptr;
	QAction*		actionClose=nullptr;

	// view
	QAction*		actionViewLibrary=nullptr;
	QAction*		actionDark=nullptr;
	QAction*		actionBigCover=nullptr;
	QAction*		actionFullscreen=nullptr;

	// help
	QAction*		actionHelp=nullptr;
	QAction*		actionAbout=nullptr;
	QAction*		actionLogger=nullptr;

	QAction*		currentLibraryMenuAction=nullptr;

	QMessageBox*	aboutBox=nullptr;
	Library::AbstractContainer* currentLibrary=nullptr;

	QLabel*			heartLabel=nullptr;
	QLabel*			donateLabel=nullptr;

	Private(Menubar* menubar)
	{
		menuFile = new QMenu(menubar);
		menuView = new QMenu(menubar);
		menuPlugins = new QMenu(menubar);
		menuHelp = new QMenu(menubar);

		menubar->insertMenu(nullptr, menuFile);
		menubar->insertMenu(nullptr, menuPlugins);
		menubar->insertMenu(nullptr, menuView);
		menuHelpAction = menubar->insertMenu(nullptr, menuHelp);

		// file
		actionOpenDir = new QAction(menuFile);
		actionOpenFile = new QAction(menuFile);
		sepAfterOpen = menuFile->addSeparator();
		sepAfterPreferences = menuFile->addSeparator();
		actionShutdown = new QAction(menuFile);
		actionClose = new QAction(menuFile);

		menuFile->insertActions(nullptr,
		{
			actionOpenFile, actionOpenDir, sepAfterOpen, sepAfterPreferences, actionShutdown, actionClose
		});

		// view
		actionViewLibrary = new QAction(menuView);
		actionViewLibrary->setCheckable(true);

		actionDark = new QAction(menuView);
		actionDark->setCheckable(true);
		actionBigCover = new QAction(menuView);
		actionBigCover->setCheckable(true);
		actionFullscreen = new QAction(menuView);
		actionFullscreen->setCheckable(true);

		menuView->insertActions(nullptr,
		{
			actionViewLibrary,
			actionBigCover,
			actionDark,
			actionFullscreen
		});

		//help
		actionHelp = new QAction(menuHelp);
		actionAbout = new QAction(menuHelp);
		actionLogger = new QAction(menuHelp);

		menuHelp->insertActions(nullptr,
		{
			actionLogger, actionHelp, menuHelp->addSeparator(), actionAbout
		});
	}
};

Menubar::Menubar(QWidget* parent) :
	Gui::WidgetTemplate<QMenuBar>(parent)
{
	m = Pimpl::make<Private>(this);

	m->actionViewLibrary->setChecked(GetSetting(Set::Lib_Show));
	m->actionViewLibrary->setText(Lang::get(Lang::Library));
	m->actionViewLibrary->setShortcut(QKeySequence("Ctrl+L"));

	m->actionBigCover->setShortcut(QKeySequence("F9"));
	ListenSetting(Set::Player_ControlStyle, Menubar::styleChanged);

	m->actionDark->setShortcut(QKeySequence("F10"));
	ListenSetting(Set::Player_ControlStyle, Menubar::styleChanged);

	m->actionFullscreen->setShortcut(QKeySequence("F11"));
	m->actionFullscreen->setChecked(GetSetting(Set::Player_Fullscreen));

#ifdef SAYONARA_WITH_SHUTDOWN
	m->actionShutdown->setVisible(true);
#else
	m->action_shutdown->setVisible(false);
#endif

	initDonateLink();
	initConnections();
	languageChanged();
	skinChanged();
	styleChanged();
}

Menubar::~Menubar() = default;

void Menubar::insertPreferenceAction(QAction* action)
{
	m->menuFile->insertAction(m->sepAfterPreferences, action);
}

QAction* Menubar::changeCurrentLibrary(Library::AbstractContainer* library)
{
	showLibraryAction(false);
	m->currentLibrary = library;

	if(!library)
	{
		if(m->currentLibraryMenuAction)
		{
			m->currentLibraryMenuAction->setVisible(false);
			m->currentLibraryMenuAction = nullptr;
		}

		return nullptr;
	}

	QMenu* newLibraryMenu = library->menu();

	if(m->currentLibraryMenuAction) {
		this->removeAction(m->currentLibraryMenuAction);
	}

	m->currentLibraryMenuAction = nullptr;

	if(!newLibraryMenu) {
		showLibraryAction(false);
		return nullptr;
	}

	m->currentLibraryMenuAction = this->insertMenu(m->menuHelpAction, newLibraryMenu);

	if(library->isLocal())
	{
		m->currentLibraryMenuAction->setText(Lang::get(Lang::Library));
	}

	else {
		m->currentLibraryMenuAction->setText(library->displayName());
	}

	bool library_visible = GetSetting(Set::Lib_Show);
	showLibraryAction(library_visible);

	return m->currentLibraryMenuAction;
}

void Menubar::showLibraryAction(bool visible)
{
	if(m->currentLibraryMenuAction)
	{
		m->currentLibraryMenuAction->setVisible(visible);
	}
}

void Menubar::setShowLibraryActionEnabled(bool b)
{
	m->actionViewLibrary->setEnabled(b);
}

void Menubar::showLibraryMenu(bool b)
{
	auto* lph = Library::PluginHandler::instance();
	this->changeCurrentLibrary(lph->currentLibrary());

	if(m->currentLibraryMenuAction)
	{
		m->currentLibraryMenuAction->setVisible(b);
	}
}

[[maybe_unused]] QString getLinkColor(QWidget* parent)
{
	if(!Style::isDark())
	{
		const QPalette p = parent->palette();
		const QColor color = p.windowText().color();
		return color.name(QColor::NameFormat::HexRgb);
	}

	else {
		return "f3841a";
	}
}

void Menubar::initDonateLink()
{
	auto* cornerWidget = new QWidget(this);
	m->heartLabel = new QLabel(this);
	m->heartLabel->setCursor(Qt::PointingHandCursor);

	m->donateLabel = new QLabel(this);
	m->donateLabel->setOpenExternalLinks(true);
	m->donateLabel->setStyleSheet(QString());
	m->donateLabel->setCursor(Qt::PointingHandCursor);

	auto* layout = new QHBoxLayout();
	layout->setSpacing(0);
	layout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::MinimumExpanding));
	layout->addWidget(m->heartLabel);
	layout->addWidget(m->donateLabel);

	cornerWidget->setLayout(layout);
	this->setCornerWidget(cornerWidget);
}

void Menubar::pluginAdded(PlayerPlugin::Base* plugin)
{
	auto* pph = PlayerPlugin::Handler::instance();
	QList<PlayerPlugin::Base*> lst = pph->allPlugins();

	QAction* action = plugin->pluginAction();

	QKeySequence ks("Shift+F" + QString::number(lst.size()));
	action->setShortcut(ks);
	action->setData(plugin->name());

	m->menuPlugins->addAction(action);
}

void Menubar::initConnections()
{
	// file
	connect(m->actionOpenFile, &QAction::triggered, this, &Menubar::openFilesClicked);
	connect(m->actionOpenDir, &QAction::triggered, this, &Menubar::openDirClicked);
	connect(m->actionClose, &QAction::triggered, this, &Menubar::sigCloseClicked);
	connect(m->actionShutdown, &QAction::triggered, this, &Menubar::shutdownClicked);

	// view
	connect(m->actionViewLibrary, &QAction::toggled, this, &Menubar::showLibraryToggled);
	connect(m->actionDark, &QAction::toggled, this, &Menubar::skinToggled);
	connect(m->actionBigCover, &QAction::toggled, this, &Menubar::bigCoverToggled);
	connect(m->actionFullscreen, &QAction::toggled, this, &Menubar::showFullscreenToggled);
	connect(m->actionLogger, &QAction::triggered, this, &Menubar::sigLoggerClicked);

	// about
	connect(m->actionAbout, &QAction::triggered, this, &Menubar::aboutClicked);
	connect(m->actionHelp, &QAction::triggered, this, &Menubar::helpClicked);

	// shortcuts
	auto* sch = ShortcutHandler::instance();
	sch->shortcut(ShortcutIdentifier::Quit).connect(this, this, SLOT(closeClicked()));
	sch->shortcut(ShortcutIdentifier::Minimize).connect(this, this, SLOT(minimizeClicked()));

	shortcutChanged(ShortcutIdentifier::Invalid);

	connect(sch, &ShortcutHandler::sigShortcutChanged, this, &Menubar::shortcutChanged);

	// Library
	auto* lph = Library::PluginHandler::instance();
	connect(lph, &Library::PluginHandler::sigLibrariesChanged, this, [=](){
		this->changeCurrentLibrary(lph->currentLibrary());
	});

	auto* pph = PlayerPlugin::Handler::instance();
	connect(pph, &PlayerPlugin::Handler::sigPluginAdded, this, &Menubar::pluginAdded);
}

void Menubar::languageChanged()
{
	m->menuFile->setTitle(Lang::get(Lang::File));
	m->menuView->setTitle(tr("View"));
	m->menuHelp->setTitle(tr("Help"));
	m->menuPlugins->setTitle(tr("Plugins"));

	m->actionOpenFile->setText(Lang::get(Lang::OpenFile).triplePt());
	m->actionOpenDir->setText(Lang::get(Lang::OpenDir).triplePt());
	m->actionShutdown->setText(Lang::get(Lang::Shutdown).triplePt());

	m->actionClose->setText(Lang::get(Lang::Quit));

	m->actionViewLibrary->setText(Lang::get(Lang::ShowLibrary));
	m->actionLogger->setText(Lang::get(Lang::Logger));
	m->actionDark->setText(Lang::get(Lang::DarkMode));
	m->actionBigCover->setText(tr("Show large cover"));
	m->actionFullscreen->setText(tr("Fullscreen"));

	m->actionHelp->setText(tr("Help"));
	m->actionAbout->setText(Lang::get(Lang::About).triplePt());

	if(m->currentLibrary && m->currentLibraryMenuAction)
	{
		if(m->currentLibrary->isLocal())
		{
			m->currentLibraryMenuAction->setText(Lang::get(Lang::Library));
		}

		else {
			m->currentLibraryMenuAction->setText(m->currentLibrary->displayName());
		}
	}
}

void Menubar::skinChanged()
{
	const QString stylesheet = Style::currentStyle();
	this->setStyleSheet(stylesheet);

	{
		using namespace Gui;
		m->actionOpenFile->setIcon(Icons::icon(Icons::Open));
		m->actionOpenDir->setIcon(Icons::icon(Icons::Open));
		m->actionClose->setIcon(Icons::icon(Icons::Exit));
		m->actionShutdown->setIcon(Icons::icon(Icons::Shutdown));
		m->actionAbout->setIcon(Icons::icon(Icons::Info));
	}

	{
		bool dark = Style::isDark();

		const QColor heartColor(243,132,26);
		const QColor textColor = (dark) ? heartColor : QColor();

		const QString heartLink = Util::createLink("❤ ", heartColor, false, "https://sayonara-player.com/donations.php");
		const QString sayonaraLink = Util::createLink("Sayonara", textColor, true, "https://sayonara-player.com/donations.php");

		m->heartLabel->setText(heartLink);
		m->donateLabel->setText(sayonaraLink);
	}
}

void Menubar::openDirClicked()
{
	const QString dir = QFileDialog::getExistingDirectory(this,
		Lang::get(Lang::OpenDir),
		QDir::homePath(),
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
	);

	if(!dir.isEmpty()){
		Playlist::Handler::instance()->createPlaylist(dir);
	}
}

void Menubar::openFilesClicked()
{
	const QString filter = Util::getFileFilter
	(
		Util::Extensions(Util::Extension::Soundfile | Util::Extension::Playlist),
		tr("Media files")
	);

	const QStringList list = QFileDialog::getOpenFileNames
	(
		this,
		tr("Open Media files"),
		QDir::homePath(),
		filter
	);

	if(!list.isEmpty()) {
		Playlist::Handler::instance()->createPlaylist(list);
	}
}

void Menubar::shutdownClicked()
{
	auto* gui = new GUI_Shutdown(this);
	gui->exec();
}

void Menubar::closeClicked()
{
	emit sigCloseClicked();
}

void Menubar::minimizeClicked()
{
	emit sigMinimizeClicked();
}

void Menubar::styleChanged()
{
	m->actionBigCover->setChecked(GetSetting(Set::Player_ControlStyle) == 1);
	m->actionDark->setChecked(Style::isDark());
}

void Menubar::skinToggled(bool b)
{
	Style::setDark(b);
}

void Menubar::bigCoverToggled(bool b)
{
	SetSetting(Set::Player_ControlStyle, b ? 1 : 0);
}

void Menubar::showLibraryToggled(bool b)
{
	m->actionViewLibrary->setChecked(b);
	SetSetting(Set::Lib_Show, b);
}

void Menubar::showFullscreenToggled(bool b)
{
	// may happened because of F11 too
	m->actionFullscreen->setChecked(b);
	SetSetting(Set::Player_Fullscreen, b);
}

void Menubar::helpClicked()
{
	const QStringList text
	{
		tr("For bug reports and feature requests please visit Sayonara's project page at GitLab"),
		Util::createLink("https://gitlab.com/luciocarreras/sayonara-player", Style::isDark()),
		"",
		tr("FAQ") + ": ",
		Util::createLink("http://sayonara-player.com/faq.php", Style::isDark()),
	};

	Message::info(text.join("<br/>"));
}

// private slot
void Menubar::aboutClicked()
{
	QString version = GetSetting(Set::Player_Version);

	if(!m->aboutBox)
	{
		m->aboutBox = new QMessageBox(this);
		m->aboutBox->setParent(this);
		m->aboutBox->setIconPixmap(Gui::Util::pixmap("logo.png", Gui::Util::NoTheme, QSize(150, 150), true));
		m->aboutBox->setWindowFlags(Qt::Dialog);
		m->aboutBox->setModal(true);
		m->aboutBox->setStandardButtons(QMessageBox::Ok);
		m->aboutBox->setWindowTitle(tr("About Sayonara"));

		m->aboutBox->setText(QStringList
		({
			"<b><font size=\"+2\">",
			"Sayonara Player " + version,
			"</font></b>"
		}).join(""));

		m->aboutBox->setInformativeText( QStringList
		({
			tr("Written by %1").arg("Michael Lugmair (Lucio Carreras)"),
			"",
			tr("License") + ": GPLv3",
			"Copyright 2011-" + QString::number(QDateTime::currentDateTime().date().year()),
			Util::createLink("http://sayonara-player.com", Style::isDark()),
			"",
			"<b>" + tr("Donate") + "</b>",
			Util::createLink("http://sayonara-player.com/donations.php", Style::isDark()),
			"",
			tr("Thanks to all the brave translators and to everyone who helps building Sayonara packages") + ".<br>" +
			tr("And special thanks to those people with local music collections") + "!"
		}).join("<br/>"));
	}

	m->aboutBox->exec();
}

void Menubar::shortcutChanged(ShortcutIdentifier identifier)
{
	Q_UNUSED(identifier)

	ShortcutHandler* sch = ShortcutHandler::instance();
	Shortcut sc = sch->shortcut(ShortcutIdentifier::ViewLibrary);
	m->actionViewLibrary->setShortcut(sc.sequence());
}
