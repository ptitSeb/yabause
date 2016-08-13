/*  Copyright 2005 Guillaume Duhamel
	Copyright 2005-2006 Theo Berkau
	Copyright 2008 Filipe Azevedo <pasnox@gmail.com>

	This file is part of Yabause.

	Yabause is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	Yabause is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Yabause; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/
#include "YabauseGL.h"
#include "QtYabause.h"
#include "../vidsoft.h"

#include <QPainter>

YabauseGL::YabauseGL( QWidget* p )
	: QWidget( p ), hideMouseTimeout(60)
{
	setFocusPolicy( Qt::StrongFocus );

	if ( p ) {
		p->setFocusPolicy( Qt::StrongFocus );
		setFocusProxy( p );
	}
}

void YabauseGL::showEvent( QShowEvent* e )
{
}

void YabauseGL::resizeGL( int w, int h )
{ updateView( QSize( w, h ) ); }

void YabauseGL::updateView( const QSize& s )
{
}

void YabauseGL::swapBuffers()
{
	this->HandleMouseHiding();
//	this->update(this->rect());
	this->repaint(this->rect());
}

QImage YabauseGL::grabFrameBuffer(bool withAlpha)
{
//	return QImage();
	int buf_width, buf_height;

	if (dispbuffer == NULL) return QImage();

	VIDCore->GetGlSize( &buf_width, &buf_height );

#ifdef USE_RGB_555
	QImage image = QImage((uchar *) dispbuffer, buf_width, buf_height, QImage::Format_RGB555);
#elif USE_RGB_565
	QImage image = QImage((uchar *) dispbuffer, buf_width, buf_height, QImage::Format_RGB16);
#else
	QImage image = QImage((uchar *) dispbuffer, buf_width, buf_height, QImage::Format_RGB32);
#endif
	return image.rgbSwapped();
}

void YabauseGL::paintEvent( QPaintEvent * event )
{
	int buf_width, buf_height;

	if (dispbuffer == NULL) return;

	VIDCore->GetGlSize( &buf_width, &buf_height );

#ifdef USE_RGB_555
	QImage image = QImage((uchar *) dispbuffer, buf_width, buf_height, QImage::Format_RGB555);
#elif USE_RGB_565
	QImage image = QImage((uchar *) dispbuffer, buf_width, buf_height, QImage::Format_RGB16);
#else
	QImage image = QImage((uchar *) dispbuffer, buf_width, buf_height, QImage::Format_RGB32);
#endif
	image = image.rgbSwapped();
	QPainter p(this);
	p.drawImage(this->rect(), image);
}

void YabauseGL::makeCurrent()
{
}


void YabauseGL::HandleMouseHiding()
{
	// Mouse watchdog timer handling. Basically, if the timeout value is
	// greater than zero, decrement it. Otherwise, check for zero, if so, then
	// hide the mouse and set the hideMouseTimeout value to -1 to signal that
	// the mouse has been hidden.
	if (hideMouseTimeout > 0)
		hideMouseTimeout--;
	else if (hideMouseTimeout == 0)
	{
		hideMouseTimeout--;
		setCursor(Qt::BlankCursor);
	}
}


// We use this as part of a watchdog system for hiding/unhiding the mouse. This
// part shows the mouse (if hidden) and resets the watchdog timer.
void YabauseGL::CheckAndRestoreMouseCursor()
{
	// Has the mouse been hidden? (-1 means mouse was hidden)
	if (hideMouseTimeout == -1)
		setCursor(Qt::ArrowCursor);

	hideMouseTimeout = 60;
}


// We check here for mouse movement; if there is any, show the mouse and reset
// the watchdog timer.
void YabauseGL::mouseMoveEvent(QMouseEvent * event)
{
	CheckAndRestoreMouseCursor();
	QWidget::mouseMoveEvent(event);
}
