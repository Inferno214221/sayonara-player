/* WidgetTemplate.h */

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

#ifndef SAYONARAWIDGETTEMPLATE_H
#define SAYONARAWIDGETTEMPLATE_H

#include "GUI/Utils/GuiClass.h"
#include "Utils/Settings/SayonaraClass.h"
#include <QShowEvent>
#include <QObject>

class QWidget;

#define combo_current_index_changed_int	static_cast<void (QComboBox::*) (int)>(&QComboBox::currentIndexChanged)
#define combo_activated_int	static_cast<void (QComboBox::*) (int)>(&QComboBox::activated)
#define spinbox_value_changed_int	static_cast<void (QSpinBox::*) (int)>(&QSpinBox::valueChanged)

namespace Gui
{
	class WidgetTemplateParent;
	class AbstrWidgetTemplate :
		public QObject,
		public SayonaraClass
	{
		Q_OBJECT

		private:
			WidgetTemplateParent* _wtp;

		public:
			AbstrWidgetTemplate(QObject* parent, WidgetTemplateParent* wtp);
			virtual ~AbstrWidgetTemplate();

		protected:
			virtual void language_changed();
			virtual void skin_changed();
	};


	class WidgetTemplateParent :
		public SayonaraClass
	{
		friend class AbstrWidgetTemplate;

		public:
			WidgetTemplateParent();
			virtual ~WidgetTemplateParent();

		protected:
			virtual void language_changed();
			virtual void skin_changed();
	};

	template<typename T>
	/**
	 * @brief Template for Sayonara Widgets. This template is responsible for holding a reference to the settings
	 * @ingroup Widgets
	 * @ingroup Interfaces
	 */

	class WidgetTemplate :
			public T,
			protected WidgetTemplateParent
	{
		friend class AbstrWidgetTemplate;

		private:
			AbstrWidgetTemplate* _awt;

		public:
			template<typename... Args>
			WidgetTemplate(Args&&... args) :
				T(std::forward<Args>(args)...),
				WidgetTemplateParent()
			{
				_awt = new AbstrWidgetTemplate(this, this);
			}

			virtual ~WidgetTemplate() {}

			virtual void showEvent(QShowEvent* e) override
			{
				language_changed();
				skin_changed();

				T::showEvent(e);
			}
		};
}

#endif // SAYONARAWIDGETTEMPLATE_H
