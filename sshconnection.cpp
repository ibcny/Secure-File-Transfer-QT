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

#include "sshconnection.h"

#include "mainwindow.h"

#include <errno.h>
#include <iostream>

#include <QInputDialog>
#include <QMessageBox>

SSHConnection::SSHConnection (
    const QString       host,
    const QString       password,
    const quint16       port,
    const QString       userName)
{
    this->host = host;
    this->port = (unsigned short)port;
    this->password = password;
    this->userName = userName;
    this->session = NULL;
    this->options = NULL;
    this->sshDir = "%s/.ssh/";
}

SSHConnection::~SSHConnection ()
{ }

void
SSHConnection::disconnect ()
{
    if (session) {
        ssh_disconnect(session);
        ssh_finalize();
    }
}

int
SSHConnection::connect ()
{
    MainWindow::logScreen->appendText(QString(
                                          "SSH connection attempt to ").append(
                                          userName)
                                      .append("@").append(host).append(" port:")
                                      .append(QString::number(port)));

    if (setOptions()) {
        MainWindow::logScreen->appendText(QString(
                                              "SSH connection attempt failed: setOptions()"));
        return 1;
    }

    if (connectToServer()) {
        MainWindow::logScreen->appendText(QString(
                                              "SSH connection attempt failed: connectToServer()"));
        return 1;
    }

    if (authenticateToServer()) {
        MainWindow::logScreen->appendText(QString(
                                              "SSH connection attempt failed: authenticateToServer()"));
        return 1;
    }
    return 0;
}

int
SSHConnection::executeCommand (
    const char* command, char* output)
{
    ssize_t len;
    CHANNEL* channel;

    if (this->session == NULL) {
        MainWindow::logScreen->appendText(
            "SSH error: Command execution operation failed. "
            "SSH session has not started yet.");
        return -1;
    }

    if (!(channel = channel_new(session))) {
        MainWindow::logScreen->appendText(QString(
                                              "Error opening channel : %1").arg(
                                              ssh_get_error(session)));
        return -1;
    }

    if (channel_open_session(channel)) {
        MainWindow::logScreen->appendText(QString(
                                              "Error opening channel : %1").arg(
                                              ssh_get_error(session)));
        return -1;
    }

    if (channel_request_exec(channel, command)) {
        MainWindow::logScreen->appendText(QString(
                                              "Error executing \"%1\" : %2").
                                          arg(command, ssh_get_error(session)));
        return -1;
    }
    /* TODO
     * Reads till EOF; what happens if the command does not put an EOF(Is this a valid case?)
     */
    len = channel_read(channel, output, BUFFSIZE, 0);
    output[len] = '\0';
    channel_free(channel);

    return len;
}

int
SSHConnection::setOptions ()
{
    options = ssh_options_new();

    if (ssh_options_set_username(options,
                                 userName.toStdString().c_str()) < 0)
    {
        MainWindow::logScreen->appendText("ssh_options_set_username failed!");
        ssh_options_free(options);
        return 1;
    }

    if (ssh_options_set_host(options, host.toStdString().c_str()) < 0) {
        MainWindow::logScreen->appendText("ssh_options_set_host failed!");
        ssh_options_free(options);
        return 1;
    }

    if (ssh_options_set_port(options, port) < 0) {
        MainWindow::logScreen->appendText("ssh_options_set_port failed!");
        ssh_options_free(options);
        return 1;
    }

    if (ssh_options_set_ssh_dir(options, this->sshDir) < 0) {
        MainWindow::logScreen->appendText("ssh_options_set_ssh_dir failed!");
        ssh_options_free(options);
        return 1;
    }
    return 0;
}

int
SSHConnection::connectToServer ()
{
    unsigned char* hash = NULL;
    int hlen, state;
    char* hexa;

    this->session = ssh_new();
    ssh_set_options(session, options);

    if (ssh_connect(session)) {
        MainWindow::logScreen->appendText(QString(
                                              "Connection attempt to server failed: ")
                                          .append(ssh_get_error(session)));
        disconnect();
        return 1;
    }
    state = ssh_is_server_known(session);
    hlen = ssh_get_pubkey_hash(session, &hash);

    if (hlen < 0) {
        disconnect();
        return 1;
    }

    switch (state) {
    case SSH_SERVER_KNOWN_OK:
        break; /* ok */

    case SSH_SERVER_KNOWN_CHANGED:
        MainWindow::logScreen->appendText(QString("Host key for server changed"));
        free(hash);
        MainWindow::logScreen->appendText(
            "For security reason, connection will be stopped");
        disconnect();
        return 1;

    case SSH_SERVER_FOUND_OTHER:
        MainWindow::logScreen->appendText(
            "The host key for this server was not found but an other type of key exists.");
        MainWindow::logScreen->appendText(
            "An attacker might change the default server key to confuse your client into thinking the key does not exist.");
        MainWindow::logScreen->appendText(
            "For security reason, connection will be stopped.");
        disconnect();
        return 1;

    case SSH_SERVER_FILE_NOT_FOUND:
        MainWindow::logScreen->appendText(QString(
                                              "Could not find known hosts' file. If you accept the host key here ")
                                          .append(
                                              "the file will be automatically created."));

    // fallback to SSH_SERVER_NOT_KNOWN behaviour
    case SSH_SERVER_NOT_KNOWN:
        hexa = ssh_get_hexa(hash, hlen);

        if (QMessageBox::question(0, "SSH: Unknown Server",
                                  QString(
                                      "The server is unknown. Do you trust the host key ?\n\n")
                                  .
                                  append("Public key hash: %1\n").arg(QString(
                                                                          hexa)),
                                  QMessageBox::Cancel | QMessageBox::No |
                                  QMessageBox::Yes) == QMessageBox::Yes)
        {
            if (QMessageBox::question(0, "SSH: New Key",
                                      QString(
                                          "This new key will be written on disk for further usage. Do you agree ?\n"),
                                      QMessageBox::Cancel | QMessageBox::No |
                                      QMessageBox::Yes) == QMessageBox::Yes)
            {
                if (ssh_write_knownhost(session) < 0) {
                    free(hash);
                    MainWindow::logScreen->appendText(QString(
                                                          "ssh_write_knownhost : error )")
                                                      .append(strerror(errno)));
                    disconnect();
                    return 1;
                } else
                    MainWindow::logScreen->appendText(
                        "Host public key was written to disk successfully!");
            } else
                MainWindow::logScreen->appendText(
                    "Host public key won't be written to disk, discarded.");
        } else {
            free(hexa);
            MainWindow::logScreen->appendText(
                "User does not trust the host key, connection will discontinue.");
            disconnect();
            return 1;
        }
        break;

    case SSH_SERVER_ERROR:
        free(hash);
        MainWindow::logScreen->appendText(QString("error : ").append(
                                              ssh_get_error(session)));
        disconnect();
        return 1;
    }
    free(hash);

    return 0;
}

int
SSHConnection::authenticateToServer ()
{
    int auth;
    char* banner;

    auth = ssh_userauth_autopubkey(session, NULL);

    if (auth == SSH_AUTH_ERROR) {
        MainWindow::logScreen->appendText(QString(
                                              "Authenticating with pubkey: %1")
                                          .arg(ssh_get_error(session)));
        disconnect();
        return 1;
    }
    banner = ssh_get_issue_banner(session);

    if (banner) {
        MainWindow::logScreen->appendText(banner);
        free(banner);
    }

    if (auth != SSH_AUTH_SUCCESS) {
        auth = authKbdInteractive();

        if (auth == SSH_AUTH_ERROR) {
            MainWindow::logScreen->appendText(QString(
                                                  "authenticating with keyb-interactive: %1")
                                              .
                                              arg(ssh_get_error(session)));
            disconnect();
            return 1;
        }
    }

    if (auth != SSH_AUTH_SUCCESS) {
        if (ssh_userauth_password(session, NULL,
                                  password.toStdString().c_str()) !=
            SSH_AUTH_SUCCESS)
        {
            MainWindow::logScreen->appendText(QString(
                                                  "Authentication failed: %1").
                                              arg(ssh_get_error(session)));
            disconnect();
            return 1;
        }
        // memset(password, 0, strlen(password) );
    }
    MainWindow::logScreen->appendText(QString("SSH Authentication succeeded!"));

    return 0;
}

int
SSHConnection::authKbdInteractive ()
{
    const char* name, * instruction, * prompt;
    const char* buffer;
    int i, n;
    char echo;
    int err = ssh_userauth_kbdint(session, NULL, NULL);

    while (err == SSH_AUTH_INFO) {
        name = ssh_userauth_kbdint_getname(session);
        instruction = ssh_userauth_kbdint_getinstruction(session);
        n = ssh_userauth_kbdint_getnprompts(session);

        MainWindow::logScreen->appendText(QString(
                                              "SSH Authorization : Number of prompts = %1")
                                          .arg(n));

        if (strlen(name) > 0)
            MainWindow::logScreen->appendText(QString("name %1").arg(name));

        if (strlen(instruction) > 0)
            MainWindow::logScreen->appendText(QString("instruction %1").arg(
                                                  instruction));

        for (i = 0 ; i < n ; ++i) {
            prompt = ssh_userauth_kbdint_getprompt(session, i, &echo);

            if (echo) {
                bool ok;
                QString text = QInputDialog::getText(0, QString("Prompt"),
                                                     QString("%1").arg(
                                                         prompt),
                                                     QLineEdit::Normal,
                                                     "", &ok);

                /* TODO
                 * Buffer length check
                 */
                if (ok)
                    buffer = text.toStdString().c_str();

                if (ssh_userauth_kbdint_setanswer(session, i, buffer) < 0) {
                    return SSH_AUTH_ERROR;
                }
            } else {
                if (ssh_userauth_kbdint_setanswer(session, i,
                                                  password.toStdString().c_str())
                    < 0)
                {
                    return SSH_AUTH_ERROR;
                }
            }
        }
        err = ssh_userauth_kbdint(session, NULL, NULL);
    }
    return err;
}

