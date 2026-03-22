//
// Created by denni on 3/21/2026.
//
#ifndef LIMITORDERBOOK_GRAPHPANEL_H
#define LIMITORDERBOOK_GRAPHPANEL_H

#include "Engine/CoreEngine.h"
#include "imgui.h"

class GraphPanel {
private:
    CoreEngine& Engine_;

public:

    GraphPanel(CoreEngine& Engine): Engine_(Engine){}
    ~GraphPanel() = default;

    void Render(const std::string& Symbol);
};

#endif //LIMITORDERBOOK_GRAPHPANEL_H
