/* ExternUrlsDragDropHandler.cpp */
/*
 * Copyright (C) 2011-2024 Michael Lugmair
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
#include "ExternUrlsDragDropHandler.h"
#include "Components/Directories/MetaDataScanner.h"

#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaDataList.h"

#include <QList>
#include <QStringList>
#include <QThread>
#include <QUrl>

using Directory::MetaDataScanner;

struct ExternUrlsDragDropHandler::Private
{
	QList<QUrl> urls;

	Private(const QList<QUrl>& urls) :
		urls(urls) {}
};

ExternUrlsDragDropHandler::ExternUrlsDragDropHandler(const QList<QUrl>& urls, QObject* parent) :
	Gui::AsyncDropHandler(parent)
{
	m = Pimpl::make<Private>(urls);
}

ExternUrlsDragDropHandler::~ExternUrlsDragDropHandler() = default;

void ExternUrlsDragDropHandler::start()
{
	QStringList files;
	Util::Algorithm::transform(m->urls, files, [](const auto& url) {
		return (url.toLocalFile());
	});

	auto* t = new QThread();
	auto* worker = new MetaDataScanner(files, true, nullptr);

	connect(t, &QThread::started, worker, &MetaDataScanner::start);
	connect(t, &QThread::finished, t, &QObject::deleteLater);
	connect(worker, &MetaDataScanner::sigFinished, this, &ExternUrlsDragDropHandler::metadataScannerFinished);
	connect(worker, &MetaDataScanner::sigFinished, t, &QThread::quit);

	worker->moveToThread(t);
	t->start();
}

void ExternUrlsDragDropHandler::metadataScannerFinished()
{
	auto* metadataScanner = dynamic_cast<MetaDataScanner*>(sender());
	setTracks(metadataScanner->metadata());
	metadataScanner->deleteLater();
}