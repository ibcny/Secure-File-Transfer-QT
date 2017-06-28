/*
 * ----------------------------------------------------------------------------
 * SFTP file manager for QT
 *
 * Secure file manager applet plug-in for QT
 *
 * Copyright (C) 2013 Ibrahim Can Yuce <canyuce[[at]]gmail.com>
 *
 * ----------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 */

#ifndef DIRECTORYMODEL_H
#define DIRECTORYMODEL_H

#include "directorymodelnode.h"

#include <QAbstractItemModel>
#include <QIcon>
#include <iostream>

class DirectoryModel :
    public QAbstractItemModel
{
public:
    DirectoryModel (QObject* parent = 0);
    ~DirectoryModel ();

    void setRootNode (DirectoryModelNode* node);

    int rowCount (const QModelIndex & parent) const;
    int columnCount (const QModelIndex & parent) const;

    QModelIndex index (int row, int column,
                       const QModelIndex & parent) const;

    QModelIndex parent (const QModelIndex & child) const;

    QVariant data (const QModelIndex & index, int role) const;

    QVariant headerData (int section, Qt::Orientation orientation,
                         int role) const;

    DirectoryModelNode * nodeFromIndex (const QModelIndex & index) const;

private:
    DirectoryModelNode* rootNode;
};

#endif // DIRECTORYMODEL_H
