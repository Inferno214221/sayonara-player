/* GUI_Lyrics.cpp */

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

#include "GUI_Lyrics.h"
#include "Gui/Lyrics/ui_GUI_Lyrics.h"

#include "Components/Lyrics/Lyrics.h"
#include "Gui/Utils/Widgets/Completer.h"
#include "Gui/Utils/Widgets/ProgressBar.h"
#include "Gui/Utils/Widgets/ProgressBar.h"
#include "Utils/Language/Language.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Settings/Settings.h"

#include <QWheelEvent>
#include <QShowEvent>
#include <QShortcut>

#include <cmath>

namespace
{
	constexpr const auto SearchIcon = "🔍";
	constexpr const auto SwitchIcon = "🔄";

	QString fatMarkdown(const QString& str)
	{
		return "### " + str;
	}
}

struct GUI_Lyrics::Private
{
	Lyrics::Lyrics* lyrics;
	Gui::ProgressBar* loadingBar {nullptr};
	bool isCloseable {true};

	Private(const bool isCloseable, QObject* parent) :
		lyrics(new Lyrics::Lyrics(parent)),
		isCloseable {isCloseable} {}
};

GUI_Lyrics::GUI_Lyrics(const bool isCloseable, QWidget* parent) :
	Widget(parent),
	m {Pimpl::make<Private>(isCloseable, this)} {}

GUI_Lyrics::~GUI_Lyrics() = default;

void GUI_Lyrics::init()
{
	if(ui)
	{
		return;
	}

	ui = std::make_shared<Ui::GUI_Lyrics>();
	ui->setupUi(this);

	ui->labHeader->setText(tr("No track loaded"));
	ui->teLyrics->setEnabled(false);

	ui->buttonBox->setVisible(m->isCloseable);
	ui->teLyrics->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui->teLyrics->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	m->loadingBar = new Gui::ProgressBar(ui->teLyrics);
	m->loadingBar->setPosition(Gui::ProgressBar::Position::Bottom);
	m->loadingBar->setVisible(false);

	const auto servers = m->lyrics->servers();
	ui->comboServers->addItems(servers);

	const auto server = GetSetting(Set::Lyrics_Server);
	const auto serverIndex = std::max(ui->comboServers->findText(server), 0);

	ui->comboServers->setCurrentIndex(serverIndex);
	ui->leArtist->setText(m->lyrics->artist());
	ui->leTitle->setText(m->lyrics->title());

	const auto zoomFactor = GetSetting(Set::Lyrics_Zoom);
	ui->sbZoom->setValue(zoomFactor);
	zoom(zoomFactor);

	connect(ui->comboServers, combo_activated_int, this, &GUI_Lyrics::lyricServerChanged);
	connect(ui->btnSearch, &QPushButton::clicked, this, &GUI_Lyrics::prepareLyrics);
	connect(ui->btnSwitch, &QPushButton::clicked, this, &GUI_Lyrics::switchPressed);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &GUI_Lyrics::sigClosed);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &GUI_Lyrics::close);
	connect(ui->sbZoom, spinbox_value_changed_int, this, [this](const auto percent) {
		zoom(percent);
	});

	connect(ui->btnSaveLyrics, &QPushButton::clicked, this, &GUI_Lyrics::saveLyricsClicked);
	connect(m->lyrics, &Lyrics::Lyrics::sigLyricsFetched, this, &GUI_Lyrics::lyricsFetched);
	connect(ui->leArtist, &QLineEdit::textChanged, this, &GUI_Lyrics::textChanged);
	connect(ui->leTitle, &QLineEdit::textChanged, this, &GUI_Lyrics::textChanged);

	textChanged({});
	languageChanged();
	setupSources();
	prepareLyrics();

	new QShortcut(QKeySequence(QKeySequence::ZoomIn), this, SLOT(zoomIn()), nullptr, Qt::WidgetWithChildrenShortcut);
	new QShortcut(QKeySequence(QKeySequence::ZoomOut), this, SLOT(zoomOut()), nullptr, Qt::WidgetWithChildrenShortcut);
}

void GUI_Lyrics::lyricServerChanged([[maybe_unused]] int idx)
{
	if(ui->comboServers->currentData().toInt() >= 0)
	{
		SetSetting(Set::Lyrics_Server, ui->comboServers->currentText());
	}

	prepareLyrics();
}

void GUI_Lyrics::saveLyricsClicked()
{
	m->lyrics->saveLyrics(ui->teLyrics->toPlainText());

	setupSources();
	setSaveButtonText();
}

void GUI_Lyrics::prepareLyrics()
{
	if(!ui)
	{
		return;
	}

	ui->teLyrics->clear();
	const auto message = tr("Looking for lyrics") + "...";
	ui->teLyrics->setMarkdown(fatMarkdown(message));

	const auto currentServerIndex = ui->comboServers->currentData().toInt();
	if(currentServerIndex < 0)
	{
		showLocalLyrics();
		return;
	}

	const auto running = m->lyrics->fetchLyrics(ui->leArtist->text(), ui->leTitle->text(), currentServerIndex);
	if(running)
	{
		m->loadingBar->show();
		m->loadingBar->setVisible(true);
		ui->btnSearch->setEnabled(false);
		ui->comboServers->setEnabled(false);
		ui->btnSaveLyrics->setEnabled(false);
	}
}

void GUI_Lyrics::showLyrics(const QString& lyrics, const QString& header, bool rich)
{
	if(ui)
	{
		if(m->lyrics->isLyricValid())
		{
			if(rich)
			{
				ui->teLyrics->setHtml(lyrics);
			}
			else
			{
				ui->teLyrics->setPlainText(lyrics);
			}
		}
		else
		{
			const auto message = tr("Sorry, could not find any lyrics for %1 by %2")
				.arg(m->lyrics->title())
				.arg(m->lyrics->artist());

			ui->teLyrics->setMarkdown(fatMarkdown(message));
		}

		ui->labHeader->setText(header);
		ui->btnSearch->setEnabled(true);
		ui->comboServers->setEnabled(true);
		ui->btnSaveLyrics->setEnabled(m->lyrics->isLyricTagSupported());
		m->loadingBar->setVisible(false);
		ui->teLyrics->setEnabled(m->lyrics->isLyricValid());
	}
}

void GUI_Lyrics::showLocalLyrics()
{
	showLyrics(m->lyrics->localLyrics(), m->lyrics->localLyricHeader(), false);
}

void GUI_Lyrics::lyricsFetched()
{
	showLyrics(m->lyrics->lyrics(), m->lyrics->lyricHeader(), true);
}

void GUI_Lyrics::setTrack(const MetaData& track)
{
	if(!track.filepath().isEmpty())
	{
		m->lyrics->setMetadata(track);
	}

	if(!ui || track.filepath().isEmpty())
	{
		return;
	}

	ui->leArtist->setText(m->lyrics->artist());
	ui->leTitle->setText(m->lyrics->title());

	auto completerEntries = QStringList()
		<< track.artist()
		<< track.albumArtist();

	completerEntries.removeDuplicates();

	if(ui->leArtist->completer())
	{
		ui->leArtist->completer()->deleteLater();
	}

	ui->leArtist->setCompleter(new Gui::Completer(completerEntries, ui->leArtist));

	setupSources();
	prepareLyrics();
	setSaveButtonText();
}

void GUI_Lyrics::switchPressed()
{
	const auto artist = ui->leArtist->text();
	const auto title = ui->leTitle->text();

	ui->leArtist->setText(title);
	ui->leTitle->setText(artist);
}

void GUI_Lyrics::zoom(int zoomFactor)
{
	const auto fontSize = QApplication::font().pointSize();
	const auto stylesheet = QString("font-size: %1pt;").arg((fontSize * zoomFactor) / 100);
	ui->teLyrics->setStyleSheet(stylesheet);

	SetSetting(Set::Lyrics_Zoom, ui->sbZoom->value());
}

void GUI_Lyrics::setupSources()
{
	ui->comboServers->clear();

	if(m->lyrics->isLyricTagAvailable())
	{
		ui->comboServers->addItem(Lang::get(Lang::File), -1);
	}

	auto i = 0;
	const auto servers = m->lyrics->servers();
	for(const auto& server: servers)
	{
		ui->comboServers->addItem(server, i++);
	}

	chooseSource();
}

void GUI_Lyrics::chooseSource()
{
	auto newIndex = 0;

	if(!m->lyrics->isLyricTagAvailable())
	{
		const auto lastServer = GetSetting(Set::Lyrics_Server);
		newIndex = std::max(0, ui->comboServers->findText(lastServer));
	}

	ui->comboServers->setCurrentIndex(newIndex);
}

void GUI_Lyrics::zoomIn()
{
	ui->sbZoom->setValue(GetSetting(Set::Lyrics_Zoom) + 10);
}

void GUI_Lyrics::zoomOut()
{
	ui->sbZoom->setValue(GetSetting(Set::Lyrics_Zoom) - 10);
}

void GUI_Lyrics::setSaveButtonText()
{
	if(!m->lyrics->isLyricTagSupported())
	{
		ui->btnSaveLyrics->setEnabled(false);
		ui->btnSaveLyrics->setText(tr("Save lyrics not supported"));
	}

	else if(m->lyrics->isLyricTagAvailable())
	{
		ui->btnSaveLyrics->setText(tr("Overwrite lyrics"));
	}

	else
	{
		ui->btnSaveLyrics->setText(tr("Save lyrics"));
	}
}

void GUI_Lyrics::textChanged(const QString& /*text*/)
{
	const auto hasText = !ui->leArtist->text().isEmpty() && !ui->leTitle->text().isEmpty();
	ui->btnSearch->setEnabled(hasText);
	ui->btnSwitch->setEnabled(hasText);
}

void GUI_Lyrics::languageChanged()
{
	if(ui)
	{
		ui->btnSearch->setText(SearchIcon + Lang::get(Lang::SearchVerb));
		ui->btnSwitch->setText(SwitchIcon + tr("Switch"));
		ui->labArtist->setText(Lang::get(Lang::Artist));
		ui->labTitle->setText(Lang::get(Lang::Title));

		setupSources();
		setSaveButtonText();
	}
}

void GUI_Lyrics::showEvent(QShowEvent* e)
{
	init();

	Widget::showEvent(e);
}

void GUI_Lyrics::wheelEvent(QWheelEvent* e)
{
	if(e->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier))
	{
		const auto deltaZoom = (e->angleDelta().y() > 0) ? 10 : -10;
		ui->sbZoom->setValue(ui->sbZoom->value() + deltaZoom);
	}
}
