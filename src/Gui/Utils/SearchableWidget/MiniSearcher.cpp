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

#include <QAbstractItemView>
#include <QBoxLayout>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QScrollBar>

namespace
{
	bool isInitiator(const QMap<QChar, QString>& triggers, const QString& text, const Qt::KeyboardModifiers& modifiers)
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
		       triggers.contains(firstChar);
	}

	QRect calcGeometry(SearchView* searchableView, const int maxWidth, const int lineEditHeight)
	{
		const auto parentWidth = searchableView->viewportWidth();
		const auto parentHeight = searchableView->viewportHeight();

		const auto targetWidth = maxWidth;
		const auto targetHeight = std::max(35, 10 + lineEditHeight);

		const auto newX = parentWidth - (targetWidth + 5);
		const auto newY = parentHeight - (targetHeight + 5);

		const auto r = QRect(newX, newY, targetWidth, targetHeight);
		spLog(Log::Develop, "MiniSearcher") << "Show Minisearcher at " << r;

		return r;
	}

	void resetToolTip(QLineEdit* lineEdit)
	{
		lineEdit->setToolTip(
			"<b>" + QObject::tr("Arrow up") + "</b> = " + QObject::tr("Previous search result") + "<br/>" +
			"<b>" + QObject::tr("Arrow down") + "</b> = " + QObject::tr("Next search result") + "<br/>" +
			"<b>" + Lang::get(Lang::Key_Escape) + "</b> = " + Lang::get(Lang::Close)
		);
	}

	void addToolTipText(QLineEdit* lineEdit, const QString& text)
	{
		if(!text.isEmpty())
		{
			const auto tooltip = QString("%1<br><br>%2")
				.arg(lineEdit->toolTip())
				.arg(text);

			lineEdit->setToolTip(tooltip);
		}
	}
}

namespace Gui
{
	struct MiniSearcher::Private
	{
		QMap<QChar, QString> triggers;

		SearchView* searchableView;
		QLineEdit* lineEdit;
		QLabel* label;
		int maxWidth;

		Private(MiniSearcher* parent, SearchView* searchableView) :
			searchableView {searchableView},
			lineEdit {new QLineEdit(parent)},
			label {new QLabel(parent)},
			maxWidth {Gui::Util::textWidth(parent->fontMetrics(), "18 good characters")}
		{
			lineEdit->setObjectName("MiniSearcherLineEdit");
			lineEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

			resetToolTip(lineEdit);
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

		auto* eventFilter = new MiniSearchEventFilter(this);
		connect(eventFilter, &MiniSearchEventFilter::sigTabPressed, this, &MiniSearcher::nextResult);
		connect(eventFilter, &MiniSearchEventFilter::sigFocusLost, this, &MiniSearcher::hide);
		m->lineEdit->installEventFilter(eventFilter);

		connect(m->lineEdit, &QLineEdit::textChanged, this, &MiniSearcher::sigTextChanged);

		setMaximumWidth(m->maxWidth);

		hide();
	}

	MiniSearcher::~MiniSearcher() = default;

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

	void MiniSearcher::reset()
	{
		m->lineEdit->clear();

		if(isVisible() && parentWidget())
		{
			parentWidget()->setFocus();
		}

		hide();
	}

	void MiniSearcher::setExtraTriggers(const QMap<QChar, QString>& triggers)
	{
		m->triggers = triggers;

		auto tooltips = QStringList {};
		for(auto it = triggers.cbegin(); it != triggers.cend(); it++)
		{
			tooltips << QString("<b>%1</b> = %2").arg(it.key()).arg(it.value());
		}

		resetToolTip(m->lineEdit);
		addToolTipText(m->lineEdit, tooltips.join("<br>"));
	}

	void MiniSearcher::setNumberResults(const int results)
	{
		spLog(Log::Info, this) << "Show number of result: " << results;
		m->label->setVisible(results >= 0);
		if(results >= 0)
		{
			const auto text = QString("(%1)").arg(results);
			m->label->setText(text);
		}
	}

	void MiniSearcher::notifyViewSearchDone() {}

	bool MiniSearcher::handleKeyPress(QKeyEvent* e)
	{
		if(isInitiator(m->triggers, e->text(), e->modifiers()))
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

			case Qt::Key_Enter:
			case Qt::Key_Return:
				if(isVisible())
				{
					reset();
					notifyViewSearchDone();
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
		setGeometry(calcGeometry(m->searchableView, m->maxWidth, m->lineEdit->height()));
	}

	void MiniSearcher::focusOutEvent(QFocusEvent* e)
	{
		reset();
		WidgetTemplate<QFrame>::focusOutEvent(e);
	}

	void MiniSearcher::languageChanged()
	{
		resetToolTip(m->lineEdit);
		setExtraTriggers(m->triggers);
	}

	bool MiniSearchEventFilter::eventFilter(QObject* o, QEvent* e)
	{
		switch(e->type())
		{
			case QEvent::KeyPress:
				if(auto* ke = dynamic_cast<QKeyEvent*>(e); (ke->key() == Qt::Key_Tab))
				{
					emit sigTabPressed();

					// Accept + true = EAT the event. No one else should see the event
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