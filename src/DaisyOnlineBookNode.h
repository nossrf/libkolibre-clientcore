/*
 * Copyright (C) 2012 Kolibre
 *
 * This file is part of kolibre-clientcore.
 *
 * Kolibre-clientcore is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Kolibre-clientcore is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with kolibre-clientcore. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DAISYONLINEBOOK_NODE
#define DAISYONLINEBOOK_NODE

#include <Nodes/VirtualMenuNode.h>

#include <string>

class DaisyNavi;
class DaisyOnlineHandler;

class DaisyOnlineBookNode: public naviengine::VirtualMenuNode
{
public:
    DaisyOnlineBookNode(std::string book_id, DaisyOnlineHandler *DOHandler);
    ~DaisyOnlineBookNode();

    bool up(naviengine::NaviEngine&);
    bool prev(naviengine::NaviEngine&);
    bool next(naviengine::NaviEngine&);
    bool select(naviengine::NaviEngine&);
    bool selectByUri(naviengine::NaviEngine&, std::string);
    bool menu(naviengine::NaviEngine&);
    bool onOpen(naviengine::NaviEngine&);
    bool process(naviengine::NaviEngine&, int, void*);
    bool onNarrate();

    enum errorType
    {
        DO_INVOKE_ERROR,
        CONTENTRESOURCES_NULL_ERROR,
        NAVIGATION_CONTROL_FILE_ERROR,
        OPEN_CONTENT_ERROR,
    };
    errorType getLastError();

private:
    std::string book_id_;
    std::string last_modified_;
    DaisyOnlineHandler *pDOHandler;
    DaisyNavi *pDaisyNavi;
    bool daisyNaviActive;

    std::string url_;
    errorType lastError;
};

#endif
