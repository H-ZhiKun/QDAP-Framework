#include "ImageTexture.h"
#include <QQuickWindow>
#include <rhi/qrhi.h>

namespace _Kits
{
ImageTexture::ImageTexture(QQuickWindow *window)
    : m_window(window), m_image(nullptr), m_rhiTexture(nullptr), m_dirty(false)
{
}

ImageTexture::~ImageTexture()
{
    if (m_rhiTexture)
    {
        m_rhiTexture->destroy();
        delete m_rhiTexture;
    }
}

void ImageTexture::setImage(const QImage &image)
{
    if (image.format() == QImage::Format_RGB888)
    {
        // 避免 QImage::convertToFormat 造成的拷贝
        QImage temp(image.width(), image.height(), QImage::Format_RGBA8888);
        uchar *dst = temp.bits();
        const uchar *src = image.constBits();
        for (int i = 0; i < image.width() * image.height(); ++i)
        {
            dst[0] = src[0]; // R
            dst[1] = src[1]; // G
            dst[2] = src[2]; // B
            dst[3] = 255;    // A
            dst += 4;
            src += 3;
        }
        m_image = image.convertedTo(QImage::Format_RGBA8888);
    }
    m_dirty = true;
}

bool ImageTexture::updateTexture()
{
    QRhi *rhi = m_window->rhi();
    if (!rhi)
    {
        qWarning() << "QRhi is not available!";
        return false;
    }

    if (m_image.isNull())
    {
        qWarning() << "Image is null!";
        return false;
    }

    if (!m_rhiTexture)
    {
        // 首次创建纹理
        m_rhiTexture = rhi->newTexture(QRhiTexture::RGBA8,
                                       m_image.size(),
                                       1,
                                       QRhiTexture::UsedAsTransferSource);
        if (!m_rhiTexture->create())
        {
            qWarning() << "Failed to create texture!";
            delete m_rhiTexture;
            m_rhiTexture = nullptr;
            return false;
        }
    }

    if (m_dirty)
    {
        // 更新纹理数据
        QRhiResourceUpdateBatch *resourceUpdates =
            rhi->nextResourceUpdateBatch();
        if (resourceUpdates)
        {
            resourceUpdates->uploadTexture(m_rhiTexture, m_image);
            m_dirty = false;
            commitTextureOperations(rhi, resourceUpdates);
        }
    }

    return true;
}

qint64 ImageTexture::comparisonKey() const
{
    return m_rhiTexture ? reinterpret_cast<qint64>(m_rhiTexture) : 0;
}

QSize ImageTexture::textureSize() const
{
    return m_image.size();
}

bool ImageTexture::hasAlphaChannel() const
{
    return m_image.hasAlphaChannel();
}

bool ImageTexture::hasMipmaps() const
{
    return false;
}

QRhiTexture *ImageTexture::rhiTexture() const
{
    return m_rhiTexture;
}
} // namespace _Kits