/*
  This file is part of the kblog library.

  SPDX-FileCopyrightText: 2007-2009 Christian Weilbach <christian_weilbach@web.de>
  SPDX-FileCopyrightText: 2007 Mike McQuaid <mike@mikemcquaid.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MOVABLETYPE_P_H
#define MOVABLETYPE_P_H

#include "movabletype.h"
#include "metaweblog_p.h"

#include <kxmlrpcclient/client.h>
class KJob;
class QByteArray;

namespace KIO
{
class Job;
}

namespace KBlog
{

class MovableTypePrivate : public MetaWeblogPrivate
{
public:
    QMap<KJob *, QByteArray> mSetPostCategoriesBuffer;
    QMap<KJob *, QString> mSetPostCategoriesMap;
    MovableTypePrivate();
    virtual ~MovableTypePrivate();
    virtual void slotListTrackBackPings(const QList<QVariant> &result,
                                        const QVariant &id);
    void slotCreatePost(const QList<QVariant> &, const QVariant &) override;
    void slotFetchPost(const QList<QVariant> &, const QVariant &) override;
    void slotModifyPost(const QList<QVariant> &, const QVariant &) override;
    void slotSetPostCategories(const QList<QVariant> &, const QVariant &);
    void slotGetPostCategories(const QList<QVariant> &, const QVariant &);
    void slotTriggerCreatePost();
    void slotTriggerModifyPost();
    void slotTriggerFetchPost();
    Q_DECLARE_PUBLIC(MovableType)

    QList<QVariant> defaultArgs(const QString &id = QString()) override;
    virtual void setPostCategories(BlogPost *post, bool publishAfterCategories);
    bool readPostFromMap(BlogPost *post, const QMap<QString, QVariant> &postInfo) override;
    bool readArgsFromPost(QList<QVariant> *args, const BlogPost &post) override;
    QMap<int, bool> mPublishAfterCategories;
    QList<BlogPost *> mCreatePostCache;
    QList<BlogPost *> mModifyPostCache;
    QList<BlogPost *> mFetchPostCache;
    QList<BlogPost *> mSilentCreationList;
};

}

#endif
