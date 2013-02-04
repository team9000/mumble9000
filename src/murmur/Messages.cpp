/* Copyright (C) 2005-2011, Thorvald Natvig <thorvald@natvig.com>
   Copyright (C) 2009-2011, Stefan Hacker <dd0t@users.sourceforge.net>

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

#include "User.h"
#include "Channel.h"
#include "ACL.h"
#include "Group.h"
#include "Message.h"
#include "ServerDB.h"
#include "Connection.h"
#include "Server.h"
#include "ServerUser.h"
#include "Version.h"

#define MSG_SETUP(st) \
	if (uSource->sState != st) { \
		return; \
	} \
	uSource->bwr.resetIdleSeconds()

#define MSG_SETUP_NO_UNIDLE(st) \
	if (uSource->sState != st) \
		return

#define VICTIM_SETUP \
	ServerUser *pDstServerUser = uSource; \
	if (msg.has_session()) \
		pDstServerUser = qhUsers.value(msg.session()); \
	if (! pDstServerUser) \
		return; \
	Q_UNUSED(pDstServerUser)

#define PERM_DENIED(who, where, what) \
	{ \
		MumbleProto::PermissionDenied mppd; \
		mppd.set_permission(static_cast<int>(what)); \
		mppd.set_channel_id(where->iId); \
		mppd.set_session(who->uiSession); \
		mppd.set_type(MumbleProto::PermissionDenied_DenyType_Permission); \
		sendMessage(uSource, mppd); \
		log(uSource, QString("%1 not allowed to %2 in %3").arg(who->qsName).arg(ChanACL::permName(what)).arg(where->qsName)); \
	}

#define PERM_DENIED_TYPE(type) \
	{ \
		MumbleProto::PermissionDenied mppd; \
		mppd.set_type(MumbleProto::PermissionDenied_DenyType_##type); \
		sendMessage(uSource, mppd); \
	}

#define PERM_DENIED_FALLBACK(type,version,text) \
	{ \
		MumbleProto::PermissionDenied mppd; \
		mppd.set_type(MumbleProto::PermissionDenied_DenyType_##type); \
		if (uSource->uiVersion < version) \
			mppd.set_reason(u8(text)); \
		sendMessage(uSource, mppd); \
	}

#define PERM_DENIED_HASH(user) \
	{ \
		MumbleProto::PermissionDenied mppd; \
		mppd.set_type(MumbleProto::PermissionDenied_DenyType_MissingCertificate); \
		if (user) \
			mppd.set_session(user->uiSession); \
		sendMessage(uSource, mppd); \
	}

void Server::msgAuthenticate(ServerUser *uSource, MumbleProto::Authenticate &msg) {
	Channel *root = qhChannels.value(0);
	Channel *c;

	QString name = u8(msg.username());
	QString pw = u8(msg.password());

	// team9000
	if(name == "chat" && pw == "fWAmGWVzcGjRzQ7YGnhK") {
		// authenticating the gatekeeper
		uSource->iId = -1337;
		uSource->qsName = "Team9000 Gatekeeper";
	} else if(uSource->iId != -1337) {
		// processing an incoming user

		msg.set_username(u8(
			QString("%1`%2`%3`%4`%5`%6`%7")
			.arg(uSource->uiSession)
			.arg(name)
			.arg(uSource->uiVersion)
			.arg(uSource->qsRelease)
			.arg(uSource->qsOS)
			.arg(uSource->qsOSVersion)
			.arg(uSource->haAddress.toStdString())
		));

		bool foundone = false;
		foreach(ServerUser *u, qhUsers) {
			if(u->iId == -1337) {
				foundone = true;
				sendMessage(u, msg);
			}
		}

		if(!foundone) {
			log(uSource, QString("Rejected connection: Auth server offline"));
			MumbleProto::TextMessage mptm;
			mptm.add_tree_id(0);
			mptm.set_message(u8(QString(QLatin1String("Sorry, there seems to be a technical issue with the Team9000 Mumble-Login system - Please try again in a bit."))));
			sendMessage(uSource, mptm);
			uSource->disconnectSocket(true);
			return;
		}
		return;
	} else {
		// processing a gatekeeper auth reply
		QStringList split = name.split(',');
		if(split.length() != 5) return;

		name = split[0];

		bool idok = false;
		int auth_session = split[1].toInt(&idok);
		if(!idok) return;
		int auth_id = split[2].toInt(&idok);
		if(!idok) return;

		QString auth_type = split[3];
		QString auth_reason = split[4];

		ServerUser *u = qhUsers.value(auth_session);
		if(!u) return;

		uSource = u;
		uSource->qsName = name;
		uSource->iId = auth_id;

		MSG_SETUP(ServerUser::Connected);

		bool ok = false;

		QString reason;
		MumbleProto::Reject_RejectType rtType = MumbleProto::Reject_RejectType_None;

		if(auth_reason != "") {
			if(auth_type == "password") {
				rtType = MumbleProto::Reject_RejectType_WrongUserPW;
			} else if(auth_type == "name") {
				rtType = MumbleProto::Reject_RejectType_InvalidUsername;
			}
			reason = auth_reason;
		} else {
			ok = true;
		}

		//QStringList groups = pw.split(',');
		//setTempGroups(uSource->iId, uSource->uiSession, NULL, groups);

		if (qhUsers.count() > iMaxUsers) {
			reason = QString::fromLatin1("Server is full (max %1 users)").arg(iMaxUsers);
			rtType = MumbleProto::Reject_RejectType_ServerFull;
			ok = false;
		}

		if (!ok) {
			log(uSource, QString("Rejected connection: %1").arg(reason));
			MumbleProto::Reject mpr;
			mpr.set_reason(u8(reason));
			mpr.set_type(rtType);
			sendMessage(uSource, mpr);
			uSource->disconnectSocket();
			return;
		}
	}

	startThread();

	// Setup UDP encryption
	uSource->csCrypt.genKey();

	MumbleProto::CryptSetup mpcrypt;
	mpcrypt.set_key(std::string(reinterpret_cast<const char *>(uSource->csCrypt.raw_key), AES_BLOCK_SIZE));
	mpcrypt.set_server_nonce(std::string(reinterpret_cast<const char *>(uSource->csCrypt.encrypt_iv), AES_BLOCK_SIZE));
	mpcrypt.set_client_nonce(std::string(reinterpret_cast<const char *>(uSource->csCrypt.decrypt_iv), AES_BLOCK_SIZE));
	sendMessage(uSource, mpcrypt);

	bool fake_celt_support = false;
	if (msg.celt_versions_size() > 0) {
		for (int i=0;i < msg.celt_versions_size(); ++i)
			uSource->qlCodecs.append(msg.celt_versions(i));
	} else {
		uSource->qlCodecs.append(static_cast<qint32>(0x8000000b));
		fake_celt_support = true;
	}
	uSource->bOpus = msg.opus();
	recheckCodecVersions(uSource);

	MumbleProto::CodecVersion mpcv;
	mpcv.set_alpha(iCodecAlpha);
	mpcv.set_beta(iCodecBeta);
	mpcv.set_prefer_alpha(bPreferAlpha);
	mpcv.set_opus(bOpus);
	sendMessage(uSource, mpcv);

	if (!bOpus && uSource->bOpus && fake_celt_support) {
		sendTextMessage(NULL, uSource, false, QLatin1String("<strong>WARNING:</strong> Your client doesn't support the CELT codec, you won't be able to talk to or hear most clients. Please make sure your client was built with CELT support."));
	}

	// Transmit channel tree
	QQueue<Channel *> q;
	QSet<Channel *> chans;
	q << root;
	MumbleProto::ChannelState mpcs;
	while (! q.isEmpty()) {
		c = q.dequeue();
		chans.insert(c);

		mpcs.Clear();

		mpcs.set_channel_id(c->iId);
		if (c->cParent)
			mpcs.set_parent(c->cParent->iId);
		if (c->iId == 0)
			mpcs.set_name(u8(qsRegName.isEmpty() ? QLatin1String("Root") : qsRegName));
		else
			mpcs.set_name(u8(c->qsName));

		mpcs.set_position(c->iPosition);

		if ((uSource->uiVersion >= 0x010202) && ! c->qbaDescHash.isEmpty())
			mpcs.set_description_hash(blob(c->qbaDescHash));
		else if (! c->qsDesc.isEmpty())
			mpcs.set_description(u8(c->qsDesc));

		sendMessage(uSource, mpcs);

		foreach(c, c->qlChannels)
			q.enqueue(c);
	}

	// Transmit links
	foreach(c, chans) {
		if (c->qhLinks.count() > 0) {
			mpcs.Clear();
			mpcs.set_channel_id(c->iId);

			foreach(Channel *l, c->qhLinks.keys())
				mpcs.add_links(l->iId);
			sendMessage(uSource, mpcs);
		}
	}

	// Transmit user profile
	MumbleProto::UserState mpus;

	// team9000
	Channel *entry = root;
	userEnterChannel(uSource, entry, mpus);

	uSource->sState = ServerUser::Authenticated;
	mpus.set_session(uSource->uiSession);
	mpus.set_name(u8(uSource->qsName));
	if (uSource->iId >= 0) {
		mpus.set_user_id(uSource->iId);

		hashAssign(uSource->qbaTexture, uSource->qbaTextureHash, getUserTexture(uSource->iId));

		if (! uSource->qbaTextureHash.isEmpty())
			mpus.set_texture_hash(blob(uSource->qbaTextureHash));
		else if (! uSource->qbaTexture.isEmpty())
			mpus.set_texture(blob(uSource->qbaTexture));

		const QMap<int, QString> &info = getRegistration(uSource->iId);
		if (info.contains(ServerDB::User_Comment)) {
			hashAssign(uSource->qsComment, uSource->qbaCommentHash, info.value(ServerDB::User_Comment));
			if (! uSource->qbaCommentHash.isEmpty())
				mpus.set_comment_hash(blob(uSource->qbaCommentHash));
			else if (! uSource->qsComment.isEmpty())
				mpus.set_comment(u8(uSource->qsComment));
		}
	}
	if (! uSource->qsHash.isEmpty())
		mpus.set_hash(u8(uSource->qsHash));
	if (uSource->cChannel->iId != 0)
		mpus.set_channel_id(uSource->cChannel->iId);

	sendAll(mpus, 0x010202);

	if ((uSource->qbaTexture.length() >= 4) && (qFromBigEndian<unsigned int>(reinterpret_cast<const unsigned char *>(uSource->qbaTexture.constData())) == 600 * 60 * 4))
		mpus.set_texture(blob(uSource->qbaTexture));
	if (! uSource->qsComment.isEmpty())
		mpus.set_comment(u8(uSource->qsComment));
	sendAll(mpus, ~ 0x010202);

	// Transmit other users profiles
	foreach(ServerUser *u, qhUsers) {
		if (u->sState != ServerUser::Authenticated)
			continue;

		if (u == uSource)
			continue;

		mpus.Clear();
		mpus.set_session(u->uiSession);
		mpus.set_name(u8(u->qsName));
		if (u->iId >= 0)
			mpus.set_user_id(u->iId);
		if (uSource->uiVersion >= 0x010202) {
			if (! u->qbaTextureHash.isEmpty())
				mpus.set_texture_hash(blob(u->qbaTextureHash));
			else if (! u->qbaTexture.isEmpty())
				mpus.set_texture(blob(u->qbaTexture));
		} else if ((uSource->qbaTexture.length() >= 4) && (qFromBigEndian<unsigned int>(reinterpret_cast<const unsigned char *>(uSource->qbaTexture.constData())) == 600 * 60 * 4)) {
			mpus.set_texture(blob(u->qbaTexture));
		}
		if (u->cChannel->iId != 0)
			mpus.set_channel_id(u->cChannel->iId);
		if (u->bDeaf)
			mpus.set_deaf(true);
		else if (u->bMute)
			mpus.set_mute(true);
		if (u->bSuppress)
			mpus.set_suppress(true);
		if (u->bPrioritySpeaker)
			mpus.set_priority_speaker(true);
		if (u->bRecording)
			mpus.set_recording(true);
		if (u->bSelfDeaf)
			mpus.set_self_deaf(true);
		else if (u->bSelfMute)
			mpus.set_self_mute(true);
		if ((uSource->uiVersion >= 0x010202) && ! u->qbaCommentHash.isEmpty())
			mpus.set_comment_hash(blob(u->qbaCommentHash));
		else if (! u->qsComment.isEmpty())
			mpus.set_comment(u8(u->qsComment));
		if (! u->qsHash.isEmpty())
			mpus.set_hash(u8(u->qsHash));

		sendMessage(uSource, mpus);
	}

	// Send syncronisation packet
	MumbleProto::ServerSync mpss;
	mpss.set_session(uSource->uiSession);
	if (! qsWelcomeText.isEmpty())
		mpss.set_welcome_text(u8(qsWelcomeText));
	mpss.set_max_bandwidth(iMaxBandwidth);

	if (uSource->iId == 0) {
		mpss.set_permissions(ChanACL::All);
	} else {
		QMutexLocker qml(&qmCache);
		ChanACL::hasPermission(uSource, root, ChanACL::Enter, &acCache);
		mpss.set_permissions(acCache.value(uSource)->value(root));
	}

	sendMessage(uSource, mpss);

	MumbleProto::ServerConfig mpsc;
	mpsc.set_allow_html(bAllowHTML);
	mpsc.set_message_length(iMaxTextMessageLength);
	mpsc.set_image_message_length(iMaxImageMessageLength);
	sendMessage(uSource, mpsc);

	MumbleProto::SuggestConfig mpsug;
	if (! qvSuggestVersion.isNull())
		mpsug.set_version(qvSuggestVersion.toUInt());
	if (! qvSuggestPositional.isNull())
		mpsug.set_positional(qvSuggestPositional.toBool());
	if (! qvSuggestPushToTalk.isNull())
		mpsug.set_push_to_talk(qvSuggestPushToTalk.toBool());
	if (mpsug.ByteSize() > 0) {
		sendMessage(uSource, mpsug);
	}

	log(uSource, "Authenticated");

	emit userConnected(uSource);
}

void Server::msgBanList(ServerUser *uSource, MumbleProto::BanList &msg) {
	MSG_SETUP(ServerUser::Authenticated);

	if (! hasPermission(uSource, qhChannels.value(0), ChanACL::Ban)) {
		PERM_DENIED(uSource, qhChannels.value(0), ChanACL::Ban);
		return;
	}
	if (msg.query()) {
		msg.clear_query();
		msg.clear_bans();
		foreach(const Ban &b, qlBans) {
			MumbleProto::BanList_BanEntry *be = msg.add_bans();
			be->set_address(b.haAddress.toStdString());
			be->set_mask(b.iMask);
			be->set_name(u8(b.qsUsername));
			be->set_hash(u8(b.qsHash));
			be->set_reason(u8(b.qsReason));
			be->set_start(u8(b.qdtStart.toString(Qt::ISODate)));
			be->set_duration(b.iDuration);
		}
		sendMessage(uSource, msg);
	} else {
		qlBans.clear();
		for (int i=0;i < msg.bans_size(); ++i) {
			const MumbleProto::BanList_BanEntry &be = msg.bans(i);

			Ban b;
			b.haAddress = be.address();
			b.iMask = be.mask();
			b.qsUsername = u8(be.name());
			b.qsHash = u8(be.hash());
			b.qsReason = u8(be.reason());
			if (be.has_start()) {
				b.qdtStart = QDateTime::fromString(u8(be.start()), Qt::ISODate);
				b.qdtStart.setTimeSpec(Qt::UTC);
			} else {
				b.qdtStart = QDateTime::currentDateTime().toUTC();
			}
			b.iDuration = be.duration();
			if (b.isValid())
				qlBans << b;
		}
		saveBans();
		log(uSource, "Updated banlist");
	}
}

void Server::msgReject(ServerUser *, MumbleProto::Reject &) {
}

void Server::msgServerSync(ServerUser *, MumbleProto::ServerSync &) {
}

void Server::msgPermissionDenied(ServerUser *, MumbleProto::PermissionDenied &) {
}

void Server::msgUDPTunnel(ServerUser *uSource, MumbleProto::UDPTunnel &msg) {
	MSG_SETUP_NO_UNIDLE(ServerUser::Authenticated);

	const std::string &str = msg.packet();
	int len = static_cast<int>(str.length());
	if (len < 1)
		return;
	QReadLocker rl(&qrwlUsers);
	processMsg(uSource, str.data(), len);
}

void Server::msgUserState(ServerUser *uSource, MumbleProto::UserState &msg) {
	uSource->bwr.resetIdleSeconds();
	VICTIM_SETUP;

	Channel *root = qhChannels.value(0);

	/*
		First check all permissions involved
	*/
	if ((pDstServerUser->iId == 0) && (uSource->iId != 0)) {
		PERM_DENIED_TYPE(SuperUser);
		return;
	}

	msg.set_session(pDstServerUser->uiSession);
	msg.set_actor(uSource->uiSession);

	// team9000
	QString comment = u8(msg.comment());

	if(uSource->iId == -1337) {
		// auth server can do whatever it wants.
		msg.set_actor(pDstServerUser->uiSession);
	} else {
		if (msg.has_channel_id()) {
			Channel *c = qhChannels.value(msg.channel_id());
			if (!c || (c == pDstServerUser->cChannel))
				return;

			if (iMaxUsersPerChannel && (c->qlUsers.count() >= iMaxUsersPerChannel)) {
				PERM_DENIED_FALLBACK(ChannelFull, 0x010201, QLatin1String("Channel is full"));
				return;
			}
		}
		if (msg.has_mute() || msg.has_deaf() || msg.has_suppress() || msg.has_priority_speaker()) {
			// nobody has access to this
			PERM_DENIED_TYPE(SuperUser);
		}

		if (msg.has_comment()) {
			bool changed = false;

			if (uSource != pDstServerUser) {
				if (! hasPermission(uSource, root, ChanACL::Move)) {
					PERM_DENIED(uSource, root, ChanACL::Move);
					return;
				}
				if (comment.length() > 0) {
					PERM_DENIED_TYPE(TextTooLong);
					return;
				}
			}


			if (! isTextAllowed(comment, changed)) {
				PERM_DENIED_TYPE(TextTooLong);
				return;
			}
			if (changed)
				msg.set_comment(u8(comment));
		}

		if (msg.has_texture()) {
			if (iMaxImageMessageLength > 0 && (msg.texture().length() > static_cast<unsigned int>(iMaxImageMessageLength))) {
				PERM_DENIED_TYPE(TextTooLong);
				return;
			}
		}


		if (msg.has_user_id()) {
			ChanACL::Perm p = (uSource == pDstServerUser) ? ChanACL::SelfRegister : ChanACL::Register;
			if ((pDstServerUser->iId >= 0) || ! hasPermission(uSource, root, p)) {
				PERM_DENIED(uSource, root, p);
				return;
			}
			if (pDstServerUser->qsHash.isEmpty()) {
				PERM_DENIED_HASH(pDstServerUser);
				return;
			}
		}
	}

	// Prevent self-targeting state changes from being applied to others
	if ((pDstServerUser != uSource) && (msg.has_self_deaf() || msg.has_self_mute() || msg.has_texture() || msg.has_plugin_context() || msg.has_plugin_identity() || msg.has_recording()))
		return;

	// team9000
	if (msg.has_channel_id() && uSource->iId != -1337) {
		// ask gatekeeper for permission
		MumbleProto::UserState mpus;
		mpus.set_plugin_identity("1");
		mpus.set_session(msg.session());
		mpus.set_channel_id(msg.channel_id());
		foreach(ServerUser *u, qhUsers) {
			if(u->iId == -1337) sendMessage(u, mpus);
		}
		msg.clear_channel_id();
		// send along the rest of the message
	}

	/*
		-------------------- Permission checks done. Now act --------------------
	*/
	bool bBroadcast = false;

	if (msg.has_texture()) {
		QByteArray qba = blob(msg.texture());
		if (uSource->iId > 0) {
			// For registered users store the texture we just received in the database
			if (! setTexture(uSource->iId, qba))
				return;
		} else {
			// For unregistered users or SuperUser only get the hash
			hashAssign(uSource->qbaTexture, uSource->qbaTextureHash, qba);
		}

		// The texture will be sent out later in this function
		bBroadcast = true;
	}

	if (msg.has_self_deaf()) {
		uSource->bSelfDeaf = msg.self_deaf();
		if (uSource->bSelfDeaf)
			msg.set_self_mute(true);
		bBroadcast = true;
	}

	if (msg.has_self_mute()) {
		uSource->bSelfMute = msg.self_mute();
		if (! uSource->bSelfMute) {
			msg.set_self_deaf(false);
			uSource->bSelfDeaf = false;
		}
		bBroadcast = true;
	}

	if (msg.has_plugin_context()) {
		uSource->ssContext = msg.plugin_context();
		// Make sure to clear this from the packet so we don't broadcast it
		msg.clear_plugin_context();
	}

	if (msg.has_plugin_identity()) {
		uSource->qsIdentity = u8(msg.plugin_identity());
		// Make sure to clear this from the packet so we don't broadcast it
		msg.clear_plugin_identity();
	}

	if (! comment.isNull()) {
		hashAssign(pDstServerUser->qsComment, pDstServerUser->qbaCommentHash, comment);

		if (pDstServerUser->iId >= 0) {
			QMap<int, QString> info;
			info.insert(ServerDB::User_Comment, pDstServerUser->qsComment);
			setInfo(pDstServerUser->iId, info);
		}
		bBroadcast = true;
	}



	if (msg.has_mute() || msg.has_deaf() || msg.has_suppress() || msg.has_priority_speaker()) {
		if (msg.has_deaf()) {
			pDstServerUser->bDeaf = msg.deaf();
			if (pDstServerUser->bDeaf)
				msg.set_mute(true);
		}
		if (msg.has_mute()) {
			pDstServerUser->bMute = msg.mute();
			if (! pDstServerUser->bMute) {
				msg.set_deaf(false);
				pDstServerUser->bDeaf = false;
			}
		}
		if (msg.has_suppress())
			pDstServerUser->bSuppress = msg.suppress();

		if (msg.has_priority_speaker())
			pDstServerUser->bPrioritySpeaker = msg.priority_speaker();

		log(uSource, QString("Changed speak-state of %1 (%2 %3 %4 %5)").arg(QString(*pDstServerUser),
		        QString::number(pDstServerUser->bMute),
		        QString::number(pDstServerUser->bDeaf),
		        QString::number(pDstServerUser->bSuppress),
		        QString::number(pDstServerUser->bPrioritySpeaker)));

		bBroadcast = true;
	}

	if (msg.has_recording() && (pDstServerUser->bRecording != msg.recording())) {
		pDstServerUser->bRecording = msg.recording();

		MumbleProto::TextMessage mptm;
		mptm.add_tree_id(0);
		if (pDstServerUser->bRecording)
			mptm.set_message(u8(QString(QLatin1String("User '%1' started recording")).arg(pDstServerUser->qsName)));
		else
			mptm.set_message(u8(QString(QLatin1String("User '%1' stopped recording")).arg(pDstServerUser->qsName)));

		sendAll(mptm, ~ 0x010203);

		bBroadcast = true;
	}

	if (msg.has_channel_id()) {
		Channel *c = qhChannels.value(msg.channel_id());

		userEnterChannel(pDstServerUser, c, msg);
		log(uSource, QString("Moved %1 to %2").arg(QString(*pDstServerUser), QString(*c)));
		bBroadcast = true;
	}

	bool bDstAclChanged = false;
	if (msg.has_user_id()) {
		// Handle user (Self-)Registration
		QMap<int, QString> info;

		info.insert(ServerDB::User_Name, pDstServerUser->qsName);
		info.insert(ServerDB::User_Hash, pDstServerUser->qsHash);
		if (! pDstServerUser->qslEmail.isEmpty())
			info.insert(ServerDB::User_Email, pDstServerUser->qslEmail.first());
		int id = registerUser(info);
		if (id > 0) {
			pDstServerUser->iId = id;
			setLastChannel(pDstServerUser);
			msg.set_user_id(id);
			bDstAclChanged = true;
		} else {
			// Registration failed
			msg.clear_user_id();
		}
		bBroadcast = true;
	}

	if (bBroadcast && uSource->sState == ServerUser::Authenticated) {
		// Texture handling for clients < 1.2.2.
		// Send the texture data in the message.
		if (msg.has_texture() && (pDstServerUser->qbaTexture.length() >= 4) && (qFromBigEndian<unsigned int>(reinterpret_cast<const unsigned char *>(pDstServerUser->qbaTexture.constData())) != 600 * 60 * 4)) {
			// This is a new style texture, don't send it because the client doesn't handle it correctly / crashes.
			msg.clear_texture();
			sendAll(msg, ~ 0x010202);
			msg.set_texture(blob(pDstServerUser->qbaTexture));
		} else {
			// This is an old style texture, empty texture or there was no texture in this packet,
			// send the message unchanged.
			sendAll(msg, ~ 0x010202);
		}

		// Texture / comment handling for clients >= 1.2.2.
		// Send only a hash of the texture / comment text. The client will request the actual data if necessary.
		if (msg.has_texture() && ! pDstServerUser->qbaTextureHash.isEmpty()) {
			msg.clear_texture();
			msg.set_texture_hash(blob(pDstServerUser->qbaTextureHash));
		}
		if (msg.has_comment() && ! pDstServerUser->qbaCommentHash.isEmpty()) {
			msg.clear_comment();
			msg.set_comment_hash(blob(pDstServerUser->qbaCommentHash));
		}

		sendAll(msg, 0x010202);

		if (bDstAclChanged)
			clearACLCache(pDstServerUser);
	}

	emit userStateChanged(pDstServerUser);
}

void Server::msgUserRemove(ServerUser *uSource, MumbleProto::UserRemove &msg) {
	MSG_SETUP(ServerUser::Authenticated);
	VICTIM_SETUP;

	if(uSource->iId != -1337) return;

	msg.set_actor(uSource->uiSession);

	bool ban = msg.has_ban() && msg.ban();

	Channel *c = qhChannels.value(0);
	QFlags<ChanACL::Perm> perm = ban ? ChanACL::Ban : (ChanACL::Ban|ChanACL::Kick);

	if (ban) {
		Ban b;
		b.haAddress = pDstServerUser->haAddress;
		b.iMask = 128;
		b.qsReason = u8(msg.reason());
		b.qsUsername = pDstServerUser->qsName;
		b.qsHash = pDstServerUser->qsHash;
		b.qdtStart = QDateTime::currentDateTime().toUTC();
		b.iDuration = 0;
		qlBans << b;
		saveBans();
	}

	sendAll(msg);
	if (ban)
		log(uSource, QString("Kickbanned %1 (%2)").arg(QString(*pDstServerUser), u8(msg.reason())));
	else
		log(uSource, QString("Kicked %1 (%2)").arg(QString(*pDstServerUser), u8(msg.reason())));
	pDstServerUser->disconnectSocket();
}

void Server::msgChannelState(ServerUser *uSource, MumbleProto::ChannelState &msg) {
	MSG_SETUP(ServerUser::Authenticated);

	if(uSource->iId != -1337) return;

	Channel *c = NULL;
	Channel *p = NULL;

	// If this message relates to an existing channel check if the id is really valid
	if (msg.has_channel_id()) {
		c = qhChannels.value(msg.channel_id());
		if (! c)
			return;
	}

	// Check if the parent exists
	if (msg.has_parent()) {
		p = qhChannels.value(msg.parent());
		if (! p)
			return;
	}

	msg.clear_links();

	QString qsName;
	if (msg.has_name()) {
		qsName = u8(msg.name());
	}

	QString qsDesc;
	if (msg.has_description()) {
		qsDesc = u8(msg.description());
		bool changed = false;
		if (changed)
			msg.set_description(u8(qsDesc));
	}

	if (! c) {
		// If we don't have a channel handle up to now we want to create a new channel
		// so check if the user has enough rights and we got everything we need.
		if (! p || qsName.isNull())
			return;

		c = addChannel(p, qsName, msg.temporary(), msg.position());
		hashAssign(c->qsDesc, c->qbaDescHash, qsDesc);

		updateChannel(c);

		msg.set_channel_id(c->iId);
		log(uSource, QString("Added channel %1 under %2").arg(QString(*c), QString(*p)));
		emit channelCreated(c);

		sendAll(msg, ~ 0x010202);
		if (! c->qbaDescHash.isEmpty()) {
			msg.clear_description();
			msg.set_description_hash(blob(c->qbaDescHash));
		}
		sendAll(msg, 0x010202);
	} else {
		if (p) {
			// If we received a parent channel check if it differs from the old one and is not
			// Temporary. If that is the case check if the user has enough rights and if the
			// channel name is not used in the target location. Abort otherwise.
			if (p == c->cParent)
				return;

			Channel *ip = p;
			while (ip) {
				if (ip == c)
					return;
				ip = ip->cParent;
			}

			QString name = qsName.isNull() ? c->qsName : qsName;
		}
		QList<Channel *> qlAdd;
		QList<Channel *> qlRemove;

		if (msg.links_add_size() || msg.links_remove_size()) {
			if (msg.links_remove_size()) {
				for (int i=0;i < msg.links_remove_size(); ++i) {
					unsigned int link = msg.links_remove(i);
					Channel *l = qhChannels.value(link);
					if (! l)
						return;
					qlRemove << l;
				}
			}
			if (msg.links_add_size()) {
				for (int i=0;i < msg.links_add_size(); ++i) {
					unsigned int link = msg.links_add(i);
					Channel *l = qhChannels.value(link);
					if (! l)
						return;
					qlAdd << l;
				}
			}
		}

		// All permission checks done -- the update is good.

		if (p) {
			log(uSource, QString("Moved channel %1 from %2 to %3").arg(QString(*c),
			        QString(* c->cParent),
			        QString(*p)));

			c->cParent->removeChannel(c);
			p->addChannel(c);
		}
		if (! qsName.isNull()) {
			log(uSource, QString("Renamed channel %1 to %2").arg(QString(*c),
			        QString(qsName)));
			c->qsName = qsName;
		}
		if (! qsDesc.isNull())
			hashAssign(c->qsDesc, c->qbaDescHash, qsDesc);

		if (msg.has_position())
			c->iPosition = msg.position();

		foreach(Channel *l, qlAdd) {
			addLink(c, l);
		}
		foreach(Channel *l, qlRemove) {
			removeLink(c, l);
		}

		updateChannel(c);
		emit channelStateChanged(c);

		sendAll(msg, ~ 0x010202);
		if (msg.has_description() && ! c->qbaDescHash.isEmpty()) {
			msg.clear_description();
			msg.set_description_hash(blob(c->qbaDescHash));
		}
		sendAll(msg, 0x010202);
	}
}

void Server::msgChannelRemove(ServerUser *uSource, MumbleProto::ChannelRemove &msg) {
	MSG_SETUP(ServerUser::Authenticated);

	if(uSource->iId != -1337) return;

	Channel *c = qhChannels.value(msg.channel_id());
	if (!c)
		return;

	log(uSource, QString("Removed channel %1").arg(*c));

	removeChannel(c);
}

void Server::msgTextMessage(ServerUser *uSource, MumbleProto::TextMessage &msg) {
	log(uSource, QString("Received text message: %1").arg(u8(msg.message())));
	log(uSource, QString("ACTOR %1").arg(msg.actor()));
	for (int i=0;i<msg.channel_id_size(); ++i) {
		log(uSource, QString("CHANNEL %1").arg(msg.channel_id(i)));
	}
	for (int i=0;i<msg.tree_id_size(); ++i) {
		log(uSource, QString("TREE %1").arg(msg.tree_id(i)));
	}
	for (int i=0;i<msg.session_size(); ++i) {
		log(uSource, QString("SESSION %1").arg(msg.session(i)));
	}

	MSG_SETUP(ServerUser::Authenticated);
	QMutexLocker qml(&qmCache);

	TextMessage tm; // for signal userTextMessage

	QSet<ServerUser *> users;
	QQueue<Channel *> q;

	QString text = u8(msg.message());
	bool changed = false;

	if (! isTextAllowed(text, changed)) {
		PERM_DENIED_TYPE(TextTooLong);
		return;
	}
	if (text.isEmpty())
		return;
	if (changed)
		msg.set_message(u8(text));

	tm.qsText = text;

	// team9000
	if(uSource->iId != -1337) {
		// received from a user
		// forward to chat server
		msg.set_actor(uSource->uiSession);
		foreach(ServerUser *u, qhUsers) {
			if(u->iId == -1337)
				sendMessage(u, msg);
		}

		// remove eventually
		log(uSource, QString("Emitting text message: %1").arg(tm.qsText));
		emit userTextMessage(uSource, tm);
	} else {
		// received from chat server
		// forward to users
		for (int i=0;i < msg.session_size(); ++i) {
			unsigned int session = msg.session(i);
			ServerUser *u = qhUsers.value(session);
			if (u && u->iId != -1337) {
				MumbleProto::TextMessage mptm;
				if(msg.has_actor()) {
					mptm.set_actor(msg.actor());
				}
				mptm.set_message(u8(text));
				mptm.add_session(u->uiSession);
				sendMessage(u, mptm);
			}
		}
	}
}

void Server::msgACL(ServerUser *uSource, MumbleProto::ACL &msg) {
}

void Server::msgQueryUsers(ServerUser *uSource, MumbleProto::QueryUsers &msg) {
	MSG_SETUP(ServerUser::Authenticated);

	MumbleProto::QueryUsers reply;

	for (int i=0;i<msg.ids_size();++i) {
		int id = msg.ids(i);
		if (id >= 0) {
			const QString &name = getUserName(id);
			if (! name.isEmpty()) {
				reply.add_ids(id);
				reply.add_names(u8(name));
			}
		}
	}

	for (int i=0;i<msg.names_size();++i) {
		QString name = u8(msg.names(i));
		int id = getUserID(name);
		if (id >= 0) {
			name = getUserName(id);
			reply.add_ids(id);
			reply.add_names(u8(name));
		}
	}

	sendMessage(uSource, reply);
}

void Server::msgPing(ServerUser *uSource, MumbleProto::Ping &msg) {
	MSG_SETUP_NO_UNIDLE(ServerUser::Authenticated);
	CryptState &cs=uSource->csCrypt;

	cs.uiRemoteGood = msg.good();
	cs.uiRemoteLate = msg.late();
	cs.uiRemoteLost = msg.lost();
	cs.uiRemoteResync = msg.resync();

	uSource->dUDPPingAvg = msg.udp_ping_avg();
	uSource->dUDPPingVar = msg.udp_ping_var();
	uSource->uiUDPPackets = msg.udp_packets();
	uSource->dTCPPingAvg = msg.tcp_ping_avg();
	uSource->dTCPPingVar = msg.tcp_ping_var();
	uSource->uiTCPPackets = msg.tcp_packets();

	quint64 ts = msg.timestamp();

	msg.Clear();
	msg.set_timestamp(ts);
	msg.set_good(cs.uiGood);
	msg.set_late(cs.uiLate);
	msg.set_lost(cs.uiLost);
	msg.set_resync(cs.uiResync);

	sendMessage(uSource, msg);
}

void Server::msgCryptSetup(ServerUser *uSource, MumbleProto::CryptSetup &msg) {
	MSG_SETUP_NO_UNIDLE(ServerUser::Authenticated);
	if (! msg.has_client_nonce()) {
		log(uSource, "Requested crypt-nonce resync");
		msg.set_server_nonce(std::string(reinterpret_cast<const char *>(uSource->csCrypt.encrypt_iv), AES_BLOCK_SIZE));
		sendMessage(uSource, msg);
	} else {
		const std::string &str = msg.client_nonce();
		if (str.size()  == AES_BLOCK_SIZE) {
			uSource->csCrypt.uiResync++;
			memcpy(uSource->csCrypt.decrypt_iv, str.data(), AES_BLOCK_SIZE);
		}
	}
}

void Server::msgContextActionModify(ServerUser *, MumbleProto::ContextActionModify &) {
}

void Server::msgContextAction(ServerUser *uSource, MumbleProto::ContextAction &msg) {
	MSG_SETUP(ServerUser::Authenticated);

	unsigned int session = msg.has_session() ? msg.session() : 0;
	int id = msg.has_channel_id() ? static_cast<int>(msg.channel_id()) : -1;

	if (session && ! qhUsers.contains(session))
		return;
	if ((id >= 0) && ! qhChannels.contains(id))
		return;
	emit contextAction(uSource, u8(msg.action()), session, id);
}

void Server::msgVersion(ServerUser *uSource, MumbleProto::Version &msg) {
	if (msg.has_version())
		uSource->uiVersion=msg.version();
	if (msg.has_release())
		uSource->qsRelease = u8(msg.release());
	if (msg.has_os()) {
		uSource->qsOS = u8(msg.os());
		if (msg.has_os_version())
			uSource->qsOSVersion = u8(msg.os_version());
	}

	log(uSource, QString("Client version %1 (%2: %3)").arg(MumbleVersion::toString(uSource->uiVersion)).arg(uSource->qsOS).arg(uSource->qsRelease));
}

void Server::msgUserList(ServerUser *uSource, MumbleProto::UserList &msg) {
	MSG_SETUP(ServerUser::Authenticated);

	if (! hasPermission(uSource, qhChannels.value(0), ChanACL::Register)) {
		PERM_DENIED(uSource, qhChannels.value(0), ChanACL::Register);
		return;
	}

	if (msg.users_size() == 0) {
		// Query mode.
		QMap<int, QString> users = getRegisteredUsers();
		QMap<int, QString>::const_iterator i;
		for (i = users.constBegin(); i != users.constEnd(); ++i) {
			if (i.key() > 0) {
				::MumbleProto::UserList_User *u = msg.add_users();
				u->set_user_id(i.key());
				u->set_name(u8(i.value()));
			}
		}
		sendMessage(uSource, msg);
	} else {
		for (int i=0;i < msg.users_size(); ++i) {
			const MumbleProto::UserList_User &u = msg.users(i);

			int id = u.user_id();
			if (id == 0)
				continue;

			if (! u.has_name()) {
				log(uSource, QString::fromLatin1("Unregistered user %1").arg(id));
				unregisterUser(id);
			} else {
				const QString &name = u8(u.name());
				if (validateUserName(name)) {
					log(uSource, QString::fromLatin1("Renamed user %1 to '%2'").arg(QString::number(id), name));

					QMap<int, QString> info;
					info.insert(ServerDB::User_Name, name);
					setInfo(id, info);
				} else {
					MumbleProto::PermissionDenied mppd;
					mppd.set_type(MumbleProto::PermissionDenied_DenyType_UserName);
					if (uSource->uiVersion < 0x010201)
						mppd.set_reason(u8(QString::fromLatin1("%1 is not a valid username").arg(name)));
					else
						mppd.set_name(u8(name));
					sendMessage(uSource, mppd);
				}
			}
		}
	}
}

void Server::msgVoiceTarget(ServerUser *uSource, MumbleProto::VoiceTarget &msg) {
	MSG_SETUP_NO_UNIDLE(ServerUser::Authenticated);

	// team9000
	return;

	int target = msg.id();
	if ((target < 1) || (target >= 0x1f))
		return;

	QWriteLocker lock(&qrwlUsers);

	uSource->qmTargetCache.remove(target);

	int count = msg.targets_size();
	if (count == 0) {
		uSource->qmTargets.remove(target);
	} else {
		WhisperTarget wt;
		for (int i=0;i<count;++i) {
			const MumbleProto::VoiceTarget_Target &t = msg.targets(i);
			for (int j=0;j<t.session_size(); ++j) {
				unsigned int s = t.session(j);
				if (qhUsers.contains(s))
					wt.qlSessions << s;
			}
			if (t.has_channel_id()) {
				unsigned int id = t.channel_id();
				if (qhChannels.contains(id)) {
					WhisperTarget::Channel wtc;
					wtc.iId = id;
					wtc.bChildren = t.children();
					wtc.bLinks = t.links();
					if (t.has_group())
						wtc.qsGroup = u8(t.group());
					wt.qlChannels << wtc;
				}
			}
		}
		if (wt.qlSessions.isEmpty() && wt.qlChannels.isEmpty())
			uSource->qmTargets.remove(target);
		else
			uSource->qmTargets.insert(target, wt);
	}
}

void Server::msgPermissionQuery(ServerUser *uSource, MumbleProto::PermissionQuery &msg) {
	MSG_SETUP_NO_UNIDLE(ServerUser::Authenticated);

	Channel *c = qhChannels.value(msg.channel_id());
	if (!c)
		return;

	sendClientPermission(uSource, c, true);
}

void Server::msgCodecVersion(ServerUser *, MumbleProto::CodecVersion &) {
}

void Server::msgUserStats(ServerUser*uSource, MumbleProto::UserStats &msg) {
	MSG_SETUP_NO_UNIDLE(ServerUser::Authenticated);
	VICTIM_SETUP;
	const CryptState &cs = pDstServerUser->csCrypt;
	const BandwidthRecord &bwr = pDstServerUser->bwr;
	const QList<QSslCertificate> &certs = pDstServerUser->peerCertificateChain();

	bool extend = (uSource == pDstServerUser) || hasPermission(uSource, qhChannels.value(0), ChanACL::Register);

	if (! extend && ! hasPermission(uSource, pDstServerUser->cChannel, ChanACL::Enter)) {
		PERM_DENIED(uSource, pDstServerUser->cChannel, ChanACL::Enter);
		return;
	}

	bool details = extend;
	bool local = extend || (pDstServerUser->cChannel == uSource->cChannel);

	if (msg.stats_only())
		details = false;

	msg.Clear();
	msg.set_session(pDstServerUser->uiSession);

	if (details) {
		foreach(const QSslCertificate &cert, certs) {
			const QByteArray &der = cert.toDer();
			msg.add_certificates(blob(der));
		}
		msg.set_strong_certificate(pDstServerUser->bVerified);
	}

	if (local) {
		MumbleProto::UserStats_Stats *mpusss;

		mpusss = msg.mutable_from_client();
		mpusss->set_good(cs.uiGood);
		mpusss->set_late(cs.uiLate);
		mpusss->set_lost(cs.uiLost);
		mpusss->set_resync(cs.uiResync);

		mpusss = msg.mutable_from_server();
		mpusss->set_good(cs.uiRemoteGood);
		mpusss->set_late(cs.uiRemoteLate);
		mpusss->set_lost(cs.uiRemoteLost);
		mpusss->set_resync(cs.uiRemoteResync);
	}

	msg.set_udp_packets(pDstServerUser->uiUDPPackets);
	msg.set_tcp_packets(pDstServerUser->uiTCPPackets);
	msg.set_udp_ping_avg(pDstServerUser->dUDPPingAvg);
	msg.set_udp_ping_var(pDstServerUser->dUDPPingVar);
	msg.set_tcp_ping_avg(pDstServerUser->dTCPPingAvg);
	msg.set_tcp_ping_var(pDstServerUser->dTCPPingVar);

	if (details) {
		MumbleProto::Version *mpv;

		mpv = msg.mutable_version();
		if (pDstServerUser->uiVersion)
			mpv->set_version(pDstServerUser->uiVersion);
		if (! pDstServerUser->qsRelease.isEmpty())
			mpv->set_release(u8(pDstServerUser->qsRelease));
		if (! pDstServerUser->qsOS.isEmpty()) {
			mpv->set_os(u8(pDstServerUser->qsOS));
			if (! pDstServerUser->qsOSVersion.isEmpty())
				mpv->set_os_version(u8(pDstServerUser->qsOSVersion));
		}

		foreach(int v, pDstServerUser->qlCodecs)
			msg.add_celt_versions(v);
		msg.set_opus(pDstServerUser->bOpus);

		msg.set_address(pDstServerUser->haAddress.toStdString());
	}

	if (local)
		msg.set_bandwidth(bwr.bandwidth());
	msg.set_onlinesecs(bwr.onlineSeconds());
	if (local)
		msg.set_idlesecs(bwr.idleSeconds());

	sendMessage(uSource, msg);
}

void Server::msgRequestBlob(ServerUser *uSource, MumbleProto::RequestBlob &msg) {
	MSG_SETUP_NO_UNIDLE(ServerUser::Authenticated);

	int ntextures = msg.session_texture_size();
	int ncomments = msg.session_comment_size();
	int ndescriptions = msg.channel_description_size();

	if (ndescriptions) {
		MumbleProto::ChannelState mpcs;
		for (int i=0;i<ndescriptions;++i) {
			int id = msg.channel_description(i);
			Channel *c = qhChannels.value(id);
			if (c && ! c->qsDesc.isEmpty()) {
				mpcs.set_channel_id(id);
				mpcs.set_description(u8(c->qsDesc));
				sendMessage(uSource, mpcs);
			}
		}
	}
	if (ntextures || ncomments) {
		MumbleProto::UserState mpus;
		for (int i=0;i<ntextures;++i) {
			int session = msg.session_texture(i);
			ServerUser *su = qhUsers.value(session);
			if (su && ! su->qbaTexture.isEmpty()) {
				mpus.set_session(session);
				mpus.set_texture(blob(su->qbaTexture));
				sendMessage(uSource, mpus);
			}
		}
		if (ntextures)
			mpus.clear_texture();
		for (int i=0;i<ncomments;++i) {
			int session = msg.session_comment(i);
			ServerUser *su = qhUsers.value(session);
			if (su && ! su->qsComment.isEmpty()) {
				mpus.set_session(session);
				mpus.set_comment(u8(su->qsComment));
				sendMessage(uSource, mpus);
			}
		}
	}
}

void Server::msgServerConfig(ServerUser *, MumbleProto::ServerConfig &) {
}

void Server::msgSuggestConfig(ServerUser *, MumbleProto::SuggestConfig &) {
}
