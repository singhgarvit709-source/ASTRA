# ASTRA GUI Prototype

A beginner-friendly Qt 6 Widgets / C++ / CMake GUI prototype for ASTRA (Adaptive Space Traffic & Risk Avoidance System).

## Requirements
- Qt 6.2 or newer
- Qt Creator
- A compatible C++ compiler kit installed for the selected Qt version
- CMake 3.21 or newer

## Open and build
1. Extract this folder.
2. In Qt Creator, choose **File → Open File or Project…**
3. Select `CMakeLists.txt`.
4. Select an installed Qt 6 kit and click **Configure Project**.
5. Build using **Build → Build Project** (Ctrl+B).
6. Run using the green Run button (Ctrl+R).

## Included screens
- Mission Overview
- Orbit Monitor
- Collision Risk
- Maneuver Planner
- Analytics
- Learn ASTRA

## Important scope note
This is a GUI demonstration using synthetic sample data. Orbit drawing is illustrative, not physically accurate. Risk scores are arbitrary demo scores, not real collision probabilities. Maneuver proposals are text-only demonstrations and are not validated or actionable. Connect your team's actual backend algorithms and validate them before describing them as functional scientific results.

## Troubleshooting
- If Qt6 is not found, install Qt 6 development components and select the matching kit.
- If no compiler kit is available, install a compatible compiler through Qt Maintenance Tool.
- Read the first compiler error in Qt Creator's Compile Output before changing code.
