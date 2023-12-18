/*
 * Copyright (C) 2011-2023 Michael Lugmair
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

#include "Common/SayonaraTest.h"
#include "Utils/Settings/SettingNotifier.h"
#include "Utils/Settings/Settings.h"

// access working directory with Test::Base::tempPath("somefile.txt");

namespace
{
	struct CallbackInstance :
		public QObject
	{
		Q_OBJECT

		public slots:

			void settingCalled()
			{
				settingCalls++;
			}

		public:
			int settingCalls {0};
	};
}

class SettingsNotifierTest :
	public Test::Base
{
	Q_OBJECT

	public:
		SettingsNotifierTest() :
			Test::Base("SettingsNotifierTest") {}

	private slots:

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testCallbackIsCalledWhenListenerIsRegistered()
		{
			struct TestCase
			{
				bool callImmediately {0};
				int expectedCalls {0};
			};

			const auto testCases = std::array {
				TestCase {true, 1},
				TestCase {false, 0},
			};

			for(const auto& testCase: testCases)
			{
				auto callback = CallbackInstance();

				Set::listen<Set::AudioConvert_QualityOgg>(&callback,
				                                          &CallbackInstance::settingCalled,
				                                          testCase.callImmediately);
				QVERIFY(callback.settingCalls == testCase.expectedCalls);
			}
		}

		// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
		[[maybe_unused]] void testCallbackIsCalledAgainWhenValueChanges()
		{
			constexpr const auto InitialQuality = 0;

			struct TestCase
			{
				int newQuality {0};
				int expectedCalls {0};
			};

			const auto testCases = std::array {
				TestCase {0, 0},
				TestCase {1, 1},
				TestCase {2, 1}
			};

			for(const auto& testCase: testCases)
			{
				SetSetting(Set::AudioConvert_QualityOgg, InitialQuality);

				auto callback = CallbackInstance();
				Set::listen<Set::AudioConvert_QualityOgg>(&callback, &CallbackInstance::settingCalled, false);

				SetSetting(Set::AudioConvert_QualityOgg, testCase.newQuality);

				QVERIFY(callback.settingCalls == testCase.expectedCalls);
			}
		}

		[[maybe_unused]] void
		tesMultipleInstancesAreNotified() // NOLINT(readability-convert-member-functions-to-static)
		{
			auto callbacks = std::vector<std::shared_ptr<CallbackInstance>> {};
			for(int i = 0; i < 5; i++) // NOLINT(readability-magic-numbers)
			{
				auto callback = std::make_shared<CallbackInstance>();
				Set::listen<Set::AudioConvert_QualityOgg>(callback.get(), &CallbackInstance::settingCalled, false);
				QVERIFY(callback->settingCalls == 0);
				callbacks.push_back(std::move(callback));
			}

			SetSetting(Set::AudioConvert_QualityOgg, 9);
			for(const auto& callback: callbacks)
			{
				QVERIFY(callback->settingCalls == 1);
			}
		}
};

QTEST_GUILESS_MAIN(SettingsNotifierTest)

#include "SettingsNotifierTest.moc"
