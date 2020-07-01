/*
  This file is part of the kblog library.

  SPDX-FileCopyrightText: 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2006-2007 Christian Weilbach <christian_weilbach@web.de>
  SPDX-FileCopyrightText: 2007-2008 Mike McQuaid <mike@mikemcquaid.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "blogger1.h"
#include "blogger1_p.h"
#include "blogpost.h"

#include <kxmlrpcclient/client.h>

#include "kblog_debug.h"
#include <KLocalizedString>

#include <QList>

#include <QStringList>

using namespace KBlog;

Blogger1::Blogger1(const QUrl &server, QObject *parent)
    : Blog(server, *new Blogger1Private, parent)
{
    qCDebug(KBLOG_LOG);
    setUrl(server);
}

Blogger1::Blogger1(const QUrl &server, Blogger1Private &dd, QObject *parent)
    : Blog(server, dd, parent)
{
    qCDebug(KBLOG_LOG);
    setUrl(server);
}

Blogger1::~Blogger1()
{
    qCDebug(KBLOG_LOG);
}

QString Blogger1::interfaceName() const
{
    return QStringLiteral("Blogger 1.0");
}

void Blogger1::setUrl(const QUrl &server)
{
    Q_D(Blogger1);
    Blog::setUrl(server);
    delete d->mXmlRpcClient;
    d->mXmlRpcClient = new KXmlRpc::Client(server);
    d->mXmlRpcClient->setUserAgent(userAgent());
}

void Blogger1::fetchUserInfo()
{
    Q_D(Blogger1);
    qCDebug(KBLOG_LOG) << "Fetch user's info...";
    QList<QVariant> args(d->blogger1Args());
    d->mXmlRpcClient->call(
        QStringLiteral("blogger.getUserInfo"), args,
        this, SLOT(slotFetchUserInfo(QList<QVariant>,QVariant)),
        this, SLOT(slotError(int,QString,QVariant)));
}

void Blogger1::listBlogs()
{
    Q_D(Blogger1);
    qCDebug(KBLOG_LOG) << "Fetch List of Blogs...";
    QList<QVariant> args(d->blogger1Args());
    d->mXmlRpcClient->call(
        QStringLiteral("blogger.getUsersBlogs"), args,
        this, SLOT(slotListBlogs(QList<QVariant>,QVariant)),
        this, SLOT(slotError(int,QString,QVariant)));
}

void Blogger1::listRecentPosts(int number)
{
    Q_D(Blogger1);
    qCDebug(KBLOG_LOG) << "Fetching List of Posts...";
    QList<QVariant> args(d->defaultArgs(blogId()));
    args << QVariant(number);
    d->mXmlRpcClient->call(
        d->getCallFromFunction(Blogger1Private::GetRecentPosts), args,
        this, SLOT(slotListRecentPosts(QList<QVariant>,QVariant)),
        this, SLOT(slotError(int,QString,QVariant)),
        QVariant(number));
}

void Blogger1::fetchPost(KBlog::BlogPost *post)
{
    if (!post) {
        qCritical() << "Blogger1::modifyPost: post is null pointer";
        return;
    }

    Q_D(Blogger1);
    qCDebug(KBLOG_LOG) << "Fetching Post with url" << post->postId();
    QList<QVariant> args(d->defaultArgs(post->postId()));
    unsigned int i = d->mCallCounter++;
    d->mCallMap[ i ] = post;
    d->mXmlRpcClient->call(
        d->getCallFromFunction(Blogger1Private::FetchPost), args,
        this, SLOT(slotFetchPost(QList<QVariant>,QVariant)),
        this, SLOT(slotError(int,QString,QVariant)),
        QVariant(i));
}

void Blogger1::modifyPost(KBlog::BlogPost *post)
{
    Q_D(Blogger1);

    if (!post) {
        qCritical() << "Blogger1::modifyPost: post is null pointer";
        return;
    }

    qCDebug(KBLOG_LOG) << "Uploading Post with postId" << post->postId();
    unsigned int i = d->mCallCounter++;
    d->mCallMap[ i ] = post;
    QList<QVariant> args(d->defaultArgs(post->postId()));
    d->readArgsFromPost(&args, *post);
    d->mXmlRpcClient->call(
        d->getCallFromFunction(Blogger1Private::ModifyPost), args,
        this, SLOT(slotModifyPost(QList<QVariant>,QVariant)),
        this, SLOT(slotError(int,QString,QVariant)),
        QVariant(i));
}

void Blogger1::createPost(KBlog::BlogPost *post)
{
    Q_D(Blogger1);

    if (!post) {
        qCritical() << "Blogger1::createPost: post is null pointer";
        return;
    }

    unsigned int i = d->mCallCounter++;
    d->mCallMap[ i ] = post;
    qCDebug(KBLOG_LOG) << "Creating new Post with blogid" << blogId();
    QList<QVariant> args(d->defaultArgs(blogId()));
    d->readArgsFromPost(&args, *post);
    d->mXmlRpcClient->call(
        d->getCallFromFunction(Blogger1Private::CreatePost), args,
        this, SLOT(slotCreatePost(QList<QVariant>,QVariant)),
        this, SLOT(slotError(int,QString,QVariant)),
        QVariant(i));
}

void Blogger1::removePost(KBlog::BlogPost *post)
{
    Q_D(Blogger1);

    if (!post) {
        qCritical() << "Blogger1::removePost: post is null pointer";
        return;
    }

    unsigned int i = d->mCallCounter++;
    d->mCallMap[ i ] = post;
    qCDebug(KBLOG_LOG) << "Blogger1::removePost: postId=" << post->postId();
    QList<QVariant> args(d->blogger1Args(post->postId()));
    args << QVariant(true);   // Publish must be set to remove post.
    d->mXmlRpcClient->call(
        QStringLiteral("blogger.deletePost"), args,
        this, SLOT(slotRemovePost(QList<QVariant>,QVariant)),
        this, SLOT(slotError(int,QString,QVariant)),
        QVariant(i));
}

Blogger1Private::Blogger1Private() :
    mXmlRpcClient(nullptr)
{
    qCDebug(KBLOG_LOG);
    mCallCounter = 1;
}

Blogger1Private::~Blogger1Private()
{
    qCDebug(KBLOG_LOG);
    delete mXmlRpcClient;
}

QList<QVariant> Blogger1Private::defaultArgs(const QString &id)
{
    qCDebug(KBLOG_LOG);
    Q_Q(Blogger1);
    QList<QVariant> args;
    args << QVariant(QLatin1String("0123456789ABCDEF"));
    if (!id.isEmpty()) {
        args << QVariant(id);
    }
    args << QVariant(q->username())
         << QVariant(q->password());
    return args;
}

// reimplemenet defaultArgs, since we may not use it virtually everywhere
QList<QVariant> Blogger1Private::blogger1Args(const QString &id)
{
    qCDebug(KBLOG_LOG);
    Q_Q(Blogger1);
    QList<QVariant> args;
    args << QVariant(QLatin1String("0123456789ABCDEF"));
    if (!id.isEmpty()) {
        args << QVariant(id);
    }
    args << QVariant(q->username())
         << QVariant(q->password());
    return args;
}

void Blogger1Private::slotFetchUserInfo(const QList<QVariant> &result, const QVariant &id)
{
    Q_Q(Blogger1);
    Q_UNUSED(id);

    qCDebug(KBLOG_LOG);
    qCDebug(KBLOG_LOG) << "TOP:" << result[0].typeName();
    QMap<QString, QString> userInfo;
    if (result[0].type() != QVariant::Map) {
        qCritical() << "Could not fetch user's info out of the result from the server,"
                    << "not a map.";
        Q_EMIT q->error(Blogger1::ParsingError,
                      i18n("Could not fetch user's info out of the result "
                           "from the server, not a map."));
        return;
    }
    const QMap<QString, QVariant> resultMap = result[0].toMap();
    userInfo[QStringLiteral("nickname")] = resultMap[QStringLiteral("nickname")].toString();
    userInfo[QStringLiteral("userid")] = resultMap[QStringLiteral("userid")].toString();
    userInfo[QStringLiteral("url")] = resultMap[QStringLiteral("url")].toString();
    userInfo[QStringLiteral("email")] = resultMap[QStringLiteral("email")].toString();
    userInfo[QStringLiteral("lastname")] = resultMap[QStringLiteral("lastname")].toString();
    userInfo[QStringLiteral("firstname")] = resultMap[QStringLiteral("firstname")].toString();

    Q_EMIT q->fetchedUserInfo(userInfo);
}

void Blogger1Private::slotListBlogs(const QList<QVariant> &result, const QVariant &id)
{
    Q_Q(Blogger1);
    Q_UNUSED(id);

    qCDebug(KBLOG_LOG);
    qCDebug(KBLOG_LOG) << "TOP:" << result[0].typeName();
    QList<QMap<QString, QString> > blogsList;
    if (result[0].type() != QVariant::List) {
        qCritical() << "Could not fetch blogs out of the result from the server,"
                    << "not a list.";
        Q_EMIT q->error(Blogger1::ParsingError,
                      i18n("Could not fetch blogs out of the result "
                           "from the server, not a list."));
        return;
    }
    const QList<QVariant> posts = result[0].toList();
    QList<QVariant>::ConstIterator it = posts.begin();
    QList<QVariant>::ConstIterator end = posts.end();
    for (; it != end; ++it) {
        qCDebug(KBLOG_LOG) << "MIDDLE:" << (*it).typeName();
        const QMap<QString, QVariant> postInfo = (*it).toMap();
        QMap<QString, QString> blogInfo;
        blogInfo[ QStringLiteral("id") ] = postInfo[QStringLiteral("blogid")].toString();
        blogInfo[ QStringLiteral("url") ] = postInfo[QStringLiteral("url")].toString();
        blogInfo[ QStringLiteral("apiUrl") ] = postInfo[QStringLiteral("xmlrpc")].toString();
        blogInfo[ QStringLiteral("title") ] = postInfo[QStringLiteral("blogName")].toString();
        qCDebug(KBLOG_LOG) << "Blog information retrieved: ID =" << blogInfo[QStringLiteral("id")]
                           << ", Name =" << blogInfo[QStringLiteral("title")];
        blogsList << blogInfo;
    }
    Q_EMIT q->listedBlogs(blogsList);
}

void Blogger1Private::slotListRecentPosts(const QList<QVariant> &result, const QVariant &id)
{
    Q_Q(Blogger1);
    int count = id.toInt(); // not sure if needed, actually the API should
// not give more posts

    qCDebug(KBLOG_LOG);
    qCDebug(KBLOG_LOG) << "TOP:" << result[0].typeName();

    QList <BlogPost> fetchedPostList;

    if (result[0].type() != QVariant::List) {
        qCritical() << "Could not fetch list of posts out of the"
                    << "result from the server, not a list.";
        Q_EMIT q->error(Blogger1::ParsingError,
                      i18n("Could not fetch list of posts out of the result "
                           "from the server, not a list."));
        return;
    }
    const QList<QVariant> postReceived = result[0].toList();
    QList<QVariant>::ConstIterator it = postReceived.begin();
    QList<QVariant>::ConstIterator end = postReceived.end();
    for (; it != end; ++it) {
        BlogPost post;
        qCDebug(KBLOG_LOG) << "MIDDLE:" << (*it).typeName();
        const QMap<QString, QVariant> postInfo = (*it).toMap();
        if (readPostFromMap(&post, postInfo)) {
            qCDebug(KBLOG_LOG) << "Post with ID:"
                               << post.postId()
                               << "appended in fetchedPostList";
            post.setStatus(BlogPost::Fetched);
            fetchedPostList.append(post);
        } else {
            qCritical() << "readPostFromMap failed!";
            Q_EMIT q->error(Blogger1::ParsingError, i18n("Could not read post."));
        }
        if (--count == 0) {
            break;
        }
    }
    qCDebug(KBLOG_LOG) << "Emitting listRecentPostsFinished()";
    Q_EMIT q->listedRecentPosts(fetchedPostList);
}

void Blogger1Private::slotFetchPost(const QList<QVariant> &result, const QVariant &id)
{
    Q_Q(Blogger1);
    qCDebug(KBLOG_LOG);

    KBlog::BlogPost *post = mCallMap[ id.toInt() ];
    mCallMap.remove(id.toInt());

    //array of structs containing ISO.8601
    // dateCreated, String userid, String postid, String content;
    // TODO: Time zone for the dateCreated!
    qCDebug(KBLOG_LOG) << "TOP:" << result[0].typeName();
    if (result[0].type() == QVariant::Map &&
            readPostFromMap(post, result[0].toMap())) {
        qCDebug(KBLOG_LOG) << "Emitting fetchedPost()";
        post->setStatus(KBlog::BlogPost::Fetched);
        Q_EMIT q->fetchedPost(post);
    } else {
        qCritical() << "Could not fetch post out of the result from the server.";
        post->setError(i18n("Could not fetch post out of the result from the server."));
        post->setStatus(BlogPost::Error);
        Q_EMIT q->errorPost(Blogger1::ParsingError,
                          i18n("Could not fetch post out of the result from the server."), post);
    }
}

void Blogger1Private::slotCreatePost(const QList<QVariant> &result, const QVariant &id)
{
    Q_Q(Blogger1);
    KBlog::BlogPost *post = mCallMap[ id.toInt() ];
    mCallMap.remove(id.toInt());

    qCDebug(KBLOG_LOG);
    //array of structs containing ISO.8601
    // dateCreated, String userid, String postid, String content;
    // TODO: Time zone for the dateCreated!
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
    } else if (result[0].type() == QVariant::Int) {
        serverID = QStringLiteral("%1").arg(result[0].toInt());
    }
    post->setPostId(serverID);
    post->setStatus(KBlog::BlogPost::Created);
    qCDebug(KBLOG_LOG) << "emitting createdPost()"
                       << "for title: \"" << post->title()
                       << "\" server id: " << serverID;
    Q_EMIT q->createdPost(post);
}

void Blogger1Private::slotModifyPost(const QList<QVariant> &result, const QVariant &id)
{
    Q_Q(Blogger1);
    KBlog::BlogPost *post = mCallMap[ id.toInt() ];
    mCallMap.remove(id.toInt());

    qCDebug(KBLOG_LOG);
    //array of structs containing ISO.8601
    // dateCreated, String userid, String postid, String content;
    // TODO: Time zone for the dateCreated!
    qCDebug(KBLOG_LOG) << "TOP:" << result[0].typeName();
    if (result[0].type() != QVariant::Bool &&
            result[0].type() != QVariant::Int) {
        qCritical() << "Could not read the result, not a boolean.";
        Q_EMIT q->errorPost(Blogger1::ParsingError,
                          i18n("Could not read the result, not a boolean."),
                          post);
        return;
    }
    post->setStatus(KBlog::BlogPost::Modified);
    qCDebug(KBLOG_LOG) << "emitting modifiedPost() for title: \""
                       << post->title() << "\"";
    Q_EMIT q->modifiedPost(post);
}

void Blogger1Private::slotRemovePost(const QList<QVariant> &result, const QVariant &id)
{
    Q_Q(Blogger1);
    KBlog::BlogPost *post = mCallMap[ id.toInt() ];
    mCallMap.remove(id.toInt());

    qCDebug(KBLOG_LOG) << "slotRemovePost";
    //array of structs containing ISO.8601
    // dateCreated, String userid, String postid, String content;
    // TODO: Time zone for the dateCreated!
    qCDebug(KBLOG_LOG) << "TOP:" << result[0].typeName();
    if (result[0].type() != QVariant::Bool &&
            result[0].type() != QVariant::Int) {
        qCritical() << "Could not read the result, not a boolean.";
        Q_EMIT q->errorPost(Blogger1::ParsingError,
                          i18n("Could not read the result, not a boolean."),
                          post);
        return;
    }
    post->setStatus(KBlog::BlogPost::Removed);
    qCDebug(KBLOG_LOG) << "emitting removedPost()";
    Q_EMIT q->removedPost(post);
}

void Blogger1Private::slotError(int number,
                                const QString &errorString,
                                const QVariant &id)
{
    Q_Q(Blogger1);
    Q_UNUSED(number);
    qCDebug(KBLOG_LOG) << "An error occurred: " << errorString;
    BlogPost *post = mCallMap[ id.toInt() ];

    if (post) {
        Q_EMIT q->errorPost(Blogger1::XmlRpc, errorString, post);
    } else {
        Q_EMIT q->error(Blogger1::XmlRpc, errorString);
    }
}

bool Blogger1Private::readPostFromMap(
    BlogPost *post, const QMap<QString, QVariant> &postInfo)
{
    // FIXME: integrate error handling
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
    //QString description( postInfo["description"].toString() );
    QString contents;
    if (postInfo[QStringLiteral("content")].type() == QVariant::ByteArray) {
        QByteArray tmpContent = postInfo[QStringLiteral("content")].toByteArray();
        contents = QString::fromUtf8(tmpContent.data(), tmpContent.size());
    } else {
        contents = postInfo[QStringLiteral("content")].toString();
    }
    QStringList category;

    // Check for hacked title/category support (e.g. in Wordpress)
    QRegExp titleMatch = QRegExp(QStringLiteral("<title>([^<]*)</title>"));
    QRegExp categoryMatch = QRegExp(QStringLiteral("<category>([^<]*)</category>"));
    if (contents.indexOf(titleMatch) != -1) {
        // Get the title value from the regular expression match
        title = titleMatch.cap(1);
    }
    if (contents.indexOf(categoryMatch) != -1) {
        // Get the category value from the regular expression match
        category = categoryMatch.capturedTexts();
    }
    contents.remove(titleMatch);
    contents.remove(categoryMatch);

    post->setTitle(title);
    post->setContent(contents);
    post->setCategories(category);
    return true;
}

bool Blogger1Private::readArgsFromPost(QList<QVariant> *args, const BlogPost &post)
{
    if (!args) {
        return false;
    }
    const QStringList categories = post.categories();
    QString content = QStringLiteral("<title>") + post.title() + QStringLiteral("</title>");
    QStringList::const_iterator it;
    QStringList::const_iterator end(categories.constEnd());
    for (it = categories.constBegin(); it != end; ++it) {
        content += QStringLiteral("<category>") + *it + QStringLiteral("</category>");
    }
    content += post.content();
    *args << QVariant(content);
    *args << QVariant(!post.isPrivate());
    return true;
}

QString Blogger1Private::getCallFromFunction(FunctionToCall type)
{
    switch (type) {
    case GetRecentPosts: return QStringLiteral("blogger.getRecentPosts");
    case CreatePost:        return QStringLiteral("blogger.newPost");
    case ModifyPost:       return QStringLiteral("blogger.editPost");
    case FetchPost:        return QStringLiteral("blogger.getPost");
    default: return QString();
    }
}

#include "moc_blogger1.cpp"
