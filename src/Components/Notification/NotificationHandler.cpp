
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

#include "NotificationHandler.h"

#include "Utils/Algorithm.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"

#include "Utils/MetaData/MetaData.h"

namespace
{
	class NotificationHandlerImpl :
		public NotificationHandler
	{
		public:
			using NotificationHandler::NotificationHandler;

			~NotificationHandlerImpl() noexcept override = default;

			void notify(const MetaData& track) override
			{
				if(auto* notificator = currentNotificator(); notificator)
				{
					notificator->notify(track);
				}
			}

			void notify(const QString& title, const QString& message, const QString& imagePath) override
			{
				const auto newImagePath = (imagePath.isEmpty())
				                          ? ":/Icons/logo.png"
				                          : imagePath;

				if(auto* notificator = currentNotificator(); notificator)
				{
					notificator->notify(title, message, newImagePath);
				}
			}

			void registerNotificator(Notificator* notificator) override
			{
				m_notificators << notificator;

				const auto preferredNotificator = GetSetting(Set::Notification_Name);
				const auto identifier = notificator->identifier();

				m_currentIndex = Util::Algorithm::indexOf(m_notificators, [&](const auto& notificator) {
					return notificator->identifier().toLower() == preferredNotificator.toLower();
				});

				if(m_currentIndex >= m_notificators.size())
				{
					m_currentIndex = 0;
				}

				emit sigNotificationsChanged();

				spLog(Log::Debug, this) << "Notification handler " << identifier << " registered";
			}

			void changeCurrentNotificator(const QString& name) override
			{
				m_currentIndex = Util::Algorithm::indexOf(m_notificators, [&](const auto& notificator) {
					return notificator->identifier().toLower() == name.toLower();
				});
			}

			Notificator* currentNotificator() const override
			{
				return Util::between(m_currentIndex, m_notificators)
				       ? m_notificators[m_currentIndex]
				       : nullptr;
			}

			QList<Notificator*> notificators() const override { return m_notificators; }

		private:
			QList<Notificator*> m_notificators;
			int m_currentIndex {-1};
	};
}

NotificationHandler* NotificationHandler::create(QObject* parent)
{
	return new NotificationHandlerImpl(parent);
}