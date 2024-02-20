/* MiniSearcher.cpp */

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

#include "MiniSearcher.h"
#include "SearchableView.h"
#include "Utils/GuiUtils.h"
#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Algorithm.h"

#include <QAbstractItemView>
#include <QBoxLayout>
#include <QCompleter>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QScrollBar>
#include <QStringListModel>

namespace
{
	constexpr const auto CommandPrefix = ':';
	constexpr const auto SearchPrefix = '/';

	bool isEnterPressed(const int key) { return (key == Qt::Key_Return) || (key == Qt::Key_Enter); }

	bool isCommand(const QString& str) { return str.startsWith(CommandPrefix); }

	QString extractCommand(const QString& str)
	{
		return str.startsWith(CommandPrefix)
		       ? str.right(str.size() - 1)
		       : QString {};
	}

	bool isInitiator(const QString& text, const Qt::KeyboardModifiers& modifiers)
	{
		const auto modifierMask = (modifiers & Qt::ControlModifier) |
		                          (modifiers & Qt::MetaModifier) |
		                          (modifiers & Qt::AltModifier);

		if((modifierMask != 0) || text.isEmpty())
		{
			return false;
		}

		const auto firstChar = text[0];
		return firstChar.isLetterOrNumber() ||
		       (firstChar == CommandPrefix) ||
		       (firstChar == SearchPrefix);
	}

	QRect calcGeometry(SearchView* searchableView, const int maxWidth, const int lineEditHeight)
	{
		const auto geometry = searchableView->viewportGeometry();
		const auto parentWidth = geometry.width();
		const auto parentHeight = geometry.height();

		const auto targetWidth = maxWidth;
		const auto targetHeight = std::max(35, 10 + lineEditHeight);

		const auto newX = parentWidth - (targetWidth + 5);
		const auto newY = parentHeight - (targetHeight + 5);

		return {newX, newY, targetWidth, targetHeight};
	}

	void addTooltip(QLineEdit* lineEdit, const QString& title, const QString& indicator,
	                const QMap<QString, QString>& items)
	{
		auto entries = QStringList {};
		for(auto it = items.cbegin(); it != items.cend(); it++)
		{
			entries << QString("<b>%1%2</b> = %3")
				.arg(indicator)
				.arg(it.key())
				.arg(it.value());
		}

		if(const auto text = entries.join("<br>"); !text.isEmpty())
		{
			auto oldTooltip = lineEdit->toolTip();
			if(!oldTooltip.isEmpty())
			{
				oldTooltip.append("<br><br>");
			}

			const auto tooltip = QString("%1%2<br>%3")
				.arg(oldTooltip)
				.arg(title)
				.arg(text);

			lineEdit->setToolTip(tooltip);
		}
	}

	void resetToolTip(QLineEdit* lineEdit, const QMap<QString, QString>& searchOptions,
	                  const QMap<QString, QString>& commands)
	{
		lineEdit->setToolTip({});

		addTooltip(lineEdit, QObject::tr("Help"), {}, {
			{QObject::tr("Arrow up"),     QObject::tr("Previous search result")},
			{QObject::tr("Arrow down"),   QObject::tr("Next search result")},
			{Lang::get(Lang::Key_Escape), Lang::get(Lang::Close)}
		});

		addTooltip(lineEdit, Lang::get(Lang::SearchVerb), {SearchPrefix}, searchOptions);
		addTooltip(lineEdit, QObject::tr("Commands"), {CommandPrefix}, commands);
	}

	QStringList createStringList(const QChar& initiator, const QMap<QString, QString>& items)
	{
		auto stringList = QStringList {};
		for(auto it = items.begin(); it != items.end(); it++)
		{
			stringList << initiator + it.key();
		}
		return stringList;
	}

	QCompleter* createCompleter(QWidget* parent, SearchView* searchView)
	{
		const auto strings = QStringList()
			<< createStringList(CommandPrefix, searchView->commands())
			<< createStringList(SearchPrefix, searchView->searchOptions());

		auto* stringListModel = new QStringListModel(strings);
		auto* completer = new QCompleter(parent);

		completer->setModel(stringListModel);
		completer->setCaseSensitivity(Qt::CaseInsensitive);
		completer->setCompletionMode(QCompleter::CompletionMode::InlineCompletion);

		return completer;
	}
}

namespace Gui
{
	struct MiniSearcher::Private
	{
		QMap<QString, QString> searchOptions;
		QMap<QString, QString> commands;
		SearchView* searchView;
		QLineEdit* lineEdit;
		QCompleter* completer;
		QLabel* label;
		int maxWidth;

		Private(MiniSearcher* parent, SearchView* searchableView) :
			searchView {searchableView},
			lineEdit {new QLineEdit(parent)},
			completer {createCompleter(lineEdit, searchableView)},
			label {new QLabel(parent)},
			maxWidth {Gui::Util::textWidth(parent->fontMetrics(), "18 good characters")}
		{
			lineEdit->setObjectName("MiniSearcherLineEdit");
			lineEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
			lineEdit->setCompleter(completer);
		}
	};

	MiniSearcher::MiniSearcher(SearchView* parent) :
		WidgetTemplate<QFrame>(parent->widget())
	{
		m = Pimpl::make<Private>(this, parent);

		auto* layout = new QHBoxLayout(this);
		layout->setContentsMargins(5, 5, 5, 5); // NOLINT(readability-magic-numbers)
		layout->addWidget(m->lineEdit);
		layout->addWidget(m->label);
		setLayout(layout);
		setMaximumWidth(m->maxWidth);

		resetToolTip(m->lineEdit, m->searchView->searchOptions(), m->searchView->commands());

		initConnections();
		hide();
	}

	MiniSearcher::~MiniSearcher() = default;

	void MiniSearcher::initConnections()
	{
		auto* eventFilter = new MiniSearchEventFilter(this);
		connect(eventFilter, &MiniSearchEventFilter::sigEnterPressed, this, &MiniSearcher::enterPressed);
		connect(eventFilter, &MiniSearchEventFilter::sigTabPressed, this, &MiniSearcher::tabPressed);
		connect(eventFilter, &MiniSearchEventFilter::sigFocusLost, this, &MiniSearcher::hide);
		m->lineEdit->installEventFilter(eventFilter);

		connect(m->lineEdit, &QLineEdit::textChanged, this, &MiniSearcher::textChanged);
	}

	void MiniSearcher::previousResult()
	{
		emit sigFindPrevRow();
		m->lineEdit->setFocus();
	}

	void MiniSearcher::nextResult()
	{
		emit sigFindNextRow();
		m->lineEdit->setFocus();
	}

	void MiniSearcher::enterPressed()
	{
		if(parentWidget() && m->searchView)
		{
			if(const auto text = m->lineEdit->text(); isCommand(text))
			{
				m->searchView->runCommand(extractCommand(text));
			}
			else
			{
				m->searchView->triggerResult();
			}
		}

		reset();
	}

	void MiniSearcher::tabPressed()
	{
		const auto currentCompletion = m->completer->currentCompletion();
		if(currentCompletion.isEmpty())
		{
			return;
		}

		if(m->lineEdit->cursorPosition() != currentCompletion.size())
		{
			m->lineEdit->setText(currentCompletion);
		}

		else if(m->completer->completionCount() > 0)
		{
			if(!m->completer->setCurrentRow(m->completer->currentRow() + 1))
			{
				m->completer->setCurrentRow(0);
			}

			m->lineEdit->setText(m->completer->currentCompletion());
		}
	}

	void MiniSearcher::textChanged(const QString& text)
	{
		if(!isCommand(text))
		{
			emit sigTextChanged(text);
		}
	}

	void MiniSearcher::reset()
	{
		m->lineEdit->blockSignals(true);
		m->lineEdit->clear();
		m->lineEdit->blockSignals(false);

		if(isVisible() && parentWidget())
		{
			parentWidget()->setFocus();
		}

		hide();
	}

	void MiniSearcher::resetCompleter()
	{
		m->completer = createCompleter(m->lineEdit, m->searchView);
		m->lineEdit->setCompleter(m->completer);
	}

	void MiniSearcher::setNumberResults(const int results)
	{
		m->label->setVisible(results >= 0);
		if(results >= 0)
		{
			const auto text = QString("(%1)").arg(results);
			m->label->setText(text);
		}
	}

	bool MiniSearcher::handleKeyPress(QKeyEvent* e)
	{
		if(isInitiator(e->text(), e->modifiers()))
		{
			m->lineEdit->setFocus();
			m->lineEdit->setText(e->text());

			show();
		}

		if(isVisible())
		{
			keyPressEvent(e);
			return true;
		}

		return false;
	}

	void MiniSearcher::keyPressEvent(QKeyEvent* e)
	{
		switch(e->key())
		{
			case Qt::Key_Escape:
				if(isVisible())
				{
					reset();
					e->accept();
				}
				break;

			case Qt::Key_Down:
				if(isVisible())
				{
					nextResult();
					e->accept();
				}
				break;

			case Qt::Key_Up:
				if(isVisible())
				{
					previousResult();
					e->accept();
				}
				break;

			default:
				QFrame::keyPressEvent(e);
				break;
		}
	}

	void MiniSearcher::showEvent(QShowEvent* e)
	{
		WidgetTemplate<QFrame>::showEvent(e);
		setGeometry(calcGeometry(m->searchView, m->maxWidth, m->lineEdit->height()));
		resetToolTip(m->lineEdit, m->searchView->searchOptions(), m->searchView->commands());
		resetCompleter();
	}

	void MiniSearcher::focusOutEvent(QFocusEvent* e)
	{
		reset();
		WidgetTemplate<QFrame>::focusOutEvent(e);
	}

	void MiniSearcher::languageChanged()
	{
		resetToolTip(m->lineEdit, m->searchView->searchOptions(), m->searchView->commands());
	}

	bool MiniSearchEventFilter::eventFilter(QObject* o, QEvent* e)
	{
		switch(e->type())
		{
			case QEvent::KeyPress:
				// Accept + true = EAT the event. No one else should see the event

				if(auto* ke = dynamic_cast<QKeyEvent*>(e); ke && isEnterPressed(ke->key()))
				{
					emit sigEnterPressed();
					e->accept();
					return true;
				}

				else if(ke && (ke->key() == Qt::Key_Tab))
				{
					emit sigTabPressed();
					e->accept();
					return true;
				}

				break;

			case QEvent::FocusOut:
				emit sigFocusLost();
				break;

			default:
				break;
		}

		return QObject::eventFilter(o, e);
	}
}