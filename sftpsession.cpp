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

#include "sftpsession.h"

#include "sshconnection.h"

SFTPSession::SFTPSession (
    SSHConnection* sshConnection)
{
    this->sshConnection = sshConnection;
    this->sftp_session = NULL;
    this->curDir = NULL;
}

SFTPSession::~SFTPSession ()
{ }

void
SFTPSession::close ()
{
    if (this->sftp_session)
        closeDir(curDir);
}

int
SFTPSession::init ()
{
    this->sftp_session = sftp_new(sshConnection->session);

    if (!this->sftp_session) {
        MainWindow::logScreen->appendText(QString(
                                              "SFTP error: initialising channel: %1")
                                          .arg(ssh_get_error(sshConnection->
                                                             session)));
        return 1;
    }

    if (sftp_init(this->sftp_session)) {
        MainWindow::logScreen->appendText(QString(
                                              "SFTP error: initialising channel: %1")
                                          .arg(ssh_get_error(sshConnection->
                                                             session)));
        return 1;
    }
    changeDir("/");

    return 0;
}

int
SFTPSession::changeDir (
    const char* dir)
{
    if (this->sftp_session == NULL) {
        MainWindow::logScreen->appendText(
            "SFTP error: Directory change operation failed. "
            "SFTP session has not started yet.");
        return 1;
    }
    // the connection is made
    // opening a directory
    curDir = sftp_opendir(this->sftp_session, dir);

    if (!dir) {
        MainWindow::logScreen->appendText(QString(
                                              "SFTP error: Directory not opened: %1")
                                          .arg(ssh_get_error(sshConnection->
                                                             session)));
        return 1;
    }
    MainWindow::logScreen->appendText(QString(
                                          "SFTP: Current directory has changed to \"%1\"")
                                      .
                                      arg(dir));
    return 0;
}

int
SFTPSession::readDir (
    SFTP_DIR* dir, QList<SFTP_ATTRIBUTES*> & fileList)
{
    if (this->sftp_session == NULL) {
        MainWindow::logScreen->appendText(
            "SFTP error: Directory change operation failed. "
            "SFTP session has not started yet.");
        return 1;
    }
    // read the whole directory, file by file */
    SFTP_ATTRIBUTES* file;

    while ((file = sftp_readdir(sftp_session, dir))) {
        fileList.append(file);

#if 0
        fprintf(stderr, "%30s(%.8o) : %.5d.%.5d : %.10llu bytes\n",
                attr->name,
                attr->permissions,
                attr->uid,
                attr->gid,
                (long long unsigned int)attr->size);

        // TODO : this may cause inconvenience or defect
        sftp_attributes_free(attr);
#endif
    }

    // when attr = NULL, an error has occured OR the directory listing
    // is end of file
    if (!sftp_dir_eof(dir)) {
        MainWindow::logScreen->appendText(QString("Error: %1").arg(
                                              ssh_get_error(sshConnection->
                                                            session)));
        return 1;
    }
    return 0;
}

int
SFTPSession::moveFileToHost (
    const char* file, const char* targetFileName)
{
    SFTP_FILE* targetFile;
    int fd = open(file, O_RDONLY);
    ssize_t len;
    char buffer[BUFFSIZE];

    if (this->sftp_session == NULL) {
        MainWindow::logScreen->appendText(
            "SFTP error: Directory change operation failed. "
            "SFTP session has not started yet.");
        return 1;
    }

    if (fd < 0) {
        MainWindow::logScreen->appendText(QString("Error: %1: %2").arg(file,
                                                                       strerror(
                                                                           errno)));
        ::close(fd);
        return 1;
    }
    /* open target file */
    targetFile = sftp_open(sftp_session, targetFileName, O_WRONLY | O_CREAT, 0);

    if (!targetFile) {
        MainWindow::logScreen->appendText(QString(
                                              "Error on opening target file: %1: %2")
                                          .
                                          arg(targetFileName,
                                              ssh_get_error(sftp_session)));
        ::close(fd);
        return 1;
    }

    while ((len = read(fd, buffer, BUFFSIZE)) > 0) {
        if (sftp_write(targetFile, buffer, len) != len) {
            MainWindow::logScreen->appendText(QString(
                                                  "Error: writing bytes: %2").
                                              arg(ssh_get_error(sftp_session)));
            ::close(fd);
            sftp_close(targetFile);
            return 1;
        }
    }
    MainWindow::logScreen->appendText(QString(
                                          "File \"%1\" has successfully moved to target host.")
                                      .arg(file));

    sftp_close(targetFile);
    ::close(fd);

    return 0;
}

int
SFTPSession::closeDir (
    SFTP_DIR* dir)
{
    if (sftp_closedir(dir)) {
        MainWindow::logScreen->appendText(QString("SFTP error: %1")
                                          .arg(ssh_get_error(sshConnection->
                                                             session)));
        return 1;
    }
    return 0;
}

SFTP_DIR*
SFTPSession::readDir (
    const char* dirName, QList<SFTP_ATTRIBUTES*> & fileList)
{
    SFTP_DIR* dir = sftp_opendir(sftp_session, dirName);

    if (!dir) {
        MainWindow::logScreen->appendText(QString(
                                              "SFTP error: Directory not opened: %1")
                                          .arg(ssh_get_error(sshConnection->
                                                             session)));
        return NULL;
    }
    readDir(dir, fileList);

    return dir;
}

SFTP_DIR*
SFTPSession::getCurDir ()
{
    return curDir;
}

