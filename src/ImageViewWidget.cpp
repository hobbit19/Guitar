#include "ImageViewWidget.h"
#include "FileDiffSliderWidget.h"

#include "MainWindow.h"
#include "common/misc.h"
#include "common/joinpath.h"
#include "Photoshop.h"
#include "MemoryReader.h"
#include "charvec.h"

#include <QDebug>
#include <QFileDialog>
#include <QMenu>
#include <QPainter>
#include <QWheelEvent>
#include <functional>
#include <QSvgRenderer>
#include <QBuffer>

typedef std::shared_ptr<QSvgRenderer> SvgRendererPtr;

struct ImageViewWidget::Private {
	MainWindow *mainwindow = nullptr;
	QPixmap transparent_pixmap;
	ObjectContentPtr content;
	FileDiffWidget::DrawData *draw_data = nullptr;
	QScrollBar *vertical_scroll_bar = nullptr;
	QString mime_type;

	QPixmap pixmap;
	SvgRendererPtr svg;

	FileViewType file_type = FileViewType::None;
	double image_scroll_x = 0;
	double image_scroll_y = 0;
	double image_scale = 1;
	int scroll_origin_x = 0;
	int scroll_origin_y = 0;
	QPoint mouse_press_pos;
	int wheel_delta = 0;
	QPointF interest_pos;
	int top_margin = 1;
	int bottom_margin = 1;
	bool draw_left_border = true;
//	bool binary_mode = false;
//	bool terminal_mode = false;
};

//FileDiffWidget::DrawData *ImageViewWidget::drawdata()
//{
//	Q_ASSERT(m->draw_data);
//	return m->draw_data;
//}

//FileDiffWidget::DrawData const *ImageViewWidget::drawdata() const
//{
//	Q_ASSERT(m->draw_data);
//	return m->draw_data;
//}

ObjectContentPtr ImageViewWidget::getContent() const
{
	return m->content;
}


TextDiffLineList const *ImageViewWidget::getLines() const
{
	ObjectContentPtr content = getContent();
	if (content) return &content->lines;
	return nullptr;
}

ImageViewWidget::ImageViewWidget(QWidget *parent)
	: QWidget(parent)
	, m(new Private)
{
#if defined(Q_OS_WIN32)
	setFont(QFont("MS Gothic"));
#elif defined(Q_OS_LINUX)
	setFont(QFont("Monospace"));
#elif defined(Q_OS_MAC)
	setFont(QFont("Menlo"));
#endif
}

ImageViewWidget::~ImageViewWidget()
{
	delete m;
}

void ImageViewWidget::bind(MainWindow *mainwindow)
{
	m->mainwindow = mainwindow;
//	m->content = content;
//	m->draw_data = drawdata;

//	updateDrawData();
}

bool ImageViewWidget::hasFocus() const
{
	QWidget *w = qApp->focusWidget();
	return w && w->isAncestorOf(this);
}

void ImageViewWidget::setLeftBorderVisible(bool f)
{
	m->draw_left_border = f;
}

void ImageViewWidget::setFileType(QString const &mimetype)
{
//	setBinaryMode(false);
//	m->mime_type = mimetype;
//	if (misc::isImage(m->mime_type)) {
		m->file_type = FileViewType::Image;
		setMouseTracking(true);
//	} else {
//		m->file_type = FileViewType::Text;
//		setMouseTracking(false);
//	}
}

void ImageViewWidget::clear()
{
	m->file_type = FileViewType::None;
	m->mime_type = QString();
	m->pixmap = QPixmap();
	setMouseTracking(false);
	update();
}





QString ImageViewWidget::formatText(Document::Line const &line)
{
	QByteArray const &ba = line.text;
	if (ba.isEmpty()) return QString();
	std::vector<char> vec;
	vec.reserve(ba.size() + 100);
	char const *begin = ba.data();
	char const *end = begin + ba.size();
	char const *ptr = begin;
	int x = 0;
	while (ptr < end) {
		if (*ptr == '\t') {
			do {
				vec.push_back(' ');
				x++;
			} while ((x % 4) != 0);
			ptr++;
		} else {
			vec.push_back(*ptr);
			ptr++;
			x++;
		}
	}
	return QString::fromUtf8(&vec[0], vec.size());
}

//void ImageViewWidget::paintText()
//{
//	TextDiffLineList const *lines = getLines();
//	if (!lines) return;

//	QPainter pr(this);

//	int x;
//	int w = width();
//	int h = height();

//	int descent;
////	updateDrawData(&pr, &descent);

//	x = 0;

//	int linenums = isTerminalMode() ? 0 : 5;

//	if (lines) {
//		int char_width = drawdata()->char_width;
//		int linenum_w = isTerminalMode() ? 0 : (char_width * linenums);
//		int x2 = x + linenum_w;
//		pr.save();
//		pr.setClipRect(x, 0, w, h);
//		int y = drawdata()->v_scroll_pos * drawdata()->line_height;
//		int i = y / drawdata()->line_height;
//		y -= i * drawdata()->line_height;
//		y = -y;
//		while (i < lines->size() && y < h) {
//			QString line = formatText(lines->at(i));
//			Document::Line::Type type = (Document::Line::Type)lines->at(i).type;

//			QColor *bgcolor;
//			switch (type) {
//			case Document::Line::Normal:
//				bgcolor = &drawdata()->bgcolor_text;
//				break;
//			case Document::Line::Add:
//				bgcolor = &drawdata()->bgcolor_add;
//				break;
//			case Document::Line::Del:
//				bgcolor = &drawdata()->bgcolor_del;
//				break;
//			default:
//				bgcolor = &drawdata()->bgcolor_gray;
//				break;
//			}

//			int line_y = y + drawdata()->line_height - descent - m->bottom_margin;

//			int text_clip_x = 0;

//			if (isTerminalMode()) {
//				pr.fillRect(0, y, w, drawdata()->line_height, QBrush(*bgcolor));
//				text_clip_x = 3; // left margin
//			} else {
//				// draw line number
//				pr.fillRect(x, y, linenum_w + char_width, drawdata()->line_height, drawdata()->bgcolor_gray);
//				int num = lines->at(i).line_number;
//				if (num >= 1 && num <= lines->size()) {
//					QString text = "     " + QString::number(num);
//					text = text.mid(text.size() - linenums);
//					pr.setPen(QColor(128, 128, 128));
//					pr.drawText(x, line_y, text);
//				}

//				text_clip_x = char_width * (linenums + 1); // 行番号＋ヘッダマーク

//				// fill bg
//				pr.fillRect(text_clip_x - 1, y, w - text_clip_x + 1, drawdata()->line_height, QBrush(*bgcolor));
//			}

//			// draw text
//			pr.setPen(QColor(0, 0, 0));

//			if (isTerminalMode()) {
//				// nop
//			} else {
//				switch (type) {
//				case Document::Line::Add: pr.drawText(x2, line_y, "+"); break;
//				case Document::Line::Del: pr.drawText(x2, line_y, "-"); break;
//				}
//			}

//			int text_x = text_clip_x - drawdata()->h_scroll_pos * char_width; // 水平スクロール

//			pr.save();
//			pr.setClipRect(text_clip_x, 0, width() - text_clip_x, height());
//			pr.drawText(text_x, line_y, line);
//			pr.restore();

//			y += drawdata()->line_height;
//			i++;
//		}
//		if (y < h) {
//			pr.fillRect(x, y, w, h - y, QColor(192, 192, 192));
//			pr.fillRect(x, y, w, 1, Qt::black);
//		}
//		pr.restore();
//	}
//}

//void ImageViewWidget::paintBinary()
//{
//	ObjectContentPtr content = getContent();
//	if (!content) return;
//	if (content->bytes.isEmpty()) return;

//	int length = content->bytes.size();
//	uint8_t const *source = (uint8_t const *)content->bytes.data();

//	QPainter pr(this);

//	int x;
//	int descent;
////	updateDrawData(&pr, &descent);

//	x = 0;

//	{//if (lines) {
//		int y = drawdata()->v_scroll_pos * drawdata()->line_height;
//		y = -y;
//		int offset = 0;
//		while (offset < length) {
//			char tmp[100];
//			int j = sprintf(tmp, " %08X ", offset);
//			for (int i = 0; i < 16; i++) {
//				if (offset + i < length) {
//					sprintf(tmp + j, "%02X ", source[offset + i]);
//				} else {
//					strcpy(tmp + j, "   ");
//				}
//				j += 3;
//			}
//			for (int i = 0; i < 16; i++) {
//				uint8_t c = '0';
//				if (offset < length) {
//					c = source[offset];
//					if (!isprint(c)) {
//						c = '.';
//					}
//					offset++;
//				}
//				tmp[j] = c;
//				j++;
//			}
//			tmp[j] = 0;
//			int line_y = y + drawdata()->line_height - descent - m->bottom_margin;
//			QColor bgcolor = drawdata()->bgcolor_text;
//			pr.fillRect(0, y, width(), drawdata()->line_height, QBrush(bgcolor));
//			pr.setPen(QColor(0, 0, 0));
//			pr.drawText(x, line_y, tmp);
//			y += drawdata()->line_height;
//			if (y >= height()) break;
//		}
//	}
//}

QSizeF ImageViewWidget::imageScrollRange() const
{
	QSize sz = imageSize();
	int w = sz.width() * m->image_scale;
	int h = sz.height() * m->image_scale;
	return QSize(w, h);
}

QBrush ImageViewWidget::getTransparentBackgroundBrush()
{
	return m->mainwindow->getTransparentPixmap();
}

bool ImageViewWidget::isValidImage() const
{
	return !m->pixmap.isNull() || (m->svg && m->svg->isValid());
}

QSize ImageViewWidget::imageSize() const
{
	if (!m->pixmap.isNull()) return m->pixmap.size();
	if (m->svg && m->svg->isValid()) return m->svg->defaultSize();
	return QSize();
}

void ImageViewWidget::paintImage()
{
	QPainter pr(this);
	QSize imagesize = imageSize();
	if (imagesize.width() > 0 && imagesize.height() > 0) {
		pr.save();
		if (!m->draw_left_border) {
			pr.setClipRect(1, 0, width() - 1, height());
		}
		double cx = width() / 2.0;
		double cy = height() / 2.0;
		double x = cx - m->image_scroll_x;
		double y = cy - m->image_scroll_y;
		QSizeF sz = imageScrollRange();
		if (sz.width() > 0 && sz.height() > 0) {
			QBrush br = getTransparentBackgroundBrush();
			pr.setBrushOrigin(x, y);
			pr.fillRect(x, y, sz.width(), sz.height(), br);
			if (!m->pixmap.isNull()) {
				pr.drawPixmap(x, y, sz.width(), sz.height(), m->pixmap, 0, 0, imagesize.width(), imagesize.height());
			} else if (m->svg && m->svg->isValid()) {
				m->svg->render(&pr, QRectF(x, y, sz.width(), sz.height()));
			}
		}
		misc::drawFrame(&pr, x - 1, y - 1, sz.width() + 2, sz.height() + 2, Qt::black);
		pr.restore();
	}
}

void ImageViewWidget::paintEvent(QPaintEvent *)
{
	switch (m->file_type) {
	case FileViewType::Image:
		paintImage();
		break;
	}
	QPainter pr(this);
	if (m->draw_left_border) {
		pr.fillRect(0, 0, 1, height(), QColor(160, 160, 160));
	}
//	if (drawdata()->forcus == this) {
//		int x = 0;
//		int y = 0;
//		int w = width();
//		int h = height();
//		if (w > 0 && h > 0) {
//			QColor color(0, 128, 255, 32);
//			pr.fillRect(x, y, w, h, color);
//		}
//	}
	if (hasFocus()) {
		misc::drawFrame(&pr, 0, 0, width(), height(), QColor(0, 128, 255, 128));
		misc::drawFrame(&pr, 1, 1, width() - 2, height() - 2, QColor(0, 128, 255, 64));
	}
}

void ImageViewWidget::resizeEvent(QResizeEvent *)
{
	emit resized();
}

void ImageViewWidget::scrollImage(double x, double y)
{
	m->image_scroll_x = x;
	m->image_scroll_y = y;
	QSizeF sz = imageScrollRange();
	if (m->image_scroll_x < 0) m->image_scroll_x = 0;
	if (m->image_scroll_y < 0) m->image_scroll_y = 0;
	if (m->image_scroll_x > sz.width()) m->image_scroll_x = sz.width();
	if (m->image_scroll_y > sz.height()) m->image_scroll_y = sz.height();
	update();
}

void ImageViewWidget::setImage(QString mimetype, QByteArray const &ba)
{
	if (mimetype.isEmpty()) {
		mimetype = "image/x-unknown";
	}
	setFileType(mimetype);
	if (m->file_type == FileViewType::Image) {
		m->pixmap = QPixmap();
		m->svg = SvgRendererPtr();
		if (!ba.isEmpty()) {
			if (misc::isSVG(mimetype)) {
				m->svg = SvgRendererPtr(new QSvgRenderer(ba));
			} else if (misc::isPSD(mimetype)) {
				if (!ba.isEmpty()) {
					MemoryReader reader(ba.data(), ba.size());
					if (reader.open(QIODevice::ReadOnly)) {
						std::vector<char> jpeg;
						photoshop::readThumbnail(&reader, &jpeg);
						if (!jpeg.empty()) {
							m->pixmap.loadFromData((uchar const *)&jpeg[0], jpeg.size());
						}
					}
				}
			} else {
				m->pixmap.loadFromData(ba);
			}
		}
		m->image_scale = 1;
		QSize sz = imageSize();
		double x = 0;
		double y = 0;
		double sx = sz.width();
		double sy = sz.height();
		if (sx > 0 && sy > 0) {
			sx = width() / sx;
			sy = height() / sy;
			m->image_scale = (sx < sy ? sx : sy) * 0.9;
			x = sz.width() * m->image_scale / 2.0;
			y = sz.height() * m->image_scale / 2.0;
		}
		scrollImage(x, y);
	}
}

FileViewType ImageViewWidget::filetype() const
{
	return m->file_type;
}

void ImageViewWidget::mousePressEvent(QMouseEvent *e)
{
	if (e->button() == Qt::LeftButton) {
		QPoint pos = mapFromGlobal(QCursor::pos());
		m->mouse_press_pos = pos;
		m->scroll_origin_x = m->image_scroll_x;
		m->scroll_origin_y = m->image_scroll_y;
	}
}

void ImageViewWidget::mouseMoveEvent(QMouseEvent *e)
{
	if (m->file_type == FileViewType::Image) {
		if (isValidImage()) {
			QPoint pos = mapFromGlobal(QCursor::pos());
			if ((e->buttons() & Qt::LeftButton) && hasFocus()) {
				int delta_x = pos.x() - m->mouse_press_pos.x();
				int delta_y = pos.y() - m->mouse_press_pos.y();
				scrollImage(m->scroll_origin_x - delta_x, m->scroll_origin_y - delta_y);
			}
			double cx = width() / 2.0;
			double cy = height() / 2.0;
			double x = (pos.x() + 0.5 - cx + m->image_scroll_x) / m->image_scale;
			double y = (pos.y() + 0.5 - cy + m->image_scroll_y) / m->image_scale;
			m->interest_pos = QPointF(x, y);
			m->wheel_delta = 0;
		}
	}
}

void ImageViewWidget::setImageScale(double scale)
{
	if (scale < 1 / 32.0) scale = 1 / 32.0;
	if (scale > 32) scale = 32;
	m->image_scale = scale;
}

void ImageViewWidget::wheelEvent(QWheelEvent *e)
{
	if (m->file_type == FileViewType::Text) {
		int delta = e->delta();
		if (delta < 0) {
			delta = -delta / 40;
			if (delta == 0) delta = 1;
			emit scrollByWheel(delta);
		} else if (delta > 0) {
			delta /= 40;
			if (delta == 0) delta = 1;
			emit scrollByWheel(-delta);

		}
	} else if (m->file_type == FileViewType::Image) {
		if (isValidImage()) {
			double scale = 1;
			const double mul = 1.189207115; // sqrt(sqrt(2))
			m->wheel_delta += e->delta();
			while (m->wheel_delta >= 120) {
				m->wheel_delta -= 120;
				scale *= mul;
			}
			while (m->wheel_delta <= -120) {
				m->wheel_delta += 120;
				scale /= mul;
			}
			setImageScale(m->image_scale * scale);

			double cx = width() / 2.0;
			double cy = height() / 2.0;
			QPoint pos = mapFromGlobal(QCursor::pos());
			double dx = m->interest_pos.x() * m->image_scale + cx - (pos.x() + 0.5);
			double dy = m->interest_pos.y() * m->image_scale + cy - (pos.y() + 0.5);
			scrollImage(dx, dy);

			update();
		}
	}
}

void ImageViewWidget::contextMenuEvent(QContextMenuEvent *e)
{
//	if (!m->mainwindow) return; // TODO:

	ObjectContentPtr content = getContent();
	if (!content) return;

	QString id = content->id;
	if (id.startsWith(PATH_PREFIX)) {
		// pass
	} else if (Git::isValidID(id)) {
		// pass
	} else {
		return; // invalid id
	}

	QPoint pos;
	if (e->reason() == QContextMenuEvent::Mouse) {
		pos = QCursor::pos() + QPoint(8, -8);
	} else {
		pos = mapToGlobal(QPoint(4, 4));
	}

	QMenu menu;
	QAction *a_save_as = id.isEmpty() ? nullptr : menu.addAction(tr("Save as..."));
	QAction *a_test = id.isEmpty() ? nullptr : menu.addAction(tr("test"));
	if (!menu.actions().isEmpty()) {
//		drawdata()->forcus = this;
		update();
		QAction *a = menu.exec(pos);
		if (a) {
			if (a == a_save_as) {
				QString path = m->mainwindow->currentWorkingCopyDir() / content->path;
				QString dstpath = QFileDialog::getSaveFileName(window(), tr("Save as"), path);
				if (!dstpath.isEmpty()) {
					m->mainwindow->saveAs(id, dstpath);
				}
				goto DONE;
			}
			if (a == a_test) {
				emit onBinaryMode();
				goto DONE;
			}
		}
	}
DONE:;
//	drawdata()->forcus = nullptr;
	update();
}
