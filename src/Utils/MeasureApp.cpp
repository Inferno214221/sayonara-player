/* MeasureApp.cpp */
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

#include "MeasureApp.h"
#include "Utils/Logger/Logger.h"
#include <QElapsedTimer>

namespace Util
{
	struct MeasureApp::Private
	{
		QElapsedTimer* timer;
		QString component;
		qint64 start;

		Private(QElapsedTimer* timer, const QString& component) :
			timer {timer},
			component {component},
			start {timer->elapsed()} {}
	};

	MeasureApp::MeasureApp(const QString& component, QElapsedTimer* t) :
		m {Pimpl::make<Private>(t, component)}
	{
		spLog(Log::Debug, this) << QString("Init %1: %2ms")
			.arg(m->component)
			.arg(m->start);
	}

	MeasureApp::~MeasureApp() noexcept
	{
		const auto end = m->timer->elapsed();
		spLog(Log::Debug, this) << QString("Init %1 finished: %2ms (%3ms)")
			.arg(m->component)
			.arg(end)
			.arg(end - m->start);
	}

	QElapsedTimer* startMeasure()
	{
		auto* timer = new QElapsedTimer();
		timer->start();
		return timer;
	}

	void measure(const QString& component, QElapsedTimer* timer, std::function<void(void)>&& task)
	{
		[[maybe_unused]] auto measureApp = MeasureApp(component, timer);
		task();
	}
}