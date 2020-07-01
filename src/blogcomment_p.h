/*
  This file is part of the kblog library.

  SPDX-FileCopyrightText: 2006-2007 Christian Weilbach <christian_weilbach@web.de>
  SPDX-FileCopyrightText: 2007 Mike McQuaid <mike@mikemcquaid.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef BLOGCOMMENT_P_H
#define BLOGCOMMENT_P_H

#include "blogcomment.h"

#include <QDateTime>
#include <QUrl>

namespace KBlog
{

class BlogCommentPrivate
{
public:
    BlogComment *q_ptr;
    QString mTitle;
    QString mContent;
    QString mEmail;
    QString mName;
    QString mCommentId;
    QUrl mUrl;
    QString mError;
    BlogComment::Status mStatus;
    QDateTime mModificationDateTime;
    QDateTime mCreationDateTime;
};

} // namespace KBlog
#endif
