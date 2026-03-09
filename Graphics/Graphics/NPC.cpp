#include "NPC.h"
#include "glut.h"
#include <cmath>
#include <algorithm>

NPC::NPC(double startX, double startY, int startHp, int t, int type)
    : x(startX), y(startY), hp(startHp), maxHp(startHp),
      team(t), npcType(type), alive(true), pathIndex(0)
{}

bool NPC::PlanPathTo(int targetRow, int targetCol)
{
    path.clear();
    pathIndex = 0;
    int sr = getGridRow();
    int sc = getGridCol();
    return FindPath(sr, sc, targetRow, targetCol, path);
}

bool NPC::FollowPlannedPath(double minDist)
{
    if (path.empty() || pathIndex >= (int)path.size())
        return true;

    auto& wp = path[pathIndex];
    // Centre of this grid cell (col+0.5, row+0.5)
    double tx = wp.second + 0.5;
    double ty = wp.first  + 0.5;

    double dx = tx - x;
    double dy = ty - y;
    double dist = std::sqrt(dx*dx + dy*dy);

    if (dist < minDist)
    {
        pathIndex++;
        if (pathIndex >= (int)path.size())
            return true;
        return false;
    }

    x += NPC_SPEED * (dx / dist);
    y += NPC_SPEED * (dy / dist);
    return false;
}

int NPC::getCurrentRoom() const
{
    int r = getGridRow();
    int c = getGridCol();
    if (r < 0 || r >= MSZ || c < 0 || c >= MSZ) return -1;
    return roomId[r][c];
}

void NPC::takeDamage(int dmg)
{
    if (!alive) return;
    hp -= dmg;
    if (hp <= 0) { hp = 0; alive = false; }
}

void NPC::heal(int amount)
{
    if (!alive) return;
    hp += amount;
    if (hp > maxHp) hp = maxHp;
}

// ---- drawBase: circular NPC body with 3-D lighting effect ----
void NPC::drawBase(double cr, double cg, double cb, char symbol, double sz) const
{
    if (!alive) return;

    const int   SEGS   = 18;
    const double TWO_PI = 6.28318530717959;

    // --- Drop shadow (slightly offset circle) ---
    glColor3d(0.0, 0.0, 0.0);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2d(x + sz * 0.22, y - sz * 0.22);
    for (int i = 0; i <= SEGS; i++)
    {
        double a = TWO_PI * i / SEGS;
        glVertex2d(x + sz*0.22 + sz * std::cos(a),
                   y - sz*0.22 + sz * std::sin(a));
    }
    glEnd();

    // --- Outer dark ring ---
    glColor3d(cr * 0.52, cg * 0.52, cb * 0.52);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2d(x, y);
    for (int i = 0; i <= SEGS; i++)
    {
        double a = TWO_PI * i / SEGS;
        glVertex2d(x + sz * std::cos(a), y + sz * std::sin(a));
    }
    glEnd();

    // --- Main body (team colour) ---
    glColor3d(cr, cg, cb);
    double inner = sz * 0.80;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2d(x, y);
    for (int i = 0; i <= SEGS; i++)
    {
        double a = TWO_PI * i / SEGS;
        glVertex2d(x + inner * std::cos(a), y + inner * std::sin(a));
    }
    glEnd();

    // --- Specular highlight (small bright circle, top-left) ---
    double hlr = std::min(cr + 0.42, 1.0);
    double hlg = std::min(cg + 0.42, 1.0);
    double hlb = std::min(cb + 0.42, 1.0);
    glColor3d(hlr, hlg, hlb);
    double hlsz = sz * 0.28;
    glBegin(GL_TRIANGLE_FAN);
    glVertex2d(x - sz * 0.27, y + sz * 0.27);
    for (int i = 0; i <= SEGS; i++)
    {
        double a = TWO_PI * i / SEGS;
        glVertex2d(x - sz*0.27 + hlsz * std::cos(a),
                   y + sz*0.27 + hlsz * std::sin(a));
    }
    glEnd();

    // --- Role letter: black shadow then white ---
    glColor3d(0.0, 0.0, 0.0);
    glRasterPos2d(x - 0.28 + 0.09, y - 0.26 - 0.09);
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, symbol);
    glColor3d(1.0, 1.0, 1.0);
    glRasterPos2d(x - 0.28, y - 0.26);
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, symbol);

    // --- HP bar above body ---
    double barW  = sz * 2.2;
    double barH  = 0.30;
    double barX0 = x - sz * 1.1;
    double barY0 = y + sz + 0.22;
    double frac  = (double)hp / (double)maxHp;
    if (frac < 0.0) frac = 0.0;
    if (frac > 1.0) frac = 1.0;

    glColor3d(0.28, 0.04, 0.04);
    glBegin(GL_QUADS);
    glVertex2d(barX0,      barY0); glVertex2d(barX0+barW, barY0);
    glVertex2d(barX0+barW, barY0+barH); glVertex2d(barX0, barY0+barH);
    glEnd();

    double rr = 1.0 - frac, gg = frac;
    glColor3d(rr, gg, 0.0);
    glBegin(GL_QUADS);
    glVertex2d(barX0,           barY0); glVertex2d(barX0+barW*frac, barY0);
    glVertex2d(barX0+barW*frac, barY0+barH); glVertex2d(barX0, barY0+barH);
    glEnd();

    glColor3d(0.0, 0.0, 0.0);
    glBegin(GL_LINE_LOOP);
    glVertex2d(barX0,      barY0); glVertex2d(barX0+barW, barY0);
    glVertex2d(barX0+barW, barY0+barH); glVertex2d(barX0, barY0+barH);
    glEnd();
}
