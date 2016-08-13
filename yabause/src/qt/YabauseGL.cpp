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

YabauseGL::YabauseGL( QWidget* p )
	: QGLWidget( p ), hideMouseTimeout(60)
{
	setFocusPolicy( Qt::StrongFocus );
	
	if ( p ) {
		p->setFocusPolicy( Qt::StrongFocus );
		setFocusProxy( p );
	}
}

void YabauseGL::showEvent( QShowEvent* e )
{
	// hack for clearing the the gl context
	QGLWidget::showEvent( e );
	QSize s = size();
	resize( 0, 0 );
	resize( s );
}

void YabauseGL::resizeGL( int w, int h )
{ updateView( QSize( w, h ) ); }

void YabauseGL::updateView( const QSize& s )
{
	const QSize size = s.isValid() ? s : this->size();
	glViewport( 0, 0, size.width(), size.height() );
	if ( VIDCore )
		VIDCore->Resize( size.width(), size.height(), 0 );
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
	QGLWidget::mouseMoveEvent(event);
}
