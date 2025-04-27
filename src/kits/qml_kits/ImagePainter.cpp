#include "ImagePainter.h"
#include "ImageTexture.h"
#include <QSGSimpleTextureNode>

using namespace _Kits;
ImagePainter::ImagePainter(QQuickItem *parent)
    : QQuickItem(parent), m_bUseRhi(false)
{
    // 这句不加会报错
    setFlag(ItemHasContents, true);
    // 默认图片不可以空，否则渲染引擎会出现异常，造成最开始的帧显示错误
}
ImagePainter::~ImagePainter()
{
}

void ImagePainter::updateImage(const QImage &image)
{
    m_image = image;
    update();
}

QSGNode *ImagePainter::updatePaintNode(QSGNode *oldNode,
                                       QQuickItem::UpdatePaintNodeData *)
{
    return m_bUseRhi ? updateByRhi(oldNode) : updateByQSG(oldNode);
}

QSGNode *ImagePainter::updateByQSG(QSGNode *oldNode)
{
    auto *node = dynamic_cast<QSGSimpleTextureNode *>(oldNode);

    if (!node)
    {
        node = new QSGSimpleTextureNode();
    }

    QSGTexture *texture = window()->createTextureFromImage(
        m_image, QQuickWindow::TextureIsOpaque);
    node->setOwnsTexture(true);
    node->setRect(boundingRect());
    node->setTexture(texture);
    return node;
}

QSGNode *ImagePainter::updateByRhi(QSGNode *oldNode)
{
    auto *node = dynamic_cast<QSGSimpleTextureNode *>(oldNode);
    if (!node)
    {
        node = new QSGSimpleTextureNode();
        m_texture = new ImageTexture(window());
        node->setTexture(m_texture);
    }
    if (m_texture && !m_image.isNull())
    {
        m_texture->setImage(m_image);  // 设置图像
        m_texture->updateTexture();    // 纹理数据同步到 GPU
        node->setRect(boundingRect()); // 设置绘制区域
        m_image = QImage();
    }
    return node;
}