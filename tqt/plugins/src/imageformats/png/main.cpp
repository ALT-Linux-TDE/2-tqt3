#ifndef QT_CLEAN_NAMESPACE
#define QT_CLEAN_NAMESPACE
#endif
#include <ntqimageformatplugin.h>

#ifndef QT_NO_IMAGEFORMATPLUGIN

#ifdef QT_NO_IMAGEIO_PNG
#undef QT_NO_IMAGEIO_PNG
#endif
#include "../../../../src/kernel/qpngio.cpp"

class PNGFormat : public TQImageFormatPlugin
{
public:
    PNGFormat();

    TQStringList keys() const;
    bool loadImage( const TQString &format, const TQString &filename, TQImage * );
    bool saveImage( const TQString &format, const TQString &filename, const TQImage& );
    bool installIOHandler( const TQString & );
};

PNGFormat::PNGFormat()
{
}


TQStringList PNGFormat::keys() const
{
    TQStringList list;
    list << "PNG";

    return list;
}

bool PNGFormat::loadImage( const TQString &format, const TQString &filename, TQImage *image )
{
    if ( format != "PNG" )
	return FALSE;

    TQImageIO io;
    io.setFileName( filename );
    io.setImage( *image );

    read_png_image( &io );

    return TRUE;
}

bool PNGFormat::saveImage( const TQString &format, const TQString &filename, const TQImage &image )
{
    if ( format != "PNG" )
	return FALSE;

    TQImageIO io;
    io.setFileName( filename );
    io.setImage( image );

    write_png_image( &io );

    return TRUE;
}

bool PNGFormat::installIOHandler( const TQString &name )
{
    if ( name != "PNG" ) 
	return FALSE;

    qInitPngIO();
    return TRUE;
}

Q_EXPORT_PLUGIN( PNGFormat )

#endif // QT_NO_IMAGEFORMATPLUGIN
