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

#include "filebrowserdialog.h"

FileBrowserDialog::FileBrowserDialog (
    QWidget *   parent,
    QString     hostName,
    QString     userName,
    QString     password) :
    QDialog(parent)
{
    setupUi(this);
    this->dirModel = new DirectoryModel(this);
    treeView->setModel(dirModel);

    setWindowTitle("File Browser: " + userName + "@" + hostName);
    connect(treeView, SIGNAL(doubleClicked(const QModelIndex &)), this,
            SLOT(itemDoubleClicked(const QModelIndex &)));

    this->sshConn = new SSHConnection(hostName.toStdString().c_str(),
                                      password.toStdString().c_str(),
                                      22,
                                      userName.toStdString().c_str());

    if (sshConn->connect()) {
        delete sshConn;
        MainWindow::logScreen->appendText("SSH connection attempt failed.");
        return;
    }
    this->rootNode = new DirectoryModelNode(DirectoryModelNode::Directory);
    DirectoryModelNode * rootDir = new DirectoryModelNode(
            DirectoryModelNode::Directory,
            "",
            "",
            "",
            "",
            "",
            "",
            "/");
    rootNode->children.append(rootDir);
    rootDir->parent = rootNode;
    rootDir->setPath();
    dirModel->setRootNode(rootNode);
}

FileBrowserDialog::~FileBrowserDialog ()
{
    delete sshConn;
}

void
FileBrowserDialog::itemDoubleClicked (
    const QModelIndex & index)
{
    char output[BUFFSIZE] = { '0' };
    int len = 0;

    DirectoryModelNode * parent = dirModel->nodeFromIndex(index);

    if (parent->type != DirectoryModelNode::Directory)
        return;

    if (parent->children.size() != 0)
        return;
    QString command = QString("cd \"%1\" \n ls -AlLBhq --file-type").arg(
            parent->path + parent->name);
    len = sshConn->executeCommand(command.toStdString().c_str(), output);

    QStringListIterator iter_1(QString(output).split("\n"));
    iter_1.next(); // the row showing the total byte count

    while (iter_1.hasNext()) {
        QStringListIterator iter_2 = iter_1.next().split(QRegExp(
                                                             "\\s+"),
                                                         QString::
                                                         SkipEmptyParts);

        while (iter_2.hasNext()) {
            QString permission = iter_2.next();
            iter_2.next();
            QString uid = iter_2.next();
            QString gid = iter_2.next();
            QString size = iter_2.next();
            QString date = iter_2.next();
            QString time = iter_2.next();
            QString name;

            while (iter_2.hasNext()) {
                name = name + iter_2.next();

                if (iter_2.hasNext())
                    name += " ";
            }
            DirectoryModelNode::Type type = DirectoryModelNode::File;

            if (name.endsWith("/"))
                type = DirectoryModelNode::Directory;
            DirectoryModelNode * node = new DirectoryModelNode(type,
                                                               permission,
                                                               uid,
                                                               gid,
                                                               size,
                                                               date,
                                                               time,
                                                               name);
            node->parent = parent;
            node->setPath();
            parent->children.append(node);
        }
    }
}

DirectoryModelNode*
FileBrowserDialog::currentNode ()
{
    return dirModel->nodeFromIndex(treeView->currentIndex());
}

