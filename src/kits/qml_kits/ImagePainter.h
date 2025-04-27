#pragma once
#include "ImageTexture.h"
#include <QImage>
#include <QQuickItem>

namespace _Kits
{

class ImagePainter : public QQuickItem
{
    Q_OBJECT
  public:
    explicit ImagePainter(QQuickItem *parent = nullptr);
    virtual ~ImagePainter();

  public slots:
    void updateImage(const QImage &);

  protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;

  private:
    QSGNode *updateByQSG(QSGNode *oldNode);
    QSGNode *updateByRhi(QSGNode *oldNode);
    QImage m_image;
    ImageTexture *m_texture = nullptr;
    bool m_bUseRhi;
};
} // namespace _Kits