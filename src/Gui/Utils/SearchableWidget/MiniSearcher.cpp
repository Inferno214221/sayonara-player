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
	bool isEnterPressed(const int key)
	{
		return (key == Qt::Key_Return) || (key == Qt::Key_Enter);
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
		       (firstChar == '/');
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
		QMap<QString, QString> searchOptions;
		SearchView* searchView;
		QLineEdit* lineEdit;
		QLabel* label;
		int maxWidth;

		Private(MiniSearcher* parent, SearchView* searchableView) :
			searchView {searchableView},
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
		connect(eventFilter, &MiniSearchEventFilter::sigEnterPressed, this, &MiniSearcher::enterPressed);
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

	void MiniSearcher::enterPressed()
	{
		if(parentWidget() && m->searchView)
		{
			m->searchView->triggerResult();
		}

		reset();
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

	void MiniSearcher::setSearchOptions(const QMap<QString, QString>& options)
	{
		m->searchOptions = options;
		auto tooltips = QStringList {};

		for(auto it = options.cbegin(); it != options.cend(); it++)
		{
			tooltips << QString("<b>/%1</b> = %2").arg(it.key()).arg(it.value());
		}

		resetToolTip(m->lineEdit);
		addToolTipText(m->lineEdit, tooltips.join("<br>"));
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
	}

	void MiniSearcher::focusOutEvent(QFocusEvent* e)
	{
		reset();
		WidgetTemplate<QFrame>::focusOutEvent(e);
	}

	void MiniSearcher::languageChanged()
	{
		resetToolTip(m->lineEdit);
		setSearchOptions(m->searchOptions);
	}

	bool MiniSearchEventFilter::eventFilter(QObject* o, QEvent* e)
	{
		switch(e->type())
		{
			case QEvent::KeyPress:
				if(auto* ke = dynamic_cast<QKeyEvent*>(e); ke && isEnterPressed(ke->key()))
				{
					emit sigEnterPressed();

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