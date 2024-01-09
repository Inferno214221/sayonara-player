/* InfoDialogContainer.h */

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

#ifndef SAYONARA_PLAYER_INFO_DIALOG_CONTAINER_H
#define SAYONARA_PLAYER_INFO_DIALOG_CONTAINER_H

#include "Utils/Pimpl.h"
#include <QObject>

enum class OpenMode :
	uint8_t
{
	Info,
	Edit,
	Lyrics,
	Cover
};

class GUI_InfoDialog;

class InfoDialogContainer;
class InfoDialogContainerAsyncHandler :
	public QObject
{
	Q_OBJECT
	PIMPL(InfoDialogContainerAsyncHandler)

		friend class InfoDialogContainer;

	public:
		~InfoDialogContainerAsyncHandler() override;

	private:
		InfoDialogContainerAsyncHandler(InfoDialogContainer* container, OpenMode openMode);

		bool start();
		[[nodiscard]] bool isRunning() const;

	private slots:
		void scannerFinished();
};

class InfoDialogContainer
{
	PIMPL(InfoDialogContainer)

		friend class InfoDialogContainerAsyncHandler;

		friend class GUI_InfoDialog;

	public:
		InfoDialogContainer();
		virtual ~InfoDialogContainer();

	protected:
		enum EditTab
		{
			TabText,
			TabCover,
			TabTagsFromPath
		};

		[[nodiscard]] virtual MD::Interpretation metadataInterpretation() const = 0;

		[[nodiscard]] virtual MetaDataList infoDialogData() const = 0;

		[[nodiscard]] virtual QWidget* getParentWidget() = 0;

		[[nodiscard]] virtual bool hasMetadata() const;

		[[nodiscard]] virtual QStringList pathlist() const;

		virtual void showInfo();

		virtual void showLyrics();

		virtual void showEdit();

		virtual void showCoverEdit();

	private:
		bool initDialog(OpenMode openMode);

		void go(OpenMode openMode, const MetaDataList& tracks);
};

#endif // SAYONARA_PLAYER_INFO_DIALOG_CONTAINER_H
