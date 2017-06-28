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

#include "directorymodel.h"

DirectoryModel::DirectoryModel (
    QObject * parent) :
    QAbstractItemModel(parent)
{
    rootNode = 0;
}

DirectoryModel::~DirectoryModel ()
{
    delete rootNode;
}

void
DirectoryModel::setRootNode (
    DirectoryModelNode * node)
{
    delete rootNode;
    rootNode = node;
    reset();
}

int
DirectoryModel::rowCount (
    const QModelIndex & parent) const
{
    if (parent.column() > 0)
        return 0;
    DirectoryModelNode * parentNode = nodeFromIndex(parent);

    if (!parentNode)
        return 0;
    return parentNode->children.count();
}

int
DirectoryModel::columnCount (
    const QModelIndex & /* parent */) const
{
    return 7;
}

QModelIndex
DirectoryModel::index (
    int row, int column,
    const QModelIndex & parent) const
{
    if (!rootNode || row < 0 || column < 0)
        return QModelIndex();
    DirectoryModelNode * parentNode = nodeFromIndex(parent);
    DirectoryModelNode * childNode = parentNode->children.value(row);

    if (!childNode)
        return QModelIndex();
    return createIndex(row, column, childNode);
}

QModelIndex
DirectoryModel::parent (
    const QModelIndex & child) const
{
    DirectoryModelNode * node = nodeFromIndex(child);

    if (!node)
        return QModelIndex();
    DirectoryModelNode * parentNode = node->parent;

    if (!parentNode)
        return QModelIndex();
    DirectoryModelNode * grandparentNode = parentNode->parent;

    if (!grandparentNode)
        return QModelIndex();
    int row = grandparentNode->children.indexOf(parentNode);

    return createIndex(row, 0, parentNode);
}

QVariant
DirectoryModel::data (
    const QModelIndex & index, int role) const
{
    if (( role != Qt::DisplayRole  ) && role != Qt::DecorationRole)
        return QVariant();
    DirectoryModelNode * node = nodeFromIndex(index);

    if (!node)
        return QVariant();

    if (role == Qt::DecorationRole && index.column() == 0) {
        if (node->type == DirectoryModelNode::Directory) {
            return QIcon(":/resources/folder.png");
        }
        return QIcon(":/resources/unknown.png");
    }

    switch (index.column()) {
    case 0:
        return node->name;

    case 1:
        return node->permission;

    case 2:
        return node->uid;

    case 3:
        return node->gid;

    case 4:
        return node->size;

    case 5:
        return node->date;

    case 6:
        return node->time;
    }
    return QVariant();
}

QVariant
DirectoryModel::headerData (
    int                 section,
    Qt::Orientation     orientation,
    int                 role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if (section == 0)
            return tr("Name");
        else if (section == 1)
            return tr("Permission");
        else if (section == 2)
            return tr("UID");
        else if (section == 3)
            return tr("GID");
        else if (section == 4)
            return tr("Size");
        else if (section == 5)
            return tr("Date");
        else if (section == 6)
            return tr("Time");
    }
    return QVariant();
}

DirectoryModelNode*
DirectoryModel::nodeFromIndex (
    const QModelIndex & index) const
{
    if (index.isValid())
        return static_cast<DirectoryModelNode*>(index.internalPointer());
    else
        return rootNode;
}

