/****************************************************************************
**
** Definition of Motif-like style class
**
** Created : 981231
**
** Copyright (C) 1998-2008 Trolltech ASA.  All rights reserved.
**
** This file is part of the widgets module of the TQt GUI Toolkit.
**
** This file may be used under the terms of the GNU General
** Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the files LICENSE.GPL2
** and LICENSE.GPL3 included in the packaging of this file.
** Alternatively you may (at your option) use any later version
** of the GNU General Public License if such license has been
** publicly approved by Trolltech ASA (or its successors, if any)
** and the KDE Free TQt Foundation.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/.
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** This file may be used under the terms of the Q Public License as
** defined by Trolltech ASA and appearing in the file LICENSE.TQPL
** included in the packaging of this file.  Licensees holding valid TQt
** Commercial licenses may use this file in accordance with the TQt
** Commercial License Agreement provided with the Software.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not granted
** herein.
**
**********************************************************************/

#ifndef TQMOTIFSTYLE_H
#define TQMOTIFSTYLE_H

#ifndef QT_H
#include "ntqcommonstyle.h"
#endif // QT_H

#if !defined(TQT_NO_STYLE_MOTIF) || defined(QT_PLUGIN)

class TQPalette;

#if defined(QT_PLUGIN)
#define Q_EXPORT_STYLE_MOTIF
#else
#define Q_EXPORT_STYLE_MOTIF TQ_EXPORT
#endif


class Q_EXPORT_STYLE_MOTIF TQMotifStyle : public TQCommonStyle
{
    TQ_OBJECT
public:
    TQMotifStyle( bool useHighlightCols=FALSE );
    virtual ~TQMotifStyle();

    void setUseHighlightColors( bool );
    bool useHighlightColors() const;

    void polish( TQPalette& );
    void polish( const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void * );
    void applicationPolish( const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void * );

    void polishPopupMenu( const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void * );

    // new style API
    void drawPrimitive( PrimitiveElement pe,
			TQPainter *p,
			const TQStyleControlElementData &ceData,
			ControlElementFlags elementFlags,
			const TQRect &r,
			const TQColorGroup &cg,
			SFlags flags = Style_Default,
			const TQStyleOption& = TQStyleOption::Default ) const;

    void drawControl( ControlElement element,
		      TQPainter *p,
		      const TQStyleControlElementData &ceData,
		      ControlElementFlags elementFlags,
		      const TQRect &r,
		      const TQColorGroup &cg,
		      SFlags how = Style_Default,
		      const TQStyleOption& = TQStyleOption::Default,
		      const TQWidget *widget = 0 ) const;

    void drawComplexControl( ComplexControl control,
			     TQPainter *p,
			     const TQStyleControlElementData &ceData,
			     ControlElementFlags elementFlags,
			     const TQRect& r,
			     const TQColorGroup& cg,
			     SFlags how = Style_Default,
#ifdef Q_QDOC
			     SCFlags sub = SC_All,
#else
			     SCFlags sub = (uint)SC_All,
#endif
			     SCFlags subActive = SC_None,
			     const TQStyleOption& = TQStyleOption::Default,
			     const TQWidget* widget = 0 ) const;

    TQRect querySubControlMetrics( ComplexControl control,
				  const TQStyleControlElementData &ceData,
				  ControlElementFlags elementFlags,
				  SubControl sc,
				  const TQStyleOption& = TQStyleOption::Default,
				  const TQWidget *widget = 0 ) const;

    int pixelMetric( PixelMetric metric, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, const TQWidget *widget = 0 ) const;

    TQSize sizeFromContents( ContentsType contents,
			    const TQStyleControlElementData &ceData,
			    ControlElementFlags elementFlags,
			    const TQSize &contentsSize,
			    const TQStyleOption& = TQStyleOption::Default,
			    const TQWidget *widget = 0 ) const;

    TQRect subRect( SubRect r, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, const TQWidget *widget ) const;

    TQPixmap stylePixmap(StylePixmap, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, const TQStyleOption& = TQStyleOption::Default, const TQWidget * = 0) const;

    int styleHint(StyleHint sh, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, const TQStyleOption & = TQStyleOption::Default,
		  TQStyleHintReturn* = 0, const TQWidget * = 0) const;

private:
    bool highlightCols;

    // Disabled copy constructor and operator=
#if defined(TQ_DISABLE_COPY)
    TQMotifStyle( const TQMotifStyle & );
    TQMotifStyle& operator=( const TQMotifStyle & );
#endif
};

#endif // TQT_NO_STYLE_MOTIF

#endif // TQMOTIFSTYLE_H
