/* PlayerPlugin.h */

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

#ifndef PLAYERPLUGIN_H
#define PLAYERPLUGIN_H

#include <QAction>
#include <QCloseEvent>

#include "Helper/Settings/Settings.h"
#include "GUI/Helper/SayonaraWidget/SayonaraWidget.h"
#include "GUI/Helper/Shortcuts/ShortcutWidget.h"

class PlayerPluginHandler;

/**
 * @brief Interface for PlayerPlugin classes.
 *   get_name() and language_changed() must be overwritten
 * @ingroup Interfaces
 */

class PlayerPluginInterface :
		public SayonaraWidget,
		public ShortcutWidget
{

	friend class PlayerPluginHandler;

    Q_OBJECT

private:
	bool		_is_initialized;
	QAction*	_pp_action=nullptr;


public:
	explicit PlayerPluginInterface(QWidget *parent=nullptr);
	virtual ~PlayerPluginInterface();


signals:
	/**
	 * @brief signal is emitted when the plugin action is triggered\n
	 * also emitted for when closeEvent is fired
	 * @param plugin this pointer to current plugin
	 * @param checked indicates whether checked or unchecked
	 */
	void sig_action_triggered(PlayerPluginInterface* plugin, bool checked);

	/**
	 * @brief emitted when reloading is requested, after firing this signal
	 * the plugin will be painted new. Useful, if the size has changed
	 */
	void sig_reload(PlayerPluginInterface*);

	/** 
	 * @brief emitted when plugin is closed
	 */
	void sig_closed();


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
	void finalize_initialization();

	/**
	 * @brief language_changed Has to be implemented and is called when language has changed
	 */
	virtual void language_changed() override=0;

	/**
	 * @brief GUI will be initialized on first show up. Please use this to make Sayonara starting fast
	 */
	virtual void init_ui()=0;


protected:

	/**
	 * @brief Check if ui already was initialized
	 * @return
	 */
	bool is_ui_initialized() const;


	template<typename T, typename UiClass>
	void setup_parent(T* widget, UiClass** ui){

		if(is_ui_initialized()){
			return;
		}

		*ui = new UiClass();
		(*ui)->setupUi(widget);

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
	 * @return miminum size of plugin
	 */
	virtual QSize			get_size() const final;

	/**
	 * @brief needed by the player ui, final
	 * @return action of plugin
	 */
	virtual QAction*		get_action() const final;


	/**
	 * @brief must be overwritten
	 * @return the NOT translated name of the plugin
	 */
	virtual QString			get_name() const=0;

	/**
	 * @brief must be overwritten
	 * @return the translated name of the plugin
	 */
	virtual QString			get_display_name() const=0;


	/**
	 * @brief indicates if title bar is shown or not
	 */ 
	virtual bool			is_title_shown() const;


	/**
	 * @brief get translated text of shortcut (overridden)
	 * @param shortcut_identifier shortcut id
	 * @return translated shortcut text
	 */
	QString get_shortcut_text(const QString &shortcut_identifier) const override;
};

Q_DECLARE_INTERFACE(PlayerPluginInterface, "com.sayonara-player.playerplugin")

#endif // PLAYERPLUGIN_H
