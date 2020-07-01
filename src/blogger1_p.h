/*
  This file is part of the kblog library.

  SPDX-FileCopyrightText: 2006-2007 Christian Weilbach <christian_weilbach@web.de>
  SPDX-FileCopyrightText: 2007 Mike McQuaid <mike@mikemcquaid.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef BLOGGER1_P_H
#define BLOGGER1_P_H

#include "blogger1.h"
#include "blog_p.h"

#include <kxmlrpcclient/client.h>

#include <QList>

namespace KBlog
{

class Blogger1Private : public BlogPrivate
{
public:
    QString mAppId;
    KXmlRpc::Client *mXmlRpcClient;
    unsigned int mCallCounter; // TODO a better counter
    QMap<unsigned int, KBlog::BlogPost *> mCallMap;
    Blogger1Private();
    virtual ~Blogger1Private();

    virtual void slotFetchUserInfo(const QList<QVariant> &result, const QVariant &id);
    virtual void slotListBlogs(const QList<QVariant> &result, const QVariant &id);
    virtual void slotListRecentPosts(const QList<QVariant> &result, const QVariant &id);
    virtual void slotFetchPost(const QList<QVariant> &result, const QVariant &id);
    virtual void slotCreatePost(const QList<QVariant> &result, const QVariant &id);
    virtual void slotModifyPost(const QList<QVariant> &result, const QVariant &id);
    virtual void slotRemovePost(const QList<QVariant> &result, const QVariant &id);
    virtual void slotError(int number, const QString &errorString, const QVariant &id);

    Q_DECLARE_PUBLIC(KBlog::Blogger1)

    enum FunctionToCall {
        GetRecentPosts,
        CreatePost,
        ModifyPost,
        FetchPost
    };
    virtual QList<QVariant> defaultArgs(const QString &id = QString());
    QList<QVariant> blogger1Args(const QString &id = QString());
    virtual bool readPostFromMap(BlogPost *post,
                                 const QMap<QString, QVariant> &postInfo);
    virtual bool readArgsFromPost(QList<QVariant> *args, const BlogPost &post);
    virtual QString getCallFromFunction(FunctionToCall type);
};

}

#endif
