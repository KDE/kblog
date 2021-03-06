/*
  This file is part of the kblog library.

  SPDX-FileCopyrightText: 2006-2007 Christian Weilbach <christian_weilbach@web.de>
  SPDX-FileCopyrightText: 2007 Mike McQuaid <mike@mikemcquaid.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KBLOG_BLOGCOMMENT_H
#define KBLOG_BLOGCOMMENT_H

#include <kblog_export.h>

#include <QString>
#include <QtAlgorithms>

class QDateTime;
class QUrl;

namespace KBlog
{

class BlogCommentPrivate;
/**
  @brief
  A class that represents a blog comment on the blog post.

  @code
  KBlog::BlogComment *comment = new BlogComment();
  comment->setTitle( "This is the title." );
  comment->setContent( "Here is some the content..." );
  @endcode

  @author Mike McQuaid \<mike\@mikemcquaid.com\>
*/

class KBLOG_EXPORT BlogComment
{
public:
    /**
      Copy Constructor for list handling.
      @param comment The comment to copy.
    */
    BlogComment(const BlogComment &comment);

    /**
     Constructor.
     @param commentId The ID of the comment on the server.
    */
    explicit BlogComment(const QString &commentId = QString());

    /**
      Virtual default destructor.
    */
    virtual ~BlogComment();

    /**
      Returns the title.
      @return The title.

      @see setTitle( const QString& )
    */
    QString title() const;

    /**
      Sets the title.
      @param title This is the title.

      @see title()
    */
    void setTitle(const QString &title);

    /**
      Returns the content.
      @return The content.

      @see setContent( const QString& )
    */
    QString content() const;

    /**
      Sets the content.
      @param content This is the content.

      @see content()
    */
    void setContent(const QString &content);

    /**
      Returns the comment's id.
      @return The comment's id

      @see setCommentId( const QString& )
    */
    QString commentId() const;

    /**
      Sets the comment's id.
      @param id The comment's id.

      @see commentId()
    */
    void setCommentId(const QString &id);

    /**
      Returns the E-Mail address of the commentator.
      @return The E-Mail.

      @see setEmail( const QString& )
    */
    QString email() const;

    /**
      Sets the E-Mail.
      @param email This is the E-Mail address of the commentator.

      @see email()
    */
    void setEmail(const QString &email);

    /**
      Returns the commentator's name.
      @return The name.

      @see setName()
    */
    QString name() const;

    /**
      Sets the name of the commentator.
      @param name This is the commenator's name.

      @see name()
    */
    void setName(const QString &name);

    /**
      Returns the commentator's homepage URL.
      @return The url of the commentator's homepage

      @see setUrl( const QUrl& )
    */
    QUrl url() const;

    /**
      Sets the commentator's homepage URL.
      @param url The commentator's homepage url.

      @see url()
    */
    void setUrl(const QUrl &url);

    /**
      Returns the modification date-time.
      @return The modification date-time.

      @see setModificationDateTime( const QDateTime& )
    */
    QDateTime modificationDateTime() const;

    /**
      Sets the modification date-time.
      @param datetime The date-time the comment has been modified.

      @see modificationDateTime( const QDateTime& )
    */
    void setModificationDateTime(const QDateTime &datetime);

    /**
      Returns the creation date-time.
      @return The creation date-time.

      @see setCreationDateTime( const QDateTime& )
    */
    QDateTime creationDateTime() const;

    /**
      Sets the creation date-time.
      @param datetime The date-time the comment has been created.

      @see creationDateTime()
    */
    void setCreationDateTime(const QDateTime &datetime);

    /**
      The enumartion of the different post status, reflecting the status changes
      on the server.
    */
    enum Status {
        /** Status of a freshly constructed comment on the client. */
        New,
        /** Status of a successfully fetched comment. */
        Fetched,
        /** Status of a successfully created comment.
        @see GData::createComment( BlogPost*, BlogComment* ) */
        Created,
        /** Status of a successfully removed comment.
        @see GData::removeComment( BlogPost*, BlogComment* ) */
        Removed,
        /** Status when an error has occurred on the server side.
        @see error() */
        Error
    };

    /**
      Returns the status on the server.
      @return The status.

      @see setStatus( Status ), Status
    */
    Status status() const;

    /**
      Sets the status.
      @param status The status on the server.

      @see status(), Status
    */
    void setStatus(Status status);

    /**
      Returns the last error.
      @returns The last error string.

      @see setError( const QString& ), Error
    */
    QString error() const;

    /**
      Sets the error.
      @param error The error string.

      @see error(), Error
    */
    void setError(const QString &error);

    /**
      Overloaded for QList handling.
    */
    BlogComment &operator=(const BlogComment &comment);

    /**
      The swap operator.
    */
    void swap(BlogComment &other)
    {
        qSwap(this->d_ptr, other.d_ptr);
    }

private:
    BlogCommentPrivate *d_ptr; //krazy:exclude=dpointer can't constify due to bic and swap being declared inline
};

} //namespace KBlog

#endif
