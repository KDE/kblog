/*
  This file is part of the kblog library.

  SPDX-FileCopyrightText: 2007 Christian Weilbach <christian_weilbach@web.de>
  SPDX-FileCopyrightText: 2007 Mike McQuaid <mike@mikemcquaid.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef BLOGPOST_P_H
#define BLOGPOST_P_H

#include "blogpost.h"

#include <QStringList>
#include <QDateTime>
#include <QUrl>

namespace KBlog
{

class BlogPostPrivate
{
public:
    bool mPrivate;
    BlogPost *q_ptr;
    QString mPostId;
    QString mTitle;
    QString mContent;
    QString mAdditionalContent;
    QString mWpSlug;
    QStringList mCategories;
    QString mError;
    QString mJournalId;
    QString mSummary;
    QStringList mTags;
    QString mMood;
    QString mMusic;
    bool mCommentAllowed;
    bool mTrackBackAllowed;
    QUrl mLink, mPermaLink;
    BlogPost::Status mStatus;
    QDateTime mCreationDateTime;
    QDateTime mModificationDateTime;
    QString cleanRichText(QString richText) const;
};

} // namespace
#endif
