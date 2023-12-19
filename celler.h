#ifndef CELLER_H
#define CELLER_H

#include <QPaintDevice>
QT_BEGIN_NAMESPACE
class QPaintDevice;
QT_END_NAMESPACE

#include <QPainter>
#include <QDebug>
#include <QVector2D>
#include <QTime>
#include <QMessageBox>

#include <set>
#include <map>
#include <vector>
#include <math.h> // ceil, floor

#include <asdloader.h>

typedef unsigned char byte;
typedef unsigned short ushort;
typedef long long int long8;

#define UNDEF_POS 0x7fffffffffffffff
#define X_Y_TO_POS(x, y) (long8(x) + 0x80000000 + long8(y) * 0x100000000)
#define PRINT_POS(pos) (qDebug("pos: %d %d", int((pos & 0xffffffff) - 0x80000000), int(pos >> 32)));
#define POS_TO_X_Y(x, y, pos) do { x = int((pos & 0xffffffff) - 0x80000000); y = int(pos >> 32); } while(false);

static int props[4] = { 99, 99, 0, 0 };

class Celler {
private:
    QPaintDevice *parent;
    QBrush rectBrush;
    QBrush rectBrush2;
    QBrush redBrush;
    QPen rectPen;
    QPen textPen;
    QFont textFont;
    QPen arrowPen;
    QBrush circleBrush;
    QPen circlePen;
    QBrush circleBrush2;
    QPen circlePen2;
    QPen linePen;

    std::set<long8> zone;
    long8 min_x = 0, min_y = 0, max_x = -1, max_y = -1;
    long8 pet_pos, target_pos;

    struct bfs_data {
       byte side;
       bool pathed; // пропутёвано ;'-}
       uint walk;
    };
    std::map<long8, bfs_data> bfs;
    void BFS(long8 EX, long8 EY);
    void BFS(long8 pos);
    void drawArrow(QPainter &painter, QRectF &rect, byte side);

    long8 *pos_arr = nullptr;
    uint pos_arr_dist;
    void tier_1(long8 BX, long8 BY, long8 EX, long8 EY);
    void tier_1(long8 begin, long8 end);

    std::vector<long8> pos_vec;
    bool line_check(qreal x, qreal y, qreal x2, qreal y2);
    byte cells_check(long8 pos, long8 pos2);
    void tier_2();

    long8 *set2arr = nullptr;
    long8 *border = nullptr;
    uint plate_size, border_size;
    void recalc_border();
    long8 rand_pos(), smart_rand_pos();

    int anim_state = 0;
    uint anim_path_n;
    float begin_x, begin_y; int end_x, end_y;
    float anim_x = 0, anim_y = 1, anim_angle = 90, anim_jump = 0, anim_L, anim_td, full_td, fact_angle = 0;
public:
    void get_pet_pos(float td, float &px, float &py, float &pz, float &angle);

private:
    static const int grove_wait = 5 * 60;
    struct plant_type {
        renderable_p part, planted;
    };
    struct plant {
        int x, y;
        QTime time;
        renderable_p type;
        bool first = true;
        int state() { int t = int(time.elapsed() / 1000.f / (grove_wait / 6.f)); return t > 6 || first ? 6 : t; }
        bool ready() { return state() == 6; }
        void reset() { time.start(); first = false; }
        void setElapsed(int elapsed) { first = false; time.start(); time = time.addMSecs(-elapsed); /* qDebug("elapsed %u -> %u", elapsed, time.elapsed()); */ }
    };
    struct waste_data {
        int power = 0;
        renderable_p model = nullptr;
    };
public:
    enum Target {
        Unknown,
        Eat,
        Clear,
        Sleep,
    };
    struct TargetEvent {
        Target target;
        plant *plant = nullptr;
        waste_data *waste = nullptr;
        renderable_p bed = nullptr;
    };
private:
    const static int plant_type_count = 6;
    plant_type plant_types[plant_type_count];
    std::map<long8, plant*> plants;
    uint plant_size;
    plant** plant_arr = nullptr;
    std::map<renderable_p, plant*> model2plant;

    long8 movementTarget = UNDEF_POS;
    TargetEvent targetEvent;
    bool set_target(long8 pos, Target target);
public:
    void init_plants(renderable_p *model_base);

    void plant_click_event(ModelEventData &event);
    void waste_click_event(ModelEventData &event);
    void   bed_click_event(ModelEventData &event);

    void target_event_handler(TargetEvent &event);
private:
    struct {
        QTime time;
        void reset() { time.start(); }
        bool ready() { return time.elapsed() / 1000.f > (props[0] <= 0 ? 1 : props[1] <= 0 ? 2 : 3); }
    } fart;
    renderable_p waste = nullptr;
    std::map<long8, waste_data> wastes;
    std::map<renderable_p, long8> waist2pos;

    std::map<long8, renderable_p> beds;
    std::map<renderable_p, long8> bed2pos;

    // 99 -> 1/1
    //  1 -> 1/1.5
    //  0 -> 1/3
    float fatigue_multiplier() { return 1 / (props[1] <= 0 ? 3 : 1 + (1 - (props[1] - 1) / 98.f) / 2); }

    int pet_id = 0;
    QString pet_name = "Noname";
    void bestowProgress();
    void replicateProgress();
public:
    Celler(QPaintDevice *pv);
    ~Celler();
    void draw(renderable_p *model_base, bool view_2d_place, bool view_tier2, QString text);
    long8* get_plates(uint &size) { size = plate_size; return set2arr; }
    long8* get_borders(uint &size) { size = border_size; return border; }
    plant** get_plants(uint &size) { size = plant_size; return plant_arr; }
    std::map<long8, waste_data>* fart_check();
    auto get_beds() { return beds; }

    void pet_tiled_pos(int &x, int &y) {
        x = int(round(anim_x));
        y = int(round(anim_y));
    }
    bool is_locked_plate(long8 pos) {
        int x, y;
        pet_tiled_pos(x, y);
        return pos != X_Y_TO_POS(x, y) && wastes.count(pos) && wastes[pos].power >= 5;
    }

    int get_pet_id() { return pet_id; }
    void set_pet_id(int id) { pet_id = id; bestowProgress(); }
    QString& get_pet_name() { return pet_name; }
    void set_pet_name(QString& name) { pet_name = name; bestowProgress(); }
};

#endif // CELLER_H
