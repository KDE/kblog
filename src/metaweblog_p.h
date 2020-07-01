/*
    This file is part of the kblog library.

    SPDX-FileCopyrightText: 2006-2007 Christian Weilbach <christian_weilbach@web.de>
    SPDX-FileCopyrightText: 2007 Mike McQuaid <mike@mikemcquaid.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef METAWEBLOG_P_H
#define METAWEBLOG_P_H

#include "metaweblog.h"
#include "blogger1_p.h"

#include <kxmlrpcclient/client.h>

namespace KBlog
{

class MetaWeblogPrivate : public Blogger1Private
{
public:
    QMap<QString, QString> mCategories;
    QList<QMap<QString, QString> > mCategoriesList;
    unsigned int mCallMediaCounter;
    QMap<unsigned int, KBlog::BlogMedia *> mCallMediaMap;
    MetaWeblogPrivate();
    ~MetaWeblogPrivate();
    virtual void loadCategories();
    virtual void saveCategories();
    virtual void slotListCategories(const QList<QVariant> &result,
                                    const QVariant &id);
    virtual void slotCreateMedia(const QList<QVariant> &result,
                                 const QVariant &id);
    Q_DECLARE_PUBLIC(MetaWeblog)

    QList<QVariant> defaultArgs(const QString &id = QString()) override;
    bool readPostFromMap(BlogPost *post, const QMap<QString, QVariant> &postInfo) override;
    bool readArgsFromPost(QList<QVariant> *args, const BlogPost &post) override;
    QString getCallFromFunction(FunctionToCall type) override;
    bool mCatLoaded;
};

}
#endif
