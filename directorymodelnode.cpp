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

#include "directorymodelnode.h"

DirectoryModelNode::DirectoryModelNode (
    Type                type,
    const QString &     permission,
    const QString &     uid,
    const QString &     gid,
    const QString &     size,
    const QString &     date,
    const QString &     time,
    const QString &     name)
{
    this->type = type;
    this->permission = permission;
    this->uid = uid;
    this->gid = gid;
    this->size = size;
    this->date = date;
    this->time = time;
    this->name = name;
    this->parent = NULL;
}

DirectoryModelNode::~DirectoryModelNode ()
{
    qDeleteAll(children);
}

void
DirectoryModelNode::setPath ()
{
    if (name == "/")
        path = "";
    else
        path = parent->path + parent->name;
}

