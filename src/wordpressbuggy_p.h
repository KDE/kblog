/*
  This file is part of the kblog library.

  SPDX-FileCopyrightText: 2007-2009 Christian Weilbach <christian_weilbach@web.de>
  SPDX-FileCopyrightText: 2007 Mike McQuaid <mike@mikemcquaid.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef WORDPRESSBUGGY_P_H
#define WORDPRESSBUGGY_P_H

#include "wordpressbuggy.h"
#include "movabletype_p.h"

#include <kxmlrpcclient/client.h>

class KJob;
template <class T, class S>class QMap;

namespace KIO
{
class Job;
}

namespace KBlog
{

class WordpressBuggyPrivate : public MovableTypePrivate
{
public:
    QMap<KJob *, KBlog::BlogPost *> mCreatePostMap;
    QMap<KJob *, KBlog::BlogPost *> mModifyPostMap;
    WordpressBuggyPrivate();
    virtual ~WordpressBuggyPrivate();
    QList<QVariant> defaultArgs(const QString &id = QString()) override;

    //adding these two lines prevents the symbols from MovableTypePrivate
    //to be hidden by the symbols below that.
    using MovableTypePrivate::slotCreatePost;
    using MovableTypePrivate::slotModifyPost;
    virtual void slotCreatePost(KJob *);
    virtual void slotModifyPost(KJob *);
    Q_DECLARE_PUBLIC(WordpressBuggy)
};

}

#endif
