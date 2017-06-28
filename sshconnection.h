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

#ifndef SSHCONNECTION_H
#define SSHCONNECTION_H

#include "sftpsession.h"

#include <libssh/libssh.h>

#include <QString>

class SSHConnection
{
    friend class SFTPSession;

public:
    static const int BUFFSIZE = 16384;

    SSHConnection (const QString        host,
                   const QString        password,
                   const quint16        port = 22,
                   const QString        userName = "root");
    ~SSHConnection ();

    void disconnect ();

    int connect ();
    int executeCommand (const char* command, char* output);

private:
    const char* sshDir;
    unsigned short port;

    QString host;
    QString password;
    QString userName;

    SSH_OPTIONS* options;
    SSH_SESSION* session;

    int setOptions ();
    int connectToServer ();
    int authenticateToServer ();
    int authKbdInteractive ();
};

#endif // SSHCONNECTION_H
