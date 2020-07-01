/*
  This file is part of the kblog library.

  SPDX-FileCopyrightText: 2007 Christian Weilbach <christian_weilbach@web.de>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KBLOG_GDATA_P_H
#define KBLOG_GDATA_P_H

#include "gdata.h"
#include "blog_p.h"

#include <syndication/loader.h>

class KJob;
class QDateTime;
template <class T, class S>class QMap;

namespace KIO
{
class Job;
}

namespace KBlog
{

class GDataPrivate : public BlogPrivate
{
public:
    QString mAuthenticationString;
    QDateTime mAuthenticationTime;
    QMap<KJob *, KBlog::BlogPost *> mCreatePostMap;
    QMap<KJob *, QMap<KBlog::BlogPost *, KBlog::BlogComment *> > mCreateCommentMap;
    QMap<KJob *, QMap<KBlog::BlogPost *, KBlog::BlogComment *> > mRemoveCommentMap;
    QMap<KJob *, KBlog::BlogPost *> mModifyPostMap;
    QMap<KJob *, KBlog::BlogPost *> mRemovePostMap;
    QMap<Syndication::Loader *, KBlog::BlogPost *> mFetchPostMap;
    QMap<Syndication::Loader *, KBlog::BlogPost *> mListCommentsMap;
    QMap<Syndication::Loader *, int> mListRecentPostsMap;
    QString mFullName;
    QString mProfileId;
    GDataPrivate();
    ~GDataPrivate();
    bool authenticate();
    virtual void slotFetchProfileId(KJob *);
    virtual void slotListBlogs(Syndication::Loader *,
                               const Syndication::FeedPtr &, Syndication::ErrorCode);
    virtual void slotListComments(Syndication::Loader *,
                                  const Syndication::FeedPtr &, Syndication::ErrorCode);
    virtual void slotListAllComments(Syndication::Loader *,
                                     const Syndication::FeedPtr &, Syndication::ErrorCode);
    virtual void slotListRecentPosts(Syndication::Loader *,
                                     const Syndication::FeedPtr &, Syndication::ErrorCode);
    virtual void slotFetchPost(Syndication::Loader *,
                               const Syndication::FeedPtr &, Syndication::ErrorCode);
    virtual void slotCreatePost(KJob *);
    virtual void slotModifyPost(KJob *);
    virtual void slotRemovePost(KJob *);
    virtual void slotCreateComment(KJob *);
    virtual void slotRemoveComment(KJob *);
    Q_DECLARE_PUBLIC(GData)
};

}
#endif
