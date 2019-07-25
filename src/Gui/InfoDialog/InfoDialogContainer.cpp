/* InfoDialogContainer.cpp */

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

#include "InfoDialogContainer.h"
#include "GUI_InfoDialog.h"
#include "Gui/Utils/GuiUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include <QMainWindow>

struct InfoDialogContainer::Private
{
	GUI_InfoDialog*	info_dialog=nullptr;
};

InfoDialogContainer::InfoDialogContainer()
{
	m = Pimpl::make<Private>();
}

InfoDialogContainer::~InfoDialogContainer() {}

void InfoDialogContainer::info_dialog_closed() {}

void InfoDialogContainer::show_info()
{
	if(init_dialog())
	{
		m->info_dialog->show(GUI_InfoDialog::Tab::Info);
	}
}

void InfoDialogContainer::show_lyrics()
{
	if(init_dialog())
	{
		m->info_dialog->show(GUI_InfoDialog::Tab::Lyrics);
	}
}

void InfoDialogContainer::show_edit()
{
	if(init_dialog())
	{
		m->info_dialog->show(GUI_InfoDialog::Tab::Edit);
	}
}

void InfoDialogContainer::show_cover_edit()
{
	if(init_dialog())
	{
		m->info_dialog->show_cover_edit_tab();
	}
}

bool InfoDialogContainer::init_dialog()
{
	if(!m->info_dialog)
	{
		m->info_dialog = new GUI_InfoDialog(this, Gui::Util::main_window());
	}

	m->info_dialog->set_metadata(info_dialog_data(), metadata_interpretation());

	return m->info_dialog->has_metadata();
}

