/* AlternateCoverItemDelegate.h */

/* Copyright (C) 2011-2016 Lucio Carreras
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


/*
 * AlternateCoverItemDelegate.h
 *
 *  Created on: Jul 1, 2011
 *      Author: luke
 */

#ifndef ALTERNATECOVERITEMDELEGATE_H_
#define ALTERNATECOVERITEMDELEGATE_H_

#include <QWidget>
#include <QTableView>
#include <QItemDelegate>
#include <QLabel>

class AlternateCoverItemDelegate : public QItemDelegate{

	Q_OBJECT

public:
	AlternateCoverItemDelegate(QObject* parent=nullptr);
	virtual ~AlternateCoverItemDelegate();


public:
	virtual void paint(QPainter *painter,
			   const QStyleOptionViewItem &option,
			   const QModelIndex &index) const override;

	virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:

	QLabel*		label;

};

#endif /* ALTERNATECOVERITEMDELEGATE_H_ */
