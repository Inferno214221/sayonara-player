/* GUI_PreferenceDialog.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#include "GUI_PreferenceDialog.h"
#include "Gui/ui_GUI_PreferenceDialog.h"

#include "Components/Preferences/PreferenceRegistry.h"

#include "Gui/Preferences/PreferenceWidget.h"
#include "Gui/Preferences/PreferenceAction.h"

#include "Utils/globals.h"
#include "Utils/Algorithm.h"
#include "Utils/Message/Message.h"
#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"

#include "Gui/Utils/Icons.h"
#include "Gui/Utils/Delegates/StyledItemDelegate.h"

#include <QLayout>
#include <QMenu>

using Preferences::Base;
using Preferences::Action;
namespace Algorithm=Util::Algorithm;

struct GUI_PreferenceDialog::Private
{
	QList<Base*>	pref_widgets;
	Action*			action=nullptr;
};

GUI_PreferenceDialog::GUI_PreferenceDialog(QWidget* parent) :
	Gui::Dialog(parent),
	PreferenceUi()
{
	m = Pimpl::make<Private>();

	PreferenceRegistry::instance()->set_user_interface(this);
}

GUI_PreferenceDialog::~GUI_PreferenceDialog()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}

void GUI_PreferenceDialog::register_preference_dialog(Base* pref_widget)
{
	m->pref_widgets << pref_widget;
	PreferenceRegistry::instance()->register_preference(pref_widget->identifier());
}

void GUI_PreferenceDialog::show_preference(const QString& identifier)
{
	init_ui();

	int i=0;
	for(Preferences::Base* pwi : Algorithm::AsConst(m->pref_widgets))
	{
		QString dialog_id = pwi->identifier();
		if(identifier.compare(dialog_id) == 0)
		{
			ui->list_preferences->setCurrentRow(i);
			row_changed(i);

			this->setModal(true);
			this->show();

			return;
		}

		i++;
	}

	sp_log(Log::Warning, this) << "Cannot find preference widget " << identifier;
}


void GUI_PreferenceDialog::language_changed()
{
	ui->retranslateUi(this);

	bool is_empty = (ui->list_preferences->count() == 0);

	int i=0;
	for(Base* dialog : Algorithm::AsConst(m->pref_widgets))
	{
		QListWidgetItem* item;
		if(is_empty){
			item = new QListWidgetItem(dialog->action_name());
			ui->list_preferences->addItem(item);
		}
		else{
			item = ui->list_preferences->item(i);
			item->setText(dialog->action_name());
		}

		i++;
	}

	if(m->action){
		m->action->setText(action_name() + "...");
	}
}


QString GUI_PreferenceDialog::action_name() const
{
	return Lang::get(Lang::Preferences);
}

QAction* GUI_PreferenceDialog::action()
{
	// action has to be initialized here, because pure
	// virtual get_action_name should not be called from ctor
	QString name = action_name();
	if(!m->action){
		m->action = new Action(name, this);
	}

	m->action->setText(action_name() + "...");
	m->action->setIcon(Gui::Icons::icon(Gui::Icons::Preferences));

	return m->action;
}

QList<QAction*> GUI_PreferenceDialog::actions(QWidget* parent)
{
	QList<QAction*> ret;
	for(Preferences::Base* dialog : Algorithm::AsConst(m->pref_widgets))
	{
		QString action_name = dialog->action_name();
		QString identifier = dialog->identifier();
		QAction* action = new QAction(parent);
		action->setText(action_name);
		ret << action;

		connect(action, &QAction::triggered, this, [=](){
			show_preference(identifier);
		});
	}

	return ret;
}


void GUI_PreferenceDialog::commit_and_close()
{
	if(commit()){
		close();
	}
}


bool GUI_PreferenceDialog::commit()
{
	bool success = true;
	for(Base* iface : Algorithm::AsConst(m->pref_widgets))
	{
		if(iface->is_ui_initialized())
		{
			if(!iface->commit())
			{
				QString error_string = iface->error_string();
				if(!error_string.isEmpty())
				{
					Message::warning(iface->action_name() + "\n\n" + error_string, iface->action_name());
					success = false;
				}
			}
		}
	}

	return success;
}


void GUI_PreferenceDialog::revert()
{
	for(Base* iface : Algorithm::AsConst(m->pref_widgets))
	{
		if(iface->is_ui_initialized()){
			iface->revert();
		}
	}

	close();
}


void GUI_PreferenceDialog::row_changed(int row)
{
	if(!between(row, m->pref_widgets)){
		return;
	}

	hide_all();

	Base* widget = m->pref_widgets[row];

	QLayout* layout = ui->widget_preferences->layout();
	layout->setContentsMargins(0,0,0,0);

	if(layout){
		layout->addWidget(widget);
		layout->setAlignment(Qt::AlignTop);
	}

	ui->lab_pref_title->setText(widget->action_name());

	widget->show();
}


void GUI_PreferenceDialog::hide_all()
{
	for(Base* iface : Algorithm::AsConst(m->pref_widgets))
	{
		iface->setParent(nullptr);
		iface->hide();
	}
}

void GUI_PreferenceDialog::showEvent(QShowEvent* e)
{
	init_ui();
	Gui::Dialog::showEvent(e);
}


void GUI_PreferenceDialog::init_ui()
{
	if(ui){
		return;
	}

	ui = new Ui::GUI_PreferenceDialog();
	ui->setupUi(this);

	for(Base* widget : Algorithm::AsConst(m->pref_widgets))
	{
		ui->list_preferences->addItem(widget->action_name());
	}

	ui->list_preferences->setMouseTracking(false);
	ui->list_preferences->setItemDelegate(
		new Gui::StyledItemDelegate(ui->list_preferences)
	);

	connect(ui->list_preferences, &QListWidget::currentRowChanged, this, &GUI_PreferenceDialog::row_changed);
	connect(ui->btn_apply, &QPushButton::clicked, this, &GUI_PreferenceDialog::commit);
	connect(ui->btn_ok, &QPushButton::clicked, this, &GUI_PreferenceDialog::commit_and_close);
	connect(ui->btn_cancel, &QPushButton::clicked, this, &GUI_PreferenceDialog::revert);
}
