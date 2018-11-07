/* MiniSearcher.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
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
#include "Utils/Language.h"
#include "GUI/Utils/GuiUtils.h"

#include <QBoxLayout>
#include <QScrollBar>
#include <QLineEdit>
#include <QAbstractItemView>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QLabel>


bool MiniSearchEventFilter::eventFilter(QObject* o, QEvent* e)
{
	switch(e->type())
	{
		case QEvent::KeyPress:
			{
				QKeyEvent* ke = static_cast<QKeyEvent*>(e);
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

	QLineEdit*              line_edit=nullptr;
	QAbstractItemView*		parent=nullptr;
	QLabel*					label=nullptr;

	int						padding;

	Private() :
		padding(0)
	{}
};


MiniSearcher::MiniSearcher(QAbstractItemView* parent) :
	WidgetTemplate<QFrame>(parent)
{
	m = Pimpl::make<MiniSearcher::Private>();
	m->parent = parent;

	QLayout* layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
	this->setLayout(layout);
	this->setMaximumWidth(150);

	MiniSearchEventFilter* msef = new MiniSearchEventFilter(this);

	m->label = new QLabel(this);
	m->line_edit = new QLineEdit(this);
	m->line_edit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	m->line_edit->installEventFilter(msef);
	reset_tooltip();

	layout->setContentsMargins(5, 5, 5, 5);
	layout->addWidget(m->line_edit);
	layout->addWidget(m->label);

	connect(m->line_edit, &QLineEdit::textChanged, this, &MiniSearcher::sig_text_changed);
	connect(msef, &MiniSearchEventFilter::sig_tab_pressed, this, &MiniSearcher::right_clicked);
	connect(msef, &MiniSearchEventFilter::sig_focus_lost, this, &MiniSearcher::line_edit_focus_lost);

	this->hide();
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


void MiniSearcher::line_edit_focus_lost()
{
	this->setVisible(false);
}

void MiniSearcher::left_clicked()
{
	emit sig_find_prev_row();
	m->line_edit->setFocus();
}

void MiniSearcher::right_clicked()
{
	emit sig_find_next_row();
	m->line_edit->setFocus();
}

void MiniSearcher::language_changed()
{
	reset_tooltip();
	set_extra_triggers(m->triggers);
}

void MiniSearcher::reset()
{
	m->line_edit->clear();

	if(this->isVisible()){
		m->parent->setFocus();
	}

	this->hide();
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


void MiniSearcher::set_extra_triggers(const QMap<QChar, QString>& triggers)
{
	reset_tooltip();

	m->triggers = triggers;
	QString tooltip;

	for(auto it=triggers.cbegin(); it != triggers.cend(); it++)
	{
		tooltip += "<b>" + QString(it.key()) + "</b> = " + it.value() + "<br />";
	}

	add_tooltip_text(tooltip);
}


QString MiniSearcher::get_current_text()
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

void MiniSearcher::add_tooltip_text(const QString& str)
{
	QString tooltip = m->line_edit->toolTip();
	tooltip += "<br /><br />" + str;
	m->line_edit->setToolTip(tooltip);
}

void MiniSearcher::reset_tooltip()
{
	m->line_edit->setToolTip(
		"<b>" + tr("Arrow up") + "</b> = " + tr("Previous search result") + "<br/>" +
		"<b>" + tr("Arrow down") + "</b> = " + tr("Next search result") + "<br/>" +
		"<b>" + Lang::get(Lang::Key_Escape) + "</b> = " + Lang::get(Lang::Close)
	);
}


void MiniSearcher::set_padding(int padding)
{
	m->padding = padding;

	QRect geo = calc_geo();
	this->setGeometry(geo);
}


void MiniSearcher::handle_key_press(QKeyEvent *e)
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
				right_clicked();
				event->accept();
			}
			break;

		case Qt::Key_Up:
			if(this->isVisible())
			{
				left_clicked();
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
	QScrollBar* v_scrollbar = m->parent->verticalScrollBar();
	QScrollBar* h_scrollbar = m->parent->horizontalScrollBar();

	int sb_width = v_scrollbar->width();
	int sb_height = h_scrollbar->height();

	int par_width = m->parent->width();
	int par_height = m->parent->height();

	if(!v_scrollbar->isVisible()) sb_width = 0;
	if(!h_scrollbar->isVisible()) sb_height = 0;

	par_width -= sb_width;
	par_height -= sb_height;

	int target_width = 150;
	int target_height = 35;
	int new_x = par_width - (target_width + 5);
	int new_y = par_height - (target_height + 5 + m->padding);

	return QRect(new_x, new_y, target_width, target_height);
}

void MiniSearcher::showEvent(QShowEvent* e)
{
	QRect geo = calc_geo();
	this->setGeometry(geo);

	WidgetTemplate<QFrame>::showEvent(e);
}


void MiniSearcher::focusOutEvent(QFocusEvent* e)
{
	this->reset();

	WidgetTemplate<QFrame>::focusOutEvent(e);
}
