#pragma once
#include <QImage>
#include <QQuickWindow>
#include <QSGDynamicTexture>

namespace _Kits
{

class ImageTexture : public QSGDynamicTexture
{
    Q_OBJECT
  public:
    explicit ImageTexture(QQuickWindow *window);
    virtual ~ImageTexture();

    void setImage(const QImage &image);

    // 实现 QSGDynamicTexture 的纯虚函数
    bool updateTexture() override;

    // 实现 QSGTexture 的纯虚函数
    qint64 comparisonKey() const override;
    QSize textureSize() const override;
    bool hasAlphaChannel() const override;
    bool hasMipmaps() const override;
    QRhiTexture *rhiTexture() const override;

  private:
    void releaseTexture();
    QQuickWindow *m_window;
    QImage m_image;
    QRhiTexture *m_rhiTexture;
    bool m_dirty;
};
} // namespace _Kits
