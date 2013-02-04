/* Copyright (C) 2005-2011, Thorvald Natvig <thorvald@natvig.com>

   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.
   - Neither the name of the Mumble Developers nor the names of its
     contributors may be used to endorse or promote products derived from this
     software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "murmur_pch.h"

#include "ServerDB.h"

#include "ACL.h"
#include "Channel.h"
#include "Connection.h"
#include "DBus.h"
#include "Group.h"
#include "Meta.h"
#include "Server.h"
#include "ServerUser.h"
#include "User.h"

#define SQLDO(x) ServerDB::exec(query, QLatin1String(x), true)
#define SQLMAY(x) ServerDB::exec(query, QLatin1String(x), false, false)
#define SQLPREP(x) ServerDB::prepare(query, QLatin1String(x))
#define SQLEXEC() ServerDB::exec(query)
#define SQLEXECBATCH() ServerDB::execBatch(query)
#define SOFTEXEC() ServerDB::exec(query, QString(), false)

class TransactionHolder {
	public:
		QSqlQuery *qsqQuery;
		TransactionHolder() {
			ServerDB::db->transaction();
			qsqQuery = new QSqlQuery();
		}

		~TransactionHolder() {
			qsqQuery->clear();
			delete qsqQuery;
			ServerDB::db->commit();
		}
};

QSqlDatabase *ServerDB::db;
Timer ServerDB::tLogClean;
QString ServerDB::qsUpgradeSuffix;

ServerDB::ServerDB() {
}

ServerDB::~ServerDB() {
}

bool ServerDB::prepare(QSqlQuery &query, const QString &str, bool fatal, bool warn) {
	return false;
}

bool ServerDB::exec(QSqlQuery &query, const QString &str, bool fatal, bool warn) {
	return false;
}

bool ServerDB::execBatch(QSqlQuery &query, const QString &str, bool fatal) {
	return false;
}

void Server::initialize() {
}

int Server::registerUser(const QMap<int, QString> &info) {
	return -1;
}

bool Server::unregisterUserDB(int id) {
	return false;
}

QMap<int, QString > Server::getRegisteredUsers(const QString &filter) {
	QMap<int, QString > m;
	return m;
}

bool Server::isUserId(int id) {
	return false;
}

QMap<int, QString> Server::getRegistration(int id) {
	QMap<int, QString> info;
	return info;
}

int Server::authenticate(QString &name, const QString &pw, int sessionId, const QStringList &emails, const QString &certhash, bool bStrongCert, const QList<QSslCertificate> &certs) {
	return -1;
}

bool Server::setInfo(int id, const QMap<int, QString> &setinfo) {
	return false;
}

bool Server::setTexture(int id, const QByteArray &texture) {
	return false;
}

void ServerDB::setSUPW(int srvnum, const QString &pw) {
}

QString Server::getUserName(int id) {
	QString name;
	return name;
}

int Server::getUserID(const QString &name) {
	return -2;
}

QByteArray Server::getUserTexture(int id) {
	QByteArray qba;
	return qba;
}

void Server::addLink(Channel *c, Channel *l) {
	c->link(l);
}

void Server::removeLink(Channel *c, Channel *l) {
	c->unlink(l);
}

Channel *Server::addChannel(Channel *p, const QString &name, bool temporary, int position) {
	int id = 0;
	while (qhChannels.contains(id))
		++id;

	Channel *c = new Channel(id, name, p);
	c->bTemporary = temporary;
	c->iPosition = position;
	qhChannels.insert(id, c);
	return c;
}

void Server::removeChannelDB(const Channel *c) {
	qhChannels.remove(c->iId);
}

void Server::updateChannel(const Channel *c) {
}

/** Reads the channel privileges (group and acl) as well as the channel information key/value pairs from the database.
 * @param c Channel to fetch information for
 */
void Server::readChannelPrivs(Channel *c) {
}

void Server::readChannels(Channel *p) {
	if(p) return;
	Channel *c = new Channel(0, "Team9000 Mumble", NULL);
	c->setParent(this);
	qhChannels.insert(c->iId, c);
	c->bInheritACL = false;
}

void Server::readLinks() {
}

void Server::setLastChannel(const User *p) {
}

int Server::readLastChannel(int id) {
	return -1;
}

void Server::dumpChannel(const Channel *c) {
	/*
	Group *g;
	ChanACL *acl;
	int pid;

	if (c == NULL) {
		c = qhChannels.value(0);
	}

	qWarning("Channel %s (ACLInherit %d)", qPrintable(c->qsName), c->bInheritACL);
	qWarning("Description: %s", qPrintable(c->qsDesc));
	foreach(g, c->qhGroups) {
		qWarning("Group %s (Inh %d  Able %d)", qPrintable(g->qsName), g->bInherit, g->bInheritable);
		foreach(pid, g->qsAdd)
			qWarning("Add %d", pid);
		foreach(pid, g->qsRemove)
			qWarning("Remove %d", pid);
	}
	foreach(acl, c->qlACL) {
		int allow = static_cast<int>(acl->pAllow);
		int deny = static_cast<int>(acl->pDeny);
		qWarning("ChanACL Here %d Sub %d Allow %04x Deny %04x ID %d Group %s", acl->bApplyHere, acl->bApplySubs, allow, deny, acl->iUserId, qPrintable(acl->qsGroup));
	}
	qWarning(" ");

	foreach(c, c->qlChannels) {
		dumpChannel(c);
	}
	*/
}

void Server::getBans() {
	qlBans.clear();
}

void Server::saveBans() {
}

QVariant Server::getConf(const QString &key, QVariant def) {
	return ServerDB::getConf(iServerNum, key, def);
}

QVariant ServerDB::getConf(int server_id, const QString &key, QVariant def) {
	return def;
}

QMap<QString, QString> ServerDB::getAllConf(int server_id) {
	QMap<QString, QString> map;
	return map;
}

void Server::setConf(const QString &key, const QVariant &value) {
	ServerDB::setConf(iServerNum, key, value);
}

void Server::dblog(const QString &str) {
}

void ServerDB::wipeLogs() {
}

QList<QPair<unsigned int, QString> > ServerDB::getLog(int server_id, unsigned int offs_min, unsigned int offs_max) {
	QList<QPair<unsigned int, QString> > ql;
	return ql;
}

int ServerDB::getLogLen(int server_id) {
	return 0;
}

void ServerDB::setConf(int server_id, const QString &k, const QVariant &value) {
}


QList<int> ServerDB::getAllServers() {
	QList<int> ql;
	ql << 1;
	return ql;
}

QList<int> ServerDB::getBootServers() {
	return getAllServers();
}

bool ServerDB::serverExists(int num) {
	return num == 1;
}

int ServerDB::addServer() {
	return 0;
}

void ServerDB::deleteServer(int server_id) {
}
