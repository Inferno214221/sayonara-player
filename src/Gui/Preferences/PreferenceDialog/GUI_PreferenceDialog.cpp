/* GUI_PreferenceDialog.cpp */

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

struct GUI_PreferenceDialog::Private
{
	QList<Preferences::Base*> preferenceWidgets;
	QMainWindow* mainWindow;
	Preferences::Action* action {nullptr};
	int currentRow {-1};

	explicit Private(QMainWindow* mainWindow) :
		mainWindow(mainWindow) {}
};

GUI_PreferenceDialog::GUI_PreferenceDialog(QMainWindow* parent) :
	Gui::Dialog(parent),
	PreferenceUi(),
	m {Pimpl::make<Private>(parent)}
{
	PreferenceRegistry::instance()->setUserInterface(this);
}

GUI_PreferenceDialog::~GUI_PreferenceDialog() = default;

void GUI_PreferenceDialog::registerPreferenceDialog(Preferences::Base* preferenceWidget)
{
	m->preferenceWidgets << preferenceWidget;
	PreferenceRegistry::instance()->registerPreference(preferenceWidget->identifier());
}

void GUI_PreferenceDialog::showPreference(const QString& identifier)
{
	initUi();

	const auto index = Util::Algorithm::indexOf(m->preferenceWidgets, [&](auto* widget) {
		return widget->identifier() == identifier;
	});

	if(index < 0)
	{
		spLog(Log::Warning, this) << "Cannot find preference widget " << identifier;
		return;
	}

	ui->listPreferences->setCurrentRow(index);
	rowChanged(index);

	setModal(true);
	show();
}

void GUI_PreferenceDialog::languageChanged()
{
	ui->retranslateUi(this);

	const auto isEmpty = (ui->listPreferences->count() == 0);

	int i = 0;
	for(auto* widget: m->preferenceWidgets)
	{
		auto* item = isEmpty
		             ? new QListWidgetItem(widget->actionName())
		             : ui->listPreferences->item(i);
		if(isEmpty)
		{
			ui->listPreferences->addItem(item);
		}

		else
		{
			item->setText(widget->actionName());
		}

		i++;
	}

	if(m->action)
	{
		m->action->setText(actionName() + "...");
	}

	if(Util::between(m->currentRow, m->preferenceWidgets.count()))
	{
		auto* currentWidget = m->preferenceWidgets[m->currentRow];
		if(currentWidget)
		{
			ui->labPreferenceTitle->setText(currentWidget->actionName());
		}
	}

	setWindowTitle(actionName());
}

QString GUI_PreferenceDialog::actionName() const { return Lang::get(Lang::Preferences); }

QAction* GUI_PreferenceDialog::action()
{
	// action has to be initialized here, because pure
	// virtual get_action_name should not be called from ctor
	if(!m->action)
	{
		m->action = new Preferences::Action(actionName(), this);
	}

	m->action->setText(actionName() + "...");
	m->action->setIcon(Gui::Icons::icon(Gui::Icons::Preferences));

	return m->action;
}

QList<QAction*> GUI_PreferenceDialog::actions(QWidget* parent)
{
	QList<QAction*> ret;
	for(auto* widget: m->preferenceWidgets)
	{
		const auto actionName = widget->actionName();
		const auto identifier = widget->identifier();

		auto* action = new QAction(parent);
		action->setText(actionName);
		ret << action;

		connect(action, &QAction::triggered, this, [identifier, this]() {
			showPreference(identifier);
		});
	}

	return ret;
}

void GUI_PreferenceDialog::commitAndClose()
{
	if(commit())
	{
		close();
	}
}

bool GUI_PreferenceDialog::commit()
{
	bool success = true;
	for(auto* widget: m->preferenceWidgets)
	{
		if(widget->isUiInitialized())
		{
			if(!widget->commit())
			{
				const auto errorString = widget->errorString();
				if(!errorString.isEmpty())
				{
					Message::warning(widget->actionName() + "\n\n" + errorString, widget->actionName());
					success = false;
				}
			}
		}
	}

	return success;
}

void GUI_PreferenceDialog::revert()
{
	for(auto* widget: m->preferenceWidgets)
	{
		if(widget->isUiInitialized())
		{
			widget->revert();
		}
	}

	close();
}

void GUI_PreferenceDialog::rowChanged(const int row)
{
	if(!Util::between(row, m->preferenceWidgets))
	{
		return;
	}

	m->currentRow = row;

	hideAll();

	auto* widget = m->preferenceWidgets[row];

	auto* layout = ui->widgetPreferences->layout();
	layout->setContentsMargins(0, 0, 0, 0);

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
	for(auto* widget: m->preferenceWidgets)
	{
		widget->setParent(nullptr);
		widget->hide();
	}
}

void GUI_PreferenceDialog::showEvent(QShowEvent* e)
{
	initUi();
	Gui::Dialog::showEvent(e);

	setWindowTitle(Lang::get(Lang::Preferences));
	ui->listPreferences->setFocus();
}

void GUI_PreferenceDialog::initUi()
{
	if(ui)
	{
		return;
	}

	setAttribute(Qt::WA_StyledBackground);

	ui = std::make_shared<Ui::GUI_PreferenceDialog>();
	ui->setupUi(this);

	for(auto* widget: m->preferenceWidgets)
	{
		ui->listPreferences->addItem(widget->actionName());
	}

	ui->listPreferences->setMouseTracking(false);
	ui->listPreferences->setItemDelegate(
		new Gui::StyledItemDelegate(ui->listPreferences)
	);

	auto* okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
	auto* applyButton = ui->buttonBox->button(QDialogButtonBox::Apply);
	auto* closeButton = ui->buttonBox->button(QDialogButtonBox::Cancel);

	connect(okButton, &QPushButton::clicked, this, &GUI_PreferenceDialog::commitAndClose);
	connect(applyButton, &QPushButton::clicked, this, &GUI_PreferenceDialog::commit);
	connect(closeButton, &QPushButton::clicked, this, &GUI_PreferenceDialog::revert);

	connect(ui->listPreferences, &QListWidget::currentRowChanged, this, &GUI_PreferenceDialog::rowChanged);
	
	resize(m->mainWindow->size() * 0.66);
}
