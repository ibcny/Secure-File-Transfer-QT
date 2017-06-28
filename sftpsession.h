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

#ifndef SFTPSESSION_H
#define SFTPSESSION_H

#include "mainwindow.h"

#include <fcntl.h>
#include <unistd.h>

#include <libssh/libssh.h>
#include <libssh/sftp.h>

#include <QList>

class SSHConnection;

class SFTPSession
{
    friend class SSHConnection;

public:
    static const unsigned short BUFFSIZE = 16384;

    SFTPSession (SSHConnection* sshConnection);
    ~SFTPSession ();

    void close ();

    int init ();
    int changeDir (const char* dir);
    int readDir (SFTP_DIR* dir, QList<SFTP_ATTRIBUTES*> & fileList);
    int moveFileToHost (const char* file, const char* targetFileName);
    int closeDir (SFTP_DIR* dir);

    SFTP_DIR * readDir (const char* dirName, QList<SFTP_ATTRIBUTES*> & fileList);
    SFTP_DIR * getCurDir ();

private:
    SFTP_SESSION* sftp_session;
    SFTP_DIR* curDir;
    SSHConnection* sshConnection;
};

#endif // SFTPSESSION_H

