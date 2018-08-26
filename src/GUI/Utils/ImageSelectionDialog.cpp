/* ImageSelectionDialog.cpp */

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



#include "ImageSelectionDialog.h"

#include <QLayout>
#include <QLabel>
#include <QStringList>
#include <QDialog>

struct ImageSelectionDialog::Private
{
	QLabel* img_label=nullptr;
	QLabel* res_label=nullptr;

	Private(QWidget* parent)
	{
		img_label = new QLabel(parent);
		img_label->setMinimumSize(100, 100);
		img_label->setMaximumSize(100, 100);

		res_label = new QLabel(parent);
	}
};

ImageSelectionDialog::ImageSelectionDialog(const QString& dir, QWidget* parent) :
    Gui::WidgetTemplate<QFileDialog>(parent)
{
	m = Pimpl::make<Private>(this);

	QStringList filters;
	    filters << "*.jpg";
		filters << "*.png";
		filters << "*.gif";

	this->setDirectory(dir);
	this->setFilter(QDir::Files | QDir::Dirs);
	this->setLabelText(QFileDialog::DialogLabel::FileName, tr("Open image files"));
	this->setNameFilters(filters);
	this->setViewMode(QFileDialog::Detail);
	this->setModal(true);
	this->setAcceptMode(QFileDialog::AcceptOpen);
	QLayout* layout = this->layout();
	if(layout)
	{
		layout->addWidget(m->img_label);
		layout->addWidget(m->res_label);
	}

	connect(this, &QFileDialog::currentChanged, this, &ImageSelectionDialog::file_selected);
}

ImageSelectionDialog::~ImageSelectionDialog()
{

}

void ImageSelectionDialog::file_selected(const QString& file)
{
	QPixmap pm(file);
	if(pm.isNull()) {
		return;
	}

	m->img_label->setPixmap(
	    pm.scaled(m->img_label->size())
	);

	m->res_label->setText(
	    QString("%1x%2").arg(pm.width()).arg(pm.height())
	);
}
