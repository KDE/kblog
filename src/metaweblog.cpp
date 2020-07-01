/*
  This file is part of the kblog library.

  SPDX-FileCopyrightText: 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2006-2007 Christian Weilbach <christian_weilbach@web.de>
  SPDX-FileCopyrightText: 2007 Mike McQuaid <mike@mikemcquaid.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "metaweblog.h"
#include "metaweblog_p.h"
#include "blogpost.h"
#include "blogmedia.h"

#include <kxmlrpcclient/client.h>
#include "kblog_debug.h"
#include <KLocalizedString>

#include <QFile>
#include <QDataStream>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>

using namespace KBlog;

MetaWeblog::MetaWeblog(const QUrl &server, QObject *parent)
    : Blogger1(server, *new MetaWeblogPrivate, parent)
{
    qCDebug(KBLOG_LOG);
}

MetaWeblog::MetaWeblog(const QUrl &server, MetaWeblogPrivate &dd, QObject *parent)
    : Blogger1(server, dd, parent)
{
    qCDebug(KBLOG_LOG);
}

MetaWeblog::~MetaWeblog()
{
    qCDebug(KBLOG_LOG);
}

QString MetaWeblog::interfaceName() const
{
    return QStringLiteral("MetaWeblog");
}

void MetaWeblog::listCategories()
{
    Q_D(MetaWeblog);
    qCDebug(KBLOG_LOG) << "Fetching List of Categories...";
    QList<QVariant> args(d->defaultArgs(blogId()));
    d->mXmlRpcClient->call(
        QStringLiteral("metaWeblog.getCategories"), args,
        this, SLOT(slotListCategories(QList<QVariant>,QVariant)),
        this, SLOT(slotError(int,QString,QVariant)));
}

void MetaWeblog::createMedia(KBlog::BlogMedia *media)
{
    Q_D(MetaWeblog);
    if (!media) {
        qCritical() << "MetaWeblog::createMedia: media is a null pointer";
        Q_EMIT error(Other, i18n("Media is a null pointer."));
        return;
    }
    unsigned int i = d->mCallMediaCounter++;
    d->mCallMediaMap[ i ] = media;
    qCDebug(KBLOG_LOG) << "MetaWeblog::createMedia: name=" << media->name();
    QList<QVariant> args(d->defaultArgs(blogId()));
    QMap<QString, QVariant> map;
    map[QStringLiteral("name")] = media->name();
    map[QStringLiteral("type")] = media->mimetype();
    map[QStringLiteral("bits")] = media->data();
    args << map;
    d->mXmlRpcClient->call(
        QStringLiteral("metaWeblog.newMediaObject"), args,
        this, SLOT(slotCreateMedia(QList<QVariant>,QVariant)),
        this, SLOT(slotError(int,QString,QVariant)),
        QVariant(i));

}

MetaWeblogPrivate::MetaWeblogPrivate()
{
    qCDebug(KBLOG_LOG);
    mCallMediaCounter = 1;
    mCatLoaded = false;
}

MetaWeblogPrivate::~MetaWeblogPrivate()
{
    qCDebug(KBLOG_LOG);
}

QList<QVariant> MetaWeblogPrivate::defaultArgs(const QString &id)
{
    Q_Q(MetaWeblog);
    QList<QVariant> args;
    if (!id.isEmpty()) {
        args << QVariant(id);
    }
    args << QVariant(q->username())
         << QVariant(q->password());
    return args;
}

void MetaWeblogPrivate::loadCategories()
{
    qCDebug(KBLOG_LOG);

    if (mCatLoaded) {
        return;
    }
    mCatLoaded = true;

    if (mUrl.isEmpty() || mBlogId.isEmpty() || mUsername.isEmpty()) {
        qCDebug(KBLOG_LOG) << "We need at least url, blogId and the username to create a unique filename.";
        return;
    }

    QString filename = QStringLiteral("kblog/") + mUrl.host() + QLatin1Char('_') + mBlogId + QLatin1Char('_') + mUsername;
    filename = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + filename;
    QDir().mkpath(QFileInfo(filename).absolutePath());
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qCDebug(KBLOG_LOG) << "Cannot open cached categories file: " << filename;
        return;
    }

    QDataStream stream(&file);
    stream >> mCategoriesList;
    file.close();
}

void MetaWeblogPrivate::saveCategories()
{
    qCDebug(KBLOG_LOG);
    if (mUrl.isEmpty() || mBlogId.isEmpty() || mUsername.isEmpty()) {
        qCDebug(KBLOG_LOG) << "We need at least url, blogId and the username to create a unique filename.";
        return;
    }

    QString filename = QStringLiteral("kblog/") + mUrl.host() + QLatin1Char('_') + mBlogId + QLatin1Char('_') + mUsername;
    filename = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + filename;
    QDir().mkpath(QFileInfo(filename).absolutePath());
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        qCDebug(KBLOG_LOG) << "Cannot open cached categories file: " << filename;
        return;
    }

    QDataStream stream(&file);
    stream << mCategoriesList;
    file.close();
}

void MetaWeblogPrivate::slotListCategories(const QList<QVariant> &result,
        const QVariant &id)
{
    Q_Q(MetaWeblog);
    Q_UNUSED(id);

    qCDebug(KBLOG_LOG) << "MetaWeblogPrivate::slotListCategories";
    qCDebug(KBLOG_LOG) << "TOP:" << result[0].typeName();
    if (result[0].type() != QVariant::Map &&
            result[0].type() != QVariant::List) {
        // include fix for not metaweblog standard compatible apis with
        // array of structs instead of struct of structs, e.g. wordpress
        qCritical() << "Could not list categories out of the result from the server.";
        Q_EMIT q->error(MetaWeblog::ParsingError,
                      i18n("Could not list categories out of the result "
                           "from the server."));
    } else {
        if (result[0].type() == QVariant::Map) {
            const QMap<QString, QVariant> serverMap = result[0].toMap();
            for (auto it = serverMap.cbegin(), end = serverMap.cend(); it != end; ++it) {
                const QString &key = it.key();
                qCDebug(KBLOG_LOG) << "MIDDLE:" << key;
                QMap<QString, QString> category;
                const QMap<QString, QVariant> serverCategory = it.value().toMap();
                category[QStringLiteral("name")] = key;
                category[QStringLiteral("description")] = serverCategory[ QStringLiteral("description") ].toString();
                category[QStringLiteral("htmlUrl")] = serverCategory[ QStringLiteral("htmlUrl") ].toString();
                category[QStringLiteral("rssUrl")] = serverCategory[ QStringLiteral("rssUrl") ].toString();
                category[QStringLiteral("categoryId")] = serverCategory[ QStringLiteral("categoryId") ].toString();
                category[QStringLiteral("parentId")] = serverCategory[ QStringLiteral("parentId") ].toString();
                mCategoriesList.append(category);
            }
            qCDebug(KBLOG_LOG) << "Emitting listedCategories";
            Q_EMIT q->listedCategories(mCategoriesList);
        }
    }
    if (result[0].type() == QVariant::List) {
        // include fix for not metaweblog standard compatible apis with
        // array of structs instead of struct of structs, e.g. wordpress
        const QList<QVariant> serverList = result[0].toList();
        QList<QVariant>::ConstIterator it = serverList.begin();
        QList<QVariant>::ConstIterator end = serverList.end();
        for (; it != end; ++it) {
            qCDebug(KBLOG_LOG) << "MIDDLE:" << (*it).typeName();
            QMap<QString, QString> category;
            const QMap<QString, QVariant> serverCategory = (*it).toMap();
            category[ QStringLiteral("name") ] = serverCategory[QStringLiteral("categoryName")].toString();
            category[QStringLiteral("description")] = serverCategory[ QStringLiteral("description") ].toString();
            category[QStringLiteral("htmlUrl")] = serverCategory[ QStringLiteral("htmlUrl") ].toString();
            category[QStringLiteral("rssUrl")] = serverCategory[ QStringLiteral("rssUrl") ].toString();
            category[QStringLiteral("categoryId")] = serverCategory[ QStringLiteral("categoryId") ].toString();
            category[QStringLiteral("parentId")] = serverCategory[ QStringLiteral("parentId") ].toString();
            mCategoriesList.append(category);
        }
        qCDebug(KBLOG_LOG) << "Emitting listedCategories()";
        Q_EMIT q->listedCategories(mCategoriesList);
    }
    saveCategories();
}

void MetaWeblogPrivate::slotCreateMedia(const QList<QVariant> &result,
                                        const QVariant &id)
{
    Q_Q(MetaWeblog);

    KBlog::BlogMedia *media = mCallMediaMap[ id.toInt() ];
    mCallMediaMap.remove(id.toInt());

    qCDebug(KBLOG_LOG) << "MetaWeblogPrivate::slotCreateMedia, no error!";
    qCDebug(KBLOG_LOG) << "TOP:" << result[0].typeName();
    if (result[0].type() != 8) {
        qCritical() << "Could not read the result, not a map.";
        Q_EMIT q->errorMedia(MetaWeblog::ParsingError,
                           i18n("Could not read the result, not a map."),
                           media);
        return;
    }
    const QMap<QString, QVariant> resultStruct = result[0].toMap();
    const QString url = resultStruct[QStringLiteral("url")].toString();
    qCDebug(KBLOG_LOG) << "MetaWeblog::slotCreateMedia url=" << url;

    if (!url.isEmpty()) {
        media->setUrl(QUrl(url));
        media->setStatus(BlogMedia::Created);
        qCDebug(KBLOG_LOG) << "Emitting createdMedia( url=" << url  << ");";
        Q_EMIT q->createdMedia(media);
    }
}

bool MetaWeblogPrivate::readPostFromMap(BlogPost *post,
                                        const QMap<QString, QVariant> &postInfo)
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
    QStringList categories(postInfo[QStringLiteral("categories")].toStringList());

    post->setTitle(title);
    post->setContent(description);
    if (!categories.isEmpty()) {
        qCDebug(KBLOG_LOG) << "Categories:" << categories;
        post->setCategories(categories);
    }
    return true;
}

bool MetaWeblogPrivate::readArgsFromPost(QList<QVariant> *args, const BlogPost &post)
{
    if (!args) {
        return false;
    }
    QMap<QString, QVariant> map;
    map[QStringLiteral("categories")] = post.categories();
    map[QStringLiteral("description")] = post.content();
    map[QStringLiteral("title")] = post.title();
    map[QStringLiteral("lastModified")] = post.modificationDateTime().toUTC();
    map[QStringLiteral("dateCreated")] = post.creationDateTime().toUTC();
    *args << map;
    *args << QVariant(!post.isPrivate());
    return true;
}

QString MetaWeblogPrivate::getCallFromFunction(FunctionToCall type)
{
    switch (type) {
    case GetRecentPosts: return QStringLiteral("metaWeblog.getRecentPosts");
    case CreatePost:        return QStringLiteral("metaWeblog.newPost");
    case ModifyPost:       return QStringLiteral("metaWeblog.editPost");
    case FetchPost:        return QStringLiteral("metaWeblog.getPost");
    default: return QString();
    }
}
#include "moc_metaweblog.cpp"
