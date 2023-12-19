#include "celler.h"

//#define MIN(a, b) (a < b ? a : b)
//#define MAX(a, b) (a > b ? a : b)
int MIN(int a, int b) { return a < b ? a : b; }
int MAX(int a, int b) { return a > b ? a : b; }
long8 MIN_L(long8 a, long8 b) { return a < b ? a : b; }
long8 MAX_L(long8 a, long8 b) { return a > b ? a : b; }
float MIN_F(float a, float b) { return a < b ? a : b; }
float MAX_F(float a, float b) { return a > b ? a : b; }
qreal MIN_Q(qreal a, qreal b) { return a < b ? a : b; }
qreal MAX_Q(qreal a, qreal b) { return a > b ? a : b; }
float MIN_F3(float a, float b, float c) { return MIN_F(a, MIN_F(b, c)); }

#define ADD_BLOCK(x, y) do { zone.insert(X_Y_TO_POS(x, y)); min_x = MIN_L(min_x, x); max_x = MAX_L(max_x, x); min_y = MIN_L(min_y, y); max_y = MAX_L(max_y, y); } while(false);
#define ADD_RECT(x1, y1, x2, y2) do { \
    for (int y = y1; y <= y2; y++) for (int x = x1; x <= x2; x++) zone.insert(X_Y_TO_POS(x, y)); \
    min_x = MIN_L(min_x, x1); max_x = MAX_L(max_x, x2); min_y = MIN_L(min_y, y1); max_y = MAX_L(max_y, y2); \
} while(false);
#define CONTAINS(x, y) (zone.count(X_Y_TO_POS(x, y)) == 1)
#define REMOVE_BLOCK(x, y) (zone.erase(X_Y_TO_POS(x, y)));
#define REMOVE_RECT(x1, y1, w, h) do { for (int y = 0; y < h; y++) for (int x = 0; x < w; x++) zone.erase(X_Y_TO_POS(x1 + x, y1 + y)); } while(false);





const float SPEED = 3; // скорость перемещения (тайлов в секунду)
const float JUMP_SPEED = 2; // скорость прыгания (прыжков в секунду)
const bool tier2_dbg = false;
const byte rotate_dbg = 0;





const float PI = float(M_PI);
const float PI2 = float(M_PI * 2);
const float PI_180 = float(M_PI / 180);





void Celler::BFS(long8 EX, long8 EY) {
    BFS(X_Y_TO_POS(EX, EY));
}
void Celler::BFS(long8 pos) {
    uint arr_size = zone.size(), item = 0, size = 1, walk = 1, item_walk = 1;
    long8 *arr = new long8[arr_size];
    arr[0] = pos;
    const long8 dxys[] = {-0x100000000, -1, 1, 0x100000000};
    bfs.clear();
    bfs[pos] = {0, false, 0};

    if (targetEvent.target != Target::Clear || movementTarget == UNDEF_POS)
        if (zone.count(pos) == 0 || is_locked_plate(pos)) size = 0;

    while (item < size) {
        if (item >= item_walk) {
            item_walk = size;
            walk++;
        }
        pos = arr[item++];
        //PRINT_POS(pos)
        for (int dxy = 0; dxy < 4; dxy += 1) {
            long8 pos2 = pos + dxys[dxy];
            if (zone.count(pos2) == 1 && bfs.count(pos2) == 0 && !is_locked_plate(pos2)) {
                arr[size++] = pos2;
                bfs[pos2] = {byte(4 - dxy), false, walk};
            }
        }
    }
    delete[] arr;
}
void Celler::tier_1(long8 BX, long8 BY, long8 EX, long8 EY) {
    tier_1(X_Y_TO_POS(BX, BY), X_Y_TO_POS(EX, EY));
}
void Celler::tier_1(long8 begin, long8 end) {
    BFS(end);
    long8 pos = begin;
    const long8 dxys[] = {0, -0x100000000, -1, 1, 0x100000000};
    pos_arr_dist = bfs[pos].walk + 1;
    uint index = 0;
    pos_arr = static_cast<long8*>(malloc(sizeof(long8) * pos_arr_dist));
    while (true) {
        auto cell = bfs[pos];
        cell.pathed = true;
        bfs[pos] = cell;
        byte side = cell.side;
        pos_arr[index++] = pos;
        if (side == 0) break;

        pos += dxys[side];
    }
    //if (pos_arr_dist == 1) qDebug("Либо начало = конец, либо нет пути");
}

bool Celler::line_check(qreal x, qreal y, qreal x2, qreal y2) {
    //qDebug("num: -5.6 -> %lf %lf %d", floor(-5.6), ceil(-5.6), int(-5.6));
    //qDebug("num: 5.6 -> %lf %lf %d", floor(5.6), ceil(5.6), int(5.6));

    if (tier2_dbg) qDebug("dots: %lg %lg -> %lg %lg", x, y, x2, y2);
    const qreal dd = .000000001f;

    bool forward_x = x2 > x, forward_y = y2 > y;
    qreal add_x = forward_x ? dd : -dd, add_y = forward_y ? dd : -dd;
    int add2_x = forward_x ? 0 : -1, add2_y = forward_y ? 0 : -1;
    qreal k = (y2 - y) / (x2 - x);
    qreal m = y - k * x;
    //qDebug("k: %lf   m: %lf   y: %lf = %lf   y2: %lf = %lf", k, m, k * x + m, y, k * x2 + m, y2);

    for (int i = 0; i < 1000000; i++) {
        qreal nx = forward_x ? ceil(x) : floor(x), ny = k * nx + m, Vdist = pow(nx - x, 2) + pow(ny - y, 2);
        qreal ny2 = forward_y ? ceil(y) : floor(y), nx2 = (ny2 - m) / k, Hdist = pow(nx2 - x, 2) + pow(ny2 - y, 2);
        //qDebug("next (vert): %lf %lf (%lf)   next (horiz): %lf %lf (%lf)", nx, ny, Vdist, nx2, ny2, Hdist);
        int px, py;
        bool yeah;
        if (Vdist < Hdist) {
            x = nx + add_x; y = ny;
            px = int(round(nx)) + add2_x; py = int(floor(ny));
            yeah = forward_x != (x2 > x);
        } else {
            x = nx2; y = ny2 + add_y;
            px = int(floor(nx2)); py = int(round(ny2)) + add2_y;
            yeah = forward_y != (y2 > y);
        }
        if (yeah) return true;
        //qDebug("test_block: %d %d %s", px, py, CONTAINS(px, py) ? "YES" : "NO");
        if (!CONTAINS(px, py) || is_locked_plate(X_Y_TO_POS(px, py))) return false;
    }
    qWarning() << "РЕШЕНИЕ (line_check) ЗАВИСЛО :/";
    return false;
}
byte Celler::cells_check(long8 pos, long8 pos2) {
    int zx, zy, zx2, zy2;
    POS_TO_X_Y(zx, zy, pos)
    POS_TO_X_Y(zx2, zy2, pos2)
    if (tier2_dbg) qDebug("cells_check: %d %d -> %d %d", zx, zy, zx2, zy2);
    if (zx == zx2 && zy == zy2) return 1;
    if (zx == zx2) {
        if (zy2 >= zy) { for (int y = zy; y <= zy2; y++) if (!CONTAINS(zx, y)) return 0; }
        else for (int y = zy; y >= zy2; y--) if (!CONTAINS(zx, y)) return 0;
        return 1;
    }
    if (zy == zy2) {
        if (zx2 >= zx) { for (int x = zx; x <= zx2; x++) if (!CONTAINS(x, zy)) return 0; }
        else for (int x = zx; x >= zx2; x--) if (!CONTAINS(x, zy)) return 0;
        return 1;
    }

    qreal x = zx + .5f, y = zy + .5f, x2 = zx2 + .5f, y2 = zy2 + .5f;
    QVector2D dist(zx2 - zx, zy2 - zy);
    float len = .5f / dist.length();
    qreal dx = -dist.y() * len, dy = dist.x() * len;
    return line_check(x + dx, y + dy, x2 + dx, y2 + dy) && line_check(x - dx, y - dy, x2 - dx, y2 - dy) ? 2 : 0;
}
void Celler::tier_2() {
    uint dist = pos_arr_dist;
    pos_vec.clear();

    long8 current = pos_arr[0];
    pos_vec.push_back(current);

    long8 prev = 0;
    byte prev_res = 0;
    uint cur_index = 0;
    for (uint i = 1; i < dist; i++) {
        long8 next = pos_arr[i];
        byte res = cells_check(current, next);
        if (tier2_dbg) qDebug("res: %u", res);
        if (res != 0) {
            prev_res = prev_res == 2 ? 2 : res;
            prev = next;
            continue;
        }
        if (prev_res == 2) {
            current = prev;
            i--;
            cur_index = i;
        } else if (prev_res == 1) {
            i = cur_index;
            current = pos_arr[++cur_index];
        } else {
            qWarning() << "РЕШЕНИЕ (tier_2) ЗАВИСЛО :/";
            break;
        }
        pos_vec.push_back(current);
        prev_res = 0;
    }

    if (dist > 1) pos_vec.push_back(pos_arr[dist - 1]);

    if (rotate_dbg) {
        bool clockwise = false; //rand() % 2;
        //qDebug("CLOCK: %u", clockwise);
        pos_vec.clear();
        if (rotate_dbg > 1) {
            pos_vec.push_back(X_Y_TO_POS(0, 1));
            pos_vec.push_back(clockwise ? X_Y_TO_POS(1, 2) : X_Y_TO_POS(-1, 2));
            pos_vec.push_back(X_Y_TO_POS(0, 3));
            pos_vec.push_back(clockwise ? X_Y_TO_POS(-1, 2) : X_Y_TO_POS(1, 2));
            pos_vec.push_back(X_Y_TO_POS(0, 1));
        } else {
            pos_vec.push_back(X_Y_TO_POS(1, 1));
            pos_vec.push_back(clockwise ? X_Y_TO_POS(1, 3) : X_Y_TO_POS(-1, 1));
            pos_vec.push_back(X_Y_TO_POS(-1, 3));
            pos_vec.push_back(clockwise ? X_Y_TO_POS(-1, 1) : X_Y_TO_POS(1, 3));
            pos_vec.push_back(X_Y_TO_POS(1, 1));
        }
    }
}





Celler::Celler(QPaintDevice *pv) {
    parent = pv;

    rectBrush = QBrush(QColor(230, 245, 255));
    rectBrush2 = QBrush(QColor(255, 255, 200));
    redBrush = QBrush(Qt::red);
    rectPen = QPen(Qt::blue);
    rectPen.setWidth(1);
    textPen = QPen(QColor(255, 128, 0));
    arrowPen = QPen(Qt::darkGreen);
    arrowPen.setWidth(2);

    circleBrush = QBrush(QColor(255, 230, 196));
    circlePen = QPen(QColor(255, 64, 128));
    circlePen.setWidth(2);
    circleBrush2 = QBrush(QColor(64, 255, 64));
    circlePen2 = QPen(QColor(64, 64, 255));
    circlePen2.setWidth(1);

    linePen = QPen(QColor(64, 64, 64));
    linePen.setWidth(4);

    ADD_RECT(-6, -1, 6, 1)
    ADD_RECT(-1, -9, 1, 9)
    ADD_BLOCK(-4, -2)
    ADD_BLOCK(4, -2)
    ADD_BLOCK(-4, 2)
    ADD_BLOCK(4, 2)
    ADD_RECT(-5, -4, -3, -3)
    ADD_RECT(3, -4, 5, -3)
    ADD_RECT(-5, 3, -3, 4)
    ADD_RECT(3, 3, 5, 4)
    ADD_BLOCK(-2, -7)
    ADD_BLOCK(2, -7)
    ADD_BLOCK(-2, 7)
    ADD_BLOCK(2, 7)
    ADD_RECT(-4, -8, -3, -6)
    ADD_RECT(3, -8, 4, -6)
    ADD_RECT(-4, 6, -3, 8)
    ADD_RECT(3, 6, 4, 8)
    REMOVE_RECT(-2, 0, 5, 1)
    REMOVE_BLOCK(2, 1)
    REMOVE_BLOCK(-2, -1)
    ADD_RECT(-6, -7, -5, -5)

    //for (long8 y = min_y; y <= max_y; y++)
    //    for (long8 x = min_x; x <= max_x; x++) qDebug("%lld %lld = %s", x, y, CONTAINS(x, y) ? "+" : "-");

    recalc_border();

    pet_pos = rand_pos();
    POS_TO_X_Y(anim_x, anim_y, pet_pos)

    fart.reset();
}
Celler::~Celler() {
    if (pos_arr != nullptr) free(pos_arr);
    if (set2arr != nullptr) free(set2arr);
    if (border != nullptr) free(border);

    for (auto plant : plant_types) {
        delete plant.part;
        delete plant.planted;
    }
    for (auto pair : plants) {
        auto plant = pair.second;
        delete plant->type;
        delete plant;
     }
    if (plant_arr != nullptr) free(plant_arr);
    if (waste != nullptr) delete waste;
}

void Celler::recalc_border() {
    if (set2arr != nullptr) free(set2arr);
    if (border != nullptr) free(border);

    std::set<long8> bord;
    const long8 dxys[] = {-0x100000000, -1, 1, 0x100000000};

    uint size = plate_size = uint(zone.size());
    set2arr = static_cast<long8*>(malloc(sizeof(long8) * size));
    uint pos = 0;
    for (long8 item : zone) {
        set2arr[pos++] = item;
        for (int dxy = 0; dxy < 4; dxy++) {
            long8 pos2 = item + dxys[dxy];
            if (!zone.count(pos2)) bord.insert(pos2);
        }
    }

    size = border_size = uint(bord.size());
    border = static_cast<long8*>(malloc(sizeof(long8) * size));
    pos = 0;
    for (long8 item : bord) border[pos++] = item;
}

void Celler::drawArrow(QPainter &painter, QRectF &rect, byte side) {
    painter.setPen(arrowPen);
    painter.translate(rect.center());
    painter.scale(rect.width() / 50, rect.height() / 50);
    if (side == 0) {
        painter.drawLine(QLineF(-10, -10, 10, 10));
        painter.drawLine(QLineF(-10, 10, 10, -10));
        painter.resetTransform();
        return;
    }
    if (side != 1) painter.rotate(side == 2 ? -90 : side == 3 ? 90 : 180);

    const qreal x1 = -5, x2 = 0, x3 = 5;
    const qreal y1 = -15, y2 = -5, y3 = 10;
    painter.drawLine(QLineF(x1, y2, x2, y1));
    painter.drawLine(QLineF(x2, y1, x3, y2));
    painter.drawLine(QLineF(x1, y2, x3, y2));
    painter.drawLine(QLineF(x2, y1, x2, y3));

    painter.resetTransform();
}

QRectF Circle(QPointF center, qreal R) {
    return QRectF(center.x() - R, center.y() - R, R * 2, R * 2);
}

void drawModelBox(QPainter &painter, renderable_p cur_model, float X, float Y, float WH, float yaw = 39.3f, float pitch = -35.4f, float roll = 0, float R = 2.5f, float Down = 0.2f) {
    int WHi = int(WH);
    painter.beginNativePainting();
    //glPushAttrib(GL_ALL_ATTRIB_BITS);

    glEnable(GL_DEPTH_TEST);
    QImage res = cur_model->toImage(WHi, WHi, yaw, pitch, roll, R, Down);
    glDisable(GL_DEPTH_TEST);

    //glPopAttrib();
    painter.endNativePainting();

    painter.drawImage(QRectF(X, Y, WHi, WHi), res);
}

void Celler::draw(renderable_p *model_base, bool view_2d_place, bool view_tier2, QString text) {
    int W = parent->width(), H = parent->height();
    QPainter painter;
    painter.begin(parent);

    if (view_2d_place) {
        long8 sx = max_x - min_x + 1, sy = max_y - min_y + 1;

        qreal intercell = 1;
        qreal width = 10;
        qreal ssx = intercell + (intercell + width) * sx;
        qreal ssy = intercell + (intercell + width) * sy;
        qreal scale = MIN_Q(W / ssx, H / ssy);
        intercell *= scale;
        width *= scale;
        qreal iw = intercell + width;
        qreal dx = MAX_Q(0, W - iw * sx);

        textFont.setPixelSize(MAX(int(width / 3), 6));
        textFont.setBold(true);
        painter.setFont(textFont);

        QFontMetrics metrics(textFont);
        int one_height = metrics.height();
        int align = Qt::AlignRight | Qt::AlignBottom | Qt::TextSingleLine;

        for (long8 y = 0; y < sy; y++) {
            qreal py = intercell + iw * y;
            long8 zy = y + min_y;
            for (long8 x = 0; x < sx; x++) {
                long8 zx = x + min_x;
                if (!CONTAINS(zx, zy)) continue;

                long8 pos = X_Y_TO_POS(zx, zy);
                auto dxy = bfs[pos];
                qreal px = dx + iw * x;
                painter.setPen(rectPen);
                QRectF rect(px, py, width, width);
                painter.setBrush(is_locked_plate(pos) ? redBrush : dxy.pathed ? rectBrush2 : rectBrush);
                painter.drawRect(rect);
                painter.setPen(textPen);

                QString text = QString::number(dxy.walk);
                int twidth = metrics.width(text) + 1;
                painter.drawText(QRectF(px + width - twidth, py + width - one_height, twidth, one_height), align, text);

                drawArrow(painter, rect, dxy.side);
            }
        }

        uint count = view_tier2 ? uint(pos_vec.size()) : 0;
        QRectF prev;
        qreal R = width / 2, R8 = R / 8;
        for (uint i = 0; i < count; i++) {
            long8 pos = pos_vec[i];
            int zx, zy;
            POS_TO_X_Y(zx, zy, pos)

            long8 x = zx - min_x, y = zy - min_y;
            qreal px = dx + iw * x;
            qreal py = intercell + iw * y;
            QRectF rect(px, py, width, width);
            painter.setPen(circlePen);
            painter.setBrush(circleBrush);
            painter.drawEllipse(rect);
            if (i) {
                QPointF cA = prev.center(), cB = rect.center();
                QVector2D line(cB - cA);
                qreal dist = R / line.length();
                QPointF c_delta(-line.y() * dist, line.x() * dist);
                QPointF cAa = cA + c_delta, cAb = cA - c_delta;
                QPointF cBa = cB + c_delta, cBb = cB - c_delta;

                painter.setPen(linePen);
                painter.drawLine(QLineF(cAa, cBa));
                painter.drawLine(QLineF(cAb, cBb));
                painter.setPen(circlePen2);
                painter.setBrush(circleBrush2);
                if (i == 1) painter.drawEllipse(Circle(cA, R8));
                painter.drawEllipse(Circle(cAa, R8));
                painter.drawEllipse(Circle(cAb, R8));
                painter.drawEllipse(Circle(cB, R8));
                painter.drawEllipse(Circle(cBa, R8));
                painter.drawEllipse(Circle(cBb, R8));
            }

            prev = rect;
        }

        painter.setPen(rectPen);
        painter.setBrush(rectBrush);
        qreal diam = iw * 2 + width;
        R = diam / 2; R8 = R / 8;
        qreal ir = intercell + R8;
        diam -= R8 * 2; R = diam / 2;
        QRectF circle_rect(ir, ir, diam, diam);
        painter.drawEllipse(circle_rect);

        QPointF center = circle_rect.center() + QPointF(cos(anim_angle * PI_180), sin(anim_angle * PI_180)) * R;
        painter.setBrush(rectBrush2);
        painter.drawEllipse(Circle(center, R8));

        center = circle_rect.center() + QPointF(cos(fact_angle * PI_180), sin(fact_angle * PI_180)) * R;
        painter.setBrush(circleBrush);
        painter.drawEllipse(Circle(center, R8));
    }

    int align = Qt::AlignLeft | Qt::AlignBottom;

    int textSize = MAX(int(H / 50), 6);
    textFont.setPixelSize(textSize);
    textFont.setBold(true);
    painter.setFont(textFont);

    QFontMetrics metrics(textFont);
    int w = 0, h = metrics.height() * (1 + text.count("\n"));
    for (auto line : text.split("\n")) w = MAX(w, metrics.width(line));

    QRectF textBox(0, H - h - 4, w + 4, h + 4);

    painter.fillRect(textBox, QBrush(QColor(0, 0, 0, 128)));
    textBox += QMarginsF(-4, 2, 4, -2);

    /*painter.setPen(QPen(QColor(0, 0, 0)));
    painter.drawText(textBox + QMarginsF(1, 0, 0, 1), align, text);
    painter.drawText(textBox + QMarginsF(1, 0, 0, -1), align, text);
    painter.drawText(textBox + QMarginsF(-1, 0, 0, 1), align, text);
    painter.drawText(textBox + QMarginsF(-1, 0, 0, -1), align, text);
    painter.drawText(textBox + QMarginsF(0, 0, 0, 1), align, text);
    painter.drawText(textBox + QMarginsF(1, 0, 0, 0), align, text);
    painter.drawText(textBox + QMarginsF(0, 0, 0, -1), align, text);
    painter.drawText(textBox + QMarginsF(-1, 0, 0, 0), align, text);*/

    textFont.setBold(false);
    painter.setPen(QPen(QColor(255, 255, 255)));
    painter.drawText(textBox, align, text);

    float rectH = MIN(W, H) / 5.f;
    auto mainRect = QRectF(0, rectH, rectH, rectH / 3 * 4);
    auto mainBrush = QBrush(QColor(255, 255, 0, 64));
    painter.setBrush(mainBrush);
    painter.setPen(QColor(64, 128, 255));
    painter.drawRect(mainRect);
    float boxH = rectH / 3.5f, boxAlign = boxH / 2 / 4, boxAH = boxH + boxAlign;
    float boxY[] = { boxAlign + rectH, boxAlign + rectH + boxAH, boxAlign + rectH + boxAH * 2, boxAlign + rectH + boxAH * 3 };
    float boxX2 = boxAlign + boxAH;

    drawModelBox(painter, model_base[190], boxAlign, boxY[0], boxH,  94.5,  -39.6f, -48, 0.855f, 0.15f);
    drawModelBox(painter, model_base[227], boxAlign, boxY[1], boxH,  37.4f, -36.4f,   7, 2.38f,  0.51f);
    drawModelBox(painter, model_base[250], boxAlign, boxY[2], boxH, -90,     30,      0, 1.4f,   0.23f);
    drawModelBox(painter, model_base[254], boxAlign, boxY[3], boxH,  89,      1,     17, 1.69f,  0.02f);

    align = Qt::AlignCenter | Qt::TextSingleLine;
    h = metrics.height();
    float boxW = boxH * 2 + boxAlign;

    for (int i = 0; i < 4; i++) {
        QString prop = QString::number(props[i]);
        int textW = metrics.width(prop);
        float mul = boxW / textW, h2 = mul * h;
        float mul2 = h2 > boxH ? boxH / h2 : 1;
        textFont.setPixelSize(int(mul * textSize * mul2));
        painter.setFont(textFont);
        painter.drawText(QRectF(boxX2, boxY[i], boxW, boxH), align, prop);
    }
    painter.end();
}



long8 Celler::rand_pos() {
    auto size = zone.size();
    int pos = rand() % int(size);
    return set2arr[pos];
}

long8 Celler::smart_rand_pos() {
    BFS(pet_pos);
    std::set<long8> access;
    for (auto pair : bfs) {
        long8 pos = pair.first;
        bfs_data data = pair.second;
        if (data.walk) access.insert(pos);
    }
    auto size = access.size();
    if (size == 0) return pet_pos;

    int idx = 0, pos = rand() % int(size);
    for (long8 res : access)
        if (idx++ >= pos) return res;
    return pet_pos; // а вообще это deadcode с точки зрения разума
}

void Celler::get_pet_pos(float td, float &px, float &py, float &pz, float &angle) {
    float fatmul = fatigue_multiplier();
    float SPEED_M = SPEED * fatmul, JUMP_SPEED_M = JUMP_SPEED / fatmul;

    switch (anim_state) {
    case 0: state_0:
    case 1: {
        if (anim_state == 0) {
            anim_path_n = 0;
            anim_jump = 0;
            full_td = 0;
        }

        bool forced = movementTarget != UNDEF_POS;
        bool recalc = forced && target_pos != movementTarget;
        if (pet_pos == movementTarget) {
            if (forced) target_event_handler(targetEvent);
            movementTarget = UNDEF_POS;
        }
        if (anim_path_n + 1 >= pos_vec.size() || recalc) {
            if (abs(anim_jump) > .000001f) {
                anim_state = 3;
                break;
            }
            pet_pos = X_Y_TO_POS(round(anim_x), round(anim_y));
            target_pos = recalc ? movementTarget : smart_rand_pos();
            tier_1(pet_pos, target_pos);
            tier_2();
            anim_state = 0;
            if (pos_vec.size() <= 1) {
                if (recalc) movementTarget = UNDEF_POS; // антиступор
                break;
            }
            goto state_0;
        }

        long8 pos = pos_vec[anim_path_n];
        long8 pos2 = pos_vec[++anim_path_n];

        POS_TO_X_Y(begin_x, begin_y, pos)
        POS_TO_X_Y(end_x, end_y, pos2)

        float dx = end_x - begin_x, dy = end_y - begin_y;
        anim_L = hypotf(dx, dy);
        anim_td = 0;
        float angle = acos(dx / anim_L);
        if (dy < 0) angle = PI2 - angle;

        anim_x = begin_x;
        anim_y = begin_y;
        anim_angle = float(angle / PI_180);

        anim_state = 2;

        break; }
    case 2: {
        anim_td += td;
        full_td += td * fatmul;

        float part = MIN_F(1, anim_td * SPEED_M / anim_L);
        anim_x = begin_x + (end_x - begin_x) * part;
        anim_y = begin_y + (end_y - begin_y) * part;
        anim_jump = abs(sin(full_td * PI * JUMP_SPEED_M)) / JUMP_SPEED_M;

        if (part >= 1) {
            anim_state = 1;
            pet_pos = X_Y_TO_POS(end_x, end_y);
        }
        break; }
    case 3:
        float end = ceil(full_td * JUMP_SPEED_M) * PI;
        full_td += td * fatmul;
        anim_jump = abs(sin(MIN_F(full_td * PI * JUMP_SPEED_M, end))) / JUMP_SPEED_M;
        if (abs(anim_jump) < .000001f) anim_state = 1;
        break;
    }
    px = anim_x;
    py = anim_jump;
    pz = anim_y;

    if (rotate_dbg) qDebug("angle: %f %f | %f", fact_angle, anim_angle, td);
    float angle_L = MIN_F(abs(fact_angle - anim_angle), 360 - abs(fact_angle - anim_angle));
    float angle2 = fmodf(fact_angle + 1, 360);
    float angle_L2 = MIN_F(abs(angle2 - anim_angle), 360 - abs(angle2 - anim_angle));
    float angle_speed = MAX_F(angle_L / 60, 1) * 360;
    if (angle_L2 < angle_L)
        fact_angle = fmodf(fact_angle + MIN_F(angle_speed * td, angle_L), 360);
    else {
        fact_angle = fmodf(fact_angle - MIN_F(angle_speed * td, angle_L), 360);
        while (fact_angle < 0) fact_angle += 360;
    }

    angle = -fact_angle;
    //qDebug("pet anim: %f %f %f (%f)", px, py, pz, angle);
}
bool Celler::set_target(long8 pos, Target target) { // ещё работает, как аварийная смена конечной точки анимации, не ломая при этом анимацию
    if (movementTarget != UNDEF_POS && is_locked_plate(movementTarget)) return false;

    movementTarget = pos;
    targetEvent.target = target;

    begin_x = anim_x; begin_y = anim_y;
    pet_tiled_pos(end_x, end_y);

    float dx = end_x - begin_x, dy = end_y - begin_y;
    anim_L = hypotf(dx, dy);
    if (anim_L > 0.00001) {
        float angle = acos(dx / anim_L);
        if (dy < 0) angle = PI2 - angle;
        anim_angle = float(angle / PI_180);
    }
    anim_td = 0;

    return true;
}





static bool is_build_dir = QFile(".qmake.stash").exists();
static text file_name = is_build_dir ? "../progress.asd" : "progress.asd";

void Celler::bestowProgress() {
    QStringList data;

    QStringList data_A;
    int x, y;
    pet_tiled_pos(x, y);
    data_A.append(QString::number(pet_id));
    data_A.append(QString::number(x));
    data_A.append(QString::number(y));
    for (int i = 0; i < 4; i++) data_A.append(QString::number(props[i]));
    data.append(data_A.join(";"));

    QStringList data_B;
    for (auto pair: plants) {
        long8 pos = pair.first;
        plant *data = pair.second;
        if (!data->ready()) {
            int elapsed = data->time.elapsed();
            data_B.append(QString::number(pos) + "," + QString::number(elapsed));
        }
    }
    data.append(data_B.join(";"));

    QStringList data_C;
    for (auto pair: wastes) {
        long8 pos = pair.first;
        int power = pair.second.power;
        data_C.append(QString::number(pos) + "," + QString::number(power));
    }
    data.append(data_C.join(";"));

    QString final = data.join("|");
    //return;
    QFile file(file_name);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        stream << final << "\n" << pet_name;
        file.close();
    } else QMessageBox::warning(nullptr, "Сохранятор", "Не получилось даровать прогресс :/");
}

void waste_click_event_wrap(void *inst, ModelEventData &event);
void Celler::replicateProgress() {
    if (false) { error: // Интересный способ слегка переопределить return :D
        QMessageBox::warning(nullptr, "Открывашка", "Не получилось реплицировать прогресс :/");
        return;
    }
    QFile file(file_name);
    if (!file.exists()) return;

    QString progress;
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        progress = stream.readAll();
        file.close();
    } else goto error;

    qDebug() << progress;
    QStringList lines = progress.split("\n");
    QStringList data = lines[0].split("|");
    QStringList data_A = data[0].split(";");
    if (lines.size() != 2 || data.size() != 3 || data_A.size() != 7) goto error;

    pet_id = data_A[0].toInt();
    anim_x = data_A[1].toInt(); anim_y = data_A[2].toInt();
    pet_pos = X_Y_TO_POS(anim_x, anim_y);
    anim_state = 0; // На всякий пожарный... ;'-}
    for (int i = 0; i < 4; i++) props[i] = data_A[i + 3].toInt();
    pet_name = lines[1];

    if (data[1].length() > 0)
        for (QString block: data[1].split(";")) {
            QStringList pair = block.split(",");
            if (pair.size() != 2) goto error;
            long8 pos = pair[0].toLongLong();
            int elapsed = pair[1].toInt();
            plants[pos]->setElapsed(elapsed);
        }

    if (data[2].length() > 0)
        for (QString block: data[2].split(";")) {
            QStringList pair = block.split(",");
            if (pair.size() != 2) goto error;
            long8 pos = pair[0].toLongLong();
            int power = pair[1].toInt();

            auto model = new model_wrapper(waste);
            model->bind_event(this, waste_click_event_wrap, ModelEvents::Click);
            model->set_option(power - 1);
            wastes[pos] = { power, model };
            waist2pos[model] = pos;
        }
}





#define NEW_TYPE(id, mid) do { \
    auto part = new model_union(7, new renderable_p[7] { model_base[170], model_base[mid + 5], model_base[mid], model_base[mid + 1], model_base[mid + 2], model_base[mid + 3], model_base[mid + 4] }); \
    auto planted = new model_group(2, new renderable_p[4] { model_base[157], part }, false ); \
    plant_types[id] = { part, planted }; \
} while(false);
#define ADD_PLANT(x, y, type) do { \
    auto model = new model_wrapper(plant_types[type].planted); \
    plant *p = new plant { x, y, QTime(), model }; \
    plants[X_Y_TO_POS(x, y)] = p; \
    model2plant[model] = p; \
    model->bind_event(this, plant_click_event_wrap, ModelEvents::Click); \
} while(false);
#define ADD_PLANTS(x1, y1, w, h, type) do { \
    for (int y = 0; y < h; y++) for (int x = 0; x < w; x++) ADD_PLANT(x + x1, y + y1, type); \
} while(false);

#define WRAPPER(T, name, code) void T::name() code \
void name##_wrap(void *inst) { static_cast<T*>(inst)->name(); }
#define WRAPPER_ARG_1(T, name, arg_T, arg, code) void T::name(arg_T arg) code \
void name##_wrap(void *inst, arg_T arg) { static_cast<T*>(inst)->name(arg); }

//WRAPPER_ARG_1(Celler, target_event_handler, Celler::TargetEvent&, event, {});
void Celler::target_event_handler(Celler::TargetEvent &event) {
    switch(event.target) {
    case Target::Unknown: return;
    case Target::Eat: {
        auto plant = event.plant;
        //qDebug("plant eated: %d %d", plant->x, plant->y);
        if (props[1] > 2) { // бодрство
            props[1] -= 3;  // бодрство
            if (plant->ready()) plant->reset();
            props[0] += 3;  // еда
            props[2]++;     // деньги
        }
        break; }
    case Target::Clear: {
        auto waste = event.waste;
        if (props[1] > 1) { // бодрство
            props[1] -= 2;  // бодрство
            long8 pos = movementTarget;
            wastes.erase(pos);
            waist2pos.erase(waste->model);
            //if (waste->model != nullptr) delete waste->model; Удаление модельки почему-то приводит к фаталити :/
            props[2]++;     // деньги
        }
        fart.reset();
        break; }
    case Target::Sleep: {
        //renderable_p bed = event.bed;
        if (props[1] <= 0) { // бодрство
            props[1] = 99;   // бодрство
            props[2] += 10;  // деньги
            props[3]++;      // сон
        }
        break; }
    }
    targetEvent.target = Target::Unknown;
    bestowProgress();
}

//const text type_names[] = {"Hover", "Hovered", "Unhover", "Press", "Move", "Release", "Click"};
WRAPPER_ARG_1(Celler, plant_click_event, ModelEventData&, event, {
    //if (event.type != 1) qDebug("test func is called ;'-} %d %d %d %s", event.x, event.y, event.mouse_button, type_names[event.type]);

    plant *p = model2plant[event.model];
    if (p->ready()) {
        //qDebug("x, y: %d %d", p->x, p->y);
        if (set_target(X_Y_TO_POS(p->x, p->y), Eat)) targetEvent.plant = p;
    }
});



void Celler::bed_click_event(ModelEventData &event) {
    long8 pos = bed2pos[event.model];
    if (set_target(pos, Target::Sleep)) targetEvent.bed = event.model;
}
void bed_click_event_wrap(void *inst, ModelEventData &event) { static_cast<Celler*>(inst)->bed_click_event(event); }

#define ADD_BED(x, y) do { \
    long8 pos = X_Y_TO_POS(x, y); \
    auto model = new model_wrapper(model_base[227]); \
    beds[pos] = model; \
    bed2pos[model] = pos; \
    model->bind_event(this, bed_click_event_wrap, ModelEvents::Click); \
} while(false);
// TODO нет очиситки из памяти кроватей



void Celler::init_plants(renderable_p *model_base) {
    NEW_TYPE(0, 110)
    NEW_TYPE(1, 116)
    NEW_TYPE(2, 122)
    NEW_TYPE(3, 128)
    NEW_TYPE(4, 134)
    NEW_TYPE(5, 140)

    //ADD_PLANT(0, 1, 0)
    //ADD_PLANT(1, 1, 0)
    ADD_PLANTS(4, 6, 1, 3, 0)
    ADD_PLANTS(-4, 6, 2, 3, 1)
    ADD_PLANTS(-5, 4, 3, 1, 2)
    ADD_PLANTS(3, 3, 3, 2, 3)
    ADD_PLANTS(6, -1, 1, 3, 4)
    ADD_PLANTS(-6, -1, 1, 3, 5)
    ADD_PLANTS(3, -8, 2, 3, 0)
    ADD_PLANTS(3, -4, 1, 2, 1)
    ADD_PLANTS(5, -4, 1, 2, 2)

    uint size = plant_size = uint(plants.size()), pos = 0;
    plant_arr = static_cast<plant**>(malloc(sizeof(plant*) * size));
    for (auto plant : plants) plant_arr[pos++] = plant.second;

    // wastes init:

    waste = new model_union(5, new renderable_p[5] { model_base[165], model_base[166], model_base[167], model_base[168], model_base[169] });

    ADD_BED(1, 1)
    ADD_BED(-1, -1)

    replicateProgress();
}





void Celler::waste_click_event(ModelEventData &event) {
    long8 pos = waist2pos[event.model];
    waste_data data = wastes[pos];

    if (set_target(pos, Target::Clear)) targetEvent.waste = &data;
}
void waste_click_event_wrap(void *inst, ModelEventData &event) { static_cast<Celler*>(inst)->waste_click_event(event); }

std::map<long8, Celler::waste_data>* Celler::fart_check() {
    int x, y;
    pet_tiled_pos(x, y);
    long8 pos = X_Y_TO_POS(x, y);
    if (anim_state == 2 && (movementTarget == UNDEF_POS || props[1] < 50) && fart.ready() && zone.count(pos)) {
        waste_data data = wastes[pos];
        if (data.power < 5) {
            if (data.model == nullptr) {
                data.model = new model_wrapper(waste);
                data.model->bind_event(this, waste_click_event_wrap, ModelEvents::Click);
                waist2pos[data.model] = pos;
            }
            data.model->set_option(data.power++);
            wastes[pos] = data;
            if (props[0] > 0) props[0]--;
            if (props[1] > 0) props[1]--;
            fart.reset();
            bestowProgress();
        }
    }
    return &wastes;
}
