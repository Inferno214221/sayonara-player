/* ImageSelectionDialog.cpp */

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

#include "ImageSelectionDialog.h"
#include "Utils/Utils.h"

#include <QLayout>
#include <QLabel>
#include <QStringList>
#include <QDialog>

using Gui::ImageSelectionDialog;

struct ImageSelectionDialog::Private
{
	QString start_dir;
	QLabel* imageLabel=nullptr;
	QLabel* resolutionLabel=nullptr;

	Private(QWidget* parent)
	{
		imageLabel = new QLabel(parent);
		imageLabel->setMinimumSize(100, 100);
		imageLabel->setMaximumSize(100, 100);

		resolutionLabel = new QLabel(parent);
	}
};

ImageSelectionDialog::ImageSelectionDialog(const QString& dir, QWidget* parent) :
	Gui::WidgetTemplate<QFileDialog>(parent)
{
	m = Pimpl::make<Private>(this);
	m->start_dir = dir;

	const QStringList filters
	{
		tr("Image files") + " (" + Util::imageExtensions().join(" ") + ")",
		tr("Any files") + " (*)"
	};

	this->setDirectory(dir);
	this->setFilter(QDir::AllEntries | QDir::AllDirs);
	this->setLabelText(QFileDialog::DialogLabel::FileName, tr("Open image files"));
	this->setNameFilters(filters);
	this->setViewMode(QFileDialog::Detail);
	this->setModal(true);
	this->setAcceptMode(QFileDialog::AcceptOpen);

	QLayout* layout = this->layout();
	if(layout)
	{
		layout->addWidget(m->imageLabel);
		layout->addWidget(m->resolutionLabel);
	}

	connect(this, &QFileDialog::currentChanged, this, &ImageSelectionDialog::fileSelected);
}

ImageSelectionDialog::~ImageSelectionDialog() {}

void ImageSelectionDialog::fileSelected(const QString& file)
{
	QPixmap pm(file);
	if(pm.isNull()) {
		return;
	}

	m->imageLabel->setPixmap(
		pm.scaled(m->imageLabel->size())
	);

	m->resolutionLabel->setText(
		QString("%1x%2").arg(pm.width()).arg(pm.height())
	);
}

void ImageSelectionDialog::showEvent(QShowEvent* e)
{
	this->setDirectory(m->start_dir);
	//this->setHistory({m->start_dir});

	Gui::WidgetTemplate<QFileDialog>::showEvent(e);
}
