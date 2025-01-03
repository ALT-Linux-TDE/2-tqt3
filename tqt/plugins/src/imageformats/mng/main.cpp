#ifndef QT_CLEAN_NAMESPACE
#define QT_CLEAN_NAMESPACE
#endif

#include <ntqimageformatplugin.h>

#ifndef QT_NO_IMAGEFORMATPLUGIN

#ifdef QT_NO_IMAGEIO_MNG
#undef QT_NO_IMAGEIO_MNG
#endif
#include "../../../../src/kernel/qmngio.cpp"

class MNGFormat : public TQImageFormatPlugin
{
public:
    MNGFormat();

    TQStringList keys() const;
    bool loadImage( const TQString &format, const TQString &filename, TQImage *image );
    bool saveImage( const TQString &format, const TQString &filename, const TQImage &image );
    bool installIOHandler( const TQString & );
};

MNGFormat::MNGFormat()
{
}


TQStringList MNGFormat::keys() const
{
    TQStringList list;
    list << "MNG";

    return list;
}

bool MNGFormat::loadImage( const TQString &, const TQString &, TQImage * )
{
    return FALSE;
}

bool MNGFormat::saveImage( const TQString &, const TQString &, const TQImage& )
{
    return FALSE;
}

bool MNGFormat::installIOHandler( const TQString &name )
{
    if ( name != "MNG" )
	return FALSE;

    qInitMngIO();
    return TRUE;
}

Q_EXPORT_PLUGIN( MNGFormat )

#endif // QT_NO_IMAGEFORMATPLUGIN
