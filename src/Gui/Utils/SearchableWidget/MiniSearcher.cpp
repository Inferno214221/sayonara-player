/* MiniSearcher.cpp */

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

#include "MiniSearcher.h"
#include "SearchableView.h"
#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"

#include <QBoxLayout>
#include <QScrollBar>
#include <QLineEdit>
#include <QAbstractItemView>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QLabel>

using Gui::MiniSearchEventFilter;
using Gui::MiniSearcher;

bool MiniSearchEventFilter::eventFilter(QObject* o, QEvent* e)
{
	switch(e->type())
	{
		case QEvent::KeyPress:
			{
				auto* ke = static_cast<QKeyEvent*>(e);
				if(ke->key() == Qt::Key_Tab)
				{
					emit sigTabPressed();

					// Accept + true = EAT the event. No one else should see the event
					e->accept();
					return true;
				}
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


struct MiniSearcher::Private
{
	QMap<QChar, QString>    triggers;

	SearchableViewInterface*	svi=nullptr;
	QLineEdit*              lineEdit=nullptr;
	QLabel*					label=nullptr;

	int						maxWidth;

	Private(MiniSearcher* parent, SearchableViewInterface* svi) :
		svi(svi),
		maxWidth(150)
	{
		label = new QLabel(parent);
		lineEdit = new QLineEdit(parent);
		lineEdit->setObjectName("MiniSearcherLineEdit");
		lineEdit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

		resetToolTip();
	}

	void resetToolTip()
	{
		lineEdit->setToolTip(
			"<b>" + tr("Arrow up") + "</b> = " + tr("Previous search result") + "<br/>" +
			"<b>" + tr("Arrow down") + "</b> = " + tr("Next search result") + "<br/>" +
			"<b>" + Lang::get(Lang::Key_Escape) + "</b> = " + Lang::get(Lang::Close)
		);
	}

	void addToolTipText(const QString& str)
	{
		if(str.isEmpty()){
			return;
		}

		QString tooltip = lineEdit->toolTip();
		tooltip += "<br /><br />" + str;
		lineEdit->setToolTip(tooltip);
	}
};


MiniSearcher::MiniSearcher(SearchableViewInterface* parent) :
	WidgetTemplate<QFrame>(parent->view())
{
	m = Pimpl::make<Private>(this, parent);

	auto* layout = new QHBoxLayout(this);
	layout->setContentsMargins(5, 5, 5, 5);
	layout->addWidget(m->lineEdit);
	layout->addWidget(m->label);
	setLayout(layout);

	auto* msef = new MiniSearchEventFilter(this);
	connect(msef, &MiniSearchEventFilter::sigTabPressed, this, &MiniSearcher::nextResult);
	connect(msef, &MiniSearchEventFilter::sigFocusLost, this, &MiniSearcher::hide);
	m->lineEdit->installEventFilter(msef);

	connect(m->lineEdit, &QLineEdit::textChanged, this, &MiniSearcher::sigTextChanged);

	this->setMaximumWidth(m->maxWidth);
	hide();
}

MiniSearcher::~MiniSearcher() = default;

void MiniSearcher::init(const QString& text)
{
	m->lineEdit->setFocus();
	m->lineEdit->setText(text);

	this->show();
}

bool MiniSearcher::isInitiator(QKeyEvent* event) const
{
	QString text = event->text();
	auto mod =	(event->modifiers() & Qt::ControlModifier) |
				(event->modifiers() & Qt::MetaModifier) |
				(event->modifiers() & Qt::AltModifier);

	if(mod != 0){
		return false;
	}

	if(text.isEmpty()){
		return false;
	}

	if(text[0].isLetterOrNumber()){
		return true;
	}

	if(m->triggers.contains(text[0]) ){
		return true;
	}

	return false;
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

bool MiniSearcher::checkAndInit(QKeyEvent* event)
{
	if(!isInitiator(event)) {
		return false;
	}

	if(!this->isVisible())
	{
		init(event->text());
		return true;
	}

	return false;
}

void MiniSearcher::reset()
{
	m->lineEdit->clear();

	if(this->isVisible() && this->parentWidget()){
		parentWidget()->setFocus();
	}

	this->hide();
}

void MiniSearcher::setExtraTriggers(const QMap<QChar, QString>& triggers)
{
	m->resetToolTip();

	m->triggers = triggers;

	QStringList tooltips;
	for(auto it=triggers.cbegin(); it != triggers.cend(); it++)
	{
		tooltips << "<b>" + QString(it.key()) + "</b> = " + it.value();
	}

	m->addToolTipText(tooltips.join("<br/>"));
}

QString MiniSearcher::currentText()
{
	return m->lineEdit->text();
}

void MiniSearcher::setNumberResults(int results)
{
	if(results < 0){
		m->label->hide();
		return;
	}

	QString text = QString("(%1)").arg(results);
	m->label->setText(text);
	m->label->show();
}


QRect MiniSearcher::calcGeometry() const
{
	int parentWidth = m->svi->viewportWidth();
	int parentHeight = m->svi->viewportHeight();

	int targetWidth = m->maxWidth;
	int targetHeight = std::max(35, 10 + m->lineEdit->height());

	int newX = parentWidth - (targetWidth + 5);
	int newY = parentHeight - (targetHeight + 5);

	QRect r(newX, newY, targetWidth, targetHeight);
	spLog(Log::Develop, this) << "Show Minisearcher at " << r;

	return r;
}

void MiniSearcher::notifyViewSearchDone()
{
	if(this->parentWidget() && m->svi)
	{
		m->svi->searchDone();
	}
}

bool MiniSearcher::handleKeyPress(QKeyEvent* e)
{
	bool wasInitialized = isVisible();
	bool initialized = checkAndInit(e);

	if(initialized || wasInitialized)
	{
		keyPressEvent(e);
		return true;
	}

	return false;
}

void MiniSearcher::keyPressEvent(QKeyEvent* event)
{
	int key = event->key();

	switch(key)
	{
		case Qt::Key_Escape:
			if(this->isVisible())
			{
				reset();
				event->accept();
			}
			break;

		case Qt::Key_Enter:
		case Qt::Key_Return:
			if(this->isVisible())
			{
				reset();
				notifyViewSearchDone();
				event->accept();
			}
			break;

		case Qt::Key_Down:
			if(this->isVisible())
			{
				nextResult();
				event->accept();
			}
			break;

		case Qt::Key_Up:
			if(this->isVisible())
			{
				previousResult();
				event->accept();
			}
			break;

		default:
			QFrame::keyPressEvent(event);
			break;
	}
}

void MiniSearcher::showEvent(QShowEvent* e)
{
	WidgetTemplate<QFrame>::showEvent(e);
	this->setGeometry(calcGeometry());
}

void MiniSearcher::hideEvent(QHideEvent* e)
{
	WidgetTemplate<QFrame>::hideEvent(e);
	spLog(Log::Develop, this) << "Hide Minisearcher";
}

void MiniSearcher::focusOutEvent(QFocusEvent* e)
{
	this->reset();

	WidgetTemplate<QFrame>::focusOutEvent(e);
}

void MiniSearcher::languageChanged()
{
	m->resetToolTip();
	setExtraTriggers(m->triggers);
}
