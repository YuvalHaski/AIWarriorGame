#include "DungeonMap.h"
#include "SecurityMap.h"
#include "glut.h"
#include <cstdlib>
#include <ctime>
#include <vector>
#include <algorithm>

// ---- Global definitions ----
int    dungeon[MSZ][MSZ];
int    roomId[MSZ][MSZ];
double securityMap[MSZ][MSZ];
int    numRooms = 0;

int ammoDepot1Row, ammoDepot1Col;
int ammoDepot2Row, ammoDepot2Col;
int medDepot1Row,  medDepot1Col;
int medDepot2Row,  medDepot2Col;

std::vector<Room> rooms;

// ---- Helpers ----
static void DrawCell(int r, int c, double cr, double cg, double cb)
{
    double x = (double)c;
    double y = (double)r;
    glColor3d(cr, cg, cb);
    glBegin(GL_QUADS);
    glVertex2d(x,     y);
    glVertex2d(x+1.0, y);
    glVertex2d(x+1.0, y+1.0);
    glVertex2d(x,     y+1.0);
    glEnd();
}

// Draw text at world coordinates
static void DrawLabel(double x, double y, const char* text, void* font = GLUT_BITMAP_HELVETICA_10)
{
    glRasterPos2d(x, y);
    for (const char* p = text; *p; p++)
        glutBitmapCharacter(font, *p);
}

// ---- Corridor carving (L-shaped) ----
static void CarveCell(int r, int c)
{
    if (r < 0 || r >= MSZ || c < 0 || c >= MSZ) return;
    if (dungeon[r][c] == CELL_WALL) { dungeon[r][c] = CELL_CORRIDOR; roomId[r][c] = -1; }
}

static void CarveCorridor(int r1, int c1, int r2, int c2)
{
    // Choose bend direction to reduce parallel corridors:
    // alternate between horizontal-first and vertical-first
    bool horizontalFirst = (std::abs(c2 - c1) >= std::abs(r2 - r1));

    int cr = r1, cc = c1;
    if (horizontalFirst)
    {
        while (cc != c2) { CarveCell(cr, cc); cc += (c2 > cc) ? 1 : -1; }
        while (cr != r2) { CarveCell(cr, cc); cr += (r2 > cr) ? 1 : -1; }
    }
    else
    {
        while (cr != r2) { CarveCell(cr, cc); cr += (r2 > cr) ? 1 : -1; }
        while (cc != c2) { CarveCell(cr, cc); cc += (c2 > cc) ? 1 : -1; }
    }
    CarveCell(cr, cc);
}

// ---- Place 2-3 obstacle clusters per room ----
static void PlaceObstacles(const Room& room)
{
    int numObs = 2 + rand() % 3; // 2-4 obstacles
    for (int k = 0; k < numObs; k++)
    {
        for (int attempt = 0; attempt < 30; attempt++)
        {
            int r = room.row + 1 + rand() % (room.height - 2);
            int c = room.col + 1 + rand() % (room.width  - 2);
            // Don't place on center (spawn point) and don't block corners
            if (dungeon[r][c] == CELL_ROOM &&
                !(r == room.centerRow() && c == room.centerCol()))
            {
                dungeon[r][c] = CELL_OBSTACLE;
                break;
            }
        }
    }
}

// ---- Pick a random walkable cell inside a room ----
static bool PickRandomRoomCell(const Room& room, int& outR, int& outC)
{
    for (int attempt = 0; attempt < 300; attempt++)
    {
        int r = room.row + rand() % room.height;
        int c = room.col + rand() % room.width;
        if (dungeon[r][c] == CELL_ROOM)
        {
            outR = r; outC = c;
            return true;
        }
    }
    outR = room.centerRow();
    outC = room.centerCol();
    return false;
}

// ---- Main dungeon generation ----
void GenerateDungeon()
{
    srand((unsigned)time(0));

    // Initialise arrays
    for (int r = 0; r < MSZ; r++)
        for (int c = 0; c < MSZ; c++)
        {
            dungeon[r][c]     = CELL_WALL;
            roomId[r][c]      = -1;
            securityMap[r][c] = 0.0;
        }

    rooms.clear();

    // Generate non-overlapping rooms
    int attempts = 0;
    while ((int)rooms.size() < NUM_ROOMS && attempts < 8000)
    {
        attempts++;
        Room room;
        room.width  = 8 + rand() % 10;   // 8..17 cells wide
        room.height = 8 + rand() % 10;
        room.row    = 3 + rand() % (MSZ - room.height - 6);
        room.col    = 3 + rand() % (MSZ - room.width  - 6);
        room.id     = (int)rooms.size();

        bool ok = true;
        for (const Room& ex : rooms)
            if (room.overlaps(ex)) { ok = false; break; }

        if (ok)
        {
            rooms.push_back(room);
            for (int r = room.row; r < room.row + room.height; r++)
                for (int c = room.col; c < room.col + room.width; c++)
                {
                    dungeon[r][c] = CELL_ROOM;
                    roomId[r][c]  = room.id;
                }
        }
    }

    numRooms = (int)rooms.size();

    // Connect rooms using Prim's MST so each room connects to its nearest
    // unconnected neighbor, producing clean non-overlapping corridors
    {
        std::vector<bool> connected(numRooms, false);
        connected[0] = true;
        int edgesAdded = 0;

        while (edgesAdded < numRooms - 1)
        {
            int bestA = -1, bestB = -1;
            double bestDist = 1e18;

            for (int a = 0; a < numRooms; a++)
            {
                if (!connected[a]) continue;
                for (int b = 0; b < numRooms; b++)
                {
                    if (connected[b]) continue;
                    double dr = rooms[a].centerRow() - rooms[b].centerRow();
                    double dc = rooms[a].centerCol() - rooms[b].centerCol();
                    double d  = dr*dr + dc*dc;
                    if (d < bestDist) { bestDist = d; bestA = a; bestB = b; }
                }
            }

            if (bestA < 0) break;
            connected[bestB] = true;
            CarveCorridor(rooms[bestA].centerRow(), rooms[bestA].centerCol(),
                          rooms[bestB].centerRow(), rooms[bestB].centerCol());
            edgesAdded++;
        }
    }

    // Place obstacles in each room
    for (const Room& room : rooms)
        PlaceObstacles(room);

    // Place depots spread across different rooms
    auto placeDepot = [&](int roomIdx, int cellType, int& outR, int& outC)
    {
        Room& rm = rooms[roomIdx % numRooms];
        int r, c;
        PickRandomRoomCell(rm, r, c);
        dungeon[r][c] = cellType;
        outR = r; outC = c;
    };

    placeDepot(0,             CELL_AMMO1, ammoDepot1Row, ammoDepot1Col);
    placeDepot(numRooms/2,    CELL_AMMO2, ammoDepot2Row, ammoDepot2Col);
    placeDepot(1,             CELL_MED1,  medDepot1Row,  medDepot1Col);
    placeDepot(numRooms/2+1,  CELL_MED2,  medDepot2Row,  medDepot2Col);
}

// ---- Drawing ----
void DrawDungeon()
{
    for (int r = 0; r < MSZ; r++)
    {
        for (int c = 0; c < MSZ; c++)
        {
            double x = (double)c;
            double y = (double)r;
            int cell = dungeon[r][c];

            switch (cell)
            {
            // ---- WALL: dark stone with subtle 2x2 block variation ----
            case CELL_WALL:
            {
                double shade = ((r / 2 + c / 2) % 2 == 0) ? 0.14 : 0.11;
                glColor3d(shade, shade * 0.88, shade * 0.76);
                glBegin(GL_QUADS);
                glVertex2d(x, y); glVertex2d(x+1, y);
                glVertex2d(x+1, y+1); glVertex2d(x, y+1);
                glEnd();
                // Thin mortar lines between 2x2 blocks
                glColor3d(0.06, 0.05, 0.04);
                glBegin(GL_LINES);
                if (r % 2 == 0) { glVertex2d(x, y+0.96); glVertex2d(x+1, y+0.96); }
                if (c % 2 == 0) { glVertex2d(x+0.96, y); glVertex2d(x+0.96, y+1); }
                glEnd();
                break;
            }

            // Corridors are drawn in a second pass below (so they overdraw wall cells)
            case CELL_CORRIDOR:
                break;

            // ---- ROOM FLOOR: stone tiles with security danger tint ----
            case CELL_ROOM:
            {
                double risk = securityMap[r][c];
                double tile = ((r / 2 + c / 2) % 2 == 0) ? 0.0 : 0.04;
                double fr = std::min(0.44 + tile + risk * 0.42, 1.0);
                double fg = std::max(0.40 + tile - risk * 0.26, 0.04);
                double fb = std::max(0.33 + tile - risk * 0.26, 0.04);
                DrawCell(r, c, fr, fg, fb);
                // Grout lines at 2-cell tile boundaries
                glColor3d(fr * 0.74, fg * 0.74, fb * 0.74);
                glBegin(GL_LINES);
                if (r % 2 == 0) { glVertex2d(x, y+0.04); glVertex2d(x+1, y+0.04); }
                if (c % 2 == 0) { glVertex2d(x+0.04, y); glVertex2d(x+0.04, y+1); }
                glEnd();
                break;
            }

            // ---- OBSTACLE: 3-D stone pillar with shadow & highlights ----
            case CELL_OBSTACLE:
            {
                // Room floor beneath the pillar
                DrawCell(r, c, 0.42, 0.38, 0.31);

                double px = x + 0.10, py = y + 0.10;
                double pw = 0.80,     ph = 0.80;
                double ed = 0.17;   // edge width for 3-D faces

                // Drop shadow (offset down-right)
                glColor3d(0.04, 0.03, 0.02);
                glBegin(GL_QUADS);
                glVertex2d(px+0.11, py-0.11); glVertex2d(px+pw+0.11, py-0.11);
                glVertex2d(px+pw+0.11, py+ph-0.11); glVertex2d(px+0.11, py+ph-0.11);
                glEnd();

                // Main pillar body
                glColor3d(0.31, 0.27, 0.22);
                glBegin(GL_QUADS);
                glVertex2d(px, py); glVertex2d(px+pw, py);
                glVertex2d(px+pw, py+ph); glVertex2d(px, py+ph);
                glEnd();

                // Top highlight face (lit from above-left)
                glColor3d(0.58, 0.52, 0.41);
                glBegin(GL_QUADS);
                glVertex2d(px,      py+ph-ed); glVertex2d(px+pw-ed, py+ph-ed);
                glVertex2d(px+pw-ed, py+ph);  glVertex2d(px,        py+ph);
                glEnd();

                // Left highlight face
                glColor3d(0.50, 0.45, 0.36);
                glBegin(GL_QUADS);
                glVertex2d(px,    py+ed); glVertex2d(px+ed, py+ed);
                glVertex2d(px+ed, py+ph); glVertex2d(px,    py+ph);
                glEnd();

                // Bottom shadow face
                glColor3d(0.12, 0.10, 0.08);
                glBegin(GL_QUADS);
                glVertex2d(px, py); glVertex2d(px+pw, py);
                glVertex2d(px+pw, py+ed); glVertex2d(px, py+ed);
                glEnd();

                // Right shadow face
                glColor3d(0.12, 0.10, 0.08);
                glBegin(GL_QUADS);
                glVertex2d(px+pw-ed, py+ed); glVertex2d(px+pw, py+ed);
                glVertex2d(px+pw, py+ph-ed); glVertex2d(px+pw-ed, py+ph-ed);
                glEnd();

                // Top-left corner: brightest highlight
                glColor3d(0.76, 0.68, 0.54);
                glBegin(GL_QUADS);
                glVertex2d(px,    py+ph-ed); glVertex2d(px+ed, py+ph-ed);
                glVertex2d(px+ed, py+ph);   glVertex2d(px,    py+ph);
                glEnd();

                // Bottom-right corner: darkest shadow
                glColor3d(0.07, 0.06, 0.04);
                glBegin(GL_QUADS);
                glVertex2d(px+pw-ed, py); glVertex2d(px+pw, py);
                glVertex2d(px+pw, py+ed); glVertex2d(px+pw-ed, py+ed);
                glEnd();

                // Outline
                glColor3d(0.06, 0.05, 0.04);
                glBegin(GL_LINE_LOOP);
                glVertex2d(px, py); glVertex2d(px+pw, py);
                glVertex2d(px+pw, py+ph); glVertex2d(px, py+ph);
                glEnd();
                break;
            }

            // ---- AMMO DEPOT: wooden crate with gold metal bands ----
            case CELL_AMMO1:
            case CELL_AMMO2:
            {
                DrawCell(r, c, 0.42, 0.38, 0.31);

                double ax = x + 0.07, ay = y + 0.07;
                double aw = 0.86,     ah = 0.86;

                // Dark wood body
                glColor3d(0.36, 0.24, 0.09);
                glBegin(GL_QUADS);
                glVertex2d(ax, ay); glVertex2d(ax+aw, ay);
                glVertex2d(ax+aw, ay+ah); glVertex2d(ax, ay+ah);
                glEnd();

                // Lighter top panel
                glColor3d(0.50, 0.34, 0.13);
                glBegin(GL_QUADS);
                glVertex2d(ax+0.09, ay+ah*0.50); glVertex2d(ax+aw-0.09, ay+ah*0.50);
                glVertex2d(ax+aw-0.09, ay+ah-0.09); glVertex2d(ax+0.09, ay+ah-0.09);
                glEnd();

                // Gold horizontal band
                glColor3d(0.92, 0.74, 0.14);
                glBegin(GL_QUADS);
                glVertex2d(ax, ay+ah*0.42); glVertex2d(ax+aw, ay+ah*0.42);
                glVertex2d(ax+aw, ay+ah*0.55); glVertex2d(ax, ay+ah*0.55);
                glEnd();

                // Gold vertical side bands
                glColor3d(0.86, 0.68, 0.12);
                glBegin(GL_QUADS);
                glVertex2d(ax,         ay); glVertex2d(ax+0.13,    ay);
                glVertex2d(ax+0.13,    ay+ah); glVertex2d(ax,      ay+ah);
                glVertex2d(ax+aw-0.13, ay); glVertex2d(ax+aw,      ay);
                glVertex2d(ax+aw,      ay+ah); glVertex2d(ax+aw-0.13, ay+ah);
                glEnd();

                // Bright gold corner bolts
                glColor3d(1.0, 0.90, 0.28);
                double bs = 0.11;
                glBegin(GL_QUADS);
                glVertex2d(ax+0.03,    ay+0.03);    glVertex2d(ax+bs,      ay+0.03);
                glVertex2d(ax+bs,      ay+bs);      glVertex2d(ax+0.03,    ay+bs);
                glVertex2d(ax+aw-bs,   ay+0.03);    glVertex2d(ax+aw-0.03, ay+0.03);
                glVertex2d(ax+aw-0.03, ay+bs);      glVertex2d(ax+aw-bs,   ay+bs);
                glVertex2d(ax+0.03,    ay+ah-bs);   glVertex2d(ax+bs,      ay+ah-bs);
                glVertex2d(ax+bs,      ay+ah-0.03); glVertex2d(ax+0.03,    ay+ah-0.03);
                glVertex2d(ax+aw-bs,   ay+ah-bs);   glVertex2d(ax+aw-0.03, ay+ah-bs);
                glVertex2d(ax+aw-0.03, ay+ah-0.03); glVertex2d(ax+aw-bs,   ay+ah-0.03);
                glEnd();

                glColor3d(0.55, 0.38, 0.05);
                glLineWidth(1.5f);
                glBegin(GL_LINE_LOOP);
                glVertex2d(ax, ay); glVertex2d(ax+aw, ay);
                glVertex2d(ax+aw, ay+ah); glVertex2d(ax, ay+ah);
                glEnd();
                glLineWidth(1.0f);
                break;
            }

            // ---- MED DEPOT: red medical station with bold white cross ----
            case CELL_MED1:
            case CELL_MED2:
            {
                DrawCell(r, c, 0.42, 0.38, 0.31);

                double mx = x + 0.07, my = y + 0.07;
                double mw = 0.86,     mh = 0.86;

                // Dark red base
                glColor3d(0.55, 0.07, 0.07);
                glBegin(GL_QUADS);
                glVertex2d(mx, my); glVertex2d(mx+mw, my);
                glVertex2d(mx+mw, my+mh); glVertex2d(mx, my+mh);
                glEnd();

                // Brighter red upper panel
                glColor3d(0.84, 0.14, 0.16);
                glBegin(GL_QUADS);
                glVertex2d(mx+0.09, my+mh*0.46); glVertex2d(mx+mw-0.09, my+mh*0.46);
                glVertex2d(mx+mw-0.09, my+mh-0.09); glVertex2d(mx+0.09, my+mh-0.09);
                glEnd();

                // Bold white cross
                glColor3d(1.0, 1.0, 1.0);
                glBegin(GL_QUADS);
                glVertex2d(mx+mw*0.38, my+mh*0.10); glVertex2d(mx+mw*0.62, my+mh*0.10);
                glVertex2d(mx+mw*0.62, my+mh*0.90); glVertex2d(mx+mw*0.38, my+mh*0.90);
                glVertex2d(mx+mw*0.10, my+mh*0.38); glVertex2d(mx+mw*0.90, my+mh*0.38);
                glVertex2d(mx+mw*0.90, my+mh*0.62); glVertex2d(mx+mw*0.10, my+mh*0.62);
                glEnd();

                glColor3d(0.35, 0.04, 0.04);
                glLineWidth(1.5f);
                glBegin(GL_LINE_LOOP);
                glVertex2d(mx, my); glVertex2d(mx+mw, my);
                glVertex2d(mx+mw, my+mh); glVertex2d(mx, my+mh);
                glEnd();
                glLineWidth(1.0f);
                break;
            }

            default:
                break;
            }
        }
    }

    // ---- Second pass: corridors drawn wider by extending into adjacent wall cells ----
    const double PAD = 0.40;  // how far each corridor cell extends into neighbouring walls
    glColor3d(0.40, 0.35, 0.27);
    for (int r = 0; r < MSZ; r++)
    {
        for (int c = 0; c < MSZ; c++)
        {
            if (dungeon[r][c] != CELL_CORRIDOR) continue;
            double x = (double)c;
            double y = (double)r;

            double x0 = x,       x1 = x + 1.0;
            double y0 = y,       y1 = y + 1.0;

            // Only extend into directions that have a wall neighbour
            if (c > 0     && dungeon[r][c-1] == CELL_WALL) x0 -= PAD;
            if (c < MSZ-1 && dungeon[r][c+1] == CELL_WALL) x1 += PAD;
            if (r > 0     && dungeon[r-1][c] == CELL_WALL) y0 -= PAD;
            if (r < MSZ-1 && dungeon[r+1][c] == CELL_WALL) y1 += PAD;

            glBegin(GL_QUADS);
            glVertex2d(x0, y0); glVertex2d(x1, y0);
            glVertex2d(x1, y1); glVertex2d(x0, y1);
            glEnd();
        }
    }

    // ---- Room outlines ----
    glColor3d(0.18, 0.15, 0.12);
    glLineWidth(2.5f);
    for (const Room& rm : rooms)
    {
        double x0 = (double)rm.col,  y0 = (double)rm.row;
        double x1 = x0 + rm.width,  y1 = y0 + rm.height;
        glBegin(GL_LINE_LOOP);
        glVertex2d(x0, y0); glVertex2d(x1, y0);
        glVertex2d(x1, y1); glVertex2d(x0, y1);
        glEnd();
    }
    glLineWidth(1.0f);

    // ---- Depot labels ----
    auto drawDepotLabel = [](int dr, int dc, const char* lbl, double lr, double lg, double lb)
    {
        glColor3d(lr, lg, lb);
        glRasterPos2d((double)dc + 0.1, (double)dr + 1.15);
        for (const char* p = lbl; *p; p++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *p);
    };

    drawDepotLabel(ammoDepot1Row, ammoDepot1Col, "AMMO", 1.0, 0.88, 0.22);
    drawDepotLabel(ammoDepot2Row, ammoDepot2Col, "AMMO", 1.0, 0.88, 0.22);
    drawDepotLabel(medDepot1Row,  medDepot1Col,  "MED",  1.0, 0.42, 0.42);
    drawDepotLabel(medDepot2Row,  medDepot2Col,  "MED",  1.0, 0.42, 0.42);

    // ---- Room labels ----
    glColor3d(0.60, 0.54, 0.46);
    for (int i = 0; i < numRooms; i++)
    {
        char buf[8];
        buf[0] = 'R'; buf[1] = '0' + i; buf[2] = '\0';
        glRasterPos2d((double)rooms[i].centerCol() - 0.5,
                      (double)rooms[i].centerRow() - 0.3);
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, buf[0]);
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, buf[1]);
    }
}
