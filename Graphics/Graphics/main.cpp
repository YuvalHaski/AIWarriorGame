#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <vector>
#include <string>
#include "glut.h"

#include "Definitions.h"
#include "DungeonMap.h"
#include "SecurityMap.h"
#include "NPC.h"
#include "WarriorNPC.h"
#include "MedicNPC.h"
#include "SupplyNPC.h"

// ---- Global NPC list ----
std::vector<NPC*> allNPCs;

// ---- Window ----
static const int WINDOW_W = 1100;
static const int WINDOW_H = 800;

// ---- The game coordinate space ----
// Left 100 units = dungeon map (0..100)
// Right 30 units = status panel (100..130)
// Total ortho: 0..130 x 0..100
static const double ORTHO_W = 130.0;
static const double ORTHO_H = 100.0;

static int  gameWinner = 0;   // 0=playing, 1=T1 wins, 2=T2 wins, 3=draw

// ---- Bitmap text helper ----
static void DrawText(double x, double y, const char* text,
                     void* font = GLUT_BITMAP_HELVETICA_12)
{
    glRasterPos2d(x, y);
    for (const char* p = text; *p; p++)
        glutBitmapCharacter(font, *p);
}

static void DrawTextStr(double x, double y, const std::string& s,
                        void* font = GLUT_BITMAP_HELVETICA_12)
{
    DrawText(x, y, s.c_str(), font);
}

// ---- Spawn position inside a room ----
static void RoomSpawn(int roomIdx, double& outX, double& outY)
{
    if (roomIdx < 0 || roomIdx >= numRooms)
    { outX = 5.5; outY = 5.5; return; }
    Room& rm = rooms[roomIdx];
    outX = (double)rm.centerCol() + 0.5;
    outY = (double)rm.centerRow() + 0.5;
}

// ---- Team creation ----
static void CreateTeam(int team, int startRoomIdx)
{
    double sx, sy;

    // Warrior 1
    RoomSpawn(startRoomIdx, sx, sy);
    allNPCs.push_back(new WarriorNPC(sx,       sy, team));

    // Warrior 2 (offset so they don't stack)
    RoomSpawn(startRoomIdx, sx, sy);
    allNPCs.push_back(new WarriorNPC(sx + 3.0, sy, team));

    // Medic
    RoomSpawn(startRoomIdx, sx, sy);
    {
        int dr = (team == TEAM1) ? medDepot1Row : medDepot2Row;
        int dc = (team == TEAM1) ? medDepot1Col : medDepot2Col;
        allNPCs.push_back(new MedicNPC(sx, sy + 3.0, team, dr, dc));
    }

    // Supply Soldier
    RoomSpawn(startRoomIdx, sx, sy);
    {
        int dr = (team == TEAM1) ? ammoDepot1Row : ammoDepot2Row;
        int dc = (team == TEAM1) ? ammoDepot1Col : ammoDepot2Col;
        allNPCs.push_back(new SupplyNPC(sx + 3.0, sy + 3.0, team, dr, dc));
    }
}

// ---- Init ----
void init()
{
    glClearColor(0.06f, 0.05f, 0.04f, 1.0f);  // warm dark stone
    glOrtho(0, ORTHO_W, 0, ORTHO_H, -1, 1);

    GenerateDungeon();

    int t2Room = (numRooms > 1) ? numRooms - 1 : 0;
    CreateTeam(TEAM1, 0);
    CreateTeam(TEAM2, t2Room);

    gameWinner = 0;
}

// ---- Helpers ----
static int CountAlive(int team)
{
    int n = 0;
    for (NPC* npc : allNPCs)
        if (npc->isAlive() && npc->getTeam() == team) n++;
    return n;
}

// ---- Draw the right-side status panel ----
static void DrawStatusPanel()
{
    // Panel background
    glColor3d(0.10, 0.10, 0.14);
    glBegin(GL_QUADS);
    glVertex2d(101.0, 0.0);   glVertex2d(ORTHO_W, 0.0);
    glVertex2d(ORTHO_W, ORTHO_H); glVertex2d(101.0, ORTHO_H);
    glEnd();

    // Divider line
    glColor3d(0.40, 0.40, 0.45);
    glBegin(GL_LINES);
    glVertex2d(101.0, 0.0); glVertex2d(101.0, ORTHO_H);
    glEnd();

    // Title
    glColor3d(1.0, 1.0, 1.0);
    DrawText(102.5, 97.0, "=== STATUS ===", GLUT_BITMAP_HELVETICA_12);

    // ---- LEGEND ----
    double ly = 93.0;
    glColor3d(0.85, 0.85, 0.85);
    DrawText(102.5, ly, "LEGEND:", GLUT_BITMAP_HELVETICA_10);
    ly -= 2.5;

    // Warrior
    glColor3d(1.00, 0.45, 0.00);
    glBegin(GL_QUADS);
    glVertex2d(102.5,ly); glVertex2d(104.5,ly); glVertex2d(104.5,ly+1.5); glVertex2d(102.5,ly+1.5);
    glEnd();
    glColor3d(0.85, 0.85, 0.85);
    DrawText(105.0, ly+0.2, "Warrior (W)", GLUT_BITMAP_HELVETICA_10);
    ly -= 2.3;

    // Medic
    glColor3d(1.00, 0.75, 0.25);
    glBegin(GL_QUADS);
    glVertex2d(102.5,ly); glVertex2d(104.5,ly); glVertex2d(104.5,ly+1.5); glVertex2d(102.5,ly+1.5);
    glEnd();
    glColor3d(0.85, 0.85, 0.85);
    DrawText(105.0, ly+0.2, "Medic   (M)", GLUT_BITMAP_HELVETICA_10);
    ly -= 2.3;

    // Supply
    glColor3d(0.80, 0.30, 0.00);
    glBegin(GL_QUADS);
    glVertex2d(102.5,ly); glVertex2d(104.5,ly); glVertex2d(104.5,ly+1.5); glVertex2d(102.5,ly+1.5);
    glEnd();
    glColor3d(0.85, 0.85, 0.85);
    DrawText(105.0, ly+0.2, "Supply  (S)", GLUT_BITMAP_HELVETICA_10);
    ly -= 2.3;

    // Ammo depot
    glColor3d(0.95, 0.80, 0.10);
    glBegin(GL_QUADS);
    glVertex2d(102.5,ly); glVertex2d(104.5,ly); glVertex2d(104.5,ly+1.5); glVertex2d(102.5,ly+1.5);
    glEnd();
    glColor3d(0.85, 0.85, 0.85);
    DrawText(105.0, ly+0.2, "Ammo Depot", GLUT_BITMAP_HELVETICA_10);
    ly -= 2.3;

    // Med depot
    glColor3d(0.90, 0.15, 0.20);
    glBegin(GL_QUADS);
    glVertex2d(102.5,ly); glVertex2d(104.5,ly); glVertex2d(104.5,ly+1.5); glVertex2d(102.5,ly+1.5);
    glEnd();
    glColor3d(0.85, 0.85, 0.85);
    DrawText(105.0, ly+0.2, "Med Depot", GLUT_BITMAP_HELVETICA_10);
    ly -= 2.3;

    // Obstacle
    glColor3d(0.22, 0.19, 0.16);
    glBegin(GL_QUADS);
    glVertex2d(102.5,ly); glVertex2d(104.5,ly); glVertex2d(104.5,ly+1.5); glVertex2d(102.5,ly+1.5);
    glEnd();
    glColor3d(0.85, 0.85, 0.85);
    DrawText(105.0, ly+0.2, "Obstacle", GLUT_BITMAP_HELVETICA_10);
    ly -= 2.3;

    // HP bar legend
    glColor3d(0.85, 0.85, 0.85);
    DrawText(102.5, ly, "HP bar (above NPC)", GLUT_BITMAP_HELVETICA_10);
    ly -= 2.3;
    DrawText(102.5, ly, "Ammo bar (below,purple)", GLUT_BITMAP_HELVETICA_10);
    ly -= 2.3;
    DrawText(102.5, ly, "Med/Supply bar (cyan/gold)", GLUT_BITMAP_HELVETICA_10);
    ly -= 3.0;

    // ---- Security map note ----
    glColor3d(0.85, 0.85, 0.85);
    DrawText(102.5, ly, "Floor color = danger:", GLUT_BITMAP_HELVETICA_10);
    ly -= 2.0;
    DrawText(102.5, ly, "Beige=safe, Red=danger", GLUT_BITMAP_HELVETICA_10);
    ly -= 3.0;

    // ---- TEAM 1 NPC list ----
    glColor3d(1.00, 0.55, 0.10);
    DrawText(102.5, ly, "-- TEAM 1 (orange) --", GLUT_BITMAP_HELVETICA_10);
    ly -= 2.2;

    for (NPC* npc : allNPCs)
    {
        if (npc->getTeam() != TEAM1) continue;
        std::string line;
        const char* role = (npc->getType()==NPC_WARRIOR) ? "W" :
                           (npc->getType()==NPC_MEDIC)   ? "M" : "S";
        line += role;
        line += " HP:";
        line += std::to_string(npc->getHp());
        line += "/";
        line += std::to_string(npc->getMaxHp());

        if (npc->getType() == NPC_WARRIOR)
        {
            WarriorNPC* w = static_cast<WarriorNPC*>(npc);
            line += " Ammo:";
            line += std::to_string(w->getAmmo());
            if (!npc->isAlive()) line = "[DEAD] " + line;
        }

        if (!npc->isAlive()) glColor3d(0.50, 0.20, 0.20);
        else                  glColor3d(0.95, 0.80, 0.55);
        DrawText(102.5, ly, line.c_str(), GLUT_BITMAP_HELVETICA_10);
        ly -= 2.0;

        // State for warriors
        if (npc->isAlive() && npc->getType() == NPC_WARRIOR)
        {
            WarriorNPC* w = static_cast<WarriorNPC*>(npc);
            if (w->pCurrentState)
            {
                std::string st = std::string("  -> ") + w->pCurrentState->getName();
                glColor3d(0.80, 0.80, 0.35);
                DrawText(102.5, ly, st.c_str(), GLUT_BITMAP_HELVETICA_10);
                ly -= 2.0;
            }
        }
    }

    ly -= 1.0;

    // ---- TEAM 2 NPC list ----
    glColor3d(0.30, 0.60, 1.00);
    DrawText(102.5, ly, "-- TEAM 2 (blue)  --", GLUT_BITMAP_HELVETICA_10);
    ly -= 2.2;

    for (NPC* npc : allNPCs)
    {
        if (npc->getTeam() != TEAM2) continue;
        std::string line;
        const char* role = (npc->getType()==NPC_WARRIOR) ? "W" :
                           (npc->getType()==NPC_MEDIC)   ? "M" : "S";
        line += role;
        line += " HP:";
        line += std::to_string(npc->getHp());
        line += "/";
        line += std::to_string(npc->getMaxHp());

        if (npc->getType() == NPC_WARRIOR)
        {
            WarriorNPC* w = static_cast<WarriorNPC*>(npc);
            line += " Ammo:";
            line += std::to_string(w->getAmmo());
        }

        if (!npc->isAlive()) glColor3d(0.20, 0.20, 0.50);
        else                  glColor3d(0.60, 0.80, 1.00);
        DrawText(102.5, ly, line.c_str(), GLUT_BITMAP_HELVETICA_10);
        ly -= 2.0;

        if (npc->isAlive() && npc->getType() == NPC_WARRIOR)
        {
            WarriorNPC* w = static_cast<WarriorNPC*>(npc);
            if (w->pCurrentState)
            {
                std::string st = std::string("  -> ") + w->pCurrentState->getName();
                glColor3d(0.55, 0.80, 0.80);
                DrawText(102.5, ly, st.c_str(), GLUT_BITMAP_HELVETICA_10);
                ly -= 2.0;
            }
        }
    }

    // ---- Instructions ----
    glColor3d(0.55, 0.55, 0.55);
    DrawText(102.5, 3.5, "Press R to restart", GLUT_BITMAP_HELVETICA_10);
    DrawText(102.5, 1.5, "Press ESC to quit",  GLUT_BITMAP_HELVETICA_10);
}

// ---- display ----
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Set viewport and projection to show only 0..100 x 0..100 for the map
    // (The status panel lives in the 100..130 strip of the same ortho)
    DrawDungeon();

    // Draw a thin separator line between map and panel
    glColor3d(0.35, 0.35, 0.40);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2d(100.5, 0.0); glVertex2d(100.5, ORTHO_H);
    glEnd();
    glLineWidth(1.0f);

    // Draw NPCs
    for (NPC* npc : allNPCs)
        npc->show();

    // Status panel (right side)
    DrawStatusPanel();

    // ---- Top HUD bar (team scores) ----
    int t1 = CountAlive(TEAM1);
    int t2 = CountAlive(TEAM2);

    glColor3d(1.00, 0.50, 0.05);
    DrawTextStr(1.0, 97.5,
        "TEAM 1 (orange): " + std::to_string(t1) + " alive",
        GLUT_BITMAP_HELVETICA_12);

    glColor3d(0.15, 0.45, 1.00);
    DrawTextStr(40.0, 97.5,
        "TEAM 2 (blue): " + std::to_string(t2) + " alive",
        GLUT_BITMAP_HELVETICA_12);

    // ---- Winner banner ----
    if (gameWinner != 0)
    {
        // Semi-transparent dark banner
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4d(0.0, 0.0, 0.0, 0.55);
        glBegin(GL_QUADS);
        glVertex2d(20.0, 44.0); glVertex2d(80.0, 44.0);
        glVertex2d(80.0, 56.0); glVertex2d(20.0, 56.0);
        glEnd();

        const char* msg = (gameWinner == 1) ? "*** TEAM 1 WINS! ***" :
                          (gameWinner == 2) ? "*** TEAM 2 WINS! ***" :
                                             "***   DRAW!   ***";
        if (gameWinner == 1)      glColor3d(1.0, 0.6, 0.1);
        else if (gameWinner == 2) glColor3d(0.3, 0.6, 1.0);
        else                      glColor3d(0.9, 0.9, 0.9);

        DrawText(25.0, 49.0, msg, GLUT_BITMAP_TIMES_ROMAN_24);
        DrawText(28.0, 45.5, "Press R to restart", GLUT_BITMAP_HELVETICA_12);
        glDisable(GL_BLEND);
    }

    glutSwapBuffers();
}

// ---- idle ----
void idle()
{
    if (gameWinner != 0)
    {
        glutPostRedisplay();
        return;
    }

    // Decay security map each frame
    SecurityMapDecay();

    // Update NPCs
    for (NPC* npc : allNPCs)
        npc->DoSomeWork();

    // Check win condition
    int t1 = CountAlive(TEAM1);
    int t2 = CountAlive(TEAM2);

    if      (t1 > 0 && t2 == 0) gameWinner = 1;
    else if (t2 > 0 && t1 == 0) gameWinner = 2;
    else if (t1 == 0 && t2 == 0) gameWinner = 3;

    glutPostRedisplay();
}

// ---- keyboard ----
void keyboard(unsigned char key, int /*x*/, int /*y*/)
{
    if (key == 27) exit(0);  // ESC
    if (key == 'r' || key == 'R')
    {
        for (NPC* npc : allNPCs) delete npc;
        allNPCs.clear();
        GenerateDungeon();
        int t2Room = (numRooms > 1) ? numRooms - 1 : 0;
        CreateTeam(TEAM1, 0);
        CreateTeam(TEAM2, t2Room);
        gameWinner = 0;
    }
}

// ---- entry point ----
int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(WINDOW_W, WINDOW_H);
    glutInitWindowPosition(80, 50);
    glutCreateWindow("Dungeon Battle - FSM + A* Pathfinding");

    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);

    init();
    glutMainLoop();
    return 0;
}
