#include "AStar.h"
#include <queue>
#include <cmath>
#include <algorithm>
#include <limits>
#include <cstring>

struct ANode
{
    int row, col;
    double g, f;
    bool operator>(const ANode& o) const { return f > o.f; }
};

static double heuristic(int r1, int c1, int r2, int c2)
{
    return std::abs(r1 - r2) + std::abs(c1 - c2);
}

bool FindPath(int si, int sj, int ti, int tj,
              std::vector<std::pair<int,int>>& outPath)
{
    outPath.clear();

    // Quick bounds check
    if (si < 0 || si >= MSZ || sj < 0 || sj >= MSZ) return false;
    if (ti < 0 || ti >= MSZ || tj < 0 || tj >= MSZ) return false;
    if (si == ti && sj == tj) return true;

    // Check target is reachable
    if (dungeon[ti][tj] == CELL_WALL || dungeon[ti][tj] == CELL_OBSTACLE)
        return false;

    static double  gMap[MSZ][MSZ];
    static int     parentRow[MSZ][MSZ];
    static int     parentCol[MSZ][MSZ];
    static bool    closed[MSZ][MSZ];

    memset(gMap,      0x7F, sizeof(gMap));      // ~1.3e308 per double (> any real path cost)
    memset(parentRow, 0xFF, sizeof(parentRow)); // -1 per int
    memset(parentCol, 0xFF, sizeof(parentCol)); // -1 per int
    memset(closed,    0,    sizeof(closed));    // false per bool

    gMap[si][sj] = 0.0;

    std::priority_queue<ANode, std::vector<ANode>, std::greater<ANode>> open;
    open.push({si, sj, 0.0, heuristic(si, sj, ti, tj)});

    const int dr[] = {-1, 1, 0, 0};
    const int dc[] = { 0, 0,-1, 1};

    while (!open.empty())
    {
        ANode cur = open.top(); open.pop();

        // Skip nodes already finalized (closed list check)
        if (closed[cur.row][cur.col])
            continue;

        // Mark node as closed (finalized)
        closed[cur.row][cur.col] = true;

        // Goal check after closing, as in Homework4
        if (cur.row == ti && cur.col == tj)
            break;

        for (int d = 0; d < 4; d++)
        {
            int nr = cur.row + dr[d];
            int nc = cur.col + dc[d];

            if (nr < 0 || nr >= MSZ || nc < 0 || nc >= MSZ) continue;
            if (closed[nr][nc]) continue;
            int cell = dungeon[nr][nc];
            if (cell == CELL_WALL || cell == CELL_OBSTACLE) continue;

            // Base movement cost = 1, weighted by security
            double risk     = securityMap[nr][nc];
            double moveCost = 1.0 + RISK_WEIGHT * risk;
            double ng       = cur.g + moveCost;

            if (ng < gMap[nr][nc])
            {
                gMap[nr][nc]      = ng;
                parentRow[nr][nc] = cur.row;
                parentCol[nr][nc] = cur.col;
                double fVal = ng + heuristic(nr, nc, ti, tj);
                open.push({nr, nc, ng, fVal});
            }
        }
    }

    if (gMap[ti][tj] > 1e9)   // unreached: memset value ~1.3e308
        return false;

    // Reconstruct path
    std::vector<std::pair<int,int>> rev;
    int r = ti, c = tj;
    while (!(r == si && c == sj))
    {
        rev.push_back({r, c});
        int pr = parentRow[r][c];
        int pc = parentCol[r][c];
        r = pr; c = pc;
    }
    std::reverse(rev.begin(), rev.end());
    outPath = rev;
    return true;
}
