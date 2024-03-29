/* GUI_History.cpp
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

#include "GUI_History.h"
#include "Gui/History/ui_GUI_History.h"
#include "DoubleCalendarDialog.h"
#include "HistoryEntryWidget.h"

#include "Components/Session/Session.h"
#include "Gui/Utils/Style.h"
#include "Gui/Utils/Widgets/CalendarWidget.h"
#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"
#include "Utils/Language/LanguageUtils.h"
#include "Utils/Set.h"

#include <QAction>
#include <QDate>
#include <QDialog>
#include <QScrollArea>
#include <QPushButton>
#include <QApplication>
#include <QShortcut>

namespace
{
	QWidget* createPage(LibraryPlaylistInteractor* libraryPlaylistInteractor, Session::Manager* sessionManager,
	                    const Session::EntryListMap& history)
	{
		auto* page = new QWidget();
		auto* pageLayout = new QVBoxLayout(page);
		pageLayout->setContentsMargins(0, 0, 0, 0);
		page->setLayout(pageLayout);

		auto sessionIds = history.keys();
		Util::Algorithm::sort(sessionIds, [](auto id1, auto id2) {
			return (id1 > id2);
		});

		Util::Set<Session::Timecode> timecodes;
		for(const auto sessionId: sessionIds)
		{
			const auto dayBegin = Session::dayBegin(sessionId);
			if(!timecodes.contains(dayBegin))
			{
				timecodes << dayBegin;
				auto* section = new HistoryEntryWidget(libraryPlaylistInteractor, sessionManager, dayBegin, page);
				page->layout()->addWidget(section);
				QObject::connect(sessionManager, &Session::Manager::sigSessionDeleted,
				                 page, [section, sessionId](const auto deletedSessionId) {
						if(sessionId == deletedSessionId)
						{
							section->deleteLater();
						}
					});
			}
		}

		return page;
	}

	QShortcut* createShortcut(const QKeySequence& keySequence, QWidget* parent)
	{
		auto* sc = new QShortcut(parent);
		sc->setKey(keySequence);
		sc->setContext(Qt::ShortcutContext::WidgetWithChildrenShortcut);

		return sc;
	}
}

struct GUI_History::Private
{
	LibraryPlaylistInteractor* libraryPlaylistInteractor;
	Session::Manager* sessionManager;

	QWidget* dateRangeWidget = nullptr;
	QDate startDate, endDate;
	QMap<Session::Id, QWidget*> enrtryWidgets;

	QPushButton* btnLoadMore {new QPushButton(tr("Load more entries"))};
	QAction* actionGoToBottom {new QAction()};
	QAction* actionGoToTop {new QAction()};
	QAction* actionSelecteDataRange {new QAction()};
	QAction* actionClearAllHistory {new QAction()};
	QAction* actionClearOldHistory {new QAction()};

	int lastPage {-1};

	Private(LibraryPlaylistInteractor* libraryPlaylistInteractor, Session::Manager* sessionManager) :
		libraryPlaylistInteractor {libraryPlaylistInteractor},
		sessionManager {sessionManager} {}
};

GUI_History::GUI_History(LibraryPlaylistInteractor* libraryPlaylistInteractor, Session::Manager* sessionManager,
                         QWidget* parent) :
	Gui::Dialog(parent),
	m {Pimpl::make<Private>(libraryPlaylistInteractor, sessionManager)},
	ui {std::make_shared<Ui::GUI_History>()}
{
	ui->setupUi(this);
	ui->scrollAreaWidgetContents->setLayout(new QVBoxLayout());

	ui->toolButton->registerAction(m->actionGoToBottom);
	ui->toolButton->registerAction(m->actionGoToTop);
	ui->toolButton->registerAction(m->actionSelecteDataRange);
	ui->toolButton->registerAction(m->actionClearAllHistory);
	ui->toolButton->registerAction(m->actionClearOldHistory);

	ui->stackedWidget->setCurrentIndex(0);

	requestData(0);

	connect(m->btnLoadMore, &QPushButton::clicked, this, &GUI_History::loadMore);
	connect(m->actionGoToTop, &QAction::triggered, this, &GUI_History::scrollToTop);
	connect(m->actionGoToBottom, &QAction::triggered, this, &GUI_History::scrollToBottom);
	connect(m->actionSelecteDataRange, &QAction::triggered, this, &GUI_History::dateRangeClicked);
	connect(m->actionClearAllHistory, &QAction::triggered, this, &GUI_History::clearAllHistoryClicked);
	connect(m->actionClearOldHistory, &QAction::triggered, this, &GUI_History::clearOldHistoryClicked);

	connect(ui->btnClear, &QPushButton::clicked, this, &GUI_History::clearRangeClicked);

	initShortcuts();
}

GUI_History::~GUI_History() = default;

QFrame* GUI_History::header() const { return ui->header; }

void GUI_History::initShortcuts()
{
	m->actionGoToTop->setShortcut(QKeySequence(Qt::Key_Home));
	m->actionGoToBottom->setShortcut(QKeySequence(Qt::Key_End));
	m->actionSelecteDataRange->setShortcut(QKeySequence("Ctrl+r"));

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
	m->actionGoToTop->setShortcutVisibleInContextMenu(true);
	m->actionGoToBottom->setShortcutVisibleInContextMenu(true);
	m->actionSelecteDataRange->setShortcutVisibleInContextMenu(true);
#endif

	connect(createShortcut({Qt::Key_Home}, this), &QShortcut::activated, this, &GUI_History::scrollToTop);
	connect(createShortcut({Qt::Key_End}, this), &QShortcut::activated, this, &GUI_History::scrollToBottom);
	connect(createShortcut({"Ctrl+r"}, this), &QShortcut::activated, this, &GUI_History::dateRangeClicked);
}

void GUI_History::scrollToTop()
{
	const auto widgets = {ui->scrollArea, ui->scrollAreaRange};
	for(auto* widget: widgets)
	{
		if(widget->isVisible())
		{
			widget->ensureVisible(0, 0, 0, 0);
		}
	}
}

void GUI_History::scrollToBottom()
{
	const auto widgets = {ui->scrollArea, ui->scrollAreaRange};
	for(auto* widget: widgets)
	{
		if(widget->isVisible())
		{
			widget->ensureVisible(0, widget->widget()->height(), 0, 0);
		}
	}

	m->btnLoadMore->setFocus();
}

void GUI_History::loadMore()
{
	requestData(m->lastPage + 1);
}

void GUI_History::dateRangeClicked()
{
	m->startDate = {};
	m->endDate = {};

	auto* calendarWidget = new Gui::DoubleCalendarDialog(this);
	connect(calendarWidget, &Gui::DoubleCalendarDialog::sigAccepted, this, &GUI_History::calendarFinished);
	connect(calendarWidget, &Gui::DoubleCalendarDialog::sigRejected, calendarWidget, &QObject::deleteLater);

	calendarWidget->resizeRelative(this, 1.0, QSize(800, 600)); // NOLINT(readability-magic-numbers)
	calendarWidget->show();
}

void GUI_History::calendarFinished()
{
	auto* calendarWidget = dynamic_cast<Gui::DoubleCalendarDialog*>(sender());

	m->startDate = calendarWidget->startDate();
	m->endDate = calendarWidget->endDate();

	calendarWidget->deleteLater();

	loadSelectedDateRange();
}

void GUI_History::languageChanged()
{
	m->actionGoToTop->setText(tr("Scroll to top"));
	m->actionGoToBottom->setText(tr("Scroll to bottom"));
	m->actionSelecteDataRange->setText(tr("Select date range") + "...");
	m->actionClearOldHistory->setText(tr("Clear history by date"));
	m->actionClearAllHistory->setText(tr("Clear all history"));
}

void GUI_History::clearRangeClicked()
{
	ui->stackedWidget->setCurrentIndex(0);
}

void GUI_History::requestData(const int index)
{
	const auto history = m->sessionManager->historyEntries(index, 10);
	m->btnLoadMore->setDisabled(history.isEmpty());

	auto* page = createPage(m->libraryPlaylistInteractor, m->sessionManager, history);
	const auto oldHeight = ui->scrollArea->widget()->height();

	ui->scrollAreaWidgetContents->layout()->removeWidget(m->btnLoadMore);
	ui->scrollAreaWidgetContents->layout()->addWidget(page);
	ui->scrollAreaWidgetContents->layout()->addWidget(m->btnLoadMore);

	m->lastPage = index;
	if(m->lastPage > 0)
	{
		ui->scrollArea->widget()->resize(ui->scrollArea->widget()->sizeHint());
		QApplication::processEvents();

		const auto viewportHeight = ui->scrollArea->viewport()->height();
		ui->scrollArea->ensureVisible(0, oldHeight + viewportHeight - m->btnLoadMore->height(), 0, 0);
	}
}

void GUI_History::loadSelectedDateRange()
{
	if(!ui->scrollAreaRangeContents->layout())
	{
		auto* layout = new QVBoxLayout();
		layout->setContentsMargins(0, 0, 0, 0);
		ui->scrollAreaRangeContents->setLayout(layout);
	}

	else
	{
		auto* layout = ui->scrollAreaRangeContents->layout();
		layout->removeWidget(m->dateRangeWidget);
		m->dateRangeWidget->deleteLater();
	}

	const auto locale = Util::Language::getCurrentLocale();
	ui->labFrom->setText(locale.toString(m->startDate));
	ui->labUntil->setText(locale.toString(m->endDate));

	const auto history = m->sessionManager->history(
		m->startDate.startOfDay(),
		m->endDate.endOfDay());

	m->dateRangeWidget = createPage(m->libraryPlaylistInteractor, m->sessionManager, history);

	ui->scrollAreaRangeContents->layout()->addWidget(m->dateRangeWidget);
	ui->stackedWidget->setCurrentIndex(1);
}

void GUI_History::clearAllHistoryClicked()
{
	m->sessionManager->clearAllHistory();
	assureOneTable();
}

void GUI_History::clearOldHistoryClicked()
{
	auto dialog = Gui::CalendarDialog(tr("Delete history"),
	                                  tr("Delete history which is older than the specified date."),
	                                  this);

	if(dialog.exec() == QDialog::Accepted)
	{
		const auto dateTime = dialog.selectedDate().startOfDay();
		m->sessionManager->clearAllHistoryBefore(dateTime);
	}

	assureOneTable();
}

void GUI_History::assureOneTable()
{
	if(m->sessionManager->isEmpty())
	{
		requestData(0);
	}
}
