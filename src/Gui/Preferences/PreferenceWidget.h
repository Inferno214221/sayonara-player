/* PreferenceWidgetInterface.h */

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

#ifndef PREFERENCEWIDGETINTERFACE_H
#define PREFERENCEWIDGETINTERFACE_H

#include "Gui/Utils/GuiClass.h"
#include "Gui/Utils/Widgets/Widget.h"
#include "Utils/Pimpl.h"

namespace Preferences
{
	/**
	 * @brief Abstract Interface you should use when creating a preferences item.
	 *
	 * For methods to be implemented see also PreferenceInterface.
	 * If you wish to subclass, reimplement void get_action_name() and void init_ui().\n
	 * In every function that makes use of the widgets call bool is_ui_initialized() first.\n
	 * Call setup_parent(this) in init_ui() first.\n
	 * If you wish to reimplement void language_changed(), call PreferenceWidgetInterface::language_changed at the end.
	 * @ingroup Preferences
	 */
	class Base :
			public Gui::Widget
	{
		Q_OBJECT
		PIMPL(Base)

	public:
		/**
		 * @brief Standard constructor
		 * @param parent
		 */
		explicit Base(const QString& identifier);
		virtual ~Base() override;

		/**
		 * @brief return the unique identifier
		 * @return
		 */
		QString	identifier() const;

	private:
		void set_initialized();

	protected:

		template<typename W, typename UiClass>
		/**
		 * @brief Sets up the Preference dialog. After this method, the dialog is "ready to use"\n
		 * This method should be the first to be called when calling init_ui()
		 * @param widget should always be "this"
		 */
		void setup_parent(W* widget, UiClass** ui)
		{
			*ui = new UiClass();
			(*ui)->setupUi(widget);

			set_initialized();

			widget->language_changed();

			skin_changed();
		}

		/**
		 * @brief automatically called when language has changed. When overriding this method.
		 * Overriding this method should look like this:
		 * void GUI_FontConfig::language_changed()\n
		 *  {\n
		 *		translate_action();\n\n
		 *
		 *		if(!is_ui_initialized()){\n
		 * 			return; \n
		 *		}\n\n
		 *
		 *		retranslateUi(this);\n
		 *		PreferenceWidgetInterface::language_changed();\n
		 *  }\n
		 */
		virtual void language_changed() override final;


		/**
		 * @brief Sets the new translated action name
		 */
		void translate_action();


	protected:

		/**
		 * @brief shows the widget and automatically calls init_ui()
		 * @param e
		 */
		void showEvent(QShowEvent* e) override;
		void closeEvent(QCloseEvent* e) override;

	public:

		/**
		 * @brief checks if ui has already been initialized.
		 * @return false, if the widget has never been activated before, true else
		 */
		virtual bool is_ui_initialized() const final;


		/**
		 * @brief get action with translated text
		 * @return
		 */
		virtual QAction* action() final;



		/**
		 * @brief has to be implemented and should return the translated action text
		 * @return translated action name
		 */
		virtual QString action_name() const=0;


		/**
		 * @brief This method is called, when OK or apply is pressed. So all settings
		 * should be written there
		 */
		virtual bool commit()=0;

		/**
		 * @brief This method is called, when cancel is clicked. So the gui should be
		 * re-initialized when this method is called. This method should also be called
		 * in the init_ui() method
		 */
		virtual void revert()=0;

		/**
		 * @brief call setup_parent(this) here.\n
		 * initialize compoenents and connections here.\n
		 * After calling setup_parent(this), the preference Dialog is ready to use, language_changed() is called automatically
		 */
		virtual void init_ui()=0;


		/**
		 * @brief call the Qt retranslateUi method here
		 */
		virtual void retranslate_ui()=0;

		/**
		 * @brief indicates if there was an error on the settings page like
		 * an invalid expression or combination of settings
		 * @return
		 */
		virtual bool has_error() const;

		/**
		 * @brief A closer description of the error
		 * @return
		 */
		virtual QString error_string() const;

	};
}

#endif // PREFERENCEWIDGETINTERFACE_H
