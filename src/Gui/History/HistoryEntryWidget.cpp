/* HistoryEntryWidget.cpp
 *
 * Copyright (C) 2011-2024 Michael Lugmair
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

#include "HistoryEntryWidget.h"
#include "HistoryTableView.h"
#include "Components/Session/Session.h"
#include "Utils/Utils.h"
#include "Utils/Language/Language.h"
#include "Utils/Language/LanguageUtils.h"

#include <QVBoxLayout>
#include <QLabel>

namespace
{
	QString dateToString(const QDateTime& date)
	{
		const auto locale = Util::Language::getCurrentLocale();
		return locale.toString(date.date());
	}

	void setBold(QWidget* w)
	{
		auto font = w->font();
		font.setBold(true);
		w->setFont(font);
	}
}

struct HistoryEntryWidget::Private
{
	Session::Timecode timecode;

	HistoryTableView* tableView;
	QLabel* dateLabel;
	QLabel* trackLabel;

	Private(LibraryPlaylistInteractor* libraryPlaylistInteractor,
	        Session::Manager* sessionManager,
	        const Session::Timecode timecode,
	        QWidget* parent) :
		timecode {timecode},
		tableView {new HistoryTableView(libraryPlaylistInteractor, sessionManager, timecode, parent)},
		dateLabel {new QLabel(parent)},
		trackLabel {new QLabel(parent)}
	{
		setBold(dateLabel);
		setBold(trackLabel);

		dateLabel->setText(dateToString(Util::intToDate(timecode)));
		trackLabel->setText(Lang::getWithNumber(Lang::NrTracks, tableView->rows()));
	}
};

HistoryEntryWidget::HistoryEntryWidget(LibraryPlaylistInteractor* libraryPlaylistInteractor,
                                       Session::Manager* sessionManager, Session::Timecode timecode, QWidget* parent) :
	Gui::Widget {parent},
	m {Pimpl::make<Private>(libraryPlaylistInteractor, sessionManager, timecode, this)}
{
	auto* labelLayout = new QHBoxLayout();
	labelLayout->addWidget(m->dateLabel);
	labelLayout->addItem(new QSpacerItem(100, // NOLINT(readability-magic-numbers)
	                                     1, QSizePolicy::MinimumExpanding, QSizePolicy::Maximum));
	labelLayout->addWidget(m->trackLabel);

	auto* layout = new QVBoxLayout();
	layout->setSpacing(10); // NOLINT(readability-magic-numbers)
	layout->addLayout(labelLayout);
	layout->addWidget(m->tableView);
	setLayout(layout);

	connect(m->tableView, &HistoryTableView::sigRowcountChanged, this, &HistoryEntryWidget::rowcountChanged);
}

Session::Id HistoryEntryWidget::id() const { return m->timecode; }

HistoryEntryWidget::~HistoryEntryWidget() = default;

void HistoryEntryWidget::languageChanged()
{
	m->dateLabel->setText(dateToString(Util::intToDate(m->timecode)));
	m->trackLabel->setText(Lang::getWithNumber(Lang::NrTracks, m->tableView->rows()));
}

void HistoryEntryWidget::rowcountChanged()
{
	m->trackLabel->setText(Lang::getWithNumber(Lang::NrTracks, m->tableView->rows()));
}
