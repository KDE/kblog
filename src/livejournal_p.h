/*
  This file is part of the kblog library.

  SPDX-FileCopyrightText: 2007 Mike McQuaid <mike@mikemcquaid.com>
  SPDX-FileCopyrightText: 2007 Christian Weilbach <christian_weilbach@web.de>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LIVEJOURNAL_P_H
#define LIVEJOURNAL_P_H

#include "livejournal.h"
#include "blog_p.h"

#include <kxmlrpcclient/client.h>

namespace KBlog
{

class LiveJournalPrivate : public BlogPrivate
{
public:
    QString mAppId;
    QMap<QString, QString> mCategories;
    KXmlRpc::Client *mXmlRpcClient;
    QMap<unsigned int, KBlog::BlogPost *> mCallMap;
    QMap<unsigned int, QString> mCallMapAddFriend;
    unsigned int mCallCounter;
    QString mServerMessage;
    QString mUserId;
    QString mFullName;

    LiveJournalPrivate();
    virtual ~LiveJournalPrivate();

    enum GenerateCookieOption {
        LongExpiriation = 0x01,
        FixedIP = 0x02
    };
    Q_DECLARE_FLAGS(GenerateCookieOptions, GenerateCookieOption)

    virtual void generateCookie(const GenerateCookieOptions &options);

    virtual void expireCookie(const QString &cookie, bool expireAll);

    virtual QMap<QString, QVariant> defaultArgs();

    virtual void slotAddFriend(const QList<QVariant> &result,
                               const QVariant &id);
    virtual void slotAssignFriendToCategory(const QList<QVariant> &result,
                                            const QVariant &id);
    virtual void slotCreatePost(const QList<QVariant> &result,
                                const QVariant &id);
    virtual void slotDeleteFriend(const QList<QVariant> &result,
                                  const QVariant &id);
//     virtual void slotExpireCookie( const QList<QVariant> &result,
//                                    const QVariant &id );
    virtual void slotError(int, const QString &, const QVariant &);
    virtual void slotFetchPost(const QList<QVariant> &result,
                               const QVariant &id);
    virtual void slotFetchUserInfo(const QList<QVariant> &result,
                                   const QVariant &id);
//     virtual void slotGenerateCookie( const QList<QVariant> &result,
//                                      const QVariant &id );
    virtual void slotListCategories(const QList<QVariant> &result,
                                    const QVariant &id);
    virtual void slotListFriends(const QList<QVariant> &result,
                                 const QVariant &id);
    virtual void slotListFriendsOf(const QList<QVariant> &result,
                                   const QVariant &id);
    virtual void slotListMoods(const QList<QVariant> &result,
                               const QVariant &id);
    virtual void slotListPictureKeywords(const QList<QVariant> &result,
                                         const QVariant &id);
    virtual void slotListRecentPosts(const QList<QVariant> &result,
                                     const QVariant &id);
    virtual void slotModifyPost(const QList<QVariant> &result,
                                const QVariant &id);
    virtual void slotRemovePost(const QList<QVariant> &result,
                                const QVariant &id);
    Q_DECLARE_PUBLIC(LiveJournal)

private:
    bool readPostFromMap(BlogPost *post, const QMap<QString, QVariant> &postInfo);
};

}

#endif
