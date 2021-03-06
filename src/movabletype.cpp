/*
  This file is part of the kblog library.

  SPDX-FileCopyrightText: 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2006-2009 Christian Weilbach <christian_weilbach@web.de>
  SPDX-FileCopyrightText: 2007-2008 Mike McQuaid <mike@mikemcquaid.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "movabletype.h"
#include "movabletype_p.h"
#include "blogpost.h"

#include <kxmlrpcclient/client.h>
#include <kio/job.h>

#include "kblog_debug.h"
#include <KLocalizedString>

#include <QStringList>

using namespace KBlog;

MovableType::MovableType(const QUrl &server, QObject *parent)
    : MetaWeblog(server, *new MovableTypePrivate, parent)
{
    qCDebug(KBLOG_LOG);
}

MovableType::MovableType(const QUrl &server, MovableTypePrivate &dd,
                         QObject *parent)
    : MetaWeblog(server, dd, parent)
{
    qCDebug(KBLOG_LOG);
}

MovableType::~MovableType()
{
    qCDebug(KBLOG_LOG);
}

QString MovableType::interfaceName() const
{
    return QStringLiteral("Movable Type");
}

void MovableType::listRecentPosts(int number)
{
    Q_D(MovableType);
    qCDebug(KBLOG_LOG);
    QList<QVariant> args(d->defaultArgs(blogId()));
    args << QVariant(number);
    d->mXmlRpcClient->call(
        QStringLiteral("metaWeblog.getRecentPosts"), args,
        this, SLOT(slotListRecentPosts(QList<QVariant>,QVariant)),
        this, SLOT(slotError(int,QString,QVariant)),
        QVariant(number));
}

void MovableType::listTrackBackPings(KBlog::BlogPost *post)
{
    Q_D(MovableType);
    qCDebug(KBLOG_LOG);
    QList<QVariant> args;
    args << QVariant(post->postId());
    unsigned int i = d->mCallCounter++;
    d->mCallMap[ i ] = post;
    d->mXmlRpcClient->call(
        QStringLiteral("mt.getTrackbackPings"), args,
        this, SLOT(slotListTrackbackPings(QList<QVariant>,QVariant)),
        this, SLOT(slotError(int,QString,QVariant)),
        QVariant(i));
}

void MovableType::fetchPost(BlogPost *post)
{
    Q_D(MovableType);
    qCDebug(KBLOG_LOG);
    d->loadCategories();
    if (d->mCategoriesList.isEmpty() &&
            post->categories().count()) {
        d->mFetchPostCache << post;
        if (d->mFetchPostCache.count()) {
            // we are already trying to fetch another post, so we don't need to start
            // another listCategories() job
            return;
        }

        connect(this, SIGNAL(listedCategories(QList<QMap<QString,QString> >)),
                this, SLOT(slotTriggerFetchPost()));
        listCategories();
    } else {
        MetaWeblog::fetchPost(post);
    }
}

void MovableType::createPost(BlogPost *post)
{
    // reimplemented because we do this:
    // http://comox.textdrive.com/pipermail/wp-testers/2005-July/000284.html
    qCDebug(KBLOG_LOG);
    Q_D(MovableType);

    // we need mCategoriesList to be loaded first, since we cannot use the post->categories()
    // names later, but we need to map them to categoryId of the blog
    d->loadCategories();
    if (d->mCategoriesList.isEmpty() &&
            !post->categories().isEmpty()) {
        qCDebug(KBLOG_LOG) << "No categories in the cache yet. Have to fetch them first.";
        d->mCreatePostCache << post;
        connect(this, SIGNAL(listedCategories(QList<QMap<QString,QString> >)),
                this, SLOT(slotTriggerCreatePost()));
        listCategories();
    } else {
        bool publish = post->isPrivate();
        // If we do setPostCategories() later than we disable publishing first.
        if (!post->categories().isEmpty()) {
            post->setPrivate(true);
            if (d->mSilentCreationList.contains(post)) {
                qCDebug(KBLOG_LOG) << "Post already in mSilentCreationList, this *should* never happen!";
            } else {
                d->mSilentCreationList << post;
            }
        }
        MetaWeblog::createPost(post);
        // HACK: uuh this a bit ugly now... reenable the original publish argument,
        // since createPost should have parsed now
        post->setPrivate(publish);
    }
}

void MovableType::modifyPost(BlogPost *post)
{
    // reimplemented because we do this:
    // http://comox.textdrive.com/pipermail/wp-testers/2005-July/000284.html
    qCDebug(KBLOG_LOG);
    Q_D(MovableType);

    // we need mCategoriesList to be loaded first, since we cannot use the post->categories()
    // names later, but we need to map them to categoryId of the blog
    d->loadCategories();
    if (d->mCategoriesList.isEmpty() &&
            !post->categories().isEmpty()) {
        qCDebug(KBLOG_LOG) << "No categories in the cache yet. Have to fetch them first.";
        d->mModifyPostCache << post;
        connect(this, SIGNAL(listedCategories(QList<QMap<QString,QString> >)),
                this, SLOT(slotTriggerModifyPost()));
        listCategories();
    } else {
        MetaWeblog::modifyPost(post);
    }
}

void MovableTypePrivate::slotTriggerCreatePost()
{
    qCDebug(KBLOG_LOG);
    Q_Q(MovableType);

    q->disconnect(q, SIGNAL(listedCategories(QList<QMap<QString,QString> >)),
                  q, SLOT(slotTriggerCreatePost()));
    // now we can recall createPost with the posts from the cache
    QList<BlogPost *>::Iterator it = mCreatePostCache.begin();
    QList<BlogPost *>::Iterator end = mCreatePostCache.end();
    for (; it != end; it++) {
        q->createPost(*it);
    }
    mCreatePostCache.clear();
}

void MovableTypePrivate::slotTriggerModifyPost()
{
    qCDebug(KBLOG_LOG);
    Q_Q(MovableType);

    q->disconnect(q, SIGNAL(listedCategories(QList<QMap<QString,QString> >)),
                  q, SLOT(slotTriggerModifyPost()));
    // now we can recall createPost with the posts from the cache
    QList<BlogPost *>::Iterator it = mModifyPostCache.begin();
    QList<BlogPost *>::Iterator end = mModifyPostCache.end();
    for (; it != end; it++) {
        q->modifyPost(*it);
    }
    mModifyPostCache.clear();
}

void MovableTypePrivate::slotTriggerFetchPost()
{
    qCDebug(KBLOG_LOG);
    Q_Q(MovableType);

    q->disconnect(q, SIGNAL(listedCategories(QList<QMap<QString,QString> >)),
                  q, SLOT(slotTriggerFetchPost()));
    QList<BlogPost *>::Iterator it = mFetchPostCache.begin();
    QList<BlogPost *>::Iterator end = mFetchPostCache.end();
    for (; it != end; it++) {
        q->fetchPost(*it);
    }
    mFetchPostCache.clear();
}

MovableTypePrivate::MovableTypePrivate()
{
    qCDebug(KBLOG_LOG);
}

MovableTypePrivate::~MovableTypePrivate()
{
    qCDebug(KBLOG_LOG);
}

void MovableTypePrivate::slotCreatePost(const QList<QVariant> &result, const QVariant &id)
{
    Q_Q(MovableType);
    // reimplement from Blogger1 to chainload the categories stuff before emit()
    qCDebug(KBLOG_LOG);
    KBlog::BlogPost *post = mCallMap[ id.toInt() ];
    mCallMap.remove(id.toInt());

    qCDebug(KBLOG_LOG);
    //array of structs containing ISO.8601
    // dateCreated, String userid, String postid, String content;
    qCDebug(KBLOG_LOG) << "TOP:" << result[0].typeName();
    if (result[0].type() != QVariant::String &&
            result[0].type() != QVariant::Int) {
        qCritical() << "Could not read the postId, not a string or an integer.";
        Q_EMIT q->errorPost(Blogger1::ParsingError,
                          i18n("Could not read the postId, not a string or an integer."),
                          post);
        return;
    }
    QString serverID;
    if (result[0].type() == QVariant::String) {
        serverID = result[0].toString();
    }
    if (result[0].type() == QVariant::Int) {
        serverID = QStringLiteral("%1").arg(result[0].toInt());
    }
    post->setPostId(serverID);
    if (mSilentCreationList.contains(post)) {
        // set the categories and publish afterwards
        setPostCategories(post, !post->isPrivate());
    } else {
        qCDebug(KBLOG_LOG) << "emitting createdPost()"
                           << "for title: \"" << post->title()
                           << "\" server id: " << serverID;
        post->setStatus(KBlog::BlogPost::Created);
        Q_EMIT q->createdPost(post);
    }
}

void MovableTypePrivate::slotFetchPost(const QList<QVariant> &result, const QVariant &id)
{
    Q_Q(MovableType);
    qCDebug(KBLOG_LOG);

    KBlog::BlogPost *post = mCallMap[ id.toInt() ];
    mCallMap.remove(id.toInt());

    //array of structs containing ISO.8601
    // dateCreated, String userid, String postid, String content;
    qCDebug(KBLOG_LOG) << "TOP:" << result[0].typeName();
    if (result[0].type() == QVariant::Map &&
            readPostFromMap(post, result[0].toMap())) {
    } else {
        qCritical() << "Could not fetch post out of the result from the server.";
        post->setError(i18n("Could not fetch post out of the result from the server."));
        post->setStatus(BlogPost::Error);
        Q_EMIT q->errorPost(Blogger1::ParsingError,
                          i18n("Could not fetch post out of the result from the server."), post);
    }
    if (post->categories().isEmpty()) {
        QList<QVariant> args(defaultArgs(post->postId()));
        unsigned int i = mCallCounter++;
        mCallMap[ i ] = post;
        mXmlRpcClient->call(
            QStringLiteral("mt.getPostCategories"), args,
            q, SLOT(slotGetPostCategories(QList<QVariant>,QVariant)),
            q, SLOT(slotError(int,QString,QVariant)),
            QVariant(i));
    } else {
        qCDebug(KBLOG_LOG) << "Emitting fetchedPost()";
        post->setStatus(KBlog::BlogPost::Fetched);
        Q_EMIT q->fetchedPost(post);
    }
}

void MovableTypePrivate::slotModifyPost(const QList<QVariant> &result, const QVariant &id)
{
    Q_Q(MovableType);
    // reimplement from Blogger1
    qCDebug(KBLOG_LOG);
    KBlog::BlogPost *post = mCallMap[ id.toInt() ];
    mCallMap.remove(id.toInt());

    //array of structs containing ISO.8601
    // dateCreated, String userid, String postid, String content;
    qCDebug(KBLOG_LOG) << "TOP:" << result[0].typeName();
    if (result[0].type() != QVariant::Bool &&
            result[0].type() != QVariant::Int) {
        qCritical() << "Could not read the result, not a boolean.";
        Q_EMIT q->errorPost(Blogger1::ParsingError,
                          i18n("Could not read the result, not a boolean."),
                          post);
        return;
    }
    if (mSilentCreationList.contains(post)) {
        post->setStatus(KBlog::BlogPost::Created);
        mSilentCreationList.removeOne(post);
        Q_EMIT q->createdPost(post);
    } else {
        if (!post->categories().isEmpty()) {
            setPostCategories(post, false);
        }
    }
}

void MovableTypePrivate::setPostCategories(BlogPost *post, bool publishAfterCategories)
{
    qCDebug(KBLOG_LOG);
    Q_Q(MovableType);

    unsigned int i = mCallCounter++;
    mCallMap[ i ] = post;
    mPublishAfterCategories[ i ] = publishAfterCategories;
    QList<QVariant> catList;
    QList<QVariant> args(defaultArgs(post->postId()));

    // map the categoryId of the server to the name
    QStringList categories = post->categories();
    for (int j = 0; j < categories.count(); j++) {
        for (int k = 0; k < mCategoriesList.count(); k++) {
            if (mCategoriesList[k][QStringLiteral("name")] == categories[j]) {
                qCDebug(KBLOG_LOG) << "Matched category with name: " << categories[ j ] << " and id: " << mCategoriesList[ k ][ QStringLiteral("categoryId") ];
                QMap<QString, QVariant> category;
                //the first in the QStringList of post->categories()
                // is the primary category
                category[QStringLiteral("categoryId")] = mCategoriesList[k][QStringLiteral("categoryId")].toInt();
                catList << QVariant(category);
                break;
            }
            if (k == mCategoriesList.count()) {
                qCDebug(KBLOG_LOG) << "Couldn't find categoryId for: " << categories[j];
            }
        }
    }
    args << QVariant(catList);

    mXmlRpcClient->call(
        QStringLiteral("mt.setPostCategories"), args,
        q, SLOT(slotSetPostCategories(QList<QVariant>,QVariant)),
        q, SLOT(slotError(int,QString,QVariant)),
        QVariant(i));
}

void MovableTypePrivate::slotGetPostCategories(const QList<QVariant> &result, const QVariant &id)
{
    qCDebug(KBLOG_LOG);
    Q_Q(MovableType);

    int i = id.toInt();
    BlogPost *post = mCallMap[ i ];
    mCallMap.remove(i);

    if (result[ 0 ].type() != QVariant::List) {
        qCritical() << "Could not read the result, not a list. Category fetching failed! We will still Q_EMIT fetched post now.";
        Q_EMIT q->errorPost(Blogger1::ParsingError,
                          i18n("Could not read the result - is not a list. Category fetching failed."), post);

        post->setStatus(KBlog::BlogPost::Fetched);
        Q_EMIT q->fetchedPost(post);
    } else {
        QList<QVariant> categoryList = result[ 0 ].toList();
        QList<QString> newCatList;
        QList<QVariant>::ConstIterator it = categoryList.constBegin();
        QList<QVariant>::ConstIterator end = categoryList.constEnd();
        newCatList.reserve(categoryList.count());
        for (; it != end; it++) {
            newCatList << (*it).toMap()[ QStringLiteral("categoryName") ].toString();
        }
        qCDebug(KBLOG_LOG) << "categories list: " << newCatList;
        post->setCategories(newCatList);
        post->setStatus(KBlog::BlogPost::Fetched);
        Q_EMIT q->fetchedPost(post);
    }
}

void MovableTypePrivate::slotSetPostCategories(const QList<QVariant> &result, const QVariant &id)
{
    qCDebug(KBLOG_LOG);
    Q_Q(MovableType);

    int i = id.toInt();
    BlogPost *post = mCallMap[ i ];
    bool publish = mPublishAfterCategories[ i ];
    mCallMap.remove(i);
    mPublishAfterCategories.remove(i);

    if (result[0].type() != QVariant::Bool) {
        qCritical() << "Could not read the result, not a boolean. Category setting failed! We will still publish if now if necessary. ";
        Q_EMIT q->errorPost(Blogger1::ParsingError,
                          i18n("Could not read the result - is not a boolean value. Category setting failed.  Will still publish now if necessary."),
                          post);
    }
    // Finally publish now, if the post was meant to be published in the beginning.
    // The first boolean is necessary to only publish if the post is created, not
    // modified.
    if (publish && !post->isPrivate()) {
        q->modifyPost(post);
    }

    // this is the end of the chain then
    if (!publish) {
        if (mSilentCreationList.contains(post)) {
            qCDebug(KBLOG_LOG) << "emitting createdPost() for title: \""
                               << post->title() << "\"";
            post->setStatus(KBlog::BlogPost::Created);
            mSilentCreationList.removeOne(post);
            Q_EMIT q->createdPost(post);
        } else {
            qCDebug(KBLOG_LOG) << "emitting modifiedPost() for title: \""
                               << post->title() << "\"";
            post->setStatus(KBlog::BlogPost::Modified);
            Q_EMIT q->modifiedPost(post);
        }
    }
}

QList<QVariant> MovableTypePrivate::defaultArgs(const QString &id)
{
    Q_Q(MovableType);
    QList<QVariant> args;
    if (!id.isEmpty()) {
        args << QVariant(id);
    }
    args << QVariant(q->username())
         << QVariant(q->password());
    return args;
}

bool MovableTypePrivate::readPostFromMap(BlogPost *post, const QMap<QString, QVariant> &postInfo)
{

    // FIXME: integrate error handling
    qCDebug(KBLOG_LOG) << "readPostFromMap()";
    if (!post) {
        return false;
    }

    qCDebug(KBLOG_LOG)
        #if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
            << endl
           #else
            << Qt::endl
           #endif
            << "Keys:" << QStringList(postInfo.keys()).join(QLatin1String(", "));
    qCDebug(KBLOG_LOG)
        #if (QT_VERSION < QT_VERSION_CHECK(5, 15, 0))
            << endl
           #else
            << Qt::endl
           #endif
               ;

    QDateTime dt = postInfo[QStringLiteral("dateCreated")].toDateTime();
    if (dt.isValid() && !dt.isNull()) {
        post->setCreationDateTime(dt.toLocalTime());
    }

    dt = postInfo[QStringLiteral("lastModified")].toDateTime();
    if (dt.isValid() && !dt.isNull()) {
        post->setModificationDateTime(dt.toLocalTime());
    }

    post->setPostId(postInfo[QStringLiteral("postid")].toString().isEmpty() ? postInfo[QStringLiteral("postId")].toString() :
                    postInfo[QStringLiteral("postid")].toString());

    QString title(postInfo[QStringLiteral("title")].toString());
    QString description(postInfo[QStringLiteral("description")].toString());
    QStringList categoryIdList = postInfo[QStringLiteral("categories")].toStringList();
    QStringList categories;
    // since the metaweblog definition is ambigious, we try different
    // category mappings
    for (int i = 0; i < categoryIdList.count(); i++) {
        for (int k = 0; k < mCategoriesList.count(); k++) {
            if (mCategoriesList[ k ][ QStringLiteral("name") ] == categoryIdList[ i ]) {
                categories << mCategoriesList[ k ][ QStringLiteral("name") ];
            } else if (mCategoriesList[ k ][ QStringLiteral("categoryId") ] == categoryIdList[ i ]) {
                categories << mCategoriesList[ k ][ QStringLiteral("name") ];
            }
        }
    }

    //TODO 2 new keys are:
    // String mt_convert_breaks, the value for the convert_breaks field
    post->setSlug(postInfo[QStringLiteral("wp_slug")].toString());
    post->setAdditionalContent(postInfo[QStringLiteral("mt_text_more")].toString());
    post->setTitle(title);
    post->setContent(description);
    post->setCommentAllowed((bool)postInfo[QStringLiteral("mt_allow_comments")].toInt());
    post->setTrackBackAllowed((bool)postInfo[QStringLiteral("mt_allow_pings")].toInt());
    post->setSummary(postInfo[QStringLiteral("mt_excerpt")].toString());
    post->setTags(postInfo[QStringLiteral("mt_keywords")].toStringList());
    post->setLink(QUrl(postInfo[QStringLiteral("link")].toString()));
    post->setPermaLink(QUrl(postInfo[QStringLiteral("permaLink")].toString()));
    QString postStatus = postInfo[QStringLiteral("post_status")].toString();
    if (postStatus != QLatin1String("publish") &&
            !postStatus.isEmpty()) {
        /**
         * Maybe this field wasn't set by server! so, on that situation, we will assume it as non-Private,
         * The postStatus.isEmpty() check is for that!
         * I found this field on Wordpress output! it's value can be: publish, private, draft (as i see)
        */
        post->setPrivate(true);
    }
    if (!categories.isEmpty()) {
        qCDebug(KBLOG_LOG) << "Categories:" << categories;
        post->setCategories(categories);
    }
    return true;
}

void MovableTypePrivate::slotListTrackBackPings(
    const QList<QVariant> &result, const QVariant &id)
{
    Q_Q(MovableType);
    qCDebug(KBLOG_LOG) << "slotTrackbackPings()";
    BlogPost *post = mCallMap[ id.toInt() ];
    mCallMap.remove(id.toInt());
    QList<QMap<QString, QString> > trackBackList;
    if (result[0].type() != QVariant::List) {
        qCritical() << "Could not fetch list of trackback pings out of the"
                    << "result from the server.";
        Q_EMIT q->error(MovableType::ParsingError,
                      i18n("Could not fetch list of trackback pings out of the "
                           "result from the server."));
        return;
    }
    const QList<QVariant> trackBackReceived = result[0].toList();
    QList<QVariant>::ConstIterator it = trackBackReceived.begin();
    QList<QVariant>::ConstIterator end = trackBackReceived.end();
    for (; it != end; ++it) {
        QMap<QString, QString> tping;
        qCDebug(KBLOG_LOG) << "MIDDLE:" << (*it).typeName();
        const QMap<QString, QVariant> trackBackInfo = (*it).toMap();
        tping[ QStringLiteral("title") ] = trackBackInfo[ QStringLiteral("pingTitle")].toString();
        tping[ QStringLiteral("url") ] = trackBackInfo[ QStringLiteral("pingURL")].toString();
        tping[ QStringLiteral("ip") ] = trackBackInfo[ QStringLiteral("pingIP")].toString();
        trackBackList << tping;
    }
    qCDebug(KBLOG_LOG) << "Emitting listedTrackBackPings()";
    Q_EMIT q->listedTrackBackPings(post, trackBackList);
}

bool MovableTypePrivate::readArgsFromPost(QList<QVariant> *args, const BlogPost &post)
{
    //TODO 2 new keys are:
    // String mt_convert_breaks, the value for the convert_breaks field
    // array mt_tb_ping_urls, the list of TrackBack ping URLs for this entry
    if (!args) {
        return false;
    }
    QMap<QString, QVariant> map;
    map[QStringLiteral("categories")] = post.categories();
    map[QStringLiteral("description")] = post.content();
    if (!post.additionalContent().isEmpty()) {
        map[QStringLiteral("mt_text_more")] = post.additionalContent();
    }
    map[QStringLiteral("title")] = post.title();
    map[QStringLiteral("dateCreated")] = post.creationDateTime().toUTC();
    map[QStringLiteral("mt_allow_comments")] = (int)post.isCommentAllowed();
    map[QStringLiteral("mt_allow_pings")] = (int)post.isTrackBackAllowed();
    map[QStringLiteral("mt_excerpt")] = post.summary();
    map[QStringLiteral("mt_keywords")] = post.tags().join(QLatin1Char(','));
    //map["mt_tb_ping_urls"] check for that, i think this should only be done on the server.
    *args << map;
    *args << QVariant(!post.isPrivate());
    return true;
}

#include "moc_movabletype.cpp"
