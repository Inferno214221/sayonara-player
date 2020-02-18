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
					emit sig_tab_pressed();

					// Accept + true = EAT the event. No one else should see the event
					e->accept();
					return true;
				}
			}
			break;

		case QEvent::FocusOut:
			emit sig_focus_lost();
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
	QLineEdit*              line_edit=nullptr;
	QLabel*					label=nullptr;

	int						max_width;

	Private(MiniSearcher* parent, SearchableViewInterface* svi) :
		svi(svi),
		max_width(150)
	{
		label = new QLabel(parent);
		line_edit = new QLineEdit(parent);
		line_edit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

		reset_tooltip();
	}

	void reset_tooltip()
	{
		line_edit->setToolTip(
			"<b>" + tr("Arrow up") + "</b> = " + tr("Previous search result") + "<br/>" +
			"<b>" + tr("Arrow down") + "</b> = " + tr("Next search result") + "<br/>" +
			"<b>" + Lang::get(Lang::Key_Escape) + "</b> = " + Lang::get(Lang::Close)
		);
	}

	void add_tooltip_text(const QString& str)
	{
		if(str.isEmpty()){
			return;
		}

		QString tooltip = line_edit->toolTip();
		tooltip += "<br /><br />" + str;
		line_edit->setToolTip(tooltip);
	}
};


MiniSearcher::MiniSearcher(SearchableViewInterface* parent) :
	WidgetTemplate<QFrame>(parent->view())
{
	m = Pimpl::make<Private>(this, parent);

	QLayout* layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
	layout->setContentsMargins(5, 5, 5, 5);
	layout->addWidget(m->line_edit);
	layout->addWidget(m->label);
	setLayout(layout);

	MiniSearchEventFilter* msef = new MiniSearchEventFilter(this);
	m->line_edit->installEventFilter(msef);

	connect(msef, &MiniSearchEventFilter::sig_tab_pressed, this, &MiniSearcher::next_result);
	connect(msef, &MiniSearchEventFilter::sig_focus_lost, this, &MiniSearcher::hide);

	connect(m->line_edit, &QLineEdit::textChanged, this, &MiniSearcher::sig_text_changed);

	this->setMaximumWidth(m->max_width);
	hide();
}

MiniSearcher::~MiniSearcher() {}


void MiniSearcher::init(const QString& text)
{
	m->line_edit->setFocus();
	m->line_edit->setText(text);

	this->show();
}


bool MiniSearcher::is_initiator(QKeyEvent* event) const
{
	QString text = event->text();

	if(event->modifiers() & Qt::ControlModifier){
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

void MiniSearcher::prev_result()
{
	emit sig_find_prev_row();
	m->line_edit->setFocus();
}

void MiniSearcher::next_result()
{
	emit sig_find_next_row();
	m->line_edit->setFocus();
}

void MiniSearcher::languageChanged()
{
	m->reset_tooltip();
	set_extra_triggers(m->triggers);
}

bool MiniSearcher::check_and_init(QKeyEvent* event)
{
	if(!is_initiator(event)) {
		return false;
	}

	if(!this->isVisible()) {
		init(event->text());
		return true;
	}

	return false;
}

void MiniSearcher::reset()
{
	m->line_edit->clear();

	if(this->isVisible() && this->parentWidget()){
		parentWidget()->setFocus();
	}

	this->hide();
}

void MiniSearcher::set_extra_triggers(const QMap<QChar, QString>& triggers)
{
	m->reset_tooltip();

	m->triggers = triggers;

	QStringList tooltips;
	for(auto it=triggers.cbegin(); it != triggers.cend(); it++)
	{
		tooltips << "<b>" + QString(it.key()) + "</b> = " + it.value();
	}

	m->add_tooltip_text(tooltips.join("<br/>"));
}

QString MiniSearcher::current_text()
{
	return m->line_edit->text();
}

void MiniSearcher::set_number_results(int results)
{
	if(results < 0){
		m->label->hide();
		return;
	}

	QString text = QString("(%1)").arg(results);
	m->label->setText(text);
	m->label->show();
}

void MiniSearcher::handle_key_press(QKeyEvent* e)
{
	bool was_initialized = isVisible();
	bool initialized = check_and_init(e);

	if(initialized || was_initialized)
	{
		keyPressEvent(e);
	}
}

void MiniSearcher::keyPressEvent(QKeyEvent* event)
{
	int key = event->key();

	switch(key)
	{
		case Qt::Key_Escape:
		case Qt::Key_Enter:
		case Qt::Key_Return:
			if(this->isVisible())
			{
				reset();
				event->accept();
			}
			break;

		case Qt::Key_Down:
			if(this->isVisible())
			{
				next_result();
				event->accept();
			}
			break;

		case Qt::Key_Up:
			if(this->isVisible())
			{
				prev_result();
				event->accept();
			}
			break;

		default:
			QFrame::keyPressEvent(event);
			break;
	}
}

QRect MiniSearcher::calc_geo() const
{
	int par_width = m->svi->viewportWidth();
	int par_height = m->svi->viewportHeight();

	int target_width = m->max_width;
	int target_height = std::max(35, 10 + m->line_edit->height());

	int new_x = par_width - (target_width + 5);
	int new_y = par_height - (target_height + 5);

	QRect r(new_x, new_y, target_width, target_height);
	spLog(Log::Develop, this) << "Show Minisearcher at " << r;

	return r;
}

void MiniSearcher::showEvent(QShowEvent* e)
{
	WidgetTemplate<QFrame>::showEvent(e);
	this->setGeometry(calc_geo());
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
