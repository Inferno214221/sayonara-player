/* PreferenceAction.h */

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

#ifndef PREFERENCEACTION_H
#define PREFERENCEACTION_H

#include "Utils/Pimpl.h"

#include <QAction>

class QPushButton;

namespace Gui
{
	/**
	 * @brief A PreferenceAction can be added to each widget supporting
	 * QActions. When triggering this action, the PreferenceDialog is
	 * openend with the appropriate entry chosen
	 * @ingroup Preferences
	 */
	class PreferenceAction :
			public QAction
	{
		Q_OBJECT
		PIMPL(PreferenceAction)

		public:
			PreferenceAction(const QString& displayName, const QString& identifier, QWidget* parent);
			virtual ~PreferenceAction();

			virtual QString label() const;
			virtual QString identifier() const=0;

			virtual QPushButton* createButton(QWidget* parent);

		protected:
			virtual QString displayName() const=0;
			void language_changed();
	};

	/**
	 * @brief The LibraryPreferenceAction class
	 * @ingroup Preferences
	 */
	class LibraryPreferenceAction :
		public PreferenceAction
	{
		Q_OBJECT
		public:
			LibraryPreferenceAction(QWidget* parent);
			~LibraryPreferenceAction() override;

			QString displayName() const override;
			QString identifier() const override;
	};

	/**
	 * @brief The PlaylistPreferenceAction class
	 * @ingroup Preferences
	 */
	class PlaylistPreferenceAction :
		public PreferenceAction
	{
		Q_OBJECT
		public:
			PlaylistPreferenceAction(QWidget* parent);
			~PlaylistPreferenceAction() override;

			QString displayName() const override;
			QString identifier() const override;
	};

	/**
	 * @brief The SearchPreferenceAction class
	 * @ingroup Preferences
	 */
	class SearchPreferenceAction :
		public PreferenceAction
	{
		Q_OBJECT
		public:
			SearchPreferenceAction(QWidget* parent);
			~SearchPreferenceAction() override;

			QString displayName() const override;
			QString identifier() const override;
	};

	/**
	 * @brief The CoverPreferenceAction class
	 * @ingroup Preferences
	 */
	class CoverPreferenceAction :
		public PreferenceAction
	{
		Q_OBJECT
		public:
			CoverPreferenceAction(QWidget* parent);
			~CoverPreferenceAction() override;

			QString displayName() const override;
			QString identifier() const override;
	};

	/**
	 * @brief The PlayerPreferencesAction class
	 * @ingroup Preferences
	 */
	class PlayerPreferencesAction :
		public PreferenceAction
	{
		Q_OBJECT
		public:
			PlayerPreferencesAction(QWidget* parent);
			~PlayerPreferencesAction() override;

			QString displayName() const override;
			QString identifier() const override;
	};

	/**
	 * @brief The StreamRecorderPreferenceAction class
	 * @ingroup Preferences
	 */
	class StreamRecorderPreferenceAction :
			public PreferenceAction
	{
		Q_OBJECT
		public:
			StreamRecorderPreferenceAction(QWidget* parent);
			~StreamRecorderPreferenceAction() override;

			QString displayName() const override;
			QString identifier() const override;
	};


	class ShortcutPreferenceAction : public Gui::PreferenceAction
	{
		Q_OBJECT

		public:
			ShortcutPreferenceAction(QWidget* parent);
			~ShortcutPreferenceAction() override;

			QString identifier() const override;
			QString displayName() const override;
	};
}

#endif // PREFERENCEACTION_H
