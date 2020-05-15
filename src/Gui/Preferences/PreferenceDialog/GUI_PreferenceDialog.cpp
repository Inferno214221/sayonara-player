/* GUI_PreferenceDialog.cpp */

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

#include "GUI_PreferenceDialog.h"
#include "Gui/Preferences/ui_GUI_PreferenceDialog.h"

#include "Components/Preferences/PreferenceRegistry.h"

#include "Gui/Preferences/PreferenceWidget.h"
#include "Gui/Preferences/PreferenceAction.h"
#include "Gui/Utils/GuiUtils.h"

#include "Utils/Algorithm.h"
#include "Utils/Message/Message.h"
#include "Utils/Language/Language.h"
#include "Utils/Logger/Logger.h"

#include "Gui/Utils/Icons.h"
#include "Gui/Utils/Delegates/StyledItemDelegate.h"

#include <QLayout>
#include <QMenu>
#include <QPushButton>

using Preferences::Base;
using Preferences::Action;
namespace Algorithm=Util::Algorithm;

struct GUI_PreferenceDialog::Private
{
	QList<Base*>	preferenceWidgets;
	Action*			action=nullptr;
	int				currentRow;

	Private() :
		currentRow(-1)
	{}
};

GUI_PreferenceDialog::GUI_PreferenceDialog(QWidget* parent) :
	Gui::Dialog(parent),
	PreferenceUi()
{
	m = Pimpl::make<Private>();

	PreferenceRegistry::instance()->setUserInterface(this);
}

GUI_PreferenceDialog::~GUI_PreferenceDialog()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}

void GUI_PreferenceDialog::registerPreferenceDialog(Base* preferenceWidget)
{
	m->preferenceWidgets << preferenceWidget;
	PreferenceRegistry::instance()->registerPreference(preferenceWidget->identifier());
}

void GUI_PreferenceDialog::showPreference(const QString& identifier)
{
	initUi();

	int i=0;
	for(Preferences::Base* pwi : Algorithm::AsConst(m->preferenceWidgets))
	{
		const QString dialogId = pwi->identifier();
		if(identifier.compare(dialogId) == 0)
		{
			ui->listPreferences->setCurrentRow(i);
			rowChanged(i);

			this->setModal(true);
			this->show();

			return;
		}

		i++;
	}

	spLog(Log::Warning, this) << "Cannot find preference widget " << identifier;
}

void GUI_PreferenceDialog::languageChanged()
{
	ui->retranslateUi(this);

	bool isEmpty = (ui->listPreferences->count() == 0);

	int i=0;
	for(Base* dialog : Algorithm::AsConst(m->preferenceWidgets))
	{
		QListWidgetItem* item;
		if(isEmpty)
		{
			item = new QListWidgetItem(dialog->actionName());
			ui->listPreferences->addItem(item);
		}

		else
		{
			item = ui->listPreferences->item(i);
			item->setText(dialog->actionName());
		}

		i++;
	}

	if(m->action){
		m->action->setText(actionName() + "...");
	}

	if(Util::between(m->currentRow, m->preferenceWidgets.count()))
	{
		Base* dialog = m->preferenceWidgets[m->currentRow];
		if(dialog){
			ui->labPreferenceTitle->setText(dialog->actionName());
		}
	}

	this->setWindowTitle(this->actionName());
}

QString GUI_PreferenceDialog::actionName() const
{
	return Lang::get(Lang::Preferences);
}

QAction* GUI_PreferenceDialog::action()
{
	// action has to be initialized here, because pure
	// virtual get_action_name should not be called from ctor
	const QString name = actionName();
	if(!m->action){
		m->action = new Action(name, this);
	}

	m->action->setText(actionName() + "...");
	m->action->setIcon(Gui::Icons::icon(Gui::Icons::Preferences));

	return m->action;
}

QList<QAction*> GUI_PreferenceDialog::actions(QWidget* parent)
{
	QList<QAction*> ret;
	for(Preferences::Base* dialog : Algorithm::AsConst(m->preferenceWidgets))
	{
		const QString actionName = dialog->actionName();
		const QString identifier = dialog->identifier();

		QAction* action = new QAction(parent);
		action->setText(actionName);
		ret << action;

		connect(action, &QAction::triggered, this, [=](){
			showPreference(identifier);
		});
	}

	return ret;
}

void GUI_PreferenceDialog::commitAndClose()
{
	if(commit()){
		close();
	}
}

bool GUI_PreferenceDialog::commit()
{
	bool success = true;
	for(Base* iface : Algorithm::AsConst(m->preferenceWidgets))
	{
		if(iface->isUiInitialized())
		{
			if(!iface->commit())
			{
				const QString errorString = iface->errorString();
				if(!errorString.isEmpty())
				{
					Message::warning(iface->actionName() + "\n\n" + errorString, iface->actionName());
					success = false;
				}
			}
		}
	}

	return success;
}

void GUI_PreferenceDialog::revert()
{
	for(Base* iface : Algorithm::AsConst(m->preferenceWidgets))
	{
		if(iface->isUiInitialized()){
			iface->revert();
		}
	}

	close();
}

void GUI_PreferenceDialog::rowChanged(int row)
{
	if(!Util::between(row, m->preferenceWidgets)){
		return;
	}

	m->currentRow = row;

	hideAll();

	Base* widget = m->preferenceWidgets[row];

	QLayout* layout = ui->widgetPreferences->layout();
	layout->setContentsMargins(0,0,0,0);

	if(layout)
	{
		layout->addWidget(widget);
		layout->setAlignment(Qt::AlignTop);
		ui->widgetPreferences->setFocusProxy(widget);
	}

	ui->labPreferenceTitle->setText(widget->actionName());

	widget->show();
}


void GUI_PreferenceDialog::hideAll()
{
	for(Base* iface : Algorithm::AsConst(m->preferenceWidgets))
	{
		iface->setParent(nullptr);
		iface->hide();
	}
}

void GUI_PreferenceDialog::showEvent(QShowEvent* e)
{
	initUi();
	Gui::Dialog::showEvent(e);

	this->setWindowTitle(Lang::get(Lang::Preferences));
	ui->listPreferences->setFocus();
}


void GUI_PreferenceDialog::initUi()
{
	if(ui){
		return;
	}

	ui = new Ui::GUI_PreferenceDialog();
	ui->setupUi(this);

	for(Base* widget : Algorithm::AsConst(m->preferenceWidgets))
	{
		ui->listPreferences->addItem(widget->actionName());
	}

	ui->listPreferences->setMouseTracking(false);
	ui->listPreferences->setItemDelegate(
		new Gui::StyledItemDelegate(ui->listPreferences)
	);

	QPushButton* okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
	QPushButton* applyButton = ui->buttonBox->button(QDialogButtonBox::Apply);
	QPushButton* closeButton = ui->buttonBox->button(QDialogButtonBox::Cancel);

	connect(okButton, &QPushButton::clicked, this, &GUI_PreferenceDialog::commitAndClose);
	connect(applyButton, &QPushButton::clicked, this, &GUI_PreferenceDialog::commit);
	connect(closeButton, &QPushButton::clicked, this, &GUI_PreferenceDialog::revert);

	connect(ui->listPreferences, &QListWidget::currentRowChanged, this, &GUI_PreferenceDialog::rowChanged);

	QSize sz = Gui::Util::mainWindow()->size();
	sz *= 0.66;
	this->resize(sz);
}
