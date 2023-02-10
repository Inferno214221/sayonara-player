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
#include "Components/LibraryManagement/LibraryPluginHandler.h"
#include "Components/LibraryManagement/AbstractLibraryContainer.h"

#include "Gui/Utils/Shortcuts/ShortcutHandler.h"
#include "Gui/Utils/Shortcuts/Shortcut.h"
#include "Gui/Utils/Widgets/DirectoryChooser.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/Style.h"

#include "Gui/Plugins/PlayerPluginBase.h"
#include "Gui/Plugins/PlayerPluginHandler.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Utils.h"
#include "Utils/Language/Language.h"
#include "Utils/Message/Message.h"

#include "Interfaces/PlaylistInterface.h"

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
	Shutdown* shutdown;
	PlaylistCreator* playlistCreator;
	QMenu* menuFile = nullptr;
	QMenu* menuView = nullptr;
	QMenu* menuPlugins = nullptr;
	QMenu* menuHelp = nullptr;

	QAction* menuHelpAction = nullptr;

	//file
	QAction* actionOpenFile = nullptr;
	QAction* actionOpenDir = nullptr;
	QAction* sepAfterOpen = nullptr; // after open file and open dir
	QAction* sepAfterPreferences = nullptr;
	QAction* actionShutdown = nullptr;
	QAction* actionClose = nullptr;

	// view
	QAction* actionViewLibrary = nullptr;
	QAction* actionDark = nullptr;
	QAction* actionBigCover = nullptr;
	QAction* actionFullscreen = nullptr;

	// help
	QAction* actionHelp = nullptr;
	QAction* actionAbout = nullptr;
	QAction* actionLogger = nullptr;

	QAction* currentLibraryMenuAction = nullptr;

	Library::AbstractContainer* currentLibrary = nullptr;

	QLabel* heartLabel = nullptr;
	QLabel* donateLabel = nullptr;

	Private(Shutdown* shutdown, PlaylistCreator* playlistCreator, Menubar* menubar) :
		shutdown {shutdown},
		playlistCreator(playlistCreator),
		menuFile(new QMenu(menubar)),
		menuView(new QMenu(menubar)),
		menuPlugins(new QMenu(menubar)),
		menuHelp(new QMenu(menubar)),
		actionOpenFile(new QAction(menuFile)),
		actionOpenDir(new QAction(menuFile)),
		sepAfterOpen(menuFile->addSeparator()),
		sepAfterPreferences(menuFile->addSeparator()),
		actionShutdown(new QAction(menuFile)),
		actionClose(new QAction(menuFile)),
		actionViewLibrary(new QAction(menuView)),
		actionDark(new QAction(menuView)),
		actionBigCover(new QAction(menuView)),
		actionFullscreen(new QAction(menuView)),
		actionHelp(new QAction(menuHelp)),
		actionAbout(new QAction(menuHelp)),
		actionLogger(new QAction(menuHelp))
	{
		menubar->insertMenu(nullptr, menuFile);
		menubar->insertMenu(nullptr, menuPlugins);
		menubar->insertMenu(nullptr, menuView);
		menuHelpAction = menubar->insertMenu(nullptr, menuHelp); // NOLINT(cppcoreguidelines-prefer-member-initializer)

		menuFile->insertActions(
			nullptr,
			{
				actionOpenFile, actionOpenDir, sepAfterOpen, sepAfterPreferences, actionShutdown,
				actionClose
			});

		actionViewLibrary->setCheckable(true);
		actionDark->setCheckable(true);
		actionBigCover->setCheckable(true);
		actionFullscreen->setCheckable(true);

		menuView->insertActions(
			nullptr,
			{
				actionViewLibrary,
				actionBigCover,
				actionDark,
				actionFullscreen
			});

		menuHelp->insertActions(
			nullptr,
			{
				actionLogger, actionHelp, menuHelp->addSeparator(), actionAbout
			});
	}
};

Menubar::Menubar(Shutdown* shutdown, PlaylistCreator* playlistCreator, QWidget* parent) :
	Gui::WidgetTemplate<QMenuBar>(parent)
{
	m = Pimpl::make<Private>(shutdown, playlistCreator, this);

	m->actionViewLibrary->setChecked(GetSetting(Set::Lib_Show));
	m->actionViewLibrary->setText(Lang::get(Lang::Library));
	m->actionViewLibrary->setShortcut(QKeySequence("Ctrl+L"));

	m->actionBigCover->setShortcut(QKeySequence("F9"));
	ListenSetting(Set::Player_ControlStyle, Menubar::styleChanged);

	m->actionDark->setShortcut(QKeySequence("F10"));
	ListenSetting(Set::Player_ControlStyle, Menubar::styleChanged);

	m->actionFullscreen->setShortcut(QKeySequence("F11"));
	m->actionFullscreen->setChecked(GetSetting(Set::Player_Fullscreen));

	m->actionShutdown->setVisible(true);

	initDonateLink();
	initConnections();
	initLanguages();
	initSkin();
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

	auto* newLibraryMenu = library->menu();

	if(m->currentLibraryMenuAction)
	{
		this->removeAction(m->currentLibraryMenuAction);
	}

	m->currentLibraryMenuAction = nullptr;

	if(!newLibraryMenu)
	{
		showLibraryAction(false);
		return nullptr;
	}

	const auto actionText = (library->isLocal())
	                        ? Lang::get(Lang::Library)
	                        : library->displayName();

	m->currentLibraryMenuAction = this->insertMenu(m->menuHelpAction, newLibraryMenu);
	m->currentLibraryMenuAction->setText(actionText);

	const auto isLibraryVisible = GetSetting(Set::Lib_Show);
	showLibraryAction(isLibraryVisible);

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
	auto* libraryPluginHandler = Library::PluginHandler::instance();
	this->changeCurrentLibrary(libraryPluginHandler->currentLibrary());

	if(m->currentLibraryMenuAction)
	{
		m->currentLibraryMenuAction->setVisible(b);
	}
}
//
//QString getLinkColor(QWidget* parent)
//{
//	if(!Style::isDark())
//	{
//		const auto palette = parent->palette();
//		const auto color = palette.windowText().color();
//		return color.name(QColor::NameFormat::HexRgb);
//	}
//
//	return "f3841a";
//}

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

	constexpr const auto LayoutMargin = 10;
	layout->addSpacerItem(new QSpacerItem(LayoutMargin, LayoutMargin, QSizePolicy::MinimumExpanding));
	layout->addWidget(m->heartLabel);
	layout->addWidget(m->donateLabel);

	cornerWidget->setLayout(layout);
	this->setCornerWidget(cornerWidget);
}

void Menubar::pluginAdded(PlayerPlugin::Base* plugin)
{
	auto* playerPluginHandler = PlayerPlugin::Handler::instance();
	auto* action = plugin->pluginAction();

	const auto allPlugins = playerPluginHandler->allPlugins();
	if(allPlugins.size() <= 12)
	{
		const auto keySequence = QKeySequence("Shift+F" + QString::number(allPlugins.size()));
		action->setShortcut(keySequence);
	}

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
	auto* shortcutHandler = ShortcutHandler::instance();
	shortcutHandler->shortcut(ShortcutIdentifier::Quit).connect(this, this, SLOT(closeClicked()));
	shortcutHandler->shortcut(ShortcutIdentifier::Minimize).connect(this, this, SLOT(minimizeClicked()));

	shortcutChanged(ShortcutIdentifier::Invalid);

	connect(shortcutHandler, &ShortcutHandler::sigShortcutChanged, this, &Menubar::shortcutChanged);

	// Library
	auto* libraryPluginHandler = Library::PluginHandler::instance();
	connect(libraryPluginHandler, &Library::PluginHandler::sigLibrariesChanged, this, [=]() {
		this->changeCurrentLibrary(libraryPluginHandler->currentLibrary());
	});

	auto* playerPluginHandler = PlayerPlugin::Handler::instance();
	connect(playerPluginHandler, &PlayerPlugin::Handler::sigPluginAdded, this, &Menubar::pluginAdded);
}

void Menubar::initLanguages()
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
		const auto actionText = (m->currentLibrary->isLocal())
		                        ? Lang::get(Lang::Library)
		                        : m->currentLibrary->displayName();

		m->currentLibraryMenuAction->setText(actionText);
	}
}

void Menubar::languageChanged()
{
	initLanguages();
}

void Menubar::initSkin()
{

	namespace Icons = Gui::Icons;
	m->actionOpenFile->setIcon(Icons::icon(Icons::Open));
	m->actionOpenDir->setIcon(Icons::icon(Icons::Open));
	m->actionClose->setIcon(Icons::icon(Icons::Exit));
	m->actionShutdown->setIcon(Icons::icon(Icons::Shutdown));
	m->actionAbout->setIcon(Icons::icon(Icons::Info));

	const auto heartColor = QColor(243, 132, 26);
	const auto textColor = (Style::isDark()) ? heartColor : QColor();

	const auto heartLink = Util::createLink(
		"❤ ",
		heartColor,
		false,
		"https://sayonara-player.com/donations.php");
	const auto sayonaraLink = Util::createLink(
		"Sayonara",
		textColor,
		true,
		"https://sayonara-player.com/donations.php");

	m->heartLabel->setText(heartLink);
	m->donateLabel->setText(sayonaraLink);
}

void Menubar::skinChanged()
{
	initSkin();
}

void Menubar::openDirClicked()
{
	const auto dir = Gui::DirectoryChooser::getDirectory(Lang::get(Lang::OpenDir), QDir::homePath(), true, this);
	if(!dir.isEmpty())
	{
		m->playlistCreator->createPlaylist(QStringList {dir});
	}
}

void Menubar::openFilesClicked()
{
	const auto filter = Util::getFileFilter
		(
			Util::Extensions(Util::Extension::Soundfile | Util::Extension::Playlist),
			tr("Media files")
		);

	const auto list = QFileDialog::getOpenFileNames
		(
			this,
			tr("Open Media files"),
			QDir::homePath(),
			filter
		);

	if(!list.isEmpty())
	{
		m->playlistCreator->createPlaylist(list);
	}
}

void Menubar::shutdownClicked()
{
	auto* gui = new GUI_Shutdown(m->shutdown, this);
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

void Menubar::skinToggled(bool b) // NOLINT(readability-convert-member-functions-to-static)
{
	Style::setDark(b);
}

void Menubar::bigCoverToggled(bool b) // NOLINT(readability-convert-member-functions-to-static)
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

void Menubar::helpClicked() // NOLINT(readability-convert-member-functions-to-static)
{
	const auto text =
		QStringList
			{
				tr("For bug reports and feature requests please visit Sayonara's project page at GitLab"),
				Util::createLink("https://gitlab.com/luciocarreras/sayonara-player", Style::isDark()),
				"",
				tr("FAQ") + ": ",
				Util::createLink("http://sayonara-player.com/faq.php", Style::isDark()),
			};

	Message::info(text.join("<br/>"));
}

void Menubar::aboutClicked()
{
	const auto version = GetSetting(Set::Player_Version);

	constexpr const auto IconSize = 150;
	const auto pixmap = Gui::Util::pixmap("logo.png", Gui::Util::NoTheme, QSize(IconSize, IconSize), true);

	auto* aboutBox = new QMessageBox(this);
	aboutBox->setIconPixmap(pixmap);
	aboutBox->setStandardButtons(QMessageBox::Ok);
	aboutBox->setWindowTitle(tr("About Sayonara"));
	aboutBox->setText(
		QStringList
			({
				 R"(<b><font size="+2">)",
				 QString("Sayonara Player %1").arg(version),
				 "</font></b>"
			 }).join(""));

	aboutBox->setInformativeText(
		QStringList
			({
				 tr("Written by %1").arg("Michael Lugmair"),
				 "",
				 tr("License") + ": GPLv3",
				 QString("Copyright 2011-%1").arg(QDateTime::currentDateTime().date().year()),
				 Util::createLink("http://sayonara-player.com", Style::isDark()),
				 "",
				 QString("<b>%1</b>").arg(tr("Donate")),
				 Util::createLink("http://sayonara-player.com/donations.php", Style::isDark()),
				 "",
				 tr("Thanks to all the brave translators and to everyone who helps building Sayonara packages") +
				 ".<br>" +
				 tr("And special thanks to those people with local music collections") + "!"
			 }).join("<br/>"));

	aboutBox->exec();
	aboutBox->deleteLater();
}

void Menubar::shortcutChanged([[maybe_unused]] ShortcutIdentifier identifier)
{
	auto* shortcutHandler = ShortcutHandler::instance();
	const auto shortcut = shortcutHandler->shortcut(ShortcutIdentifier::ViewLibrary);
	m->actionViewLibrary->setShortcut(shortcut.sequence());
}

