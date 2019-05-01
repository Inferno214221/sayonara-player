/* PlayerPlugin.h */

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

#ifndef PLAYERPLUGIN_H
#define PLAYERPLUGIN_H

#include "GUI/Utils/Widgets/Widget.h"
#include "GUI/Utils/Shortcuts/ShortcutWidget.h"
#include "Utils/Pimpl.h"

class QAction;

/**
 * @brief Interface for PlayerPlugin classes.
 *   get_name() and language_changed() must be overwritten
 * @ingroup Interfaces
 */

namespace PlayerPlugin
{
	class Handler;
	class Base :
		public Gui::Widget,
		public ShortcutWidget
	{
		friend class Handler;

		Q_OBJECT

	private:
		PIMPL(Base)

	public:
		explicit Base(QWidget *parent=nullptr);
		virtual ~Base();

		virtual void skin_changed() override;


	signals:
		/**
		 * @brief signal is emitted when the plugin action is triggered\n
		 * also emitted for when closeEvent is fired
		 * @param plugin this pointer to current plugin
		 * @param checked indicates whether checked or unchecked
		 */
		void sig_action_triggered(bool checked);

		/**
		 * @brief emitted when reloading is requested, after firing this signal
		 * the plugin will be painted new. Useful, if the size has changed
		 */
		void sig_reload(PlayerPlugin::Base* plugin);

		void sig_opened();


	private slots:
		/**
		 * @brief Checks/unchecks the action and emits sig_action_triggered signal
		 * also called when closeEvent is fired
		 * @param checked if action is checked or unchecked
		 */
		void action_triggered(bool checked);

	private:

		/**
		 * @brief mark ui as initialized
		 */
		void set_ui_initialized();


		/**
		 * @brief language_changed. Calls retranslate_ui in subclasses
		 */
		virtual void language_changed() final override;

		/**
		 * @brief GUI will be initialized on first show up. Please use this to make Sayonara starting fast
		 */
		virtual void init_ui()=0;


	protected:

		virtual void finalize_initialization();

		/**
		 * @brief Check if ui already was initialized
		 * @return
		 */
		bool is_ui_initialized() const;
		virtual void assign_ui_vars();

		virtual void retranslate_ui()=0;

		template<typename T, typename UiClass>
		void setup_parent(T* widget, UiClass** ui)
		{
			if(is_ui_initialized()){
				return;
			}

			*ui = new UiClass();
			(*ui)->setupUi(widget);

			assign_ui_vars();
			finalize_initialization();
		}

		/**
		 * @brief Event fired when closed overrides QWidget::closeEvent
		 * @param e the event
		 */
		void closeEvent(QCloseEvent* e) override;
		void showEvent(QShowEvent* e) override;


	public:

		/**
		 * @brief needed by the player ui, final
		 * @return action of plugin
		 */
		virtual QAction*	get_action() const final;


		/**
		 * @brief must be overwritten
		 * @return the NOT translated name of the plugin
		 */
		virtual QString		get_name() const=0;

		/**
		 * @brief must be overwritten
		 * @return the translated name of the plugin
		 */
		virtual QString		get_display_name() const=0;


		/**
		 * @brief indicates if title bar is shown or not
		 */
		virtual bool		is_title_shown() const;

		/**
		 * @brief indicates if the widget has a loading bar. If yes, there will be reserved
		 * some extra space at the bottom of the widget
		 * @return
		 */
		virtual bool		has_loading_bar() const;
	};
}

Q_DECLARE_INTERFACE(PlayerPlugin::Base, "com.sayonara-player.playerplugin")

#endif // PLAYERPLUGIN_H
